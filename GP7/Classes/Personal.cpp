#include    	"PERSONAL.H"
#include	"FINGER.H"
#include	"UTILITY.H"
#include  	"NOTIFY.H"
#include  	"VIEW.H"
#include  	"STATUS.H"
#include	"SHOWTIME.H"
#include	"SETTING.H"
#include	"DATETIME.H"
#include        "COMMUNICATION.H"
#include        "BEEP.H"
#include        "Mifare.h"
#include        "MifareEx.h"
#include        "Menu.h"

/*OS_STACKPTR int Personal::stack_person_exist[650];
OS_STACKPTR int Personal::stack_get_person_information[400];
OS_STACKPTR int Personal::stack_add_person[350];
OS_STACKPTR int Personal::stack_edit_person[350];
OS_STACKPTR int Personal::stack_delete_person[350];
OS_STACKPTR int Personal::stack_delete_all_person[350];
//OS_STACKPTR int Personal::stack_get_last_person[256];	
OS_STACKPTR int Personal::stack_handle_card[450];	
OS_STACKPTR int Personal::stack_handle_finger[450];	
OS_STACKPTR int Personal::stack_handle_key[450];	
OS_STACKPTR int Personal::stack_internal_save[400];	
OS_STACKPTR int Personal::stack_is_admin_finger[450];	

OS_TASK Personal::tcb_person_exist;
OS_TASK Personal::tcb_get_person_information;
OS_TASK Personal::tcb_add_person;
OS_TASK Personal::tcb_edit_person;
OS_TASK Personal::tcb_delete_person;
OS_TASK Personal::tcb_delete_all_person;
//OS_TASK Personal::tcb_get_last_person;
OS_TASK Personal::tcb_handle_card;	
OS_TASK Personal::tcb_handle_finger;
OS_TASK Personal::tcb_handle_key;
OS_TASK Personal::tcb_internal_save;
OS_TASK Personal::tcb_is_admin_finger;*/

OS_STACKPTR int Personal::stack_main_task[900];	
OS_TASK Personal::tcb_main_task;

OS_STACKPTR int Personal::stack_get_person_information_task[700];	
OS_TASK Personal::tcb_get_person_information_task;

PersonalTaskIdentifier Personal::pti;

PreCard Personal::person_pre_card;

MifareType Personal::mifareType;

const BYTE Personal::MasterFileRecordLength = sizeof(PersonInformation);
const BYTE Personal::FingerFileRecordLength = sizeof(IDPerson);
const BYTE Personal::CardFileRecordLength = sizeof(IDPerson);

EventPointer Personal::ep_person_exist;
EventPointer Personal::ep_person_not_exist;
EventPointer Personal::ep_person_add_success;
EventPointer Personal::ep_person_add_fail;
EventPointer Personal::ep_person_edit_success;
EventPointer Personal::ep_person_edit_fail;
EventPointer Personal::ep_person_delete_success;
EventPointer Personal::ep_person_delete_fail;
EventPointer Personal::ep_person_delete_all_success;
EventPointer Personal::ep_person_delete_all_fail;
void (*Personal::ep_save_success)(TrafficInformation *);
EventPointer Personal::ep_time_out_input;
EventPointer Personal::ep_valid_admin_finger;
EventPointer Personal::ep_records_delete_success;
EventPointer Personal::ep_card_is_unique;
EventPointer Personal::ep_card_is_unique_ex;
EventPointer Personal::ep_card_is_repetitive;

void (*Personal::ep_get_person_information) (PersonInformation* PI);
void (*Personal::ep_get_last_personal) (DWORD PN);

OS_TIMER Personal::timer_save;

DeviceSequence Personal::first_device_sequence;
DeviceSequence Personal::second_device_sequence;

TrafficInformation Personal::ti;

Personal::Personal ()
{
	
}

Personal::~Personal ()
{
	
}

