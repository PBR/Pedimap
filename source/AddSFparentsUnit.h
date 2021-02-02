//---------------------------------------------------------------------------

#ifndef AddSFparentsUnitH
#define AddSFparentsUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TAddSFparentsForm : public TForm
{
__published:	// IDE-managed Components
 TLabel *Label4;
 TButton *OKbtn;
 TButton *Cancelbtn;
 TEdit *FilenameEd;
 TButton *Browsebtn;
 TCheckBox *Importcbx;
private:	// User declarations
public:		// User declarations
 __fastcall TAddSFparentsForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TAddSFparentsForm *AddSFparentsForm;
//---------------------------------------------------------------------------
#endif

bool writePedimapDatafile(AnsiString filename, bool withIBD);
void AddSFparents(AnsiString NewFileName);
