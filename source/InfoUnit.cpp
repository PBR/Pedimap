 //---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "InfoUnit.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TInfoForm *InfoForm;
//---------------------------------------------------------------------------
__fastcall TInfoForm::TInfoForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TInfoForm::FormShow(TObject *Sender)
{ NotesMemo->SetFocus(); }
//---------------------------------------------------------------------------

void __fastcall TInfoForm::FlagCombxDrawItem(TWinControl *Control,
      int Index, TRect &Rect, TOwnerDrawState State)
{ if (Index>-1 && Index<5)
   MainForm->FlagImageList->Draw(FlagCombx->Canvas,Rect.Left+5,Rect.Top+1,Index,true);
}
//---------------------------------------------------------------------------

