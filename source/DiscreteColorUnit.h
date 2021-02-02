#include "PCH.h"
#include <Classes.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <StdCtrls.hpp>
#pragma hdrstop
//---------------------------------------------------------------------------

#ifndef DiscreteColorUnitH
#define DiscreteColorUnitH
//---------------------------------------------------------------------------
#include "DataUnit.h"
//---------------------------------------------------------------------------
class TDiscreteColorForm : public TForm
{
__published:	// IDE-managed Components
        TListBox *ColorListbox;
        TButton *Currentbtn;
        TButton *Allbtn;
        TButton *OKbtn;
        TButton *Cancelbtn;
        TColorDialog *ColorDialog1;
        void __fastcall ColorListboxDrawItem(TWinControl *Control,
          int Index, TRect &Rect, TOwnerDrawState State);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall FormDestroy(TObject *Sender);
        void __fastcall CurrentbtnClick(TObject *Sender);
        void __fastcall AllbtnClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TDiscreteColorForm(TComponent* Owner);
        TColor *Col, MVCol;
        int ColCount;
        int InDat; //index to IndivData
        TDataType DataType;
        int TypeIndex;
        AnsiString *Labels;
        //void FillColors(int ColCount, TColor *Col, TColor LoCol, TColor HiCol, int InDat);
};
//---------------------------------------------------------------------------
extern PACKAGE TDiscreteColorForm *DiscreteColorForm;
//---------------------------------------------------------------------------
#endif
