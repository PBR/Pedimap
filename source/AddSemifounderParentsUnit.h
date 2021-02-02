//---------------------------------------------------------------------------

#ifndef AddSemifounderParentsUnitH
#define AddSemifounderParentsUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TAddSemifounderParentsForm : public TForm
{
__published:	// IDE-managed Components
 TLabel *Label1;
 TEdit *AlleleEd;
 TLabel *Label2;
 TLabel *Label3;
 TButton *OKbtn;
 TButton *Cancelbtn;
 TLabel *Label4;
 TEdit *FilenameEd;
 TButton *Browsebtn;
 TCheckBox *Importcbx;
 TSaveDialog *SaveDialog1;
 void __fastcall BrowsebtnClick(TObject *Sender);
 void __fastcall OKbtnClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
 __fastcall TAddSemifounderParentsForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TAddSemifounderParentsForm *AddSemifounderParentsForm;
//---------------------------------------------------------------------------
#endif
bool TestMissingAlleleString();
void AddSemifounderParents(AnsiString NewFileName, AnsiString sfpAllele);
