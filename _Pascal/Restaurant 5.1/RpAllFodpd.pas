unit RpAllFodpd;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  Qrctrls, quickrpt, ExtCtrls, Db, AdoDB, Globals, DateProc, DBS_Proc;

type
  TfrmRptAllFood = class(TForm)
    qryAllFood: TAdoQuery;
    qrptAllFood: TQuickRep;
    bndCoName: TQRBand;
    lblCoName: TQRLabel;
    ChildBand1: TQRChildBand;
    lblName: TQRLabel;
    QRLabel16: TQRLabel;
    lblDate: TQRLabel;
    QRSysData2: TQRSysData;
    QRLabel19: TQRLabel;
    QRLabel15: TQRLabel;
    QRSysData1: TQRSysData;
    lblRepDate: TQRLabel;
    grpDate: TQRGroup;
    bndDetail: TQRBand;
    QRDBText4: TQRDBText;
    grpRejim: TQRGroup;
    QRLabel2: TQRLabel;
    QRBand1: TQRBand;
    bndGuestFooter: TQRBand;
    exprPay: TQRExpr;
    exprNotPay: TQRExpr;
    SummaryBand1: TQRBand;
    QRLabel1: TQRLabel;
    QRShape13: TQRShape;
    QRExpr6: TQRExpr;
    QRExpr7: TQRExpr;
    QRShape1: TQRShape;
    QRShape2: TQRShape;
    QRShape3: TQRShape;
    QRShape4: TQRShape;
    QRShape5: TQRShape;
    QRShape6: TQRShape;
    QRShape7: TQRShape;
    QRShape8: TQRShape;
    QRShape9: TQRShape;
    QRShape10: TQRShape;
    QRShape11: TQRShape;
    QRShape12: TQRShape;
    exprAllPrc: TQRExpr;
    QRExpr9: TQRExpr;
    QRLabel3: TQRLabel;
    QRLabel4: TQRLabel;
    QRLabel5: TQRLabel;
    QRLabel6: TQRLabel;
    QRLabel7: TQRLabel;
    lblTotFish: TQRLabel;
    lblAllTotFish: TQRLabel;
    bndDate: TQRBand;
    exprSAllPrc: TQRExpr;
    QRShape14: TQRShape;
    exprSNotPay: TQRExpr;
    QRShape15: TQRShape;
    exprSPay: TQRExpr;
    QRShape16: TQRShape;
    lblTotDate: TQRLabel;
    QRShape17: TQRShape;
    QRLabel9: TQRLabel;
    ChildBand2: TQRChildBand;
    grpGuest: TQRGroup;
    bndSign: TQRChildBand;
    lblPrsType: TQRLabel;
    lblEndRep4: TQRLabel;
    lblEndRep3: TQRLabel;
    lblEndRep2: TQRLabel;
    lblEndRep1: TQRLabel;
    procedure FormCreate(Sender: TObject);
    procedure bndGuestFooterBeforePrint(Sender: TQRCustomBand;
      var PrintBand: Boolean);
    procedure SummaryBand1BeforePrint(Sender: TQRCustomBand;
      var PrintBand: Boolean);
    procedure bndDetailBeforePrint(Sender: TQRCustomBand;
      var PrintBand: Boolean);
    procedure bndGuestFooterAfterPrint(Sender: TQRCustomBand;
      BandPrinted: Boolean);
    procedure bndDateBeforePrint(Sender: TQRCustomBand;
      var PrintBand: Boolean);
    procedure grpDateBeforePrint(Sender: TQRCustomBand;
      var PrintBand: Boolean);
    procedure QRBand1BeforePrint(Sender: TQRCustomBand;
      var PrintBand: Boolean);
    procedure exprPayPrint(sender: TObject; var Value: String);
    procedure exprNotPayPrint(sender: TObject; var Value: String);
    procedure exprAllPrcPrint(sender: TObject; var Value: String);
    procedure exprSPayPrint(sender: TObject; var Value: String);
    procedure exprSNotPayPrint(sender: TObject; var Value: String);
    procedure exprSAllPrcPrint(sender: TObject; var Value: String);
    procedure QRExpr6Print(sender: TObject; var Value: String);
    procedure QRExpr7Print(sender: TObject; var Value: String);
    procedure QRExpr9Print(sender: TObject; var Value: String);
    procedure qrptAllFoodBeforePrint(Sender: TCustomQuickRep;
      var PrintReport: Boolean);
  private
    { Private declarations }
    LastFish, TotFish,
    AllTotFish, AllDateFish : Integer;

    procedure SetWinF;
  public
    { Public declarations }
  end;

implementation

{$R *.DFM}

procedure TfrmRptAllFood.SetWinF;
begin
end;

