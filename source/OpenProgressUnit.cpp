//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "OpenProgressUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TOpenProgressForm *OpenProgressForm;
//---------------------------------------------------------------------------
__fastcall TOpenProgressForm::TOpenProgressForm(TComponent* Owner)
 : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TOpenProgressForm::StopBtnClick(TObject *Sender)
{
  StopPressed = true;
  Hide(); 
}
//---------------------------------------------------------------------------
