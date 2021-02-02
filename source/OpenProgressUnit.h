//---------------------------------------------------------------------------

#ifndef OpenProgressUnitH
#define OpenProgressUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TOpenProgressForm : public TForm
{
__published:	// IDE-managed Components
 TLabel *Label1;
 TLabel *Label2;
 TLabel *FileLbl;
 TLabel *ReadLbl1;
 TLabel *ReadLbl2;
 TProgressBar *ProgressBar1;
 TButton *StopBtn;
 void __fastcall StopBtnClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
 __fastcall TOpenProgressForm(TComponent* Owner);
 bool StopPressed;
};
//---------------------------------------------------------------------------
extern PACKAGE TOpenProgressForm *OpenProgressForm;
//---------------------------------------------------------------------------
#endif
