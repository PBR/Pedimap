//---------------------------------------------------------------------------

#ifndef RenameUnitH
#define RenameUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TRenameForm : public TForm
{
__published:	// IDE-managed Components
 TEdit *RenameEd;
 TButton *OKbtn;
 TButton *Cancelbtn;
private:	// User declarations
public:		// User declarations
 __fastcall TRenameForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TRenameForm *RenameForm;
//---------------------------------------------------------------------------
#endif
