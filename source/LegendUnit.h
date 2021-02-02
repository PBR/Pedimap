#include "PCH.h"
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
#ifndef LegendUnitH
#define LegendUnitH
//---------------------------------------------------------------------------
#include "DataUnit.h"
//---------------------------------------------------------------------------
class TLegendForm : public TForm
{
__published:	// IDE-managed Components
        TPageControl *PageControl1;
        TTabSheet *ContinuousTraitColSheet;
        TTabSheet *LGMapSheet;
        TTabSheet *IBDColSheet;
        TListBox *IBDColorListbox;
        TTabSheet *MarkerSheet;
        TTabSheet *DiscreteTraitColSheet;
        TListBox *DiscrColListbox;
        TPanel *DiscrTraitPanel;
        TPanel *LGmapPanel;
        TListBox *LGmapLbox;
 TListBox *MarkerLbox;
        TScrollBox *ContColScrollBox;
        TLabel *ContTraitLbl;
        TLabel *MaxVallbl;
        TPaintBox *ContColPaintBox;
        TLabel *MinVallbl;
        TShape *MVshape;
        TLabel *LabelMV;
        void __fastcall IBDColorListboxDrawItem(TWinControl *Control,
          int Index, TRect &Rect, TOwnerDrawState State);
        void __fastcall LGmapLboxDrawItem(TWinControl *Control, int Index,
          TRect &Rect, TOwnerDrawState State);
        void __fastcall MarkerLboxDrawItem(TWinControl *Control, int Index,
          TRect &Rect, TOwnerDrawState State);
        void __fastcall ContColPaintBoxPaint(TObject *Sender);
        void __fastcall DiscrColListboxDrawItem(TWinControl *Control,
          int Index, TRect &Rect, TOwnerDrawState State);
private:	// User declarations
public:		// User declarations
        __fastcall TLegendForm(TComponent* Owner);
        int LocnameWidth, LGnameWidth, PositionWidth;
        bool LGstart; //is the start beyond the first locus?
        int *IBDcolIndex;
        TColor HiCol, LoCol, MVCol;
};
//---------------------------------------------------------------------------
extern PACKAGE TLegendForm *LegendForm;
//---------------------------------------------------------------------------
#endif
