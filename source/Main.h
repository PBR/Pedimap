#include "PCH.h"
#include <ActnList.hpp>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Forms.hpp>
#include <ImgList.hpp>
#include <Menus.hpp>
#include <StdActns.hpp>
#include <StdCtrls.hpp>
#include <ToolWin.hpp>
#pragma hdrstop
//---------------------------------------------------------------------------
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include "DataUnit.h"
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------



class TMainForm : public TForm
{
__published:	// IDE-managed Components
   TMainMenu *MainMenu1;
   TToolBar *ToolBar1;
   TPanel *LeftPanel;
   TPanel *LeftTopPanel;
   TLabel *SubpopLabel;
   TSplitter *Splitter1;
   TMemo *InfoMemo;
   TPanel *LeftMidPanel;
   TLabel *Label1;
   TActionList *ActionList1;
   TAction *ToolsViewOptions;
   TAction *ToolsCreateSubpop;
   TAction *ToolsCreateView;
   TMenuItem *FileMenu;
   TMenuItem *Openproject1;
   TMenuItem *Saveproject1;
   TMenuItem *SaveprojectAs1;
   TMenuItem *N1;
   TMenuItem *Exit1;
   TMenuItem *Import1;
   TMenuItem *N2;
 TMenuItem *ToolsMenu;
   TMenuItem *Options1;
   TMenuItem *CreateView1;
   TMenuItem *CreateSubpop1;
   TToolButton *btnOpen;
   TToolButton *btnSave;
   TToolButton *btnImport;
   TImageList *ImageList1;
   TToolButton *btnSepar1;
   TToolButton *btnOptions;
   TSplitter *Splitter2;
   TPanel *MainPanel;
   TTabControl *MainTabctrl;
   TListView *MainListview;
   TScrollBox *ViewScrollbox;
   TPanel *PagePanel;
   TImage *ViewImage;
   TPrintDlg *FilePrint;
   TAction *ToolsSelectRelatives;
   TTreeView *PopTreeView;
   TPopupMenu *PopulationMenu;
   TAction *PopDelete;
   TAction *PopInfo;
   TMenuItem *Delete1;
   TMenuItem *SelectionInfo1;
   TAction *FileImport;
   TAction *FileOpen;
   TAction *FileSave;
   TAction *FileSaveAs;
   TOpenDialog *OpenDialog;
   TSaveDialog *SaveDialog;
   TAction *FileClear;
   TMenuItem *FileClear1;
   TToolButton *ToolButton1;
   TLabel *Label2;
   TComboBox *ZoomCombx;
   TToolButton *btnCreateView;
   TToolButton *btnSelRelatives;
   TToolButton *ToolButton2;
   TToolButton *btnCreateSubpop;
   TAction *ChartCopy;
   TAction *ChartSaveAs;
 TMenuItem *ExportMenu;
   TMenuItem *Copychart1;
   TMenuItem *ChartSaveas1;
   TMenuItem *N3;
   TMenuItem *N4;
   TAction *ListCopy;
   TAction *ListSaveAs;
   TMenuItem *Copylist1;
   TMenuItem *Savelistas1;
   TAction *PopExportAllData;
   TMenuItem *N5;
        TMenuItem *Exportsorteddata;
   TPopupMenu *ViewTabMenu;
   TAction *ViewTabDelete;
   TAction *ViewTabRename;
   TMenuItem *ViewTabRename1;
   TMenuItem *DeleteView1;
   TAction *ViewTabCopyOptions;
   TAction *ViewTabPasteOptions;
   TMenuItem *CopyViewoptions1;
   TMenuItem *PasteViewoptions1;
   TMenuItem *N6;
   TAction *PopViewsSource;
   TAction *PopAddViews;
   TMenuItem *Addviewsfromsource1;
   TAction *FileExit;
   TAction *PopExportPopData;
        TMenuItem *ExportSubpopulationdata1;
        TMenuItem *Exportsubpopulationdata2;
        TAction *ToolsShowLegend;
        TToolButton *btnShowLegend;
        TToolButton *ToolButton4;
        TMenuItem *N7;
        TMenuItem *ToolsShowLegend1;
        TAction *PopSortAlpha;
        TMenuItem *Sortsubpopulation1;
 TAction *PopSortOrig;
 TMenuItem *Alphabetically1;
 TMenuItem *Originalorder1;
        TImageList *FlagImageList;
 TMenuItem *MRUbreak;
        TAction *PopCombinePops;
        TMenuItem *Combinesubpops1;
        TAction *HelpAbout;
        TMenuItem *H1;
        TMenuItem *About1;
 TMenuItem *Manual1;
 TAction *HelpManual;
 TAction *ToolsImputeSFparentAlleles;
 TMenuItem *N9;
 TMenuItem *ImputeSFparentAlleles1;
 TAction *ToolsAddSFparents;
 TMenuItem *Addsemifounderparents1;
   // void __fastcall FileImportAccept(TObject *Sender);
   void __fastcall ToolsViewOptionsExecute(TObject *Sender);
   void __fastcall ViewScrollboxResize(TObject *Sender);
   void __fastcall FilePrintAccept(TObject *Sender);
   void __fastcall ViewImageMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
   void __fastcall ToolsCreateViewExecute(TObject *Sender);
   void __fastcall MainTabctrlChange(TObject *Sender);
   void __fastcall ViewImageMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
   void __fastcall ViewScrollboxExit(TObject *Sender);
   void __fastcall ToolsCreateSubpopExecute(TObject *Sender);
   //void __fastcall PopListboxClick(TObject *Sender);
   void __fastcall MainListviewDataI(TObject *Sender, TListItem *Item);
   //void __fastcall MainListviewChange(TObject *Sender, TListItem *Item,TItemChange Change);
   //void __fastcall MainListviewSelectItem(TObject *Sender,TListItem *Item, bool Selected);
   //void __fastcall MainListviewMouseUp(TObject *Sender,TMouseButton Button, TShiftState Shift, int X, int Y);
   void __fastcall MainListviewEnter(TObject *Sender);
   void __fastcall MainListviewExit(TObject *Sender);
   void __fastcall MainTabctrlChanging(TObject *Sender, bool &AllowChange);
   void __fastcall ToolsSelectRelativesExecute(TObject *Sender);
   void __fastcall MainListviewColumnClick(TObject *Sender,
          TListColumn *Column);
   //void __fastcall FileOpenAccept(TObject *Sender);
   void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
   void __fastcall MainListviewSelectItem(TObject *Sender,
          TListItem *Item, bool Selected);
   void __fastcall PopTreeViewDeletion(TObject *Sender,
          TTreeNode *Node);
   //void __fastcall PopTreeViewClick(TObject *Sender);
   void __fastcall PopDeleteExecute(TObject *Sender);
   void __fastcall PopInfoExecute(TObject *Sender);
   //void __fastcall FileSaveExecute(TObject *Sender);
   //void __fastcall FileSaveAsBeforeExecute(TObject *Sender);
   void __fastcall FileImportExecute(TObject *Sender);
   void __fastcall FileOpenExecute(TObject *Sender);
   void __fastcall FileSaveExecute(TObject *Sender);
   void __fastcall FileSaveAsExecute(TObject *Sender);
   void __fastcall FileClearExecute(TObject *Sender);
   void __fastcall ZoomCombxChange(TObject *Sender);
   void __fastcall ChartCopyExecute(TObject *Sender);
   void __fastcall ChartSaveAsExecute(TObject *Sender);
   void __fastcall FormShow(TObject *Sender);
   void __fastcall ListSaveAsExecute(TObject *Sender);
   void __fastcall ListCopyExecute(TObject *Sender);
   void __fastcall PopExportAllDataExecute(TObject *Sender);
   void __fastcall MainTabctrlMouseUp(TObject *Sender, TMouseButton Button,
            TShiftState Shift, int X, int Y);
   void __fastcall ViewTabDeleteExecute(TObject *Sender);
   void __fastcall ViewTabRenameExecute(TObject *Sender);
   void __fastcall ViewTabMenuPopup(TObject *Sender);
   void __fastcall ViewTabCopyOptionsExecute(TObject *Sender);
   void __fastcall ViewTabPasteOptionsExecute(TObject *Sender);
   void __fastcall PopulationMenuPopup(TObject *Sender);
   void __fastcall PopViewsSourceExecute(TObject *Sender);
   void __fastcall PopAddViewsExecute(TObject *Sender);
   void __fastcall FileExitExecute(TObject *Sender);
   void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
   void __fastcall PopExportPopDataExecute(TObject *Sender);
   void __fastcall ToolsShowLegendExecute(TObject *Sender);
   void __fastcall PopSortAlphaExecute(TObject *Sender);
   void __fastcall PopSortOrigExecute(TObject *Sender);
   void __fastcall PopTreeViewMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
   void __fastcall PopTreeViewChange(TObject *Sender,
          TTreeNode *Node);
   void __fastcall LeftPanelResize(TObject *Sender);
   void __fastcall FileMenuClick(TObject *Sender);
        void __fastcall PopCombinePopsExecute(TObject *Sender);
        void __fastcall HelpAboutExecute(TObject *Sender);
 void __fastcall HelpManualExecute(TObject *Sender);
 void __fastcall ToolsImputeSFparentAllelesExecute(TObject *Sender);
 void __fastcall ToolsAddSFparentsExecute(TObject *Sender);
private:	// User declarations
   bool SettingScrollbarRange, ClearingTabs;
   TWndMethod ScrollBoxOldWndProc;
   #ifdef VISAREA
     Graphics::TBitmap *VisArea;
     void DrawVisArea(void);
   #else
     //Graphics::TMetafile *ZoomMF;
     Graphics::TBitmap *ZoomBM;
   #endif
   void __fastcall ScrollBoxNewWndProc(TMessage &Message);
   void __fastcall MRUClick(TObject *Sender);
   //TLicenseInfo GetLicenseInfo(void);

public:		// User declarations
   //TList *IndShapeList; //all shapes drawn behind ViewImage as highlights; some are NULL
   //TShape *FocusShape;
   int ScrollboxBorders, //difference between Width and ClientWidth with no scrollbars
       ScrollboxLastWidth,
       ScrollboxLastHeight;
   __fastcall TMainForm(TComponent* Owner);
   void ShowCurrentView(TViewPop *VP);
   void SetTabs(void);
   void ShowIndivList(void);
   void ShowCrossingList(void);
   //void DeleteAllShapes(void);
   //void FillIndShapeList(void);
   //void DeleteIndShape(int VPI);
   //TShape* CreateIndShape(int VPI, TRecType RT);
   //TShape* CreateCrossShape(TCrossing* Cro);
   //void SetShapeRect(int VPI, TRecType RT, TShape *Sh);
   //void AddIndShape(int VPI, TRecType RT); //calls CreateIndShape and adds to ShapeList
   void DataListWidths(void); //calculates the width of IndivData columns in MainListView
   void WriteNode(TTreeNode *Node, TStringList *SL);
   TTreeNode* FindNode(int VPnum);
   void ReadViewData(TViewPop *VP, int OldIBDAlleleCount, TStringList *SL);
   void ReadViewpopData(int &r, TStringList *SL, int OldIBDAlleles);
   void SaveProject(AnsiString SaveDir, AnsiString SaveName);
   void StartProject(void);
   bool ImportDataFile(AnsiString FullFileName);
   bool OpenProjectFile(AnsiString FileName, bool ReadData);
   void ExportData(TViewPop *VP);
   void FillLegend(TView *VW);
   void DoFileOpen(const AnsiString FName);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------

const IndividualsTab=0, //Tabindex of Individuals page
      //CrossingsTab=1, //Tabindex of Crossingss page, not implemented
      OverviewTab=1;    //Tabindex of Overview page

void UpdateMRUfiles(const AnsiString Fname);

bool TestLicense(void);
void ShowUnlicensedMessage(void);
AnsiString GetDataFromCommandline(void);
void CommandlineProcessing(AnsiString datafile);

#endif

