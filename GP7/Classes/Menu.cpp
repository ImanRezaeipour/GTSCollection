#include	"MENU.H"
#include	"MIFARE.H"
#include	"FINGER.H"
#include	"KEY.H"
#include	"MY_USB.H"
#include	"NETWORK.H"
#include	"SHOWTIME.H"
#include	"LCD.H"
#include	"BACKLIGHT.H"
#include	"DATETIME.H"
#include	"UTILITY.H"
#include 	"SETTING.H"
#include	"DIGITALDATA.H"
#include	"MifareEx.H"

OS_TIMER Menu::timer_menu;
OS_TIMER Menu::timer_message;
OS_TIMER Menu::timer_second;
OS_TIMER Menu::timer_special;

int Menu::is_f1_pressed;
bool Menu::is_OK = true;

EventPointer Menu::ep_exit_menu;

enum MenuIndex Menu::menu_index;
enum MenuIndex Menu::highlighted_menu;

int Menu::sub_menu_index;
int Menu::sub_menu_temp_index;

PersonInformation Menu::pi;
Buffer Menu::keypad_data;

DWORD Menu::last_card_id;

OS_STACKPTR int Menu::stack_Modem[256];
OS_TASK Menu::tcb_Modem;
CommunicationPacket Menu::receive_packet;

Menu::Menu ()
{

}

Menu::~Menu ()
{
	
}

void Menu::Show (void)
{
	Mifare m;
        MifareEx mx;
	Finger f;
	Key k;

	ShowTime st;
	LCD lcd;
	BackLight b(FM_OnOffOn, S_VeryFast);

	Setting set;
	Personal p;
	
	m.DisableEvents();
        mx.DisableEvents();
	f.DisableEvents();
	k.DisableEvents();
	
	set.DisableEvents();
	p.DisableEvents();
	
	f.SetEnrollSuccessEvent (enroll_success);
	f.SetIdentifySuccessEvent (idenfify_success);
	f.SetIdentifyFailEvent (identify_fail);
	f.SetFirstTemplateScanSuccessEvent (first_template_scan_success);
	f.SetEnrollFailEvent (enroll_fail);
        f.SetEnrollRepetitiveEvent(finger_is_repetitive);
	f.SetDeleteTemplateSuccessEvent(delete_template_success);
	f.SetDeleteTemplateFailEvent(delete_template_fail);
	f.SetDeleteAllTemplateSuccessEvent(delete_all_templates_success);
	f.SetDeleteAllTemplateFailEvent(delete_all_templates_fail);
        f.SetCancelSuccessEvent(cancel_success);
        f.SetErrRecieveFinger(Err_Recieve_Finger);
		
	m.SetCardPresentEvent(card_present);
        mx.SetCardPresentEvent(card_present_ex);

	k.SetAnyKeyEvent(key_process);
	k.SetOKKeyEvent(command_process);
	k.SetESCKeyEvent(command_cancel);
	
	p.SetPersonExistEvent(person_is_exist);
	p.SetPersonIsNotExistEvent(person_is_not_exist);
	p.SetPersonAddSuccessEvent(person_add_success);
   	p.SetPersonAddFailEvent(person_add_fail);
	p.SetPersonEditSuccessEvent(person_edit_success);
   	p.SetPersonEditFailEvent(person_edit_fail);
	p.SetPersonInformationRequestEvent(person_information_receive);
	p.SetPersonDeleteSuccessEvent(delete_person_success);
	p.SetPersonDeleteFailEvent(delete_person_fail);
	p.SetAllPersonDeleteSuccessEvent(delete_all_person_success);
	p.SetAllPersonDeleteFailEvent(delete_all_person_fail);
	p.SetValidAdministratorFingerIDEvent(valid_admin_finger);
	p.SetDeleteRecordsEvent(delete_records_success);
	p.SetRepetitiveCardIDEvent(card_is_repetitive);
        p.SetUniqueCardIDEvent(card_is_unique);
        p.SetUniqueCardIDEventEx(card_is_unique_ex);

	set.SetCommandExecuteSuccessEvent(command_execute_successfully);
	set.SetCommandExecuteFailEvent(command_execute_fail);

	st.UpdateScreenPermission(false);
	
	initial_members();	
	menu_index = MI_WaitForPassword;

	Key::ClearKeyBuffer();
	
	OS_RetriggerTimer(&timer_menu);
	
	show_context();
}

void Menu::SetExitMenuEvent (EventPointer EP)
{
	ep_exit_menu = EP;
}

void Menu::DisableEvents (void)
{
	ep_exit_menu = NULL;	
}
void Menu::StopTimers (void)
{
	OS_StopTimer(&timer_menu);
	OS_StopTimer(&timer_message);
	OS_StopTimer(&timer_second);
	OS_StopTimer(&timer_special);
}

void Menu::initial_members (void)
{
	menu_index = MI_None;
	
	highlighted_menu = MI_None;
	
	keypad_data.Clear();
	
	pi.PersonNumber = 0;
	pi.Password = 0;
	pi.FingerID = 0;
	pi.CardID = 0;
	pi.PIS = PIS_CardOrFinger;
	pi.PAR = PAR_User;
	pi.FingerTemplateNumber = 0;
}

void Menu::initial_modem_task  (void)                                  //d.k
{
        COM     com;
        bool    bln;
        
        COM::Modem_Packet = MDM_IS_Packet;
        
        bln = Send_ATcommand_Modem(AT_CMD_AT);
        bln &= Send_ATcommand_Modem(AT_CMD_F0);
        bln &= Send_ATcommand_Modem(AT_CMD_Z0);
        bln &= Send_ATcommand_Modem(AT_CMD_E0);
        bln &= Send_ATcommand_Modem(AT_CMD_S0);
        bln &= Send_ATcommand_Modem(AT_CMD_D0);
        bln &= Send_ATcommand_Modem(AT_CMD_L3);
        bln &= Send_ATcommand_Modem(AT_CMD_W0);
        
        if(bln)
          command_execute_successfully();
        else    
          command_execute_fail(); 
        
        COM::Modem_Packet = MDM_None;
        OS_Terminate(0);
}

bool Menu::Send_ATcommand_Modem (Modem_ATcommand  AT_CMD)             //d.k
{
        COM com;
        string AT_String;
  
        AT_String = com.GetStrATcommand(AT_CMD);
        
        com.Send(AT_String);
        OS_Delay(250);
        if(RecievePacket_modem_true(AT_String))
          return true;
        else
          return false;
}

bool Menu::RecievePacket_modem_true  (string AT_string)                //d.k
{
  int  i;
  COM com;
  string str_ok = "\r\nOK\r\n";
  
  if(COM::Modem_Packet == MDM_Recived)
  {
    receive_packet = com.GetReceivedData();
    com.ClearReceiveBuffer();
    
    if(receive_packet.Size() == str_ok.size())
    {
     for(i=0 ; i<receive_packet.Size() ; i++)
     {
       if(receive_packet[i] != str_ok[i])
         return false;
     }
      return true;
    }
    else
    {
     AT_string += str_ok;
     for(i=0 ; i<receive_packet.Size() ; i++)
     {
       if(receive_packet[i] != AT_string[i])
         return false;
     }
     return true;
    }
  }
    com.ClearReceiveBuffer();
    return false;
}

void Menu::create_timers (void)
{
	OS_CREATETIMER(&timer_menu, timer_elapse_menu, 120000);	
	OS_CREATETIMER(&timer_message, timer_elapse_message, 3000);	
	OS_CREATETIMER(&timer_second, timer_elapse_second, 1000);	
	OS_CREATETIMER(&timer_special, timer_elapse_special, 4000);	
}

void Menu::timer_elapse_menu (void)
{
	Finger f;
	
	if (menu_index == MI_None)
		return;

        OS_StopTimer(&timer_menu);
	OS_StopTimer(&timer_message);
	OS_StopTimer(&timer_second);
	OS_StopTimer(&timer_special);
        
        if (menu_index == MI_AddPerson || menu_index == MI_EditPerson)            
	{
                OS_RetriggerTimer(&timer_menu);
		return;
	}
	
	safe_call_handler(ep_exit_menu);	
}

void Menu::timer_elapse_second (void)
{
	LCD lcd(Farsi);
	DateTime d;

	if (menu_index == MI_None)
		return;

        switch (menu_index)
	{
	case MI_SetTime:
		lcd.Clear(2);
		lcd.Write(A_Center, 2, d.GetTime());
                OS_RetriggerTimer(&timer_second);
		break;
	}	
	
	
}

void Menu::timer_elapse_special (void)
{
	string str;
	LCD lcd(Farsi);
	
	if (menu_index == MI_None)
		return;

        if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
		{
                        //last_card_id = 0;
			
                        lcd.Clear (2);

                        lcd.Write(A_Right, 2, get_appropriate_message(FM_Card, Farsi));
                        lcd.Write(A_Left, 2, Utility::ToString(pi.CardID));
		
                        lcd.Clear(4);
			str = "";
				
			str += get_appropriate_message(FM_Finger, Farsi);
		
			str += ")";
			string st = Utility::ToString (pi.FingerTemplateNumber+1);
                        if(st.length() == 2)
                        {
                          char s;
                           s = st[1];
                           st[1] = st[0];
                           st[0] = s;
                        }
                        str += st;
			str += "(";
			str += ":";
	
			lcd.Write(A_Right, 4, str);
		}
	
}

void Menu::timer_elapse_message (void)
{
	LCD lcd(Farsi);
	Key k;

        if (menu_index == MI_None)
		return;

	k.SetAnyKeyEvent(key_process);
	k.SetOKKeyEvent(command_process);
	k.SetESCKeyEvent(command_cancel);
	
	init_members_for_next_command();
	
	if (menu_index == MI_WaitForPassword)
		show_context();
	
	else
	{		
		menu_index = get_menu_characteristic(menu_index).Parent;
		highlighted_menu = get_menu_characteristic(menu_index).FirstChild;
		menu_handler();		
	}
}

void Menu::show_message (FixMessage index, Language l)
{
	string message;
	LCD lcd;
	BackLight b(FM_OnOffOn, S_VeryFast);
	Key k;

        k.DisableEvents();
	
	//b.DoFade();
	lcd.Clear();
	
	message.clear();
	
	message = get_appropriate_message(index, l);
	
	lcd.Write(message);			
			
	OS_RetriggerTimer(&timer_message);
}