procedure TfrmRptAllFood.FormCreate(Sender: TObject);
begin
  SetWinF;
  qryAllFood.connection := frmDBS.DBConnection;
  lblCoName.Caption := g_options.CoName;
  if g_options.Shobeh <> '' then
    lblCoName.Caption := lblCoName.Caption + ' - ' + g_options.Shobeh;
  lblDate.Caption := CurrentDate;

  bndDetail.Enabled := True;
  bndDetail.Height := 0;
  grpGuest.Enabled := True;
  grpGuest.Height := 0;
end;

procedure TfrmRptAllFood.QRBand1BeforePrint(Sender: TQRCustomBand;
  var PrintBand: Boolean);
begin
  AllTotFish := 0;
  AllDateFish := 0;
end;

procedure TfrmRptAllFood.bndDetailBeforePrint(Sender: TQRCustomBand;
  var PrintBand: Boolean);
begin
  if LastFish <> qryAllFood.FieldByName('FF_FishNo').AsInteger then
  begin
    LastFish := qryAllFood.FieldByName('FF_FishNo').AsInteger;
    Inc(TotFish);
    Inc(AllDateFish);
  end;
end;

procedure TfrmRptAllFood.bndGuestFooterAfterPrint(Sender: TQRCustomBand;
  BandPrinted: Boolean);
begin
  AllTotFish := AllTotFish + TotFish;
  LastFish := 0;
  TotFish := 0;
end;

procedure TfrmRptAllFood.bndDateBeforePrint(Sender: TQRCustomBand;
  var PrintBand: Boolean);
begin
  lblTotDate.Caption := IntToStr(AllDateFish);
end;

procedure TfrmRptAllFood.grpDateBeforePrint(Sender: TQRCustomBand;
  var PrintBand: Boolean);
begin
  AllDateFish := 0;
end;

procedure TfrmRptAllFood.bndGuestFooterBeforePrint(Sender: TQRCustomBand;
  var PrintBand: Boolean);
begin
  lblTotFish.Caption := IntToStr(TotFish);

  if not qryAllFood.FieldByName('P_Rejim').AsBoolean and
     not qryAllFood.FieldByName('P_IsGuest').AsBoolean then
    lblPrsType.Caption := ('����� ����')
  else if qryAllFood.FieldByName('P_Rejim').AsBoolean and
          not qryAllFood.FieldByName('P_IsGuest').AsBoolean then
    lblPrsType.Caption := ('����� ю���')
  else if not qryAllFood.FieldByName('P_Rejim').AsBoolean and
          qryAllFood.FieldByName('P_IsGuest').AsBoolean then
    lblPrsType.Caption := ('������ ����')
    {lblPrsType.Caption := ('������')}
  else
    lblPrsType.Caption := ('������ ю���');
end;

procedure TfrmRptAllFood.SummaryBand1BeforePrint(Sender: TQRCustomBand;
  var PrintBand: Boolean);
begin
  lblAllTotFish.Caption := IntToStr(AllTotFish);
end;

procedure TfrmRptAllFood.exprPayPrint(sender: TObject; var Value: String);
begin
  Value := CurrencyFormat(Value);
end;

procedure TfrmRptAllFood.exprNotPayPrint(sender: TObject;
  var Value: String);
begin
  Value := CurrencyFormat(Value);
end;

procedure TfrmRptAllFood.exprAllPrcPrint(sender: TObject;
  var Value: String);
begin
  Value := CurrencyFormat(Value);
end;

procedure TfrmRptAllFood.exprSPayPrint(sender: TObject; var Value: String);
begin
  Value := CurrencyFormat(Value);
end;

procedure TfrmRptAllFood.exprSNotPayPrint(sender: TObject;
  var Value: String);
begin
  Value := CurrencyFormat(Value);
end;

procedure TfrmRptAllFood.exprSAllPrcPrint(sender: TObject;
  var Value: String);
begin
  Value := CurrencyFormat(Value);
end;

procedure TfrmRptAllFood.QRExpr6Print(sender: TObject; var Value: String);
begin
  Value := CurrencyFormat(Value);
end;

procedure TfrmRptAllFood.QRExpr7Print(sender: TObject; var Value: String);
begin
  Value := CurrencyFormat(Value);
end;

procedure TfrmRptAllFood.QRExpr9Print(sender: TObject; var Value: String);
begin
  Value := CurrencyFormat(Value);
end;

procedure TfrmRptAllFood.qrptAllFoodBeforePrint(Sender: TCustomQuickRep;
  var PrintReport: Boolean);
begin
  lblCoName.Font.Name := RptTitleFont;
  lblCoName.Font.Size := RptTitleFntSize;
  lblName.Font.Name := RptTitleFont;
  lblName.Font.Size := RptTitleFntSize;
  qrptAllFood.Font.Name := RptFont;
  qrptAllFood.Font.Size := RptFntSize;
end;

end.
