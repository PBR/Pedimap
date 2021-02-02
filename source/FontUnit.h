#include "PCH.h"
#include <Buttons.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>
//#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------

#ifndef C:\1Roeland\Delphi\DelToCpp\FontUnitH
#define C:\1Roeland\Delphi\DelToCpp\FontUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TFontForm : public TForm
{
__published:    // IDE-managed Components
        TPanel *Panel2;
        TComboBox *FontNameCbx;
        TEdit *FontSizeEd;
        TSpeedButton *UnderBtn;
        TSpeedButton *ItalBtn;
        TSpeedButton *BoldBtn;
        TPanel *Panel1;
        TLabel *FontLbl;
        TPanel *Panel3;
        TBitBtn *BitBtn1;
        TBitBtn *BitBtn2;
        void __fastcall FontNameCbxChange(TObject *Sender);
        void __fastcall FontSizeEdExit(TObject *Sender);
        void __fastcall FontSizeEdKeyPress(TObject *Sender, char &Key);
        void __fastcall BoldBtnClick(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall FontSizeEdEnter(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall ItalBtnClick(TObject *Sender);
        void __fastcall UnderBtnClick(TObject *Sender);
private:        // User declarations
public:         // User declarations
        __fastcall TFontForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFontForm *FontForm;
//---------------------------------------------------------------------------
#endif