void Menu::key_process (void)
{
	string str;
	LCD lcd(Farsi);
       // Setting s;
	
	if (menu_index == MI_WaitForPassword)
	{		
		if (Key::GetLastKey().Function == KF_Arrow)
			if (Key::GetLastKey().Status == KS_Left)
				if (keypad_data.Size() >0)
					keypad_data.DeleteLastCell();
			
		if (keypad_data.Size() == 6)
			return;
		
		str.clear();
		lcd.Clear(4);
			
		if ((Key::GetLastKey().Function == KF_Number))
			keypad_data += (BYTE) Key::GetLastKey().Num;	

		for (int ctr=0 ; ctr<keypad_data.Size() ; ctr++)
			str+= "*";
		
		lcd.Write(A_Center, 4, str);

	}
		
	else if (highlighted_menu != MI_None)
	{
		if (Key::GetLastKey().Function == KF_Arrow)
		{	
			switch (Key::GetLastKey().Status)
			{
			case KS_Up:
				if (get_menu_characteristic(highlighted_menu).PreviousMenu != MI_None)
					highlighted_menu = get_menu_characteristic(highlighted_menu).PreviousMenu;						
				break;
				
			case KS_Down:
				if (get_menu_characteristic(highlighted_menu).NextMenu != MI_None)
					highlighted_menu = get_menu_characteristic(highlighted_menu).NextMenu;						
				break;
			}

			draw_menu();
		}
	}
	else
	{
		switch (menu_index)
		{
		case MI_SetDate:
			if (Key::GetLastKey().Function == KF_Arrow)
				if (Key::GetLastKey().Status == KS_Left)
					if (keypad_data.Size() >0)
						keypad_data.DeleteLastCell();
		
			if (keypad_data.Size() == 8)
				break;
			
			if ((Key::GetLastKey().Function == KF_Number))
			{

				keypad_data += (BYTE) Key::GetLastKey().Num;				
				
				if (keypad_data.Size() == 4)
					if (keypad_data.GetValue(0,3) < 1389)
					{
						keypad_data.Fill(1, 0);
						keypad_data.Fill(3, 1);
						keypad_data.Fill(8, 2);
						keypad_data.Fill(9, 3);
					}
				
				if (keypad_data.Size() == 6)
					if (keypad_data.GetValue(4,5) > 12)
					{
						keypad_data.Fill(1, 4);
						keypad_data.Fill(2, 5);
					}
				
				if (keypad_data.Size() == 8)
				{
					if (keypad_data.GetValue(4,5) <= 6)
						if (keypad_data.GetValue(6,7) > 31)
						{
							keypad_data.Fill(3, 6);
							keypad_data.Fill(1, 7);
						}
	
					if (keypad_data.GetValue(4,5) > 6 && keypad_data.GetValue(4,5) < 12)
						if (keypad_data.GetValue(6,7) > 30)
						{
							keypad_data.Fill(3, 6);
							keypad_data.Fill((BYTE) 0, 7);
						}
					
					if (keypad_data.GetValue(4,5) == 12)
						if (keypad_data.GetValue(6,7) > 29)
						{
							keypad_data.Fill(2, 6);
							keypad_data.Fill(9, 7);
						}
				}
			}
									
			str.clear();
			lcd.Clear(6);
		
			for (int ctr=0 ; ctr<4 ; ctr++)
				if (ctr < keypad_data.Size())
					str+=Utility::ToChar(keypad_data[ctr]);
				else
					str += '-';
				
			str += '/';
			
			for (int ctr=4 ; ctr<6 ; ctr++)
				if (ctr < keypad_data.Size())
					str+= Utility::ToChar(keypad_data[ctr]);
				else
					str += '-';
				
			str += '/';
				
			for (int ctr=6 ; ctr<8 ; ctr++)
				if (ctr < keypad_data.Size())
					str+= Utility::ToChar(keypad_data[ctr]);
				else
					str += '-';
		
			lcd.Write(A_Center, 6, str);
	
			break;
				
		case MI_SetTime:
			if (Key::GetLastKey().Function == KF_Arrow)
				if (Key::GetLastKey().Status == KS_Left)
					if (keypad_data.Size() >0)
						keypad_data.DeleteLastCell();
			
			if (keypad_data.Size() == 6)
				break;
			
			if ((Key::GetLastKey().Function == KF_Number))
			{
				keypad_data += (BYTE) Key::GetLastKey().Num;				
				
				if (keypad_data.Size() == 2)
					if (keypad_data.GetValue(0,1) > 23)
					{
						keypad_data.Fill(2, 0);
						keypad_data.Fill(3, 1);
					}
				
				if (keypad_data.Size() == 4)
					if (keypad_data.GetValue(2,3) > 59)
					{
						keypad_data.Fill(5, 2);
						keypad_data.Fill(9, 3);
					}
				
				if (keypad_data.Size() == 6)
					if (keypad_data.GetValue(4,5) > 59)
					{
						keypad_data.Fill(5, 4);
						keypad_data.Fill(9, 5);
					}
			}
								
			str.clear();
			lcd.Clear(6);
			
			for (int ctr=0 ; ctr<2 ; ctr++)
				if (ctr < keypad_data.Size())
					str+=Utility::ToChar(keypad_data[ctr]);
				else
					str += '-';
				
				str += ':';
			
			for (int ctr=2 ; ctr<4 ; ctr++)
				if (ctr < keypad_data.Size())
					str+= Utility::ToChar(keypad_data[ctr]);
				else
					str += '-';
			
				str += ':';
			
			for (int ctr=4 ; ctr<6 ; ctr++)
				if (ctr < keypad_data.Size())
					str+= Utility::ToChar(keypad_data[ctr]);
				else
					str += '-';
	
			lcd.Write(A_Center, 6, str);

		break;			

		case MI_ChangePassword:
			if (Key::GetLastKey().Function == KF_Arrow)
				if (Key::GetLastKey().Status == KS_Left)
					if (keypad_data.Size() >0)
						keypad_data.DeleteLastCell();
			
			if (keypad_data.Size() == 6)
				break;
			
			str.clear();
			lcd.Clear(4);
			
			if ((Key::GetLastKey().Function == KF_Number))
				keypad_data += (BYTE) Key::GetLastKey().Num;	

			for (int ctr=0 ; ctr<keypad_data.Size() ; ctr++)
				str+= Utility::ToChar(keypad_data[ctr]);
			
			lcd.Write(A_Center, 4, str);

			break;

                case MI_SetSubnetMask:			
		case MI_SetIP:
                case MI_SetServerIP:
                case MI_SetGateway:
			if (Key::GetLastKey().Function == KF_Arrow)
				if (Key::GetLastKey().Status == KS_Left)
					if (keypad_data.Size() >0)
						keypad_data.DeleteLastCell();
		
			if (keypad_data.Size() == 12)
				break;
			
			if ((Key::GetLastKey().Function == KF_Number))
			{
				keypad_data += (BYTE) Key::GetLastKey().Num;	
				
				if (keypad_data.Size() % 3 == 0 && keypad_data.Size() != 0)
					if (keypad_data.GetValue(keypad_data.Size()-3,keypad_data.Size()-1) > 255)
					{
						keypad_data.Fill(2, keypad_data.Size()-3);
						keypad_data.Fill(5, keypad_data.Size()-2);
						keypad_data.Fill(5, keypad_data.Size()-1);
					}
			}
									
			str.clear();
			lcd.Clear(6);
		
			for (int ctr=0 ; ctr<12 ; ctr++)
			{
				if (ctr < keypad_data.Size())
					str+=Utility::ToChar(keypad_data[ctr]);
				else
					str += '-';
				
				if (ctr!=0 && ctr!= 11 && (ctr+1)%3 == 0 )
					str += " . ";
			}
			
			lcd.Write(A_Center, 6, str);
	
			break;
		
               // case MI_SetDeviceID:
		case MI_SetPort:
			if (Key::GetLastKey().Function == KF_Arrow)
				if (Key::GetLastKey().Status == KS_Left)
					if (keypad_data.Size() >0)
						keypad_data.DeleteLastCell();
		
			if (keypad_data.Size() == 5)
				break;
			
			if ((Key::GetLastKey().Function == KF_Number))
			{
				keypad_data += (BYTE) Key::GetLastKey().Num;				
			
				if (keypad_data.Size() == 5)
					if (keypad_data.GetValue() > 65530)
					{
						keypad_data.Fill(6, 0);
						keypad_data.Fill(5, 1);
						keypad_data.Fill(5, 2);
						keypad_data.Fill(3, 3);
						keypad_data.Fill((BYTE)0, 4);						
					}			
			}
									
			str.clear();
			lcd.Clear(6);
		
			for (int ctr=0 ; ctr<5 ; ctr++)
			{
				if (ctr < keypad_data.Size())
					str+=Utility::ToChar(keypad_data[ctr]);
				else
					str += '-';				
			}
			
			lcd.Write(A_Center, 6, str);
	
		break;
			
		case MI_AddPerson:
		case MI_EditPerson:                     
			if (sub_menu_index == 0)
			{
				if (Key::GetLastKey().Function == KF_Arrow)
					if (Key::GetLastKey().Status == KS_Left)
						if (keypad_data.Size() >0)
							keypad_data.DeleteLastCell();
		
				if (keypad_data.Size() == 9)
					break;
			
				if ((Key::GetLastKey().Function == KF_Number))
				{
					keypad_data += (BYTE) Key::GetLastKey().Num;				
					
					if (keypad_data.Size() == 10)
						if (keypad_data.GetValue() > 4000000000)
						{
							keypad_data.Fill(4, 0);
							keypad_data.Fill((BYTE)0, 1);
							keypad_data.Fill((BYTE)0, 2);
							keypad_data.Fill((BYTE)0, 3);
							keypad_data.Fill((BYTE)0, 4);	
							keypad_data.Fill((BYTE)0, 5);
							keypad_data.Fill((BYTE)0, 6);
							keypad_data.Fill((BYTE)0, 7);
							keypad_data.Fill((BYTE)0, 8);
							keypad_data.Fill((BYTE)0, 9);							
						}
				}
									
				str.clear();
				lcd.Clear(2);
			
				for (int ctr=0 ; ctr < keypad_data.Size() ; ctr++)
						str+=Utility::ToChar(keypad_data[ctr]);
				
				lcd.Write(A_Center, 2, str);
			}
			
			if (sub_menu_index == 1)
			{
				if (Key::GetLastKey().Function == KF_Arrow)
					if (Key::GetLastKey().Status == KS_Down)
						if (sub_menu_temp_index != 1)
							sub_menu_temp_index ++;
							
				if (Key::GetLastKey().Function == KF_Arrow)
					if (Key::GetLastKey().Status == KS_Up)
						if (sub_menu_temp_index != 0)
							sub_menu_temp_index --;
							
				lcd.Clear(2);
				lcd.Clear(4);
				
				if (sub_menu_temp_index == 0)
					lcd.SetPixelFormat(PF_Invert);
				else
					lcd.SetPixelFormat(PF_Normal);
					
				lcd.Write(A_Center, 2, get_appropriate_message(FM_User, Farsi));

				if (sub_menu_temp_index == 1)
					lcd.SetPixelFormat(PF_Invert);
				else
					lcd.SetPixelFormat(PF_Normal);

				lcd.Write(A_Center, 4, get_appropriate_message(FM_Administrator, Farsi));
			
			}
			
			if (sub_menu_index == 2)
			{
				if (Key::GetLastKey().Function == KF_Arrow)
					if (Key::GetLastKey().Status == KS_Down)
						if (sub_menu_temp_index != 2)
							sub_menu_temp_index ++;
							
				if (Key::GetLastKey().Function == KF_Arrow)
					if (Key::GetLastKey().Status == KS_Up)
						if (sub_menu_temp_index != 0)
							sub_menu_temp_index --;
							
				lcd.Clear(2);
				lcd.Clear(4);
				lcd.Clear(6);
				
				if (sub_menu_temp_index == 0)
					lcd.SetPixelFormat(PF_Invert);
				else
					lcd.SetPixelFormat(PF_Normal);
					
				lcd.Write(A_Center, 2, get_appropriate_message(FM_DeviceMode, Farsi));

				if (sub_menu_temp_index == 1)
					lcd.SetPixelFormat(PF_Invert);
				else
					lcd.SetPixelFormat(PF_Normal);

				lcd.Write(A_Center, 4, get_appropriate_message(FM_CardOrFinger, Farsi));
			
				if (sub_menu_temp_index == 2)
					lcd.SetPixelFormat(PF_Invert);
				else
					lcd.SetPixelFormat(PF_Normal);

				lcd.Write(A_Center, 6, get_appropriate_message(FM_CardANDFinger, Farsi));
			}
			
			if (sub_menu_index == 3)
			{
                                if (menu_index == MI_EditPerson && Key::GetLastKey().Function == KF_Function && Key::GetLastKey().Num == 1)
		                {
			                 Finger f;   
                                         if (is_f1_pressed==1) 
                                           return;
                                             
                                         is_f1_pressed=1; 
                                         //f.DeleteTemplates(pi.FingerID);
                                         is_OK = false;
                                         f.CancelCommand();                                       
			                  return;
		                }
                                
				if (Key::GetLastKey().Function == KF_Arrow)
					if (Key::GetLastKey().Status == KS_Left)
						if (keypad_data.Size() >0)
							keypad_data.DeleteLastCell();
		
				if (keypad_data.Size() == 5)
					break;
			
				if ((Key::GetLastKey().Function == KF_Number))
				{
					if (keypad_data.Size() == 0)
						if (Key::GetLastKey().Num == 0)
							break;
					
					keypad_data += (BYTE) Key::GetLastKey().Num;				
				}
					
				str.clear();
				lcd.Clear(0);
			
				for (int ctr=0 ; ctr < keypad_data.Size() ; ctr++)
						str+=Utility::ToChar(keypad_data[ctr]);
				
				lcd.Write(A_Right, 0, get_appropriate_message(FM_Password, Farsi));
				lcd.Write(A_Left, 0, str);
			}
		
		break;	
			
		case MI_DeletePerson:
			if (sub_menu_index == 0)
			{
				if (Key::GetLastKey().Function == KF_Arrow)
					if (Key::GetLastKey().Status == KS_Left)
						if (keypad_data.Size() >0)
							keypad_data.DeleteLastCell();
		
				if (keypad_data.Size() == 9)
					break;
			
				if ((Key::GetLastKey().Function == KF_Number))
				{
					keypad_data += (BYTE) Key::GetLastKey().Num;				
					
					if (keypad_data.Size() == 10)
						if (keypad_data.GetValue() > 4000000000)
						{
							keypad_data.Fill(4, 0);
							keypad_data.Fill((BYTE)0, 1);
							keypad_data.Fill((BYTE)0, 2);
							keypad_data.Fill((BYTE)0, 3);
							keypad_data.Fill((BYTE)0, 4);	
							keypad_data.Fill((BYTE)0, 5);
							keypad_data.Fill((BYTE)0, 6);
							keypad_data.Fill((BYTE)0, 7);
							keypad_data.Fill((BYTE)0, 8);
							keypad_data.Fill((BYTE)0, 9);							
						}
				}
									
				str.clear();
				lcd.Clear(2);
			
				for (int ctr=0 ; ctr < keypad_data.Size() ; ctr++)
						str+=Utility::ToChar(keypad_data[ctr]);
				
				lcd.Write(A_Center, 2, str);
			}			
		break;
		
		case MI_DeviceMode:
			if (Key::GetLastKey().Function == KF_Arrow)
				if (Key::GetLastKey().Status == KS_Down)
					if (sub_menu_temp_index != 1)
						sub_menu_temp_index ++;
						
			if (Key::GetLastKey().Function == KF_Arrow)
				if (Key::GetLastKey().Status == KS_Up)
					if (sub_menu_temp_index != 0)
						sub_menu_temp_index --;
						
			lcd.Clear(2);
			lcd.Clear(4);
			if (sub_menu_temp_index == 0)
                        {       
                               // s.SetFingerFreeScanActivity(A_Enable);
				lcd.SetPixelFormat(PF_Invert);
                        }
			else
                        {
                               // s.SetFingerFreeScanActivity(A_Disable);
				lcd.SetPixelFormat(PF_Normal);
                        }
				
			lcd.Write(A_Center, 2, get_appropriate_message(FM_CardOrFinger, Farsi));
				
			if (sub_menu_temp_index == 1)
				lcd.SetPixelFormat(PF_Invert);
			else
				lcd.SetPixelFormat(PF_Normal);
			
			lcd.Write(A_Center, 4, get_appropriate_message(FM_CardANDFinger, Farsi));		
			
			break;
			
		case MI_FMActivity:
			if (Key::GetLastKey().Function == KF_Arrow)
				if (Key::GetLastKey().Status == KS_Down)
					if (sub_menu_temp_index != 1)
						sub_menu_temp_index ++;
						
			if (Key::GetLastKey().Function == KF_Arrow)
				if (Key::GetLastKey().Status == KS_Up)
					if (sub_menu_temp_index != 0)
						sub_menu_temp_index --;
						
			lcd.Clear(2);
			lcd.Clear(4);
			
			if (sub_menu_temp_index == 0)
				lcd.SetPixelFormat(PF_Invert);
			else
				lcd.SetPixelFormat(PF_Normal);
				
			lcd.Write(A_Center, 2, get_appropriate_message(FM_Disable, Farsi));
				
			if (sub_menu_temp_index == 1)
				lcd.SetPixelFormat(PF_Invert);
			else
				lcd.SetPixelFormat(PF_Normal);
			
			lcd.Write(A_Center, 4, get_appropriate_message(FM_Enable, Farsi));		
			
			break;			
			
		case MI_DevicePreCard:
			if (Key::GetLastKey().Function == KF_Arrow)
				if (Key::GetLastKey().Status == KS_Down)
					if (sub_menu_temp_index != 2)
						sub_menu_temp_index ++;
						
			if (Key::GetLastKey().Function == KF_Arrow)
				if (Key::GetLastKey().Status == KS_Up)
					if (sub_menu_temp_index != 0)
						sub_menu_temp_index --;
						
			lcd.Clear(2);
			lcd.Clear(4);
			lcd.Clear(6);			
			
			if (sub_menu_temp_index == 0)
				lcd.SetPixelFormat(PF_Invert);
			else
				lcd.SetPixelFormat(PF_Normal);
				
			lcd.Write(A_Center, 2, get_appropriate_message(FM_Normal, Farsi));
				
			if (sub_menu_temp_index == 1)
				lcd.SetPixelFormat(PF_Invert);
			else
				lcd.SetPixelFormat(PF_Normal);
			
			lcd.Write(A_Center, 4, get_appropriate_message(FM_Enter, Farsi));		
			
			if (sub_menu_temp_index == 2)
				lcd.SetPixelFormat(PF_Invert);
			else
				lcd.SetPixelFormat(PF_Normal);
			
			lcd.Write(A_Center, 6, get_appropriate_message(FM_Exit, Farsi));		
			lcd.SetPixelFormat(PF_Normal);
			
			break;			
			
		}
	}	
	
	Key::ClearKeyBuffer();
	OS_RetriggerTimer(&timer_menu);
}

