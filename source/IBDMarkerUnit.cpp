//---------------------------------------------------------------------------
#include "PCH.h"
//#include <vcl.h>
#pragma hdrstop

#include "IBDMarkerUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TIBDMarkerForm *IBDMarkerForm;
//---------------------------------------------------------------------------
__fastcall TIBDMarkerForm::TIBDMarkerForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
/* 13-02-2010: deleted all; there seems to be no problem
     if no markers are selectedvoid __fastcall TIBDMarkerForm::FormCloseQuery(TObject *Sender,
      bool &CanClose)
{
  int i,n;
  if (ModalResult==mrCancel) CanClose=true;
  else
  { n=0;
    for (i=0; i<LG->LocCount; i++)
      if (IBDmrkLview->Items->Item[i]->Checked) n++;
    CanClose = AllMrkRbtn->Checked || n>0;

    if (!CanClose)
      ShowMessage("At least one marker must be selected!");
  }
}

  //---------------------------------------------------------------------------

void __fastcall TIBDMarkerForm::IBDmrkLviewChange(TObject *Sender,
      TListItem *Item, TItemChange Change)
{ SelMrkRbtn->Checked=true; }
//---------------------------------------------------------------------------
*/
void __fastcall TIBDMarkerForm::CheckAllMarkersbtnClick(TObject *Sender)
{ int i;
  for (i=0; i<IBDmrkLview->Items->Count; i++)
        IBDmrkLview->Items->Item[i]->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TIBDMarkerForm::ClearAllMarkersbtnClick(TObject *Sender)
{ int i;
  for (i=0; i<IBDmrkLview->Items->Count; i++)
        IBDmrkLview->Items->Item[i]->Checked = false;
}
//---------------------------------------------------------------------------

