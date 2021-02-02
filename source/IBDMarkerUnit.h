//---------------------------------------------------------------------------
#include "PCH.h"
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#pragma hdrstop
//---------------------------------------------------------------------------
#ifndef IBDMarkerUnitH
#define IBDMarkerUnitH
//---------------------------------------------------------------------------
#include "DataUnit.h"
//---------------------------------------------------------------------------
class TIBDMarkerForm : public TForm
{
__published:	// IDE-managed Components
        TLabel *Label1;
        TEdit *LGnameEd;
        TListView *IBDmrkLview;
        TLabel *Label2;
        TButton *OKbtn;
        TButton *Cancelbtn;
        TButton *CheckAllMarkersbtn;
        TButton *ClearAllMarkersbtn;
        void __fastcall CheckAllMarkersbtnClick(TObject *Sender);
        void __fastcall ClearAllMarkersbtnClick(TObject *Sender);
        //void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
        //void __fastcall IBDmrkLviewChange(TObject *Sender, TListItem *Item, TItemChange Change);
private:	// User declarations
public:		// User declarations
  TLinkGrp *LG;
        __fastcall TIBDMarkerForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TIBDMarkerForm *IBDMarkerForm;
//---------------------------------------------------------------------------
#endif