void Menu::command_process (void)
{
	BackLight b(FM_OnOffOn, S_VeryFast);
	LCD lcd(Farsi);
	DateTime d;	
	RTCTime t;
	Setting set;
	DigitalData dd;
	Finger f;
	//Mifare m;
	Personal p;
	
	
	if (menu_index == MI_WaitForPassword)
	{
		if (keypad_data.GetValue() == set.GetSettings().Password)
		{
			//b.DoFade();
			menu_index = MI_MainMenu;
			highlighted_menu = get_menu_characteristic(menu_index).FirstChild;
			menu_handler();
			f.FreeScanMode(Off);
		}
		else
		{			
			show_message(FM_IncorrectPassword, Farsi);
		}
	}
	
	else if (get_menu_characteristic(menu_index).FirstChild == MI_None)
	{
		switch (menu_index)
		{
		case MI_SetDate:
			if (keypad_data.Size() <8)
				return;
			
			t.RTC_Year = keypad_data[0] *1000 + keypad_data[1] *100 + keypad_data[2] *10 + keypad_data[3];
			t.RTC_Mon = keypad_data[4] *10 + keypad_data[5];
			t.RTC_Mday = keypad_data[6] *10 + keypad_data[7];
			t = DateTime::ConvertToMiladi(t);
			
			set.SetDate(t);
                        command_execute_successfully();			
			break;

		case MI_SetTime:
			if (keypad_data.Size() <6)
				return;

			t.RTC_Hour = keypad_data[0] *10 + keypad_data[1];
			t.RTC_Min = keypad_data[2] *10 + keypad_data[3];
			t.RTC_Sec = keypad_data[4] *10 + keypad_data[5];
			
                        OS_StopTimer(&timer_second);
                        set.SetTime(t);	
                        command_execute_successfully();
			break;
			
		case MI_ChangePassword:
			if (keypad_data.Size() <1)
				return;

			set.SetPassword((DWORD)keypad_data.GetValue());	
			set.SaveSettings();
			break;

                case MI_SetDeviceID:
			//if (keypad_data.Size() <5)
				//return;
			
			//set.SetDeviceID(keypad_data[0]*10000 + keypad_data[1]*1000 + keypad_data[2]*100 + keypad_data[3]*10 + keypad_data[4]);		
			//set.SaveSettings();
			break;	

                case MI_SetFactorySettings:
			set.LoadFactorySettings();
			set.SaveSettings();
			break;	
			
		case MI_SetIP:
			if (keypad_data.Size() <12)
				return;
			
			dd.SetByte(3,keypad_data[0]*100+keypad_data[1]*10+keypad_data[2]);
			dd.SetByte(2,keypad_data[3]*100+keypad_data[4]*10+keypad_data[5]);
			dd.SetByte(1,keypad_data[6]*100+keypad_data[7]*10+keypad_data[8]);
			dd.SetByte(0,keypad_data[9]*100+keypad_data[10]*10+keypad_data[11]);
			
			set.SetDeviceIP(dd.GetDword());		
			set.SaveSettings();
			break;	
                        
                case MI_SetServerIP:                                     //dk
			if (keypad_data.Size() <12)
				return;
			
			dd.SetByte(3,keypad_data[0]*100+keypad_data[1]*10+keypad_data[2]);
			dd.SetByte(2,keypad_data[3]*100+keypad_data[4]*10+keypad_data[5]);
			dd.SetByte(1,keypad_data[6]*100+keypad_data[7]*10+keypad_data[8]);
			dd.SetByte(0,keypad_data[9]*100+keypad_data[10]*10+keypad_data[11]);
			
			set.SetServerIP(dd.GetDword());		
			set.SaveSettings();
			break;  
                        
                 case MI_SetGateway:                                     //dk
			if (keypad_data.Size() <12)
				return;
			
			dd.SetByte(3,keypad_data[0]*100+keypad_data[1]*10+keypad_data[2]);
			dd.SetByte(2,keypad_data[3]*100+keypad_data[4]*10+keypad_data[5]);
			dd.SetByte(1,keypad_data[6]*100+keypad_data[7]*10+keypad_data[8]);
			dd.SetByte(0,keypad_data[9]*100+keypad_data[10]*10+keypad_data[11]);
			
			set.SetGatewaye(dd.GetDword());		
			set.SaveSettings();
			break;        
			
		case MI_SetSubnetMask:
			if (keypad_data.Size() <12)
				return;
			
			dd.SetByte(3,keypad_data[0]*100+keypad_data[1]*10+keypad_data[2]);
			dd.SetByte(2,keypad_data[3]*100+keypad_data[4]*10+keypad_data[5]);
			dd.SetByte(1,keypad_data[6]*100+keypad_data[7]*10+keypad_data[8]);
			dd.SetByte(0,keypad_data[9]*100+keypad_data[10]*10+keypad_data[11]);
			
			set.SetAddressMask(dd.GetDword());		
			set.SaveSettings();
			break;	

                case MI_SetPort:
			if (keypad_data.Size() <5)
				return;
			
			set.SetDevicePort(keypad_data[0]*10000 + keypad_data[1]*1000 + keypad_data[2]*100 + keypad_data[3]*10 + keypad_data[4]);		
			set.SaveSettings();
			break;	
			
                case MI_InitModem:                      //d.k
			//initial_modem();
                        OS_CREATETASK(&tcb_Modem, "Modem_Task", initial_modem_task, TP_LowestPeriority, stack_Modem);
			break;	        
                        
		case MI_AddPerson:
		case MI_EditPerson:
			
			if (sub_menu_index == 0)
			{
				if (keypad_data.Size() == 0)
					break;				

				p.PersonIsExist((DWORD) keypad_data.GetValue());
			}
			
			else if (sub_menu_index == 1)
			{
				if (sub_menu_temp_index == 0)
					pi.PAR = PAR_User;
				
				if (sub_menu_temp_index == 1)
					pi.PAR = PAR_Administrator;
				
				init_members_for_next_command();
								
				sub_menu_index = 2;
				
				show_context();
			}
			
			else if (sub_menu_index == 2)
			{
				if (sub_menu_temp_index == 0)
					pi.PIS = PIS_DeviceMode;
				
				if (sub_menu_temp_index == 1)
					pi.PIS = PIS_CardOrFinger;
				
				if (sub_menu_temp_index == 2)
					pi.PIS = PIS_CardAndFinger;

				init_members_for_next_command();
								
				sub_menu_index = 3;
                                
                               	show_context();
				
                                OS_RetriggerTimer(&timer_special);
	
		                OS_RetriggerTimer(&timer_menu);
		
                                if(menu_index == MI_AddPerson )
                                {
                                        if (pi.FingerTemplateNumber == 0 )
                                                f.EnrollByScan ();
                                        else
                                                f.AddTemplate(pi.FingerID);
                                }
		
				//f.FreeScanMode(On);
			}
			
			else if (menu_index == MI_AddPerson && sub_menu_index == 3)
			{
				pi.Password = (DWORD) keypad_data.GetValue();
				
				p.AddPerson(pi);
                                
                                is_OK = true;
                                f.CancelCommand(); 
			}
			
			else if (menu_index == MI_EditPerson && sub_menu_index == 3)
			{
				pi.Password = (DWORD) keypad_data.GetValue();
			
				p.EditPerson(pi);	
                                
                                is_OK = true;
                                f.CancelCommand(); 
			}	
                          
		break;	

		case MI_DeletePerson:

			if (sub_menu_index == 0)
			{
				if (keypad_data.Size() == 0)
					break;				

				p.PersonIsExist((DWORD) keypad_data.GetValue());
			}

			else if (sub_menu_index == 1)
				p.DeletePerson(pi.PersonNumber);			
		break;
		
		case MI_DeleteAllPerson:
			p.DeleteAllPerson ();
			break;
			
		case MI_DeviceMode:
			if (sub_menu_temp_index == 0)
				set.SetDeviceMode(PIS_CardOrFinger);		
				
			if (sub_menu_temp_index == 1)
				set.SetDeviceMode(PIS_CardAndFinger);		

			set.SaveSettings();
			break;	

		case MI_FMActivity:
			if (sub_menu_temp_index == 0)
			{
				set.SetFingerFreeScanActivity(A_Disable);
				set.SaveSettings();
			}						
				
			if (sub_menu_temp_index == 1)
			{
				set.SetFingerFreeScanActivity(A_Enable);		
				set.SaveSettings();
			}						
			
			break;	
			
		case MI_DevicePreCard:
			if (sub_menu_temp_index == 0)
				set.SetDevicePreCard(PC_NormalEnter);		
				
			if (sub_menu_temp_index == 1)
				set.SetDevicePreCard(PC_Enter);		

			if (sub_menu_temp_index == 2)
				set.SetDevicePreCard(PC_Exit);		

			set.SaveSettings();
			break;	

                        case MI_DeleteTraffics:
                                p.DeleteAllRecords();
                                break;
		}		
	}
	
	else if (get_menu_characteristic(highlighted_menu).FirstChild != MI_None)
	{
		menu_index = highlighted_menu;
		highlighted_menu = get_menu_characteristic(highlighted_menu).FirstChild;
		menu_handler();
	}
	
	else
	{
		menu_index = highlighted_menu;
		highlighted_menu = MI_None;
		
		sub_menu_index = 0;
		sub_menu_temp_index = 0;
		
		show_context();
	}
}

