//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "AboutboxUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAboutboxForm *AboutboxForm;
//---------------------------------------------------------------------------
__fastcall TAboutboxForm::TAboutboxForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TAboutboxForm::OkPanelClick(TObject *Sender)
//we use a panel instead of a button because a button's Color cannot be set
{ ModalResult=mrOk;
}
//---------------------------------------------------------------------------

void __fastcall TAboutboxForm::Website1LblClick(TObject *Sender)
{ ShellExecute(Application->Handle,
               "open",
               Website1Lbl->Caption.c_str(),
               NULL,
               NULL,
               SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

