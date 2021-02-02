//---------------------------------------------------------------------------

#ifndef AboutboxUnitH
#define AboutboxUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TAboutboxForm : public TForm
{
__published:	// IDE-managed Components
        TPanel *SoftwarePanel;
        TLabel *LicensedtoLbl;
        TLabel *NameLbl;
        TLabel *AffiliationLbl;
        TLabel *SoftwareLbl;
        TLabel *AuthorLbl;
        TLabel *CopyrightLbl;
        TPanel *ButtonPanel;
        TLabel *PLbl;
        TPanel *OkPanel;
        TLabel *ExpiryLbl;
        TLabel *Website1Lbl;
        TLabel *Website2Lbl;
        void __fastcall OkPanelClick(TObject *Sender);
 void __fastcall Website1LblClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TAboutboxForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TAboutboxForm *AboutboxForm;
//---------------------------------------------------------------------------
#endif
