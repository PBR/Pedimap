#include "PCH.h"
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>
#include <CheckLst.hpp>
#pragma hdrstop
//---------------------------------------------------------------------------

#ifndef OptionsUnitH
#define OptionsUnitH
//---------------------------------------------------------------------------
#include <Forms.hpp>
#include "DataUnit.h"
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TOptionsForm : public TForm
{
__published:	// IDE-managed Components
  TPageControl *PageControl1;
  TPanel *ButtonPanel;
  TButton *OKbtn;
  TButton *Cancelbtn;
  TColorDialog *ColorDialog1;
  TTabSheet *IndivSheet;
  TButton *INameFontbtn;
  TTabSheet *PageSheet;
  TGroupBox *PaperSizeGrpbx;
  TLabel *Label4;
  TLabel *Label5;
  TComboBox *PaperSizeCbox;
  TEdit *PapWidthEd;
  TEdit *PapHeightEd;
  TRadioGroup *OrientRGroup;
  TGroupBox *MarginsGrpbx;
  TLabel *LeftLbl;
  TLabel *RightLbl;
  TLabel *TopLbl;
  TLabel *Bottom;
  TEdit *LeftMargEd;
  TEdit *RightMargEd;
  TEdit *TopMargEd;
  TEdit *BottomMargEd;
  TTabSheet *ColorsSheet;
  TTabSheet *IBDSheet;
  TLabel *LinkGrpLbl;
  TComboBox *LinkGrpCombx;
  TGroupBox *ProbRectGroupBox;
  TLabel *Label1;
  TLabel *Label2;
  TEdit *ProbWidthEd;
  TEdit *ProbHeightEd;
  TGroupBox *AlleleGroupBox;
  TLabel *AlleleColLbl;
  TLabel *Label3;
  TButton *AlleleColbtn;
  TComboBox *AlleleCombx;
  TButton *HomFounderBtn;
  TButton *HeterzFounderBtn;
  TTabSheet *MarkerSheet;
  TTabSheet *TraitSheet;
  TGroupBox *GroupBox1;
  TRadioButton *NameRbtn;
  TRadioButton *IBDprobRbtn;
  TRadioButton *MarkRbtn;
  TListView *MarkerLview;
  TButton *MarkFontbtn;
  TLabel *Label7;
  TComboBox *MarkColorCombx;
  TButton *CodeColbtn;
  TButton *CheckAllMarkersbtn;
  TButton *ClearAllMarkersbtn;
  TRadioButton *IBDallRbtn;
  TGroupBox *ProbAlleleGroupBox;
  TButton *AlleleFontbtn;
  TLabel *AlleleThresholdLbl;
  TEdit *BoldEd;
  TLabel *BoldLabel;
  TEdit *ItalEd;
  TLabel *ItalicLabel;
  TLabel *NormalLabel;
  TGroupBox *FillColorGrpBox;
  TRadioButton *FixedColbtn;
  TRadioButton *TraitColbtn;
  TComboBox *TraitCombx;
  TButton *SetFixedColBtn;
  TPanel *FixedColorPanel;
  TButton *SetTraitColbtn;
  TTabSheet *LayoutSheet;
  TGroupBox *BetwGenerGbox;
  TEdit *BetwGenerEd;
  TUpDown *BetwGenerUpDown;
  TLabel *Label9;
  TRadioGroup *PedOrientRGrp;
  TGroupBox *InGenerGrbox;
  TLabel *Label8;
  TEdit *BetwFamEd;
  TUpDown *BetwFamUpDown;
  TLabel *Label10;
  TLabel *Label11;
  TEdit *BetwSibEd;
  TUpDown *BetwSibUpDown;
  TLabel *Label12;
  TGroupBox *CossSymbGrbox;
  TCheckBox *ShowCrossCbx;
  TLabel *CrossSizeLbl;
  TEdit *CrossSizeEd;
  TButton *CrossColBtn;
  TButton *Par1ColBtn;
  TButton *Par2ColBtn;
  TButton *ParSelfedColBtn;
  TButton *PageColBtn;
  TPanel *PageColPanel;
  TPanel *CrossColPanel;
  TPanel *Par1ColPanel;
  TPanel *Par2ColPanel;
  TPanel *ParSelfedColPanel;
  TButton *Defaultbtn;
 TButton *AllSamebtn;
        TButton *SelectMrkbtn;
 TPanel *MrkOrderPanel;
 TLabel *Label6;
 TRadioButton *MrkPosRbtn;
 TRadioButton *MrkNameRbtn;
        TTabSheet *InfoSheet;
        TLabel *InfoTraitsLbl;
        TCheckListBox *InfoTraitChlist;
        TButton *InfAllbtn;
        TButton *InfNonebtn;
        TCheckBox *InfoSelChbx;
        TCheckBox *InfoParentsChbx;
  void __fastcall INameFontbtnClick(TObject *Sender);
  void __fastcall AlleleFontbtnClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall AlleleCombxDrawItem(TWinControl *Control,
                  int Index, TRect &Rect, TOwnerDrawState State);
  void __fastcall AlleleColbtnClick(TObject *Sender);
  void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
  void __fastcall ColorbtnClick(TObject *Sender);
  void __fastcall HomFounderBtnClick(TObject *Sender);
  void __fastcall HeterzFounderBtnClick(TObject *Sender);
  void __fastcall MarkColorCombxDrawItem(TWinControl *Control, int Index,
                  TRect &Rect, TOwnerDrawState State);
  void __fastcall CodeColbtnClick(TObject *Sender);
  void __fastcall CheckAllMarkersbtnClick(TObject *Sender);
  void __fastcall ClearAllMarkersbtnClick(TObject *Sender);
  void __fastcall IBackColbtnClick(TObject *Sender);
  void __fastcall SetTraitColbtnClick(TObject *Sender);
  void __fastcall DefaultbtnClick(TObject *Sender);
  void __fastcall IBDprobRbtnClick(TObject *Sender);
  void __fastcall IBDallRbtnClick(TObject *Sender);
  void __fastcall MarkRbtnClick(TObject *Sender);
 void __fastcall AllSamebtnClick(TObject *Sender);
        void __fastcall SelectMrkbtnClick(TObject *Sender);
        void __fastcall LinkGrpCombxChange(TObject *Sender);
 void __fastcall MrkPosRbtnClick(TObject *Sender);
        void __fastcall InfAllbtnClick(TObject *Sender);
        void __fastcall InfNonebtnClick(TObject *Sender);
private:	// User declarations
  void OptionsToForm(void);
  void GetSelectedLoci(void);
  bool FormToOptions(void);
  void CheckDiscreteColors(void);
  void ShowMarkerLview(void);
public:		// User declarations
  TViewOptions OwnOptions;
  __fastcall TOptionsForm(TComponent* Owner);
  void SetParentColorCaptions(int FemaleParent);
};


//---------------------------------------------------------------------------
extern PACKAGE TOptionsForm *OptionsForm;
//---------------------------------------------------------------------------
#endif