void Personal::PersonIsExist (DWORD PersonNumber)
{
	if (is_task_running(&tcb_main_task))
		return;
	
	static DWORD PN;
	PN = PersonNumber;
	
	pti =	PTI_IsPersonExist;
		
	OS_CREATETASK_EX(&tcb_main_task, "Person_Exist_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) &PN);	
}

void Personal::IsCardRepetitive (DWORD CardID)
{
	if (is_task_running(&tcb_main_task))
		return;
	
	static DWORD CID;
	CID = CardID;
	
	pti =	PTI_IsCardRepetitive;
		
	OS_CREATETASK_EX(&tcb_main_task, "Is_Card_Repetitive_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) &CID);	
}

void Personal::IsCardRepetitiveEx (DWORD CardID)
{
	if (is_task_running(&tcb_main_task))
		return;
	
	static DWORD CID;
	CID = CardID;
	
	pti =	PTI_IsCardRepetitiveEx;
		
	OS_CREATETASK_EX(&tcb_main_task, "Is_Card_Repetitive_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) &CID);	
}

void Personal::GetPersonInformation (DWORD PersonNumber)
{
	if (is_task_running(&tcb_get_person_information_task))
		return;
	
	static DWORD PN;
	PN = PersonNumber;
	
	pti = PTI_GetPersonInformation;
	
	OS_CREATETASK_EX(&tcb_get_person_information_task, "Get_Person_info_Task", task_get_person_information, TP_RealTimePeriority, stack_get_person_information_task, (void*) &PN);	
}

void Personal::AddPerson (PersonInformation NewPI)
{
	if (is_task_running(&tcb_main_task))
		return;
	
	static PersonInformation PI;
	PI = NewPI;
	
	pti = PTI_AddPerson;
	
	OS_CREATETASK_EX(&tcb_main_task, "Personal_Main_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) &PI);	
}
	
void Personal::EditPerson (PersonInformation NewPI)
{
	if (is_task_running(&tcb_main_task))
		return;
	
	static PersonInformation PI;
	PI = NewPI;
	
	pti = PTI_EditPerson;
	
	OS_CREATETASK_EX(&tcb_main_task, "Personal_Main_Task", task_main, TP_HighPeriority, stack_main_task, (void*) &PI);	
}

void Personal::DeletePerson (DWORD PersonNumber)
{
	if (is_task_running(&tcb_main_task))
		return;

	static DWORD PN;
	PN = PersonNumber;
	
	pti = PTI_DeletePerson;
	
	OS_CREATETASK_EX(&tcb_main_task, "Personal_Main_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) &PN);	
}	

void Personal::DeleteAllPerson (void)
{
	if (is_task_running(&tcb_main_task))
		return;
	
	pti = PTI_DeleteAllPerson;
	
	OS_CREATETASK_EX(&tcb_main_task, "Personal_Main_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) 0);	
}	

void Personal::DeleteAllRecords (void)
{
	if (is_task_running(&tcb_main_task))
		return;
	
	pti = PTI_DeleteAllRecords;
	
	OS_CREATETASK_EX(&tcb_main_task, "Personal_Main_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) 0);	
}	

/*void Personal::GetLastPersonNumber (void)
{
	if (is_task_running(&tcb_get_last_person))
		return;
	
	OS_CREATETASK(&tcb_get_last_person, "Get_Last_Person_Number_Task", task_get_last_person, TP_HighestPeriority, stack_get_last_person);		
}*/

void Personal::SetPersonInformationRequestEvent (void (*EP) (PersonInformation *PI))
{
	ep_get_person_information = EP;
}	

void Personal::SetLastPersonNumberEvent (void (*EP)(DWORD))
{
	ep_get_last_personal = EP;
}

void Personal::SetPersonExistEvent (EventPointer EP)
{
	ep_person_exist = EP;
}

void Personal::SetPersonIsNotExistEvent (EventPointer EP)
{
	ep_person_not_exist = EP;
}

void Personal::SetPersonAddSuccessEvent (EventPointer EP)
{
	ep_person_add_success = EP;
}

void Personal::SetPersonAddFailEvent (EventPointer EP)
{
	ep_person_add_fail = EP;
}

void Personal::SetPersonEditSuccessEvent (EventPointer EP)
{
	ep_person_edit_success = EP;
}

void Personal::SetPersonEditFailEvent (EventPointer EP)
{
	ep_person_edit_fail = EP;
}

void Personal::SetPersonDeleteSuccessEvent (EventPointer EP)
{
	ep_person_delete_success = EP;
}

void Personal::SetPersonDeleteFailEvent (EventPointer EP)
{
	ep_person_delete_fail = EP;
}

void Personal::SetAllPersonDeleteSuccessEvent (EventPointer EP)
{
	ep_person_delete_all_success = EP;
}

void Personal::SetAllPersonDeleteFailEvent (EventPointer EP)
{
	ep_person_delete_all_fail = EP;
}

void Personal::SetSaveTrafficEvent (void (*EP)(TrafficInformation *))
{
	ep_save_success = EP;
}

void Personal::SetTimeoutInputEvent (EventPointer EP)
{
	ep_time_out_input = EP;
}

void Personal::SetValidAdministratorFingerIDEvent (EventPointer EP)
{
	ep_valid_admin_finger = EP;
}

void Personal::SetDeleteRecordsEvent (EventPointer EP)
{
	ep_records_delete_success = EP;
}

void Personal::SetRepetitiveCardIDEvent (EventPointer EP)
{
	ep_card_is_repetitive = EP;
}

void Personal::SetUniqueCardIDEvent (EventPointer EP)
{
	ep_card_is_unique = EP;
}

void Personal::SetUniqueCardIDEventEx (EventPointer EP)
{
	ep_card_is_unique_ex = EP;
}

void Personal::DisableEvents (void)
{
	ep_person_exist = NULL;
	ep_person_not_exist = NULL;
	ep_person_add_success = NULL;
	ep_person_add_fail = NULL;
	ep_person_edit_success = NULL;
	ep_person_edit_fail = NULL;
	ep_person_delete_success = NULL;
	ep_person_delete_fail = NULL;
	ep_save_success = NULL;
	ep_valid_admin_finger = NULL;
	
	ep_get_last_personal = NULL;
	ep_get_person_information = NULL;
}	

void Personal::HandleCard (DWORD CardID, PreCard PC, MifareType Type)
{	
	if (is_task_running(&tcb_main_task))
		return;

	person_pre_card = PC;

	static DWORD CI;
	CI = CardID;
	
        mifareType = Type;
	pti = PTI_HandleCard;
	
	OS_CREATETASK_EX(&tcb_main_task, "Personal_Main_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) &CI);	
}

void Personal::HandleFinger (DWORD FingerID, PreCard PC)
{
	if (is_task_running(&tcb_main_task))
		return;

	person_pre_card = PC;
		
	static DWORD FI;
	FI = FingerID;
	
	pti = PTI_HandleFinger;
	
	OS_CREATETASK_EX(&tcb_main_task, "Personal_Main_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) &FI);	
}

void Personal::HandleKey (DWORD KeyID, PreCard PC)
{
	if (is_task_running(&tcb_main_task))
		return;

	person_pre_card = PC;
	
	static DWORD KI;
	KI = KeyID;

	pti = PTI_HandleKey;

	OS_CREATETASK_EX(&tcb_main_task, "Personal_Main_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) &KI);	
}

void Personal::IsAdministratorFinger (DWORD FingerID)
{
	if (is_task_running(&tcb_main_task))
		return;

	static DWORD FI;
	FI = FingerID;
	
	pti = PTI_IsAdminFingerID;
	
	OS_CREATETASK_EX(&tcb_main_task, "Personal_Main_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) &FI);	
}

void Personal::StopTimers (void)
{
	NotifyBar  n;
	n.TurnSaveIcon(Off);	

	OS_StopTimer(&timer_save);
}		


void Personal::initial_members (void)
{
	//
}

void Personal::create_timers (void)
{
	OS_CreateTimer(&timer_save, timer_elapse_save, 7000);
}

bool Personal::is_file_exist (char file_path[], char file_name[])
{
	char path [30];
	FS_FILE   *fp;

	strcpy (path, "");

	strcat(path, file_path);
	strcat(path, "\\");
	strcat(path, file_name);
	
	fp = FS_FOpen(path,"rb");
	
	if (fp == NULL)
	{
		//FS_FClose(fp);
		return false;
	}
	
	else
	{
		FS_FClose(fp);
		return true;
	}
}

DWORD Personal::get_records_count (char file_path[], char file_name[], int record_length)
{
    if (!is_file_exist (file_path, file_name))
	    return 0;
	
	DWORD file_size;
	unsigned int  records_count;
	char path [30];
	FS_FILE   *fp;
	
	strcpy (path, "");

	strcat(path, file_path);
	strcat(path, "\\");
	strcat(path, file_name);
	
	fp = FS_FOpen(path,"rb");

	file_size = FS_GetFileSize(fp);
    	records_count = file_size / record_length;

	FS_FClose(fp);

	return  records_count;
}

SearchHit Personal::search (char file_path[], char file_name[], DWORD desire_data, int record_length)
{
	FS_FILE *fp;
	char path [30];
	int top, bottom, middle, records_count, index;
	DWORD   record;
	SearchHit sh;

	if (!is_file_exist (file_path, file_name))
	{
		sh.Hit = false;
		sh.Index = 0;
		
		return sh;
	}
	
	strcpy (path, "");

	strcat(path, file_path);
	strcat(path, "\\");	
	strcat(path, file_name);
	
	records_count = get_records_count(file_path, file_name, record_length);
	
	fp = FS_FOpen(path,"rb");
	
	top = records_count-1;
	bottom = 0;
	
	while(top>=bottom)
	{
		middle = (top+bottom) / 2;
		index = middle * record_length;

		FS_FSeek(fp, index, FS_SEEK_SET);
		FS_FRead(&record, sizeof(DWORD), 1, fp);
		
		if (record == desire_data)
		{
			sh.Hit = true;
			sh.Index = middle;
			sh.Value = record;

			FS_FClose(fp);
			return sh;
		}
		
		if (record < desire_data)
			bottom = middle+1;
		else
			top = middle-1;
	}	

	FS_FClose(fp);
	
	sh.Hit = false;
	sh.Index = middle;
	sh.Value = record;

	return sh;
}

bool Personal::copy_file_section (char source_file_path[], char source_file_name[], char destination_file_path[], char destination_file_name[], DWORD first_record_index_on_source_file, DWORD records_count, DWORD record_lenght)
{
	BYTE *s;
	DWORD ctr;
	FS_FILE *fp_source, *fp_destination;
	char path [30];
	
	create_directory(destination_file_path);
	
	strcpy (path, "");

	strcat(path, source_file_path);
	strcat(path, "\\");	
	strcat(path, source_file_name);
	
	fp_source = FS_FOpen(path,"rb");

	strcpy (path, "");

	strcat(path, destination_file_path);
	strcat(path, "\\");	
	strcat(path, destination_file_name);
	
	fp_destination = FS_FOpen(path,"ab");
	
	if (fp_source == NULL || fp_destination == NULL)
	{
		FS_FClose(fp_source);
		FS_FClose(fp_destination);	
		return false;
	}

	if (FS_GetFileSize(fp_source) == 0)
	{
		FS_FClose(fp_source);
		FS_FClose(fp_destination);	
		return true;
	}
	
        s = new BYTE[record_lenght];
	
	FS_SetFilePos (fp_source, first_record_index_on_source_file * record_lenght,  FS_FILE_BEGIN);
	
	for (ctr=0 ; ctr<records_count ; ctr++)
	{
		FS_Read (fp_source,s,record_lenght);
		FS_Write(fp_destination,s,record_lenght);
	}
	
	delete [] s;	
	
	FS_FClose(fp_source);
	FS_FClose(fp_destination);	
	
	return true;
}

bool Personal::create_file (char file_path[], char file_name[])
{
	FS_FILE * fp;
	char path [30];
	
	create_directory(file_path);
	
	strcpy (path, "");

	strcat(path, file_path);
	strcat(path, "\\");	
	strcat(path, file_name);
	
	fp = FS_FOpen(path,"wb");
	
	if (fp == NULL)
	{
		//FS_FClose (fp);
		return false;
	}
	
	FS_FClose (fp);
	
	return true;
}

bool Personal::append (char file_path[], char file_name[], const BYTE *data, DWORD size)
{
	FS_FILE * fp;	
	char path [30];

	create_directory(file_path);
	
	//if (!is_file_exist("Personal","Master.INF"))
	//	create_file(file_path, file_name);
	
	strcpy (path, "");

	strcat(path, file_path);
	strcat(path, "\\");	
	strcat(path, file_name);
	
	fp = FS_FOpen(path,"ab");

	if (fp == NULL)
	{
		//FS_FClose(fp);
		return false;
	}
	
	FS_Write(fp, data, size);
	
	FS_FClose(fp);
	
	return true;
}	

PersonInformation Personal::get_person_information (DWORD person_number)
{

	static PersonInformation pi;
	FS_FILE * fp;	
	SearchHit sh;
	
	pi.PersonNumber = 0;
	pi.Password = 0;
	pi.FingerID = 0;
	pi.CardID = 0;
	pi.PIS = PIS_CardOrFinger;
	pi.PAR = PAR_User;
	
	sh = search("Personal", "Master.INF", person_number, MasterFileRecordLength);

	if (sh.Hit == false)
		return pi;
	
	fp = FS_FOpen("Personal\\Master.INF" ,"rb");
	
	if (fp == NULL)
	{
		//FS_FClose (fp);
		return pi;
	}
	
	FS_SetFilePos (fp, sh.Index * MasterFileRecordLength,  FS_FILE_BEGIN);
	
	FS_Read (fp, &pi, MasterFileRecordLength);
	
	FS_FClose (fp);
	
	return pi;		
}

void Personal::create_directory (char file_path[40])
{
	char p[40];
	int ctr_main, ctr_sub;

	strcpy(p, "");
	
	ctr_sub = 0;
	
	for (ctr_main = 0; ctr_main<strlen(file_path); ctr_main++, ctr_sub++)
	{
		if (file_path[ctr_main] == '\\')
		{
			p[ctr_sub] = 0;
			FS_MkDir(p);
			strcat(p, "\\");
		}
		
		p[ctr_sub] = file_path[ctr_main];
	}
	
	p[ctr_sub] = 0;
	
	FS_MkDir(p);
}

bool Personal::insert_record (char file_path[], char file_name[], char backup_file_name[], const DWORD index, const BYTE * Data, const DWORD size)
{
	bool flag = true;
	DWORD i;
	char s[30];
	
	flag &= copy_file_section(file_path, file_name, file_path, backup_file_name, 0, index, size);
			
	flag &= append (file_path, backup_file_name, Data, size);
	
	i = get_records_count(file_path, file_name, size) - index;
	
	flag &= copy_file_section(file_path, file_name, file_path, backup_file_name, index, i, size);
	
	strcpy(s, file_path);
	strcat(s, "\\");
	strcat(s, file_name);
	
	FS_Remove(s);
	
	strcpy(s, file_path);
	strcat(s, "\\");
	strcat(s, backup_file_name);
	
	FS_Rename(s , file_name);
	
	return flag;	
}

bool Personal::edit_record (char file_path[], char file_name[], char backup_file_name[], const DWORD index, const BYTE * Data, const DWORD size)
{
	bool flag = true;
	DWORD i;
	char s[30];
	
	flag &= copy_file_section(file_path, file_name, file_path, backup_file_name, 0, index, size);
			
	flag &= append (file_path, backup_file_name, Data, size);
	
	i = get_records_count(file_path, file_name, size) - index - 1;
	
	flag &= copy_file_section(file_path, file_name, file_path, backup_file_name, index + 1, i, size);
	
	strcpy(s, file_path);
	strcat(s, "\\");
	strcat(s, file_name);
	
	FS_Remove(s);
	
	strcpy(s, file_path);
	strcat(s, "\\");
	strcat(s, backup_file_name);
	
	FS_Rename(s , file_name);
	
	return flag;	
}

bool Personal::delete_record (char file_path[], char file_name[], char backup_file_name[], const DWORD index, const DWORD size)
{
	bool flag = true;
	DWORD i;
	char s[30];
	
	flag &= copy_file_section(file_path, file_name, file_path, backup_file_name, 0, index, size);
			
	i = get_records_count(file_path, file_name, size) - index - 1;
	
	flag &= copy_file_section(file_path, file_name, file_path, backup_file_name, index + 1, i, size);
	
	strcpy(s, file_path);
	strcat(s, "\\");
	strcat(s, file_name);
	
	FS_Remove(s);
	
	strcpy(s, file_path);
	strcat(s, "\\");
	strcat(s, backup_file_name);
	
	FS_Rename(s , file_name);
	
	return flag;		
}


void Personal::task_get_person_information (void * PN)
{
	OS_EnterRegion();

        static PersonInformation pi;
	
	pi = get_person_information(*(DWORD*) PN);
	
	if (ep_get_person_information)
		ep_get_person_information (&pi);
	
        OS_LeaveRegion();
	OS_Terminate(0);		
}

/*void Personal::task_is_person_exist (void * PN)
{
	if (!is_file_exist("Personal","Master.INF"))
		safe_call_handler (ep_person_not_exist);			
	
	else if (search("Personal", "Master.INF", *(DWORD*)PN, MasterFileRecordLength).Hit == 1)
		safe_call_handler(ep_person_exist);
	
	else
		safe_call_handler (ep_person_not_exist);

	OS_Terminate(0);	
}

void Personal::task_add_person (void * PI)
{
	SearchHit sh;
	bool flag = true;
	
	if (!is_file_exist("Personal","Master.INF"))
		flag &= create_file("Personal", "Master.INF");
	
	sh = search ("Personal", "Master.INF", ((PersonInformation*)PI)->PersonNumber,MasterFileRecordLength);
	
	if (((PersonInformation*)PI)->PersonNumber > sh.Value)
		sh.Index++;
	
	flag &= insert_record ("Personal", "Master.INF", "MBack.INF", sh.Index, (BYTE*) PI, MasterFileRecordLength);	
	
	if (((PersonInformation*)PI)->FingerID != 0)
	{
		if (!is_file_exist("Personal","Finger.INF"))
			flag &= create_file("Personal", "Finger.INF");
	
		sh = search ("Personal", "Finger.INF", ((PersonInformation*)PI)->FingerID, FingerFileRecordLength);
	
		if (((PersonInformation*)PI)->FingerID > sh.Value)
			sh.Index++;
	
		IDPerson idp;
		
		idp.PersonNumber = ((PersonInformation*)PI)->PersonNumber;
		idp.ID = ((PersonInformation*)PI)->FingerID;
		
		flag &= insert_record ("Personal", "Finger.INF", "FBack.INF", sh.Index, (BYTE*) &idp, FingerFileRecordLength);	
	}
		
	if (((PersonInformation*)PI)->CardID != 0)
	{
		if (!is_file_exist("Personal","Card.INF"))
			flag &= create_file("Personal", "Card.INF");
		
		sh = search ("Personal", "Card.INF", ((PersonInformation*)PI)->CardID, CardFileRecordLength);
	
		if (((PersonInformation*)PI)->CardID > sh.Value)
			sh.Index++;
		
		IDPerson idp;
		
		idp.PersonNumber = ((PersonInformation*)PI)->PersonNumber;
		idp.ID = ((PersonInformation*)PI)->CardID;
		
		flag &= insert_record ("Personal", "Card.INF", "CBack.INF", sh.Index, (BYTE*) &idp, CardFileRecordLength);	
	}
	
	if (flag == true)
		safe_call_handler (ep_person_add_success);
	else
		safe_call_handler (ep_person_add_fail);		
	
	OS_Terminate(0);	
}


void Personal::task_edit_person (void * PI)
{
	PersonInformation old_pi, new_pi;
	SearchHit sh;
	bool flag = true;

	new_pi = *(PersonInformation*) PI;
	
	old_pi = get_person_information (new_pi.PersonNumber);

	sh = search ("Personal", "Master.INF", old_pi.PersonNumber ,MasterFileRecordLength);
	
	flag &= edit_record ("Personal", "Master.INF", "MBack.INF", sh.Index, (BYTE*) & new_pi, MasterFileRecordLength);	
	

	if (old_pi.FingerID != 0)
	{
		sh = search ("Personal", "Finger.INF", old_pi.FingerID, FingerFileRecordLength);
	
		flag &= delete_record ("Personal", "Finger.INF", "FBack.INF", sh.Index , FingerFileRecordLength);
		
		sh = search ("Personal", "Finger.INF", new_pi.FingerID, FingerFileRecordLength);
	
		if (new_pi.FingerID > sh.Value)
			sh.Index++;
	
		IDPerson idp;
		
		idp.PersonNumber = new_pi.PersonNumber;
		idp.ID = new_pi.FingerID;
		
		flag &= insert_record ("Personal", "Finger.INF", "FBack.INF", sh.Index, (BYTE*) &idp, FingerFileRecordLength);	
	}
		
	if (old_pi.CardID != 0)
	{				
		sh = search ("Personal", "Card.INF", old_pi.CardID, CardFileRecordLength);
	
		flag &= delete_record ("Personal", "Card.INF", "CBack.INF", sh.Index , CardFileRecordLength);
		
		sh = search ("Personal", "Card.INF", new_pi.CardID, CardFileRecordLength);
	
		if (new_pi.CardID > sh.Value)
			sh.Index++;
	
		IDPerson idp;
		
		idp.PersonNumber = new_pi.PersonNumber;
		idp.ID = new_pi.CardID;
		
		flag &= insert_record ("Personal", "Card.INF", "CBack.INF", sh.Index, (BYTE*) &idp, CardFileRecordLength);			
	}
	
	if (flag == true)
		safe_call_handler (ep_person_edit_success);
	else
		safe_call_handler (ep_person_edit_fail);		
	
	OS_Terminate(0);	
}

void Personal::task_delete_person (void * PN)
{
	PersonInformation pi;
	DWORD pn;
	SearchHit sh;
	bool flag = true;
	Finger f;

	pn = *(DWORD*)PN;
	
	pi = get_person_information (pn);

	sh = search ("Personal", "Master.INF", pn ,MasterFileRecordLength);
	
	flag &= delete_record ("Personal", "Master.INF", "MBack.INF", sh.Index, MasterFileRecordLength);	
	
	if (pi.FingerID != 0)
	{
		sh = search ("Personal", "Finger.INF", pi.FingerID, FingerFileRecordLength);
	
		flag &= delete_record ("Personal", "Finger.INF", "FBack.INF", sh.Index , FingerFileRecordLength);		
		
		f.DeleteTemplates(pi.FingerID);
	}
		
	if (pi.CardID != 0)
	{				
		sh = search ("Personal", "Card.INF", pi.CardID, CardFileRecordLength);
	
		flag &= delete_record ("Personal", "Card.INF", "CBack.INF", sh.Index , CardFileRecordLength);						
	}
	
	if (flag == true)
		safe_call_handler (ep_person_delete_success);
	else
		safe_call_handler (ep_person_delete_fail);		
	
	OS_Terminate(0);	
}

void Personal::task_delete_all_peron (void)
{
	Finger f;
	int err;

	f.DeleteAllTemplates();
	
	err = FS_Remove ("Personal\\Card.INF");
	err += FS_Remove ("Personal\\Finger.INF");
	err += FS_Remove ("Personal\\Master.INF");
	
	//if (err<0)
		//safe_call_handler(ep_person_delete_all_fail);

	//else if (err == 0)
		safe_call_handler(ep_person_delete_all_success);	
	
	OS_Terminate(0);	
}

void Personal::task_get_last_person (void)
{	
	PersonInformation pi;
	FS_FILE * fp;	
	
	if (is_file_exist("Personal", "Master.INF") == 0)
		if (ep_get_last_personal != NULL)
		{
			ep_get_last_personal(0);

			OS_Terminate(0);	
		}
	
	if (get_records_count("Personal", "Master.INF", MasterFileRecordLength) == 0)
		if (ep_get_last_personal != NULL)
		{
			ep_get_last_personal(0);

			OS_Terminate(0);	
		}

	if (get_records_count("Personal", "Master.INF", MasterFileRecordLength) > 0)
	{	
		fp = FS_FOpen("Personal\\Master.INF" ,"rb");
	
		FS_SetFilePos (fp, (get_records_count("Personal", "Master.INF", MasterFileRecordLength) - 1) * MasterFileRecordLength,  FS_FILE_BEGIN);
	
		FS_Read (fp, &pi, MasterFileRecordLength);
	
		if (ep_get_last_personal != NULL)
		{
			ep_get_last_personal(pi.PersonNumber);

			OS_Terminate(0);	
		}

	}
}
		
void Personal::task_handle_card (void * ci)
{
	SearchHit sh;
	IDPerson idp;
	PersonInformation pi;
	Setting set;
	StatusBar s;
	static ID_Time idt;
	
	if (idt.ID == (*(DWORD*)ci))
		if (DateTime::GetPassedSecondesUntilNow(idt.Time)<5)
			OS_Terminate(0);	

	idt.ID = (*(DWORD*)ci);		
	idt.Time = DateTime::Now();
	
	sh = search ("Personal", "Card.INF", (*(DWORD*)ci), CardFileRecordLength);
	
	if (sh.Hit == false)
	{		
		s.ShowMessageTemporarily(get_appropriate_message(PM_InvalidPersonal, Farsi));
		OS_Terminate(0);
	}
	
	else
	{
		FS_FILE *fp;
	
		fp = FS_FOpen("Personal\\Card.INF","rb");
	
		if (fp == NULL)
		{
			s.ShowMessageTemporarily(get_appropriate_message(PM_ErrorWhileSave, Farsi));
			OS_Terminate(0);	
		}

		FS_SetFilePos (fp, CardFileRecordLength * sh.Index,  FS_FILE_BEGIN);
	
		FS_Read (fp,&idp,CardFileRecordLength);
	
		FS_FClose(fp);
		
		pi = get_person_information(idp.PersonNumber);
		
		if (pi.PIS == PIS_DeviceMode)
			pi.PIS = set.GetSettings().DeviceMode;
		
		ti.PN = pi.PersonNumber;
		ti.DT= DateTime::Now();
		ti.PIS = pi.PIS;
		ti.PC = set.GetSettings().DevicePreCard;
		
		if (pi.PIS == PIS_CardOnly)
			save_traffic(ti);
		
		else if (pi.PIS == PIS_PersonalNumberORCardAndFinger)
		{
			if (pi.FingerTemplateNumber == 0)
			{		
				s.ShowMessageTemporarily(get_appropriate_message(PM_HasNotFinger, Farsi));
				OS_Terminate(0);
			}				
			
			Finger f;
			
			f.DisableEvents();
			
			f.SetVerifySuccessEvent (E_valid_finger);
			f.SetVerifyFailEvent (E_invalid_finger);
			
			first_device_sequence = DS_Card;
			
			s.ShowMessageTemporarily(get_appropriate_message(PM_PlaceFinger, Farsi));
			
			f.VerifyByScan (pi.FingerID);	
		}
	}
	
	OS_Terminate(0);	
}

void Personal::task_handle_finger (void * fi)
{
	SearchHit sh;
	IDPerson idp;
	PersonInformation pi;
	Setting set;
	StatusBar s;
	
	sh = search ("Personal", "Finger.INF", (*(DWORD*)fi), FingerFileRecordLength);
	
	if (sh.Hit == false)
	{		
		s.ShowMessageTemporarily(get_appropriate_message(PM_InvalidPersonal, Farsi));
		OS_Terminate(0);	
	}
	
	else
	{
		FS_FILE *fp;
	
		fp = FS_FOpen("Personal\\Finger.INF","rb");
	
		if (fp == NULL)
		{
			s.ShowMessageTemporarily(get_appropriate_message(PM_ErrorWhileSave, Farsi));
			OS_Terminate(0);	
		}

		FS_SetFilePos (fp, FingerFileRecordLength * sh.Index,  FS_FILE_BEGIN);
	
		FS_Read (fp,&idp,FingerFileRecordLength);
	
		FS_FClose(fp);
		
		pi = get_person_information(idp.PersonNumber);
		
		ti.PN = pi.PersonNumber;
		ti.DT= DateTime::Now();
		ti.PIS = PIS_FingerOnly;
		ti.PC = set.GetSettings().DevicePreCard;
			
		save_traffic(ti);
	}
	
	OS_Terminate(0);	
}

void Personal::task_is_admin_finger (void * fi)
{
	SearchHit sh;
	IDPerson idp;
	PersonInformation pi;
	Setting set;
	StatusBar s;
	
	sh = search ("Personal", "Finger.INF", (*(DWORD*)fi), FingerFileRecordLength);
	
	if (sh.Hit == false)
		OS_Terminate(0);	

	FS_FILE *fp;
	
	fp = FS_FOpen("Personal\\Finger.INF","rb");
	
	if (fp == NULL)
		OS_Terminate(0);	

	FS_SetFilePos (fp, FingerFileRecordLength * sh.Index,  FS_FILE_BEGIN);
	
	FS_Read (fp,&idp,FingerFileRecordLength);
	
	FS_FClose(fp);
		
	pi = get_person_information(idp.PersonNumber);
		
	if (pi.PAR == PAR_Administrator)
		safe_call_handler (ep_valid_admin_finger);
	
	OS_Terminate(0);	
}

void Personal::task_handle_key (void * ki)
{
	SearchHit sh;
	PersonInformation pi;
	Setting set;
	StatusBar s;
	
	sh = search ("Personal", "Master.INF", (*(DWORD*)ki), MasterFileRecordLength);
	
	if (sh.Hit == false)
	{		
		s.ShowMessageTemporarily(get_appropriate_message(PM_InvalidPersonal, Farsi));
		OS_Terminate(0);
	}
	
	else
	{	
		pi = get_person_information((*(DWORD*)ki));
		
		if (pi.PIS == PIS_DeviceMode)
			pi.PIS = set.GetSettings().DeviceMode;
		
		ti.PN = (*(DWORD*)ki);
		ti.DT= DateTime::Now();
		ti.PC = set.GetSettings().DevicePreCard;
		
		if (pi.PIS == PIS_PersonalNumberORCardAndFinger)
		{
			if (pi.FingerTemplateNumber == 0)
			{		
				s.ShowMessageTemporarily(get_appropriate_message(PM_HasNotFinger, Farsi));
				OS_Terminate(0);
			}				
			
			Finger f;
			
			f.DisableEvents();
			
			f.SetVerifySuccessEvent (E_valid_finger);
			f.SetVerifyFailEvent (E_invalid_finger);
			
			first_device_sequence = DS_Key;
			
			s.ShowMessageTemporarily(get_appropriate_message(PM_PlaceFinger, Farsi));
			
			f.VerifyByScan (pi.FingerID);	
		}
	}


	OS_Terminate(0);	
}

void Personal::task_internal_save (void)
{
	save_traffic(ti);

	OS_Terminate(0);	
}*/

void Personal::task_main (void * in)
{
	SearchHit sh;
	bool flag = true;
	PersonInformation old_pi, new_pi;
	PersonInformation pi;
	Finger f;

        Setting set;
        StatusBar s;

        IDPerson idp;

        char fn[20];
        char fp[30];



        OS_EnterRegion();

        if (pti == PTI_IsPersonExist)
	{
		if (!is_file_exist("Personal","Master.INF"))
			safe_call_handler (ep_person_not_exist);			
	
		else if (search("Personal", "Master.INF", *(DWORD*)in, MasterFileRecordLength).Hit == 1)
			safe_call_handler(ep_person_exist);
		
		else
			safe_call_handler (ep_person_not_exist);
	
        	OS_LeaveRegion();
		OS_Terminate(0);	
	}

        if (pti == PTI_IsCardRepetitive)
	{
		if (!is_file_exist("Personal","Card.INF"))
			safe_call_handler (ep_card_is_unique);			
	
		else if (search("Personal", "Card.INF", *(DWORD*)in, CardFileRecordLength).Hit == 1)
			safe_call_handler(ep_card_is_repetitive);
		
		else
			safe_call_handler (ep_card_is_unique);
	
        	OS_LeaveRegion();
		OS_Terminate(0);	
	}
        
         if (pti == PTI_IsCardRepetitiveEx)
	{
		if (!is_file_exist("Personal","Card.INF"))
			safe_call_handler (ep_card_is_unique_ex);			
	
		else if (search("Personal", "Card.INF", *(DWORD*)in, CardFileRecordLength).Hit == 1)
			safe_call_handler(ep_card_is_repetitive);
		
		else
			safe_call_handler (ep_card_is_unique_ex);
	
        	OS_LeaveRegion();
		OS_Terminate(0);	
	}


        /*	if (pti == PTI_GetPersonInformation)
	{
		static PersonInformation pi;
	
		pi = get_person_information(*(DWORD*) in);
	
		if (ep_get_person_information)
			ep_get_person_information (pi);
	
		OS_Terminate(0);		
	}*/

	if (pti == PTI_AddPerson)
	{
		if (!is_file_exist("Personal","Master.INF"))
			flag &= create_file("Personal", "Master.INF");
	
		sh = search ("Personal", "Master.INF", ((PersonInformation*)in)->PersonNumber,MasterFileRecordLength);
	
		if (((PersonInformation*)in)->PersonNumber > sh.Value)
			sh.Index++;
	
		flag &= insert_record ("Personal", "Master.INF", "MBack.INF", sh.Index, (BYTE*) in, MasterFileRecordLength);	
	
                if (!is_file_exist("Personal","Finger.INF"))
			flag &= create_file("Personal", "Finger.INF");
                
                if (!is_file_exist("Personal","Card.INF"))
			flag &= create_file("Personal", "Card.INF");
                
		if (((PersonInformation*)in)->FingerID != 0)
		{
		
			sh = search ("Personal", "Finger.INF", ((PersonInformation*)in)->FingerID, FingerFileRecordLength);
		
			if (((PersonInformation*)in)->FingerID > sh.Value)
				sh.Index++;
		
			IDPerson idp;
			
			idp.PersonNumber = ((PersonInformation*)in)->PersonNumber;
			idp.ID = ((PersonInformation*)in)->FingerID;
			
			flag &= insert_record ("Personal", "Finger.INF", "FBack.INF", sh.Index, (BYTE*) &idp, FingerFileRecordLength);	
		}
			
		if (((PersonInformation*)in)->CardID != 0)
		{
			
			sh = search ("Personal", "Card.INF", ((PersonInformation*)in)->CardID, CardFileRecordLength);
		
			if (((PersonInformation*)in)->CardID > sh.Value)
				sh.Index++;
			
			IDPerson idp;
			
			idp.PersonNumber = ((PersonInformation*)in)->PersonNumber;
			idp.ID = ((PersonInformation*)in)->CardID;
			
			flag &= insert_record ("Personal", "Card.INF", "CBack.INF", sh.Index, (BYTE*) &idp, CardFileRecordLength);	
		}
		
		    if (flag == true)
			safe_call_handler (ep_person_add_success);
		    else
			safe_call_handler (ep_person_add_fail);	
		
        	OS_LeaveRegion();
		OS_Terminate(0);	
	}

	if (pti == PTI_EditPerson)
	{
		new_pi = *(PersonInformation*) in;
		
		old_pi = get_person_information (new_pi.PersonNumber);
	
		sh = search ("Personal", "Master.INF", old_pi.PersonNumber ,MasterFileRecordLength);
		
		flag &= edit_record ("Personal", "Master.INF", "MBack.INF", sh.Index, (BYTE*) & new_pi, MasterFileRecordLength);	
		
	
		if (old_pi.FingerID != 0)
		{
			sh = search ("Personal", "Finger.INF", old_pi.FingerID, FingerFileRecordLength);
		
			flag &= delete_record ("Personal", "Finger.INF", "FBack.INF", sh.Index , FingerFileRecordLength);
			
		}
                if (new_pi.FingerID != 0)
                {
                       sh = search ("Personal", "Finger.INF", new_pi.FingerID, FingerFileRecordLength);
		
			if (new_pi.FingerID > sh.Value)
				sh.Index++;
		
			IDPerson idp;
			
			idp.PersonNumber = new_pi.PersonNumber;
			idp.ID = new_pi.FingerID;
			
			flag &= insert_record ("Personal", "Finger.INF", "FBack.INF", sh.Index, (BYTE*) &idp, FingerFileRecordLength);	  
                }
			
		if (old_pi.CardID != 0)
		{				
			sh = search ("Personal", "Card.INF", old_pi.CardID, CardFileRecordLength);
		
			flag &= delete_record ("Personal", "Card.INF", "CBack.INF", sh.Index , CardFileRecordLength);
		
                }
                if (new_pi.CardID != 0)
                {
                      	sh = search ("Personal", "Card.INF", new_pi.CardID, CardFileRecordLength);
		
			if (new_pi.CardID > sh.Value)
				sh.Index++;
		
			IDPerson idp;
			
			idp.PersonNumber = new_pi.PersonNumber;
			idp.ID = new_pi.CardID;
			
			flag &= insert_record ("Personal", "Card.INF", "CBack.INF", sh.Index, (BYTE*) &idp, CardFileRecordLength);			   
                }
                
		if(Menu::is_OK == true)
                {
                        if (flag == true)
                                safe_call_handler (ep_person_edit_success);
                        else
                                safe_call_handler (ep_person_edit_fail);	
                }
		
        	OS_LeaveRegion();		
                OS_Terminate(0);	
	}

	if (pti == PTI_DeletePerson)
	{
        	DWORD pn;

                pn = *(DWORD*)in;
		
		pi = get_person_information (pn);
	
		sh = search ("Personal", "Master.INF", pn ,MasterFileRecordLength);
		
		flag &= delete_record ("Personal", "Master.INF", "MBack.INF", sh.Index, MasterFileRecordLength);	
		
		if (pi.FingerID != 0)
		{
			sh = search ("Personal", "Finger.INF", pi.FingerID, FingerFileRecordLength);
		
			flag &= delete_record ("Personal", "Finger.INF", "FBack.INF", sh.Index , FingerFileRecordLength);		
			
			f.DeleteTemplates(pi.FingerID);
		}
			
		if (pi.CardID != 0)
		{				
			sh = search ("Personal", "Card.INF", pi.CardID, CardFileRecordLength);
		
			flag &= delete_record ("Personal", "Card.INF", "CBack.INF", sh.Index , CardFileRecordLength);						
		}
                if (flag == true)
                        safe_call_handler (ep_person_delete_success);
                else
                        safe_call_handler (ep_person_delete_fail);	

		
                OS_LeaveRegion();
                OS_Terminate(0);	
	}

	if (pti == PTI_DeleteAllPerson)
	{
		int err;
	
		f.DeleteAllTemplates();
		
		err = FS_Remove ("Personal\\Card.INF");
		err += FS_Remove ("Personal\\Finger.INF");
		err += FS_Remove ("Personal\\Master.INF");
		
		//if (err<0)
			//safe_call_handler(ep_person_delete_all_fail);
	
		//else if (err == 0)
			safe_call_handler(ep_person_delete_all_success);	
		
        	OS_LeaveRegion();
                OS_Terminate(0);			
	}

	if (pti == PTI_DeleteAllRecords)
	{
		FS_FIND_DATA fd;		

                if (FS_FindFirstFile(&fd, "Traffic", fn, sizeof(fn)) == 0)
                        do
                        {
                                strcpy(fp,"Traffic\\");
                                strcat(fp, fn);

                                FS_Remove(fp);
                        }
                        while(FS_FindNextFile(&fd));


                FS_FindClose (&fd);

		safe_call_handler(ep_records_delete_success);	

        	OS_LeaveRegion();		
                OS_Terminate(0);			
	}

	if (pti == PTI_HandleCard)
	{
                Beep b;
                MifareEx mx;
		sh = search ("Personal", "Card.INF", (*(DWORD*)in), CardFileRecordLength);
		
		if (sh.Hit == false)
		{	
                        if(mifareType == MT_Exernal)
                            mx.MifareSetAlarmrErr();
                        else
                        {
                            s.ShowMessageTemporarily(get_appropriate_message(PM_InvalidPersonal, Farsi));
                            b.BeepErr();
                        }
                        OS_LeaveRegion();
			OS_Terminate(0);
		}
		
		else
		{
			FS_FILE *fp;
		
			fp = FS_FOpen("Personal\\Card.INF","rb");
		
			if (fp == NULL)
			{
                               if(mifareType == MT_Exernal)
                                    mx.MifareSetAlarmrErr();
                                else
                                {
                                    s.ShowMessageTemporarily(get_appropriate_message(PM_ErrorWhileSave, Farsi));
                                    //FS_FClose (fp);
                                    b.BeepErr();
                                }
                        	OS_LeaveRegion();				
                                OS_Terminate(0);	
			}
		
			FS_SetFilePos (fp, CardFileRecordLength * sh.Index,  FS_FILE_BEGIN);
		
			FS_Read (fp,&idp,CardFileRecordLength);
		
			FS_FClose(fp);
			
			pi = get_person_information(idp.PersonNumber);
			
                        if(pi.PersonNumber == 0)
                        {
                                 if(mifareType == MT_Exernal)
                                    mx.MifareSetAlarmrErr();
                                else
                                {
                                    s.ShowMessageTemporarily(get_appropriate_message(PM_InvalidPersonal, Farsi));
                                    b.BeepErr();
                                }
                                OS_LeaveRegion();
			        OS_Terminate(0);
                        }
                        
			if (pi.PIS == PIS_DeviceMode)
				pi.PIS = set.GetSettings().DeviceMode;
			
			ti.PN = pi.PersonNumber;
			ti.DT= DateTime::GetPackedDateTime(DateTime::Now());
			ti.PIS = pi.PIS;
			ti.PC = person_pre_card;
			
                         if(mifareType == MT_Exernal)
                                    mx.MifareSetAlarmOK();

			if (pi.PIS == PIS_CardOrFinger)
				save_traffic(ti);
			
			else if (pi.PIS == PIS_CardAndFinger)
			{
                                b.DoBeep(50);
				if (pi.FingerTemplateNumber == 0)
				{		
					s.ShowMessageTemporarily(get_appropriate_message(PM_HasNotFinger, Farsi));
                                        OS_LeaveRegion();
					OS_Terminate(0);
				}				
				
				Finger f;
                                Mifare m;
				
				f.DisableEvents();
				
				f.SetVerifySuccessEvent (E_valid_finger);
				f.SetVerifyFailEvent (E_invalid_finger);
				
				first_device_sequence = DS_Card;
				
				s.ShowMessageTemporarily(get_appropriate_message(PM_PlaceFinger, Farsi));
					
                                m.AnalyzePermission(false);
                                mx.AnalyzePermission(false);
				f.VerifyByScan (pi.FingerID);	
                                f.start_timeout();
			}
		}
		
        	OS_LeaveRegion();		
                OS_Terminate(0);	
	}

	if (pti == PTI_HandleFinger)
	{
		sh = search ("Personal", "Finger.INF", (*(DWORD*)in), FingerFileRecordLength);
		Beep b;
                
		if (sh.Hit == false)
		{		
			s.ShowMessageTemporarily(get_appropriate_message(PM_InvalidPersonal, Farsi));
                       	b.BeepErr();
                        OS_LeaveRegion();
			OS_Terminate(0);	
		}
		
		else
		{
			FS_FILE *fp;
		
			fp = FS_FOpen("Personal\\Finger.INF","rb");
		
			if (fp == NULL)
			{
				s.ShowMessageTemporarily(get_appropriate_message(PM_ErrorWhileSave, Farsi));
				//FS_FClose (fp);
                                b.BeepErr();
                        	OS_LeaveRegion();
				OS_Terminate(0);	
			}
	
			FS_SetFilePos (fp, FingerFileRecordLength * sh.Index,  FS_FILE_BEGIN);
		
			FS_Read (fp,&idp,FingerFileRecordLength);
		
			FS_FClose(fp);
			
			pi = get_person_information(idp.PersonNumber);
			
                        if(pi.PersonNumber == 0)
                        {
                                s.ShowMessageTemporarily(get_appropriate_message(PM_InvalidPersonal, Farsi));
                                b.BeepErr();
                                OS_LeaveRegion();
			        OS_Terminate(0);
                        }
                        
                        if (pi.PIS == PIS_DeviceMode)
				pi.PIS = set.GetSettings().DeviceMode;
                        
			ti.PN = pi.PersonNumber;
			ti.DT= DateTime::GetPackedDateTime(DateTime::Now());
			ti.PIS = PIS_FingerOnly;
			ti.PC = person_pre_card;
			
                        if (pi.PIS == PIS_CardAndFinger)
                        {
				s.ShowMessageTemporarily(get_appropriate_message(PM_ErrorCardFirst, Farsi));
				//FS_FClose (fp);
                                b.DoBeep(50);
                        	OS_LeaveRegion();
				OS_Terminate(0);	
			}
                          
			save_traffic(ti);
		}
		
        	OS_LeaveRegion();		
                OS_Terminate(0);			
	}

	if (pti == PTI_HandleKey)
	{
		/*sh = search ("Personal", "Master.INF", (*(DWORD*)in), MasterFileRecordLength);
		
		if (sh.Hit == false)
		{		
			s.ShowMessageTemporarily(get_appropriate_message(PM_InvalidPersonal, Farsi));
                	OS_LeaveRegion();
			OS_Terminate(0);
		}
		
		else
		{	
			pi = get_person_information((*(DWORD*)in));
			
			if (pi.PIS == PIS_DeviceMode)
				pi.PIS = set.GetSettings().DeviceMode;
			
			ti.PN = (*(DWORD*)in);
			ti.DT= DateTime::GetPackedDateTime(DateTime::Now());
			ti.PC = person_pre_card;
			
			if (pi.PIS == PIS_PersonalNumberORCardAndFinger)
			{
				if (pi.FingerTemplateNumber == 0)
				{		
					s.ShowMessageTemporarily(get_appropriate_message(PM_HasNotFinger, Farsi));
                                	OS_LeaveRegion();
					OS_Terminate(0);
				}				
				
				Finger f;
				
				f.DisableEvents();
				
				f.SetVerifySuccessEvent (E_valid_finger);
				f.SetVerifyFailEvent (E_invalid_finger);
				
				first_device_sequence = DS_Key;
				
				s.ShowMessageTemporarily(get_appropriate_message(PM_PlaceFinger, Farsi));
				
				f.VerifyByScan (pi.FingerID);	
			}
		}*/
	
        	OS_LeaveRegion();
		OS_Terminate(0);	
	}

	if (pti == PTI_InternalSave)
	{
		save_traffic(ti);

                OS_LeaveRegion();
		OS_Terminate(0);	
	}

	if (pti == PTI_IsAdminFingerID)
	{
		sh = search ("Personal", "Finger.INF", (*(DWORD*)in), FingerFileRecordLength);
		
		if (sh.Hit == false)
                {
                        OS_LeaveRegion();
			OS_Terminate(0);	
                }
	
		FS_FILE *fp;
		
		fp = FS_FOpen("Personal\\Finger.INF","rb");
		
		if (fp == NULL)
		{
			//FS_FClose(fp);
                	OS_LeaveRegion();			
                        OS_Terminate(0);	
		}
	
		FS_SetFilePos (fp, FingerFileRecordLength * sh.Index,  FS_FILE_BEGIN);
		
		FS_Read (fp,&idp,FingerFileRecordLength);
		
		FS_FClose(fp);
			
		pi = get_person_information(idp.PersonNumber);
			
		if (pi.PAR == PAR_Administrator)
			safe_call_handler (ep_valid_admin_finger);
		
		OS_LeaveRegion();
                OS_Terminate(0);			
	}
}
		
void Personal::save_traffic (TrafficInformation ti)
{
	string str;
	char c[30];
	RTCTime dt;
	bool res;
	
        OS_WaitCSema(&Communication::sema);
        
#ifdef Trace

        Mifare::t ++;
        ti.DT += (Mifare::t*60);

#endif 
        dt = DateTime::GetRealDateTime(ti.DT);
       
	
	str += Utility::ToString(dt.RTC_Year%100);
	str += "_";
	str += Utility::ToString(dt.RTC_Mon);
	str += "_";
	str += Utility::ToString(dt.RTC_Mday);	
        str += ".RCD";
	
	Utility::ToCharArray(str, c);
	
	res = append ("Traffic", c, (BYTE*) &ti, sizeof(TrafficInformation));

	//res &= append ("Traffic", "Uncol.Inf", (BYTE*) &ti, sizeof(TrafficInformation)); 	
	
	if (res)
	{
		NotifyBar  n;
		View v;
		StatusBar s;
	
		switch (ti.PC)
		{

                case PC_NormalEnter:
			str = get_appropriate_message(PM_NormalEnter, Farsi);
			break;
		
                case PC_Enter:
			str = get_appropriate_message(PM_Enter, Farsi);
			break;
			
		case PC_Exit:
			str = get_appropriate_message(PM_Exit, Farsi);
			break;

		case PC_Mission:
			str = get_appropriate_message(PM_Mission, Farsi);
			break;

		case PC_Vacation:
			str = get_appropriate_message(PM_Vacation, Farsi);
			break;
		}		

		//n.TurnSaveIcon(On);
		v.ShowInvertMessageTemporarily(Utility::ToString(ti.PN));

		s.ShowMessageInvertTemporarily(str);	
                n.ShowMessageInvertTemporarily("��� �����");

 		OS_RetriggerTimer(&timer_save);
		if (ep_save_success!=NULL)
                        (ep_save_success(&ti));
	}

        OS_SignalCSema(&Communication::sema);
}
	

void Personal::timer_elapse_save (void)
{
	NotifyBar  n;
	ShowTime st;
	n.TurnSaveIcon(Off);	
	
	st.UpdateViewScopePermission (true);
	st.UpdateStatusScopePermission (true);
}
	
string Personal::get_appropriate_message (PersonalMessage index, Language l)
{
	string message;
	
	message.clear();
	
	switch (l)
	{
	case Farsi:
		switch (index)
		{
                case PM_NormalEnter:
			message = "����";
			break;

                case PM_Enter:
			message = "����";
			break;
			
		case PM_Exit:
			message = "����";
			break;		

		case PM_Mission:
			message = "�������";
			break;		

		case PM_Vacation:
			message = "�����";
			break;		
			
		case PM_InvalidPersonal:
			message = "��� ����� �����";
			break;		
			
		case PM_ErrorWhileSave:
			message = "�Ԙ�� �� ��� ����";
			break;
                
                case PM_ErrorCardFirst:  
 			message = "����� ���� �� Ȑ�����";
			break;                 
			
		case PM_HasNotFinger:
			message = "���� ��� ����";
			break;	

		case PM_PlaceFinger:
			message = "���� ��� �� Ȑ�����";
			break;	
			
		case PM_TimeOut:
			message = "����� ���� �����";
			break;		

		case PM_CardRepetitive:
			message = "���� ʘ����";
			break;				
			
		case PM_InvalidFinger:
			message = "��� ���� �������";
			break;			

		}			
	}
	
	return message;
}

void Personal::E_valid_finger (void)
{
	if (first_device_sequence == DS_Card)
		ti.PIS = PIS_CardAndFinger;
	else if (first_device_sequence == DS_Key)
		ti.PIS =PIS_PersonalNumberANDFinger;		
	

	if (is_task_running(&tcb_main_task))
		return;
	
	pti = PTI_InternalSave;
	
	OS_CREATETASK_EX(&tcb_main_task, "Personal_Main_Task", task_main, TP_RealTimePeriority, stack_main_task, (void*) 0);	
}

void Personal::E_invalid_finger (void)
{
        StatusBar s;
        Beep b;
        b.BeepErr();
	s.ShowMessageTemporarily(get_appropriate_message(PM_InvalidFinger, Farsi));	
        safe_call_handler (ep_time_out_input);
}

bool Personal::IsUserExit (DWORD PersonNumber)
{
        bool bln;
        
        if (!is_file_exist("Personal","Master.INF"))
                bln = false;	

        else if (search("Personal", "Master.INF", PersonNumber, MasterFileRecordLength).Hit == 1)
                bln = true;
        else
                bln = false;
	
        return bln;
}

bool Personal::IsCardRepet (DWORD CardID)
{
        bool bln;
  
        if (!is_file_exist("Personal","Card.INF"))
                bln = false;

        else if (search("Personal", "Card.INF", CardID, CardFileRecordLength).Hit == 1)
                bln = true;	
        else
                bln = false;
        
        return bln;
}

bool Personal::AddPersonInfo (PersonInformation NewPI)
{
        bool flag = true;
        SearchHit sh;
  
        if (!is_file_exist("Personal","Master.INF"))
                flag &= create_file("Personal", "Master.INF");

        sh = search ("Personal", "Master.INF", NewPI.PersonNumber,MasterFileRecordLength);

        if (NewPI.PersonNumber > sh.Value)
                sh.Index++;

        flag &= insert_record ("Personal", "Master.INF", "MBack.INF", sh.Index, (BYTE*)&NewPI, MasterFileRecordLength);	

        if (!is_file_exist("Personal","Finger.INF"))
                flag &= create_file("Personal", "Finger.INF");
        
        if (!is_file_exist("Personal","Card.INF"))
                flag &= create_file("Personal", "Card.INF");
        
        if (NewPI.FingerID != 0)
        {
        
                sh = search ("Personal", "Finger.INF", NewPI.FingerID, FingerFileRecordLength);
        
                if (NewPI.FingerID > sh.Value)
                        sh.Index++;
        
                IDPerson idp;
                
                idp.PersonNumber = NewPI.PersonNumber;
                idp.ID = NewPI.FingerID;
                
                flag &= insert_record ("Personal", "Finger.INF", "FBack.INF", sh.Index, (BYTE*) &idp, FingerFileRecordLength);	
        }
                
        if (NewPI.CardID != 0)
        {
                
                sh = search ("Personal", "Card.INF", NewPI.CardID, CardFileRecordLength);
        
                if (NewPI.CardID > sh.Value)
                        sh.Index++;
                
                IDPerson idp;
                
                idp.PersonNumber = NewPI.PersonNumber;
                idp.ID = NewPI.CardID;
                
                flag &= insert_record ("Personal", "Card.INF", "CBack.INF", sh.Index, (BYTE*) &idp, CardFileRecordLength);	
        }
        
        return flag;
}

bool Personal::DeleteUser (DWORD PersonNumber)
{
        DWORD pn;
        PersonInformation pi;
        SearchHit sh;
        bool flag = true;

        pn = PersonNumber;
        
        pi = get_person_information (pn);

        sh = search ("Personal", "Master.INF", pn ,MasterFileRecordLength);
        
        flag &= delete_record ("Personal", "Master.INF", "MBack.INF", sh.Index, MasterFileRecordLength);	
        
       /* if (pi.FingerID != 0)
        {
                sh = search ("Personal", "Finger.INF", pi.FingerID, FingerFileRecordLength);
        
                flag &= delete_record ("Personal", "Finger.INF", "FBack.INF", sh.Index , FingerFileRecordLength);		
                
                f.DeleteTemplates(pi.FingerID);
        }*/
                
        if (pi.CardID != 0)
        {				
                sh = search ("Personal", "Card.INF", pi.CardID, CardFileRecordLength);
        
                flag &= delete_record ("Personal", "Card.INF", "CBack.INF", sh.Index , CardFileRecordLength);						
        }

        return flag;
}

bool Personal::DeleteAllUser (void)
{
        int err;
        Finger f;

        f.DeleteAllTemplates();
        
        err = FS_Remove ("Personal\\Card.INF");
        err += FS_Remove ("Personal\\Finger.INF");
        err += FS_Remove ("Personal\\Master.INF");
        
        if(err == 0)
            return true;
        
        return false;
}	

