#include "PCH.h"
//#include <vcl.h>
#pragma hdrstop

#include "FontUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFontForm *FontForm;
//---------------------------------------------------------------------------
__fastcall TFontForm::TFontForm(TComponent* Owner)
        : TForm(Owner)
{ FontNameCbx->Items = Screen->Fonts;

}

void __fastcall TFontForm::FontNameCbxChange(TObject *Sender)
{ if (FontNameCbx->Items->IndexOf(FontNameCbx->Text)!=-1)
  { FontLbl->Font->Name=FontNameCbx->Text; }
} //TFontForm::FontNameCbxChange

void __fastcall TFontForm::FontSizeEdExit(TObject *Sender)
{ try
  { FontLbl->Font->Size=abs(StrToInt(FontSizeEd->Text));
      //always use positive font->size: height of characters themselves,
      //  without any internal leading space on top}
  }
  catch (...)
  { //illegal input; restore text to size of font:}
    FontSizeEd->Text=IntToStr(abs(FontLbl->Font->Size));
  }
} //TFontForm::FontSizeEdExit

void __fastcall TFontForm::FontSizeEdKeyPress(TObject *Sender, char &Key)
{ if (Key==*"\r") //Enter
  { Key=*"\0";
    FontSizeEdExit(Sender);
  }
} //FontSizeEdKeyPress

void __fastcall TFontForm::BoldBtnClick(TObject *Sender)
{ if ( FontLbl->Font->Style.Contains(fsBold))
  { FontLbl->Font->Style = FontLbl->Font->Style >> fsBold; }
  else  FontLbl->Font->Style = FontLbl->Font->Style << fsBold;
  BoldBtn->Down = FontLbl->Font->Style.Contains(fsBold);
  FontSizeEdExit(NULL);
} //BoldBtnClick

void __fastcall TFontForm::ItalBtnClick(TObject *Sender)
{ if ( FontLbl->Font->Style.Contains(fsItalic))
  { FontLbl->Font->Style = FontLbl->Font->Style >> fsItalic; }
  else  FontLbl->Font->Style = FontLbl->Font->Style << fsItalic;
  ItalBtn->Down = FontLbl->Font->Style.Contains(fsItalic);
  FontSizeEdExit(NULL);
} //ItalBtnClick

void __fastcall TFontForm::UnderBtnClick(TObject *Sender)
{ if ( FontLbl->Font->Style.Contains(fsUnderline))
  { FontLbl->Font->Style = FontLbl->Font->Style >> fsUnderline; }
  else  FontLbl->Font->Style = FontLbl->Font->Style << fsUnderline;
  UnderBtn->Down = FontLbl->Font->Style.Contains(fsUnderline);
  FontSizeEdExit(NULL);
} //UnderBtnClick

void __fastcall TFontForm::FormShow(TObject *Sender)
{ FontNameCbx->Text = FontLbl->Font->Name;
  FontSizeEd->Text = IntToStr(abs(FontLbl->Font->Size));
  BoldBtn->Down = FontLbl->Font->Style.Contains(fsBold);
  UnderBtn->Down = FontLbl->Font->Style.Contains(fsUnderline);
  ItalBtn->Down = FontLbl->Font->Style.Contains(fsItalic);
} //TFontForm::FormShow

void __fastcall TFontForm::FontSizeEdEnter(TObject *Sender)
{ FontSizeEdExit(NULL);
}

void __fastcall TFontForm::FormClose(TObject *Sender, TCloseAction &Action)
{ ActiveControl = FontSizeEd; //will ensure a FontSizeEd->OnEnter event after FormShow;
                              //necessary to set the Font of Fontlbl correctly
} //TFontForm::FormClose
//---------------------------------------------------------------------------