void Menu::command_cancel (void)        //Key ESC
{
	BackLight b(FM_OnOffOn, S_VeryFast);
	
	Finger f;
	
	if (get_menu_characteristic(menu_index).Parent == MI_None)
	{
		init_members_for_next_command();
		
		OS_StopTimer(&timer_menu);
		OS_StopTimer(&timer_message);
		OS_StopTimer(&timer_second);
		OS_StopTimer(&timer_special);

		safe_call_handler(ep_exit_menu);	
	}		
	
	else
	{
		//if (menu_index == MI_EditPerson && sub_menu_index == 3)
			//return;

		if ( menu_index == MI_EditPerson && sub_menu_index == 3 || menu_index == MI_AddPerson && sub_menu_index == 3)
                {
                  if (is_f1_pressed == 1)
                  {
                     is_f1_pressed = 2;
                  
                     is_OK = false;
                     f.CancelCommand();
                  }
                  else if(menu_index == MI_AddPerson)
                  {
                    is_OK = false;
                    f.CancelCommand();
                  }
                }
                
                //if (menu_index == MI_EditPerson && sub_menu_index == 3 && is_f1_pressed == false)
			//f.CancelCommand();
		//else
			//f.FreeScanMode(Off);

		menu_index = get_menu_characteristic(menu_index).Parent;
		highlighted_menu = get_menu_characteristic(menu_index).FirstChild;
	
		menu_handler();
	}
}

void Menu::command_execute_successfully (void)
{
	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
	OS_StopTimer(&timer_special);
	
	switch (menu_index)
	{
	case MI_SetTime:
		show_message(FM_TimeAdjustSuceess, Farsi);
		break;
		
	case MI_SetDate:
		show_message(FM_DateAdjustSuceess, Farsi);
		break;

	case MI_ChangePassword:
		show_message(FM_ChangePasswordSuccess, Farsi);
		break;
		
	case MI_SetIP:
        case MI_SetServerIP:
        case MI_SetGateway:   
		show_message(FM_IPAdjustSuccess, Farsi);
		break;

        case MI_SetDeviceID:
		//show_message(FM_DeviceIDAdjustSuccess, Farsi);
		break;

        case MI_SetFactorySettings:
		show_message(FM_FactorySettingsLoadSuccess, Farsi);
		break;

        case MI_SetSubnetMask:
		show_message(FM_SubnetMaskAdjustSuccess, Farsi);
		break;
	
	case MI_SetPort:
		show_message(FM_PortAdjustSuccess, Farsi);
		break;
                
        case MI_InitModem:                                          //d.k
		show_message(FM_ModemInitSuccess, Farsi);
		break;        
		
	case MI_DeviceMode:
		show_message(FM_DeviceModeChangeSuccess, Farsi);
		break;
		
	case MI_FMActivity:
		show_message(FM_FMActivityChangeSuccess, Farsi);
		break;
		
	case MI_DevicePreCard:
		show_message(FM_PreCardChangeSuccess, Farsi);
		break;
	}		
}

void Menu::command_execute_fail (void)
{
	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
	OS_StopTimer(&timer_special);

	switch (menu_index)
	{
	case MI_SetTime:
		show_message(FM_TimeAdjustFail, Farsi);
		break;
		
	case MI_SetDate:
		show_message(FM_DateAdjustFail, Farsi);
		break;
		
	case MI_ChangePassword:
		show_message(FM_ChangePasswordFail, Farsi);
		break;

	case MI_SetIP:
        case MI_SetServerIP:
        case MI_SetGateway:  
		show_message(FM_IPAdjustFail, Farsi);
		break;

        case MI_SetDeviceID:
		//show_message(FM_DeviceIDAdjustFail, Farsi);
		break;

        case MI_SetFactorySettings:
		show_message(FM_FactorySettingsLoadFail, Farsi);
		break;

        case MI_SetSubnetMask:
		show_message(FM_SubnetMaskAdjustFail, Farsi);
		break;
	
	case MI_SetPort:
		show_message(FM_PortAdjustFail, Farsi);
		break;
		
        case MI_InitModem:                                  //d.k
		show_message(FM_ModemInitFail, Farsi);
		break;         
                
	case MI_DeviceMode:
		show_message(FM_DeviceModeChangeFail, Farsi);
		break;
		
	case MI_FMActivity:
		show_message(FM_FMActivityChangeFail, Farsi);
		break;
		
	case MI_DevicePreCard:
		show_message(FM_PreCardChangeFail, Farsi);
		break;
	}		

	//menu_index = get_menu_characteristic(menu_index).Parent;
	//highlighted_menu = get_menu_characteristic(menu_index).FirstChild;
	//menu_handler();	
}

void Menu::init_members_for_next_command (void)
{
	Key::ClearKeyBuffer();
	
	keypad_data.Clear();
}

void Menu::menu_handler (void)
{
	BackLight b(FM_OnOffOn, S_VeryFast);
	LCD lcd(Farsi);
	
	//b.DoFade();
	lcd.Clear();
	
	init_members_for_next_command();
	
	draw_menu();
}

