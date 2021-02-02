//---------------------------------------------------------------------------

#ifndef InfoUnitH
#define InfoUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TInfoForm : public TForm
{
__published:	// IDE-managed Components
        TPanel *TopPanel;
        TMemo *SelMemo;
        TSplitter *Splitter1;
        TPanel *Panel1;
        TMemo *NotesMemo;
        TPanel *Panel2;
        TButton *Closebtn;
        TComboBox *FlagCombx;
        TLabel *Label1;
        void __fastcall FormShow(TObject *Sender);
        void __fastcall FlagCombxDrawItem(TWinControl *Control, int Index,
          TRect &Rect, TOwnerDrawState State);
private:	// User declarations
public:		// User declarations
        __fastcall TInfoForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TInfoForm *InfoForm;
//---------------------------------------------------------------------------
#endif
