object frmPrnParam: TfrmPrnParam
  Left = 201
  Top = 107
  HelpContext = 109
  BiDiMode = bdRightToLeft
  BorderStyle = bsDialog
  Caption = '����� ��������� �ǁ��'
  ClientHeight = 177
  ClientWidth = 304
  Color = clBtnFace
  Font.Charset = ARABIC_CHARSET
  Font.Color = clWindowText
  Font.Height = -13
  Font.Name = 'Arabic Transparent'
  Font.Style = []
  OldCreateOrder = False
  ParentBiDiMode = False
  Position = poScreenCenter
  Scaled = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 15
  object cmdOK: TBitBtn
    Left = 8
    Top = 16
    Width = 89
    Height = 25
    Caption = '&���'
    ModalResult = 1
    TabOrder = 2
    OnClick = cmdOKClick
    Glyph.Data = {
      DE010000424DDE01000000000000760000002800000024000000120000000100
      0400000000006801000000000000000000001000000000000000000000000000
      80000080000000808000800000008000800080800000C0C0C000808080000000
      FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
      3333333333333333333333330000333333333333333333333333F33333333333
      00003333344333333333333333388F3333333333000033334224333333333333
      338338F3333333330000333422224333333333333833338F3333333300003342
      222224333333333383333338F3333333000034222A22224333333338F338F333
      8F33333300003222A3A2224333333338F3838F338F33333300003A2A333A2224
      33333338F83338F338F33333000033A33333A222433333338333338F338F3333
      0000333333333A222433333333333338F338F33300003333333333A222433333
      333333338F338F33000033333333333A222433333333333338F338F300003333
      33333333A222433333333333338F338F00003333333333333A22433333333333
      3338F38F000033333333333333A223333333333333338F830000333333333333
      333A333333333333333338330000333333333333333333333333333333333333
      0000}
    NumGlyphs = 2
  end
  object cmdHelp: TBitBtn
    Left = 8
    Top = 112
    Width = 89
    Height = 25
    Caption = '��&����'
    TabOrder = 3
    Kind = bkHelp
  end
  object cmdCancel: TBitBtn
    Left = 8
    Top = 144
    Width = 89
    Height = 25
    Caption = '&������'
    TabOrder = 4
    Kind = bkCancel
  end
  object grpPrn: TGroupBox
    Left = 104
    Top = 8
    Width = 193
    Height = 161
    TabOrder = 1
    object lbl2: TLabel
      Left = 111
      Top = 96
      Width = 74
      Height = 15
      Caption = '����� &���� ����'
      FocusControl = txtLineFeed
    end
    object lbl1: TLabel
      Left = 128
      Top = 24
      Width = 54
      Height = 15
      Caption = '��� �ǁ��'
    end
    object txtLineFeed: TEdit
      Left = 48
      Top = 96
      Width = 49
      Height = 23
      BiDiMode = bdLeftToRight
      ParentBiDiMode = False
      TabOrder = 1
      OnKeyPress = chkPrintFishKeyPress
    end
    object cmbPrinters: TComboBox
      Left = 8
      Top = 48
      Width = 177
      Height = 23
      Style = csDropDownList
      BiDiMode = bdLeftToRight
      ItemHeight = 15
      ParentBiDiMode = False
      TabOrder = 0
      OnKeyPress = chkPrintFishKeyPress
    end
    object chkAllFihPrc: TCheckBox
      Left = 56
      Top = 136
      Width = 129
      Height = 17
      Caption = '�ǁ ���� ���� �����'
      TabOrder = 2
      Visible = False
    end
  end
  object chkPrintFish: TCheckBox
    Left = 168
    Top = 8
    Width = 65
    Height = 17
    Caption = '�ǁ &���'
    TabOrder = 0
    OnClick = chkPrintFishClick
    OnKeyPress = chkPrintFishKeyPress
  end
end