void Menu::show_context (void)
{
	BackLight b(FM_OnOffOn, S_VeryFast);
	LCD lcd(Farsi);
	DateTime d;
	Setting set;
	string str;
	DigitalData dd;
	//Personal p;

	Finger f;
	
	//b.DoFade();
	lcd.Clear();
	
	init_members_for_next_command();
	
	OS_StartTimer(&timer_second);
	
	switch (menu_index)
	{
	case MI_WaitForPassword:
		lcd.Write(A_Center, 0, get_appropriate_message(FM_EnterPassword, Farsi));
		f.FreeScanMode(On);
		break;
		
	case MI_SetDate:
		lcd.Write(A_Center, 0, get_appropriate_message(FM_CurrentDate, Farsi));
		lcd.Write(A_Center, 2, d.GetShamsiDate());
		lcd.Write(A_Center, 4, get_appropriate_message(FM_NewDate, Farsi));
		lcd.Write(A_Center, 6, "----/--/--");
		break;

	case MI_SetTime:
		lcd.Write(A_Center, 0, get_appropriate_message(FM_CurrentTime, Farsi));
		lcd.Write(A_Center, 2, d.GetTime());
		lcd.Write(A_Center, 4, get_appropriate_message(FM_NewTime, Farsi));
		lcd.Write(A_Center, 6, "--:--:--");	

                OS_RetriggerTimer(&timer_second);
		break;	
	
	case MI_ChangePassword:
		lcd.Write(A_Center, 0, get_appropriate_message(FM_ChangePassword, Farsi));
		break;		
	
	case MI_SetDeviceID:
		lcd.Write(A_Center, 2, get_appropriate_message(FM_CurrentID, Farsi));
		lcd.Write(A_Center, 4, Utility::ToString(set.GetSettings().DeviceID));
		//lcd.Write(A_Center, 4, get_appropriate_message(FM_NewID, Farsi));
		//lcd.Write(A_Center, 6, "-----");				
		break;	

	case MI_SetFactorySettings:
		lcd.Write(A_Center, 0, get_appropriate_message(FM_RestoreFactorySettings, Farsi));			
		lcd.SetPixelFormat(PF_Invert);
		lcd.Write(A_Center, 4, get_appropriate_message(FM_Yes, Farsi));			
		lcd.SetPixelFormat(PF_Normal);
	
		break;		


	case MI_SetIP:
		lcd.Write(A_Center, 0, get_appropriate_message(FM_CurrentIP, Farsi));
		
		str = "";
		dd.SetDword(set.GetSettings().DeviceIP);
		
		str += Utility::ToString(dd.GetByte(3));
		str += " . ";
		str += Utility::ToString(dd.GetByte(2));
		str += " . ";
		str += Utility::ToString(dd.GetByte(1));
		str += " . ";
		str += Utility::ToString(dd.GetByte(0));
		
		
		lcd.Write(A_Center, 2, str);
		lcd.Write(A_Center, 4, get_appropriate_message(FM_NewIP, Farsi));
		lcd.Write(A_Center, 6, "--- . --- . --- . ---");				
		break;	
                
        case MI_SetGateway:                                          //dk
		lcd.Write(A_Center, 0, get_appropriate_message(FM_CurrentIP, Farsi));
		
		str = "";
		dd.SetDword(set.GetSettings().Gateway);
		
		str += Utility::ToString(dd.GetByte(3));
		str += " . ";
		str += Utility::ToString(dd.GetByte(2));
		str += " . ";
		str += Utility::ToString(dd.GetByte(1));
		str += " . ";
		str += Utility::ToString(dd.GetByte(0));
		
		
		lcd.Write(A_Center, 2, str);
		lcd.Write(A_Center, 4, get_appropriate_message(FM_NewIP, Farsi));
		lcd.Write(A_Center, 6, "--- . --- . --- . ---");				
		break;	
       
        case MI_SetServerIP:                                            //dk
		lcd.Write(A_Center, 0, get_appropriate_message(FM_CurrentIP, Farsi));
		
		str = "";
		dd.SetDword(set.GetSettings().ServerIP);
		
		str += Utility::ToString(dd.GetByte(3));
		str += " . ";
		str += Utility::ToString(dd.GetByte(2));
		str += " . ";
		str += Utility::ToString(dd.GetByte(1));
		str += " . ";
		str += Utility::ToString(dd.GetByte(0));
		
		
		lcd.Write(A_Center, 2, str);
		lcd.Write(A_Center, 4, get_appropriate_message(FM_NewIP, Farsi));
		lcd.Write(A_Center, 6, "--- . --- . --- . ---");				
		break;	        

	case MI_SetSubnetMask:
		lcd.Write(A_Center, 0, get_appropriate_message(FM_CurrentMask, Farsi));
		
		str = "";
		dd.SetDword(set.GetSettings().Mask);
		
		str += Utility::ToString(dd.GetByte(3));
		str += " . ";
		str += Utility::ToString(dd.GetByte(2));
		str += " . ";
		str += Utility::ToString(dd.GetByte(1));
		str += " . ";
		str += Utility::ToString(dd.GetByte(0));
		
		
		lcd.Write(A_Center, 2, str);
		lcd.Write(A_Center, 4, get_appropriate_message(FM_NewMask, Farsi));
		lcd.Write(A_Center, 6, "--- . --- . --- . ---");				
		break;	

        case MI_SetPort:
		lcd.Write(A_Center, 0, get_appropriate_message(FM_CurrentPort, Farsi));
		lcd.Write(A_Center, 2, Utility::ToString(set.GetSettings().DevicePort));
		lcd.Write(A_Center, 4, get_appropriate_message(FM_NewPort, Farsi));
		lcd.Write(A_Center, 6, "-----");				
		break;		         
        case MI_InitModem:                      //d.k
			//initial_modem();
                OS_CREATETASK(&tcb_Modem, "Modem_Task", initial_modem_task, TP_LowestPeriority, stack_Modem);
	        break;
	case MI_AddPerson:
	case MI_EditPerson:
		
                last_card_id = 0;
		//delete_old_templates = false;
		
		if (sub_menu_index == 0)
		{
			lcd.Write(A_Center, 0, get_appropriate_message(FM_EnterPersonalNumber, Farsi));			
		}
		
		if (sub_menu_index == 1)
		{
			lcd.SetPixelFormat(PF_Normal);
			lcd.Write(A_Center, 0, get_appropriate_message(FM_AccessRight, Farsi));

			if (pi.PAR == PAR_User)
			{
				sub_menu_temp_index = 0;
				lcd.SetPixelFormat(PF_Invert);
			}
			else
				lcd.SetPixelFormat(PF_Normal);

			lcd.Write(A_Center, 2, get_appropriate_message(FM_User, Farsi));

			if (pi.PAR == PAR_Administrator)
			{
				sub_menu_temp_index = 1;
				lcd.SetPixelFormat(PF_Invert);
			}
			else
				lcd.SetPixelFormat(PF_Normal);

			lcd.Write(A_Center, 4, get_appropriate_message(FM_Administrator, Farsi));
		}

		if (sub_menu_index == 2)
		{
			lcd.SetPixelFormat(PF_Normal);
			lcd.Write(A_Center, 0, get_appropriate_message(FM_TrafficMode, Farsi));

			if (pi.PIS == PIS_DeviceMode)
			{
				sub_menu_temp_index = 0;
				lcd.SetPixelFormat(PF_Invert);
			}
			else
				lcd.SetPixelFormat(PF_Normal);

			lcd.Write(A_Center, 2, get_appropriate_message(FM_DeviceMode, Farsi));

			if (pi.PIS == PIS_CardOrFinger)
			{
				sub_menu_temp_index = 1;
				lcd.SetPixelFormat(PF_Invert);
			}
			else
				lcd.SetPixelFormat(PF_Normal);

			lcd.Write(A_Center, 4, get_appropriate_message(FM_CardOrFinger, Farsi));
			
			if (pi.PIS == PIS_CardAndFinger)
			{
				sub_menu_temp_index = 2;
				lcd.SetPixelFormat(PF_Invert);
			}
			else
				lcd.SetPixelFormat(PF_Normal);
			
			lcd.Write(A_Center, 6, get_appropriate_message(FM_CardANDFinger, Farsi));
		}
		
		if (sub_menu_index == 3)
		{
                        is_f1_pressed=0;
                        
			lcd.Write(A_Right, 0, get_appropriate_message(FM_Password, Farsi));
			lcd.Write(A_Left, 0, Utility::ToString(pi.Password));

			lcd.Write(A_Right, 2, get_appropriate_message(FM_Card, Farsi));
			lcd.Write(A_Left, 2, Utility::ToString(pi.CardID));
			
			str = "";
				
			str += get_appropriate_message(FM_Finger, Farsi);
	
			str += ")";
			str+= Utility::ToString (pi.FingerTemplateNumber+1);
			str += "(";
			str += ":";
			
			lcd.Write(A_Right, 4, str);
			
			//lcd.Write(A_Right, 6, get_appropriate_message(FM_TemplateCount, Farsi));
			//lcd.Write(A_Left, 6, Utility::ToString(pi.FingerTemplateNumber));
                        if (menu_index == MI_EditPerson)
                        {
                          lcd.Write(A_Right, 6, get_appropriate_message(FM_EditFingers, Farsi));
                          
                          lcd.font.NumberRepresentation(English);
                            
                          lcd.Write(A_Left, 6, "F1");
                          
                        }
                        else
                        {
                                lcd.Write(A_Right, 6, get_appropriate_message(FM_TemplateCount, Farsi));
			        lcd.Write(A_Left, 6, Utility::ToString(pi.FingerTemplateNumber));
                        }
		}		
	break;
	
	case MI_DeletePerson:
		if (sub_menu_index == 0)
		{
			sub_menu_index = 0;
			lcd.Write(A_Center, 0, get_appropriate_message(FM_EnterPersonalNumber, Farsi));			
		}
		
		if (sub_menu_index == 1)
		{
			lcd.Write(A_Center, 0, get_appropriate_message(FM_DeleteUser, Farsi));			
			lcd.SetPixelFormat(PF_Invert);
			lcd.Write(A_Center, 4, get_appropriate_message(FM_Yes, Farsi));			
			lcd.SetPixelFormat(PF_Invert);
		}			
	
		break;		
		
	case MI_DeleteAllPerson:
		lcd.Write(A_Center, 0, get_appropriate_message(FM_DeleteAllUser, Farsi));			
		lcd.SetPixelFormat(PF_Invert);
		lcd.Write(A_Center, 4, get_appropriate_message(FM_Yes, Farsi));			
		lcd.SetPixelFormat(PF_Normal);
	
		break;		
		
	case MI_DeviceMode:
		
		lcd.SetPixelFormat(PF_Normal);
		lcd.Write(A_Center, 0, get_appropriate_message(FM_ChooseDeviceMode, Farsi));			
		
		if (set.GetSettings().DeviceMode == PIS_CardOrFinger)
		{
			lcd.SetPixelFormat(PF_Invert);			
			sub_menu_temp_index = 0;
		}

		lcd.Write(A_Center, 2, get_appropriate_message(FM_CardOrFinger, Farsi));					
		lcd.SetPixelFormat(PF_Normal);

		if (set.GetSettings().DeviceMode == PIS_CardAndFinger)
		{
			lcd.SetPixelFormat(PF_Invert);			
			sub_menu_temp_index = 1;
		}
		
		lcd.Write(A_Center, 4, get_appropriate_message(FM_CardANDFinger, Farsi));					
		lcd.SetPixelFormat(PF_Normal);

		break;	
		
	case MI_FMActivity:
		
		lcd.SetPixelFormat(PF_Normal);
		lcd.Write(A_Center, 0, get_appropriate_message(FM_FingerActivity, Farsi));			
		
		if (set.GetSettings().FingerFreeScanActivity == A_Disable)
		{
			lcd.SetPixelFormat(PF_Invert);			
			sub_menu_temp_index = 0;
		}
		
		lcd.Write(A_Center, 2, get_appropriate_message(FM_Disable, Farsi));					
		lcd.SetPixelFormat(PF_Normal);
		
		if (set.GetSettings().FingerFreeScanActivity == A_Enable)
		{
			lcd.SetPixelFormat(PF_Invert);			
			sub_menu_temp_index = 1;
		}
		
		lcd.Write(A_Center, 4, get_appropriate_message(FM_Enable, Farsi));					
		lcd.SetPixelFormat(PF_Normal);
		
		break;	

	case MI_DevicePreCard:
		
		lcd.SetPixelFormat(PF_Normal);
		lcd.Write(A_Center, 0, get_appropriate_message(FM_PreCard, Farsi));			
		
		if (set.GetSettings().DevicePreCard == PC_NormalEnter)
		{
			lcd.SetPixelFormat(PF_Invert);			
			sub_menu_temp_index = 0;
		}
		
		lcd.Write(A_Center, 2, get_appropriate_message(FM_Normal, Farsi));					
		lcd.SetPixelFormat(PF_Normal);
		
		if (set.GetSettings().DevicePreCard == PC_Enter)
		{
			lcd.SetPixelFormat(PF_Invert);			
			sub_menu_temp_index = 1;
		}
		
		lcd.Write(A_Center, 4, get_appropriate_message(FM_Enter, Farsi));					
		lcd.SetPixelFormat(PF_Normal);
		
		if (set.GetSettings().DevicePreCard == PC_Exit)
		{
			lcd.SetPixelFormat(PF_Invert);			
			sub_menu_temp_index = 2;
		}
		
		lcd.Write(A_Center, 6, get_appropriate_message(FM_Exit, Farsi));					
		lcd.SetPixelFormat(PF_Normal);
		
		break;	

        case MI_DeleteTraffics:
                lcd.Write(A_Center, 0, get_appropriate_message(FM_DeleteAllTraffics, Farsi));			
		lcd.SetPixelFormat(PF_Invert);
		lcd.Write(A_Center, 4, get_appropriate_message(FM_Yes, Farsi));			
		lcd.SetPixelFormat(PF_Normal);

                break;
		
	}
}

