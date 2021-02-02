//---------------------------------------------------------------------------

#ifndef ImputeSFparentAllelesUnitH
#define ImputeSFparentAllelesUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TImputeSFparentAllelesForm : public TForm
{
__published:	// IDE-managed Components
 TLabel *Label1;
 TEdit *AlleleEd;
 TLabel *Label2;
 TButton *OKbtn;
 TButton *Cancelbtn;
 TLabel *Label4;
 TEdit *FilenameEd;
 TButton *Browsebtn;
 TCheckBox *Importcbx;
 TSaveDialog *SaveDialog1;
 TLabel *Label5;
 TComboBox *ColorCodeCombx;
 void __fastcall BrowsebtnClick(TObject *Sender);
 void __fastcall OKbtnClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
 __fastcall TImputeSFparentAllelesForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TImputeSFparentAllelesForm *ImputeSFparentAllelesForm;
//---------------------------------------------------------------------------
#endif
bool TestMissingAlleleString();
void ImputeSFparentAlleles(AnsiString NewFileName, AnsiString sfpAllele, int colorCode);
