#include "PCH.h"
#include <Classes.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>
#pragma hdrstop
//---------------------------------------------------------------------------

#ifndef ContinuousColorUnitH
#define ContinuousColorUnitH
//---------------------------------------------------------------------------
class TContinuousColorForm : public TForm
{
__published:	// IDE-managed Components
        TButton *HiColbtn;
        TButton *LowColbtn;
        TButton *MVColbtn;
        TPaintBox *PaintBox1;
        TButton *OKbtn;
        TButton *Cancelbtn;
        TColorDialog *ColorDialog1;
        void __fastcall PaintBox1Paint(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall HiColbtnClick(TObject *Sender);
        void __fastcall LowColbtnClick(TObject *Sender);
        void __fastcall MVColbtnClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TContinuousColorForm(TComponent* Owner);
        TColor HiCol, LoCol, MVCol;
};
//---------------------------------------------------------------------------
extern PACKAGE TContinuousColorForm *ContinuousColorForm;
//---------------------------------------------------------------------------
#endif