void Menu::draw_menu ()
{
	LCD lcd;	
	MenuIndex mi;
	int line;

	mi = get_menu_characteristic(menu_index).FirstChild;
	line = 0;
	
	lcd.Clear();	
		
	while ((mi != MI_None) && (line < 8))
	{
		if (mi == highlighted_menu)
		{
			lcd.Clear(line, White);
			lcd.SetPixelFormat(PF_Invert);	
		}
		else
		{
			lcd.Clear(line, Black);
			lcd.SetPixelFormat(PF_Normal);
		}
		
		lcd.Write(A_Right, line, get_menu_title(mi, Farsi));
				
		if (get_menu_characteristic(mi).FirstChild != MI_None)
			lcd.Write(A_Left,line, "<");	

		mi = get_menu_characteristic(mi).NextMenu;
		line+=2;
	}		
}
	
MenuCharacteristic Menu::get_menu_characteristic (MenuIndex index)
{
	MenuCharacteristic mc;
		
	mc.FirstChild = MI_None;
	mc.Parent = MI_None;
	mc.NextMenu = MI_None;
	mc.PreviousMenu = MI_None;	
	
	switch (index)
	{
	case MI_WaitForPassword:
		mc.FirstChild = MI_None;
		mc.Parent = MI_None;
		mc.NextMenu = MI_None;
		mc.PreviousMenu = MI_None;
		break;
		
	case MI_MainMenu:
		mc.FirstChild = MI_BaseSetting;
		mc.Parent = MI_None;
		mc.NextMenu = MI_None;
		mc.PreviousMenu = MI_None;
		break;		
		
	case MI_BaseSetting:
		mc.FirstChild = MI_SetDateTime;
		mc.Parent = MI_MainMenu;
		mc.NextMenu = MI_LANSetting;
		mc.PreviousMenu = MI_None;
		break;
		
	case MI_LANSetting:
		mc.FirstChild = MI_SetAddress;
		mc.Parent = MI_MainMenu;
		mc.NextMenu = MI_PersonalSetting;
		mc.PreviousMenu = MI_BaseSetting;
		break;
		
	case MI_PersonalSetting:
		mc.FirstChild = MI_AddPerson;
		mc.Parent = MI_MainMenu;
		mc.NextMenu = MI_DeviceSetting;
		mc.PreviousMenu = MI_LANSetting;
		break;		

	case MI_DeviceSetting:
		mc.FirstChild = MI_DeviceMode;
		mc.Parent = MI_MainMenu;
		mc.NextMenu = MI_None;
		mc.PreviousMenu = MI_PersonalSetting;
		break;		
	
	case MI_SetDateTime:
		mc.FirstChild = MI_SetDate;
		mc.Parent = MI_BaseSetting;
		mc.NextMenu = MI_ChangePassword;
		mc.PreviousMenu = MI_None;
		break;		

        case MI_SetDate:
		mc.FirstChild = MI_None;
		mc.Parent = MI_SetDateTime;
		mc.NextMenu = MI_SetTime;
		mc.PreviousMenu = MI_None;
		break;		
		
	case MI_SetTime:
		mc.FirstChild = MI_None;
		mc.Parent = MI_SetDateTime;
		mc.NextMenu = MI_None;
		mc.PreviousMenu = MI_SetDate;
		break;		
		
	case MI_ChangePassword:
		mc.FirstChild = MI_None;
		mc.Parent = MI_BaseSetting;
		mc.NextMenu = MI_SetDeviceID;
		mc.PreviousMenu = MI_SetDateTime;
		break;		
		
	case MI_SetDeviceID:
		mc.FirstChild = MI_None;
		mc.Parent = MI_BaseSetting;
		mc.NextMenu = MI_SetFactorySettings;
		mc.PreviousMenu = MI_ChangePassword;
		break;		

	case MI_SetFactorySettings:
		mc.FirstChild = MI_None;
		mc.Parent = MI_BaseSetting;
		mc.NextMenu = MI_None;
		mc.PreviousMenu = MI_SetDeviceID;
		break;		

        case MI_SetAddress:
		mc.FirstChild = MI_SetIP;
		mc.Parent = MI_LANSetting;
		mc.NextMenu = MI_SetPort;           //dk
		mc.PreviousMenu = MI_None;
		break;

	case MI_SetPort:                            //dk
		mc.FirstChild = MI_None;
		mc.Parent = MI_LANSetting;
		mc.NextMenu = MI_SetGateway;         
		mc.PreviousMenu = MI_SetAddress;
		break;
                
        case MI_SetGateway:                             //dk
		mc.FirstChild = MI_None;
		mc.Parent = MI_LANSetting;
		mc.NextMenu = MI_InitModem;
		mc.PreviousMenu = MI_SetPort;
		break;
		
       /* case MI_SetServerIP:                          //dk
		mc.FirstChild = MI_None;
		mc.Parent = MI_LANSetting;
		mc.NextMenu = MI_InitModem;         
		mc.PreviousMenu = MI_SetPort;
		break;
         */       
        case MI_SetIP:                                //dk
		mc.FirstChild = MI_None;
		mc.Parent = MI_SetAddress;
		mc.NextMenu = MI_SetSubnetMask;         
		mc.PreviousMenu = MI_None;
		break;
                
	case MI_SetSubnetMask:                        //dk
		mc.FirstChild = MI_None;
		mc.Parent = MI_SetAddress;
		mc.NextMenu = MI_None;
		mc.PreviousMenu = MI_SetIP;
		break;		
                
           case MI_InitModem:                         //d.k
		mc.FirstChild = MI_None;
		mc.Parent = MI_LANSetting;
		mc.NextMenu = MI_None;
		mc.PreviousMenu = MI_SetGateway;
		break; 	
		
	case MI_AddPerson:
		mc.FirstChild = MI_None;
		mc.Parent = MI_PersonalSetting;
		mc.NextMenu = MI_EditPerson;
		mc.PreviousMenu = MI_None;
		break;		
		
	case MI_EditPerson:		
		mc.FirstChild = MI_None;
		mc.Parent = MI_PersonalSetting;
		mc.NextMenu = MI_DeletePerson;
		mc.PreviousMenu = MI_AddPerson;
		break;
		
	case MI_DeletePerson:
		mc.FirstChild = MI_None;
		mc.Parent = MI_PersonalSetting;
		mc.NextMenu = MI_DeleteAllPerson;
		mc.PreviousMenu = MI_EditPerson;
		break;		
		
	case MI_DeleteAllPerson:
		mc.FirstChild = MI_None;
		mc.Parent = MI_PersonalSetting;
		mc.NextMenu = MI_None;
		mc.PreviousMenu = MI_DeletePerson;
		break;	
		
	case MI_DeviceMode:		
		mc.FirstChild = MI_None;
		mc.Parent = MI_DeviceSetting;
		mc.NextMenu = MI_FMActivity;
		mc.PreviousMenu = MI_None;
		break;	

	case MI_FMActivity:		
		mc.FirstChild = MI_None;
		mc.Parent = MI_DeviceSetting;
		mc.NextMenu = MI_DevicePreCard;
		mc.PreviousMenu = MI_DeviceMode;
		break;	
		
	case MI_DevicePreCard:		
		mc.FirstChild = MI_None;
		mc.Parent = MI_DeviceSetting;
		mc.NextMenu = MI_DeleteTraffics;
		mc.PreviousMenu = MI_FMActivity;
		break;	

	case MI_DeleteTraffics:		
		mc.FirstChild = MI_None;
		mc.Parent = MI_DeviceSetting;
		mc.NextMenu = MI_None;
		mc.PreviousMenu = MI_DevicePreCard;
		break;	
	}
	
	return mc;	
}

