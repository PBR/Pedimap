//---------------------------------------------------------------------------

#ifndef SelectDlgUnitH
#define SelectDlgUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TSelectDlgForm : public TForm
{
__published:	// IDE-managed Components
        TLabel *Label1;
        TLabel *Label2;
        TCheckBox *ProgParentsCbx;
  TCheckBox *SibsCbx;
        TEdit *AncestorEd;
        TEdit *ProgenyEd;
        TButton *OKbtn;
        TButton *Cancelbtn;
  TUpDown *AncesterUpdown;
  TUpDown *ProgUpdown;
  TRadioButton *SelectRbtn;
  TRadioButton *DeselectRbtn;
 TRadioGroup *ParentlineRgrp;
private:	// User declarations
public:		// User declarations
        __fastcall TSelectDlgForm(TComponent* Owner);
        void SetParentlineRgrpItems(int FemaleParent);
};
//---------------------------------------------------------------------------
extern PACKAGE TSelectDlgForm *SelectDlgForm;
//---------------------------------------------------------------------------
#endif
