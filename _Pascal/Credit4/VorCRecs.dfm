�
 TFRMVORCRECS 0�  TPF0TfrmVorCRecsfrmVorCRecsLeft Top� BiDiModebdRightToLeftBorderStylebsDialogCaption)   ليست وکلا/صاحبان صندوقClientHeight�ClientWidtheColor	clBtnFaceConstraints.MinHeightFont.CharsetDEFAULT_CHARSET
Font.ColorclWindowTextFont.Height�	Font.NameTahoma
Font.Style OldCreateOrderParentBiDiModePositionpoDesktopCenterScaledOnCloseQueryFormCloseQueryOnShowFormShowPixelsPerInch`
TextHeight TLabelLabel33LeftDTop Width<Height	AlignmenttaRightJustifyCaption   تصوير چهره  TLabelLabel37LeftBTop� Width>Height	AlignmenttaRightJustifyCaption   نمونه امضاء  TLabellblCKindLeft� Top Width�Height	AlignmenttaCenterAutoSizeCaption   صندوق انفرادي  TDBGrid
dbgPersonsLeft� TopWidth�Height�
DataSource
dtaPersonsOptionsdgTitlesdgIndicatordgColumnResize
dgColLines
dgRowLinesdgTabsdgConfirmDeletedgCancelOnExit ReadOnly	TabOrderTitleFont.CharsetDEFAULT_CHARSETTitleFont.ColorclWindowTextTitleFont.Height�TitleFont.NameTahomaTitleFont.Style VisibleColumnsExpanded	FieldName	np_FAMILYTitle.AlignmenttaCenterTitle.Caption   نام خانوادگيWidth� Visible	 Expanded	FieldNamenp_NAMETitle.AlignmenttaCenterTitle.Caption   F'EWidthnVisible	 	AlignmenttaCenterExpanded	FieldNamePermitTitle.AlignmenttaCenterTitle.Caption   مجوز استفادهWidthZVisible	 Expanded	FieldNamenp_IDNOTitle.AlignmenttaCenterTitle.Caption   ش.شناسنامهWidthXVisible	 Expanded	FieldName	np_FATHERTitle.AlignmenttaCenterTitle.Caption   نام پدرWidthnVisible	 	AlignmenttaCenterExpanded	FieldNamenp_BirthDateTitle.AlignmenttaCenterTitle.Caption   تاريخ تولدWidthPVisible	 Expanded	FieldNamenp_BirthPlaceTitle.AlignmenttaCenterTitle.Caption   محل صدورWidthnVisible	 	AlignmenttaRightJustifyExpanded	FieldName	np_MOBILETitle.AlignmenttaCenterTitle.Caption   تلفن همراهWidthjVisible	 	AlignmenttaRightJustifyExpanded	FieldName
np_HomeTelTitle.AlignmenttaCenterTitle.Caption   تلفن منزلWidthsVisible	 	AlignmenttaRightJustifyExpanded	FieldName
np_WorkTelTitle.AlignmenttaCenterTitle.Caption   تلفن محل کارWidthlVisible	    TDBGriddbgVakilLeft� TopWidth�Height�
DataSourcedtaVakilOptionsdgTitlesdgIndicatordgColumnResize
dgColLines
dgRowLinesdgTabsdgConfirmDeletedgCancelOnExit ReadOnly	TabOrderTitleFont.CharsetDEFAULT_CHARSETTitleFont.ColorclWindowTextTitleFont.Height�TitleFont.NameTahomaTitleFont.Style VisibleColumnsExpanded	FieldName
nP_VFAMILYTitle.AlignmenttaCenterTitle.Caption   نام خانوادگيWidth� Visible	 Expanded	FieldNamenP_VNAMETitle.AlignmenttaCenterTitle.Caption   F'EWidthnVisible	 Expanded	FieldNamenP_VIDNOTitle.AlignmenttaCenterTitle.Caption   ش.شناسنامهWidthXVisible	 	AlignmenttaCenterExpanded	FieldNamenP_VExpireDateTitle.AlignmenttaCenterTitle.Caption   تاريخ خاتمهWidthPVisible	 	AlignmenttaRightJustifyExpanded	FieldNamenP_VProxyIDTitle.AlignmenttaCenterTitle.Caption   شماره قراردادWidthZVisible	 Expanded	FieldName
nP_VFATHERTitle.AlignmenttaCenterTitle.Caption   نام پدرWidthnVisible	 Expanded	FieldNamenP_VBirthPlaceTitle.AlignmenttaCenterTitle.Caption   محل صدورWidthnVisible	    TPanel
pnlPictureLeftTopWidth� Height� 	AlignmenttaLeftJustify
BevelInner	bvLoweredFont.CharsetARABIC_CHARSET
Font.ColorclBlueFont.Height�	Font.NameArabic Transparent
Font.StylefsBold 
ParentFontTabOrder  TImage
imgPictureLeftTopWidth� Height� Stretch	   TPanelpnlSignPictureLeftTop Width� Height� 	AlignmenttaLeftJustify
BevelInner	bvLoweredFont.CharsetARABIC_CHARSET
Font.ColorclBlueFont.Height�	Font.NameArabic Transparent
Font.StylefsBold 
ParentFontTabOrder TImageimgSignPictureLeftTopWidth� Height� Stretch	   	TADOQueryadoVakilConnectionStringoProvider=SQLOLEDB.1;Integrated Security=SSPI;Persist Security Info=False;Initial Catalog=amanat_m;Data Source=.
CursorTypectStatic
Parameters SQL.Stringsselect * from nProxies"where nP_CustomerCode = '00000239'order by np_VProxi LeftTop  TDataSourcedtaVakilDataSetadoVakilOnDataChangedtaVakilDataChangeLeft8Top  	TADOQuery
adoPersonsConnectionStringoProvider=SQLOLEDB.1;Integrated Security=SSPI;Persist Security Info=False;Initial Catalog=amanat_m;Data Source=.
CursorTypectStatic
Parameters SQL.Stringsselect * ,case(np_IsMemberCommonBoxType3)   when 0 then 'ندارد'   when 1 then 'دارد'   else 'ندارد'end as Permitfrom nPersonswhere np_code = '00000303'%order by np_FAMILY, np_NAME, np_Proxi LeftTop8  TDataSource
dtaPersonsDataSet
adoPersonsOnDataChangedtaPersonsDataChangeLeft8Top8   