string Menu::get_appropriate_message (FixMessage index, Language l)
{
	string message;
	
	message.clear();
	
	switch (l)
	{
	case Farsi:
		switch (index)
		{
		case FM_EnterPassword:
			message = "��� ���� �� ���� ����";
			break;
			
		case FM_IncorrectPassword:
			message = "��� ������ ���";
			break;		

		case FM_CurrentDate:
			message = "����� ����";
			break;		

		case FM_NewDate:
			message = "����� ����";
			break;		

		case FM_CurrentTime:
			message = "���� ����";
			break;		

		case FM_NewTime:
			message = "���� ����";
			break;		
			
		case FM_TimeAdjustSuceess:
			message = "���� ����� ��";
			break;	
			
		case FM_TimeAdjustFail:
			message = "�Ԙ�� �� ����� ����";
			break;			
			
		case FM_DateAdjustSuceess:
			message = "����� ����� ��";
			break;	
			
		case FM_DateAdjustFail:
			message = "�Ԙ�� �� ����� �����";
			break;	
			
		case FM_ChangePassword:
			message = "��� ���� �� ���� ����";
			break;				

		case FM_CurrentIP:
			message = "PI ����";
			break;		

		case FM_NewIP:
			message = "PI ����";
			break;		
			
		case FM_CurrentPort:
			message = "torP ����";
			break;		

		case FM_NewPort:
			message = "troP ����";
			break;	
			
		case FM_IPAdjustSuccess:
			message = "PI �� ��";
			break;
			
		case FM_IPAdjustFail:
			message = "�Ԙ�� �� ����� PI";
			break;
			
		case FM_PortAdjustSuccess:
			message = "troP �� ��";
			break;
                        
                case FM_ModemInitSuccess:
			message = "���� ����� ��";
			break;         
			
		case FM_PortAdjustFail:
			message = "�Ԙ�� �� ����� troP";
			break;
                        
                case FM_ModemInitFail:                      //d.k
			message = "�Ԙ�� �� ����� ����";
			break;        
			
		case FM_EnterPersonalNumber:
			message = "����� ������";
			break;
			
		case FM_TrafficMode:
			message = "����� ����";
			break;		

		case FM_PNORCardANDFinger:
			message = "� / ���� + ����";
			break;		
                        
                case FM_CardANDFinger:
			message = "���� � ����";
			break;	
			
		case FM_CardOrFinger:
			message = "���� �� ����";
			break;		
			
		case FM_DeviceMode:
			message = "��� ��� ��ʐ��";
			break;				
			
		case FM_AccessRight:
			message = "��� ������";
			break;		
			
		case FM_User:
			message = "�����";
			break;		
			
		case FM_Administrator:
			message = "����";
			break;				
			
		case FM_Password:
			message = "��� :";
			break;				
			
		case FM_Card:
			message = "���� :";
			break;		
			
		case FM_Finger:
			message = "���� ";
			break;		
			
		case FM_TemplateCount:
                  message = "������ ��� ���� :";
			break;	
                        
                case FM_EditFingers:
                  message = "������ ��� ���� :";
			break;
                  
		case FM_Repetitive:
			message = "ʘ����";
			break;				
			
		case FM_Place:
			message = "Ȑ�����";
			break;	
			
		case FM_PlaceAgain:
			message = "�Ә� ����";
			break;		
			
		case FM_Register:
			message = "��� ��";
			break;		
			
		case FM_NotRegister:
			message = "��� �����";
			break;
			
		case FM_RepetitivePersonalNumber:
			message = "� � ʘ���� ���";
			break;
			
		case FM_InvalidPersonalNumber:
			message = "� � ������ ���";
			break;
			
		case FM_PersonAddSuccessfully:
			message = "����� ����� ��";
			break;
			
		case FM_PersonAddFail:
			message = "�Ԙ�� �� ����� �����";
			break;
			
		case FM_PersonEditSuccessfully:
			message = "����� ������ ��";
			break;
			
		case FM_PersonEditFail:
			message = "�Ԙ�� �� ������ �����";
			break;
			
		case FM_DeleteUser:
			message = "����� ��� ��� �";
			break;			

		case FM_DeleteAllUser:
			message = "������� ��� ���� �";
			break;			

		case FM_Yes:
			message = "���";
			break;			

		case FM_UserDeleteSuccess:
			message = "����� ��� ��";
			break;			
		
		case FM_DeleteFail:
			message = "�Ԙ�� �� ��� �����";
			break;			

		case FM_AllUserDeleteSuccess:
			message = " ������� ��� ����";
			break;			

		case FM_AllUserDeleteFail:
			message = "�Ԙ�� �� ��� �������";
			break;		
			
		case FM_ChooseDeviceMode:
			message = "�� ��ʐ��";
			break;			
		
		case FM_FingerActivity:
			message = "�ǎ�� ��� ����";
			break;			

		case FM_Enable:
			message = "����";
			break;			

		case FM_Disable:
			message = "��� ����";
			break;		
			
		case FM_DeviceModeChangeSuccess:
			message = "�� ��ʐ�� ����� ��";
			break;			
		
		case FM_DeviceModeChangeFail:
			message = "�Ԙ�� �� ����� ��";
			break;			

		case FM_FMActivityChangeSuccess:
			message = "� � ����� ��";
			break;			

		case FM_FMActivityChangeFail:
			message = "�Ԙ�� �� ����� � �";
			break;		
			
		case FM_PreCard:
			message = "��� ����";
			break;				

		case FM_PreCardChangeSuccess:
			message = "��� ���� ����� ��";
			break;				

		case FM_PreCardChangeFail:
			message = "�Ԙ�� �� ����� � �";
			break;				
			
                case FM_Normal:
			message = "����";
			break;		

                case FM_Enter:
			message = "����";
			break;				

		case FM_Exit:
			message = "����";
			break;				

		case FM_Mission:
			message = "�������";
			break;		

		case FM_ChangePasswordSuccess:
			message = "��� ���� ����� ��";
			break;					
			
		case FM_ChangePasswordFail:
			message = "�Ԙ�� �� ����� ���";
			break;		

		case FM_CurrentID:
			message = "����� ��ʐ��";
			break;					

		case FM_NewID:
			message = "����� ����";
			break;					

		case FM_RestoreFactorySettings:
			message = "������� ������� �����";
			break;					

		case FM_CurrentMask:
			message = "tenbuS ���� ";
			break;					

		case FM_NewMask:
			message = "tenbuS ����";
			break;					

		case FM_DeleteAllTraffics:
			message = "��� ������ ��� ����";
			break;					
			
                case FM_DeleteAllTrafficsuccess:
			message = "��� ������ ��� ����";
			break;		

		case FM_DeviceIDAdjustSuccess:
			message = "����� ����� ��";
			break;					

		case FM_FactorySettingsLoadSuccess:
			message = "������� ������� ��";
			break;					

		case FM_SubnetMaskAdjustSuccess:
			message = "tenbuS �� ��";
			break;					

		case FM_DeviceIDAdjustFail:
			message = "�Ԙ�� �� ����� �����";
			break;					

		case FM_FactorySettingsLoadFail:
			message = "�Ԙ�� �� ������� �������";
			break;					

		case FM_SubnetMaskAdjustFail:
			message = "�Ԙ�� �� tenbuS";
			break;					




		}			
	}
	
	return message;
}

string Menu::get_menu_title (MenuIndex index, Language l)
{
	string message;
	
	message.clear();
	
	switch (l)
	{
	case Farsi:
		switch (index)
		{
		case MI_BaseSetting:
			message = "������� ����";
			break;		
			
		case MI_LANSetting:
			message = "������� �Ș�";
			break;		
			
		case MI_PersonalSetting:
			message = "������� �����";
			break;	
			
		case MI_DeviceSetting:
			message = "������� ��ʐ��";
			break;		

		case MI_SetDateTime:
			message = "����� ����� � ����";
			break;					

                case MI_SetDate:
			message = "����� �����";
			break;					

		case MI_SetTime:
			message = "����� ����";
			break;	
			
		case MI_ChangePassword:
			message = "����� ��� ����";
			break;					

		case MI_SetDeviceID:
			message = "����� ��ʐ��";
			break;					

                case MI_SetFactorySettings:
			message = "������� �������";
			break;					

                case MI_SetAddress:
			message = "����� PI";
			break;					

                case MI_SetSubnetMask:
			message = "����� tenbuS";
			break;					

		case MI_SetPort:
			message = "����� troP";
			break;
                        
                case MI_SetIP:                          //dk
                        message = "����� sserdda PI";
			break;
                        
                case MI_SetServerIP:                     //dk
			message = "����� revres SND";
			break;
         
                case MI_SetGateway:                          //dk
			message = "����� yawetaG";
			break;
                        
                case MI_InitModem:                              //d.k
			message = "����� ����� ����";
			break;

		case MI_AddPerson:
			message = "����� ���� �����";
			break;				

		case MI_EditPerson:
			message = "������ �����";
			break;				

		case MI_DeletePerson:
			message = "��� �����";
			break;				
			
		case MI_DeleteAllPerson:
			message = "��� ��� �������";
			break;	
			
		case MI_DeviceMode:
			message = "����� �� ��ʐ��";
			break;			

		case MI_FMActivity:
			message = "������ ��� ����";
			break;			
		
		case MI_DevicePreCard:
			message = "����� ��� ����";
			break;			

                case MI_DeleteTraffics:
			message = "��� ���� ������";
			break;		
		}					
	}
	
	return message;
}

/*void Menu::last_person_number (DWORD PN)
{
	int ctr, ctr2;
	
	BYTE temp[20];
	
	for (ctr=0 ; PN > 0 ; ctr++)
	{
		temp [ctr] = PN%10;
		PN /= 10;
	}
	
	for (ctr2 = ctr; ctr2 > 0; ctr2--)
		keypad_data += temp[ctr - ctr2];		

     lcd.Write(A_Center, 0, Utility::ToString(PN+1));
}	*/

void Menu::enroll_success (DWORD UserID)
{
	LCD lcd(Farsi);
	Finger f;
	string str;	


	if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
	{
		OS_RetriggerTimer(&timer_special);
		
		OS_RetriggerTimer(&timer_menu);


		if (pi.FingerTemplateNumber == 0)
			pi.FingerID = UserID;

		lcd.Clear(4);	
	
		pi.FingerTemplateNumber ++;

		str = "";
				
		str += get_appropriate_message(FM_Finger, Farsi);
	
		str += ")";
		string st = Utility::ToString (pi.FingerTemplateNumber+1);
                if(st.length() == 2)
                {
                  char s;
                   s = st[1];
                   st[1] = st[0];
                   st[0] = s;
                }
                str += st;
		str += "(";
		str += ":";

		lcd.Write(A_Right, 4, str);
		lcd.Write(A_Left, 4, get_appropriate_message(FM_Register, Farsi));	
		
		lcd.Clear(6);
	
		lcd.Write(A_Right, 6, get_appropriate_message(FM_TemplateCount, Farsi));
		lcd.Write(A_Left, 6, Utility::ToString (pi.FingerTemplateNumber));
                
                f.AddTemplate(pi.FingerID);
	}
}

void Menu::idenfify_success (DWORD UserID)
{
	LCD lcd(Farsi);
	string str;	
	Personal p;

	
	if (menu_index == MI_WaitForPassword)
		p.IsAdministratorFinger(UserID);		
	
	if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
	{
		OS_RetriggerTimer(&timer_special);
	
		OS_RetriggerTimer(&timer_menu);

		lcd.Clear(4);
		str = "";
				
		str += get_appropriate_message(FM_Finger, Farsi);
	
		str += ")";
		str+= Utility::ToString (pi.FingerTemplateNumber+1);
		str += "(";
		str += ":";

		lcd.Write(A_Right, 4, str);
		lcd.Write(A_Left, 4, get_appropriate_message(FM_Repetitive, Farsi));
	}
}

void Menu::identify_fail (void)
{
	//LCD lcd(Farsi);
	//Finger f;
	//string str;
	
	/*if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
	{
		OS_RetriggerTimer(&timer_special);
	
		OS_RetriggerTimer(&timer_menu);
		
		if (pi.FingerTemplateNumber == 0)
			f.EnrollByScan ();
		else
			f.AddTemplate(pi.FingerID);
		
		lcd.Clear(4);

		str = "";
				
		str += get_appropriate_message(FM_Finger, Farsi);
	
		str += ")";
		str+= Utility::ToString (pi.FingerTemplateNumber+1);
		str += "(";
		str += ":";

		lcd.Write(A_Right, 4, str);
		lcd.Write(A_Left, 4, get_appropriate_message(FM_Place, Farsi));
	}*/
}	

void Menu::first_template_scan_success (void)
{
	LCD lcd(Farsi);
        Finger f;
	string str;
	
	if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
	{
		OS_RetriggerTimer(&timer_special);
	
		OS_RetriggerTimer(&timer_menu);

		lcd.Clear(4);
	
		str = "";
				
		str += get_appropriate_message(FM_Finger, Farsi);
	
		str += ")";
                string st = Utility::ToString (pi.FingerTemplateNumber+1);
		if(st.length() == 2)
                {
                     char s;
                     s = st[1];
                     st[1] = st[0];
                     st[0] = s;
                }
                str += st;
		str += "(";
		str += ":";

		lcd.Write(A_Right, 4, str);

		lcd.Write(A_Left, 4, get_appropriate_message(FM_PlaceAgain, Farsi));	
                
                f.AddTemplateRepet(pi.FingerID);
	}
}

void Menu::enroll_fail (void)
{
	LCD lcd(Farsi);
	Finger f;
	string str;
	
	if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
	{
		OS_RetriggerTimer(&timer_special);
		
		OS_RetriggerTimer(&timer_menu);

		lcd.Clear(4);

		str = "";
				
		str += get_appropriate_message(FM_Finger, Farsi);
	
		str += ")";
                string st = Utility::ToString (pi.FingerTemplateNumber+1);
		if(st.length() == 2)
                {
                     char s;
                     s = st[1];
                     st[1] = st[0];
                     st[0] = s;
                }
                str += st;
		str += "(";
		str += ":";

		lcd.Write(A_Right, 4, str);

		lcd.Write(A_Left, 4, get_appropriate_message(FM_NotRegister, Farsi));	
               
                if (pi.FingerTemplateNumber == 0)
                   f.EnrollByScan ();
               else
                   f.AddTemplate(pi.FingerID);

	}
}

void Menu::finger_is_repetitive (void)
{
        LCD lcd(Farsi);
	Finger f;
	string str;

        if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
	{
                OS_RetriggerTimer(&timer_special);
                
                OS_RetriggerTimer(&timer_menu);

                lcd.Clear(4);
                
               str = "";
				
		str += get_appropriate_message(FM_Finger, Farsi);
	
		str += ")";
		str+= Utility::ToString (pi.FingerTemplateNumber+1);
		str += "(";
		str += ":";

	        lcd.Write(A_Right, 4, str);
                //lcd.Write(A_Right, 2, get_appropriate_message(FM_Card, Farsi));
                lcd.Write(A_Left, 4, get_appropriate_message(FM_Repetitive, Farsi));
                
                if ( pi.FingerTemplateNumber == 0)
                   f.EnrollByScan ();
                else
                   f.AddTemplate(pi.FingerID);
        }
}

