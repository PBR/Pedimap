//---------------------------------------------------------------------------

#ifndef DefaultsUnitH
#define DefaultsUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TDefaultsForm : public TForm
{
__published:	// IDE-managed Components
 TRadioGroup *DefaultsRgroup;
 TButton *OKbtn;
 TButton *Cancelbtn;
 TBevel *Bevel1;
private:	// User declarations
public:		// User declarations
 __fastcall TDefaultsForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TDefaultsForm *DefaultsForm;
//---------------------------------------------------------------------------
#endif