void Menu::card_present (void)
{
	Mifare m;
	Personal p;

	if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
	{
        	if (last_card_id == m.GetLastCardID())
        		return;
	
        	last_card_id = m.GetLastCardID();	

                p.IsCardRepetitive(m.GetLastCardID());		
		
		OS_RetriggerTimer(&timer_menu);		
	}
}

void Menu::card_present_ex (void)
{
	MifareEx mx;
	Personal p;

	if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
	{
        	if (last_card_id == mx.GetLastCardID())
        		return;
	
        	last_card_id = mx.GetLastCardID();	

                p.IsCardRepetitiveEx(mx.GetLastCardID());		
		
		OS_RetriggerTimer(&timer_menu);		
	}
}

void Menu::person_is_exist (void)
{
	Personal p;
	
	if (menu_index == MI_AddPerson && sub_menu_index == 0)
	{						
		show_message(FM_RepetitivePersonalNumber, Farsi);
	
		init_members_for_next_command();
	}
	
	if (menu_index == MI_EditPerson && sub_menu_index == 0)
	{	
		p.GetPersonInformation ((DWORD) keypad_data.GetValue());
	}
	
	if (menu_index == MI_DeletePerson && sub_menu_index == 0)
	{
	 	pi.PersonNumber = (DWORD) keypad_data.GetValue();
		pi.Password = 0;
		pi.FingerID = 0;
		pi.CardID = 0;
		pi.PIS = PIS_CardOrFinger;
		pi.PAR = PAR_User;
		pi.FingerTemplateNumber = 0;
		
		sub_menu_index = 1;
				
		show_context();
	}

	OS_StopTimer(&timer_second);
}

void Menu::person_is_not_exist (void)
{	
	if (menu_index == MI_AddPerson && sub_menu_index == 0)
	{
		pi.PersonNumber = 0;
		pi.Password = 0;
		pi.FingerID = 0;
		pi.CardID = 0;
		pi.PIS = PIS_DeviceMode;
		pi.PAR = PAR_User;
		pi.FingerTemplateNumber = 0;
		
		pi.PersonNumber = (DWORD) keypad_data.GetValue();
	
		init_members_for_next_command();
							
		sub_menu_index = 1;
			
		show_context();	
	}
	
	if (menu_index == MI_EditPerson && sub_menu_index == 0)
	{						
		show_message(FM_InvalidPersonalNumber, Farsi);
	}
	
	if (menu_index == MI_DeletePerson && sub_menu_index == 0)
	{						
		show_message(FM_InvalidPersonalNumber, Farsi);
	}

	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
}

void Menu::person_add_success (void)
{
	Finger f;
	
	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
	
	if (menu_index == MI_AddPerson && sub_menu_index == 3)
	{	
                OS_RetriggerTimer(&timer_special);
		show_message(FM_PersonAddSuccessfully, Farsi);
		//f.FreeScanMode(Off);
	}	
}

void Menu::person_add_fail (void)
{
	Finger f;

	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
	
	if (menu_index == MI_AddPerson && sub_menu_index == 3)
	{	
                OS_RetriggerTimer(&timer_special);
		show_message(FM_PersonAddFail, Farsi);
		//f.FreeScanMode(Off);		
	}	
}

void Menu::person_edit_success (void)
{
	Finger f;

	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
	
	if (menu_index == MI_EditPerson && sub_menu_index == 3)
	{	
                OS_RetriggerTimer(&timer_special);
		show_message(FM_PersonEditSuccessfully, Farsi);
		//f.FreeScanMode(Off);		
	}	
}

void Menu::person_edit_fail (void)
{
	Finger f;
	
	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
	
	if (menu_index == MI_EditPerson && sub_menu_index == 3)
	{	
                OS_RetriggerTimer(&timer_special);
		show_message(FM_PersonEditFail, Farsi);
		//f.FreeScanMode(Off);		
	}	
}

void Menu::person_information_receive (PersonInformation* PI)
{
	if (menu_index == MI_EditPerson && sub_menu_index == 0)
	{		
		pi = *PI;
	
		init_members_for_next_command();
							
		sub_menu_index = 1;
			
		show_context();	
	}
}

void Menu::delete_person_success (void)
{
	Finger f;
	LCD lcd(Farsi);
	string str;
	
	if (!OS_GetTimerStatus(&timer_menu))
	{
		safe_call_handler(ep_exit_menu);	
		return;
	}
	
	if (menu_index == MI_EditPerson)
	{
		pi.FingerTemplateNumber = 0;
		
		//delete_old_templates = true;
		
		lcd.Clear(4);
		lcd.Clear(6);

		str = "";
				
		str += get_appropriate_message(FM_Finger, Farsi);
	
		str += ")";
		str+= Utility::ToString (pi.FingerTemplateNumber+1);
		str += "(";
		str += ":";

		lcd.Write(A_Right, 4, str);

		lcd.Write(A_Right, 6, get_appropriate_message(FM_TemplateCount, Farsi));
		lcd.Write(A_Left, 6, Utility::ToString(pi.FingerTemplateNumber));
		
		return;
	}
	
	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
	
	if (menu_index == MI_DeletePerson)
	{						
		show_message(FM_UserDeleteSuccess, Farsi);
	}
	
	//f.FreeScanMode(Off);		
}

void Menu::delete_person_fail (void)
{
	/*Finger f;
	string str;
	LCD lcd(Farsi);
	
	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
	
	if (menu_index == MI_EditPerson)
	{
		delete_old_templates = true;
		
		lcd.Clear(4);
		lcd.Clear(6);

		str = "";
				
		str += get_appropriate_message(FM_Finger, Farsi);
	
		str += ")";
		str+= Utility::ToString (pi.FingerTemplateNumber+1);
		str += "(";
		str += ":";

		lcd.Write(A_Right, 4, str);

		lcd.Write(A_Right, 6, get_appropriate_message(FM_TemplateCount, Farsi));
		lcd.Write(A_Left, 6, Utility::ToString(pi.FingerTemplateNumber));
		
		return;
	}
	
	if (menu_index == MI_DeletePerson)
	{						
		show_message(FM_DeleteFail, Farsi);
	}
	
	f.FreeScanMode(Off);	*/	
}

void Menu::delete_all_person_success (void)
{
	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
	
	if (menu_index == MI_DeleteAllPerson)
	{						
		show_message(FM_AllUserDeleteSuccess, Farsi);
	}		
}

void Menu::delete_all_person_fail (void)
{
	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
	
	if (menu_index == MI_DeleteAllPerson)
	{						
		show_message(FM_AllUserDeleteFail, Farsi);
	}			
}

void Menu::delete_template_success (void)
{
        Finger f;
        Personal p;
        LCD lcd(Farsi);
        string str;
             
  if (menu_index == MI_EditPerson && is_f1_pressed == 1)
  {   
        pi.FingerTemplateNumber=0;
        p.EditPerson(pi);	
        
        lcd.Clear(4);
	lcd.Clear(6);
        
        str = "";
				
	str += get_appropriate_message(FM_Finger, Farsi);
	
	str += ")";
	str+= Utility::ToString (pi.FingerTemplateNumber+1);
	str += "(";
	str += ":";

	lcd.Write(A_Right, 4, str);
        
        lcd.Write(A_Right, 6, get_appropriate_message(FM_TemplateCount, Farsi));
	lcd.Write(A_Left, 6, Utility::ToString(pi.FingerTemplateNumber));
                
        f.AddTemplate(pi.FingerID);
  }
/*  
  if (menu_index == MI_AddPerson )
  {
        is_OK = true;     
        f.CancelCommand();  
  }
*/			
}

void Menu::delete_template_fail (void)
{
        Finger f;
        Personal p;
        LCD lcd(Farsi);
        string str;
        
  if (menu_index == MI_EditPerson && is_f1_pressed == 1)
  {   
        pi.FingerTemplateNumber=0;
        p.EditPerson(pi);	
        
        lcd.Clear(4);
        lcd.Clear(6);
        
        str = "";
				
	str += get_appropriate_message(FM_Finger, Farsi);
	
	str += ")";
	str+= Utility::ToString (pi.FingerTemplateNumber+1);
	str += "(";
	str += ":";

	lcd.Write(A_Right, 4, str);
        
        lcd.Write(A_Right, 6, get_appropriate_message(FM_TemplateCount, Farsi));
	lcd.Write(A_Left, 6, Utility::ToString(pi.FingerTemplateNumber));
                
        f.EnrollByScan();
  }
}

void Menu::delete_all_templates_success (void)
{
		
}

void Menu::delete_all_templates_fail (void)
{
			
}

void Menu::valid_admin_finger (void)
{
	Finger f;
	BackLight b(FM_OnOffOn, S_VeryFast);
	
	//b.DoFade();
	
	menu_index = MI_MainMenu;
	highlighted_menu = get_menu_characteristic(menu_index).FirstChild;
	menu_handler();
	f.FreeScanMode(Off);
}

void Menu::delete_records_success (void)
{
	init_members_for_next_command();
	
	OS_StopTimer(&timer_second);
	
	if (menu_index == MI_DeleteTraffics)
	{						
		show_message(FM_DeleteAllTrafficsuccess, Farsi);
	}			
}

void Menu::card_is_repetitive (void)
{
        LCD lcd(Farsi);

        if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
	{
                OS_RetriggerTimer(&timer_special);

                lcd.Clear(2);
	
                lcd.Write(A_Right, 2, get_appropriate_message(FM_Card, Farsi));
                lcd.Write(A_Left, 2, get_appropriate_message(FM_Repetitive, Farsi));
        }
}

void Menu::card_is_unique (void)
{
        Mifare m;
        LCD lcd(Farsi);

        if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
	{
		lcd.Clear(2);

		lcd.Write(A_Right, 2, get_appropriate_message(FM_Card, Farsi));
		lcd.Write(A_Left, 2, Utility::ToString(m.GetLastCardID()));
		
		pi.CardID = m.GetLastCardID();		
		
		OS_RetriggerTimer(&timer_menu);		
	}
}

void Menu::card_is_unique_ex (void)
{
        MifareEx mx;
        LCD lcd(Farsi);

        if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
	{
		lcd.Clear(2);

		lcd.Write(A_Right, 2, get_appropriate_message(FM_Card, Farsi));
		lcd.Write(A_Left, 2, Utility::ToString(mx.GetLastCardID()));
		
		pi.CardID = mx.GetLastCardID();		
		
		OS_RetriggerTimer(&timer_menu);		
	}
}

void Menu::Err_Recieve_Finger(void)
{
        Finger f;
        if ((menu_index == MI_AddPerson || menu_index == MI_EditPerson) && sub_menu_index == 3)
        {
                if ( pi.FingerTemplateNumber == 0)
                   f.EnrollByScan ();
                else
                   f.AddTemplate(pi.FingerID);
        }
}

void Menu::cancel_success (void)
{
        Finger f;
        if(! is_OK)
        {
          //OS_Delay(100);
          f.DeleteTemplates(pi.FingerID);
          //is_OK = true; 
        }
        else if(Finger::is_Busy)
        {
           if ( pi.FingerTemplateNumber == 0)
                   f.EnrollByScan ();
                else
                   f.AddTemplate(pi.FingerID); 
                
            Finger::is_Busy = false;    
        }
}



	
