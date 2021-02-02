#include "PCH.h"
//#include <vcl.h>
#pragma hdrstop

#include "Main.h"
#include "DataUnit.h"
#include "ParseUnit.h"
#include "OptionsUnit.h"
#include "SelectDlgUnit.h"
#include "InfoUnit.h"
#include "RenameUnit.h"
#include "LegendUnit.h"
#include "RegistryUnit.h"
#include "AboutboxUnit.h"
#include "AddSFparentsUnit.h"
#include "ImputeSFparentAllelesUnit.h"
#include <typeinfo.h>
#include <iostream>

//#include "ReadViewpopUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
TMainForm *MainForm;
//---------------------------------------------------------------------------

int NodesWritten; //used to limit number of ViewPops saved if unlicensed

bool MainListviewVisible = false;
//kludge! Normally the MainListview->Visible should be set to false at the
//start of TMainForm::ShowIndivList (because if visible the updating is
//very slow with large pedigrees), but not when it is called at the end
//of ToolsSelectRelativesExecute (because in that case the new selection
//is lost, if the selection is made with the IndivList open)

AnsiString __fastcall MyGetEnvironmentVariable(const AnsiString Name)
//because of problem with GetEnvironmentVariable
{
  int Len = GetEnvironmentVariable(Name.c_str(), NULL, 0);
  AnsiString Result;
  if (Len > 0)
  {
    Result.SetLength(Len - 1);
    GetEnvironmentVariable(Name.c_str(), Result.c_str(), Len);
  } return Result;
} //MyGetEnvironmentVariable

AnsiString GetFileFromCommandLine()
//Returns "" if no file specified, '*'+FName if file not found;
//FName is with full drive/path even if not given on commandline
{ int i;
  AnsiString s, Result;

   i=1; Result="";
   while ( Result=="" && i<=ParamCount() )
   { s=ParamStr(i);
     if (s[1]!='/')
       Result=ExpandFileName( s );
         //by expanding, even file NONAME_1.pmp will be different from unnamed new file
     i++;
   }
   return Result;
} //GetFileFromCommandLine

GetIndivDrawLimitFromCommandline() {
//sets IndivDrawLimit if valid command line option /DL= found
   int i=1;
   while ( (i<=ParamCount()) &&
           (LeftStr(ParamStr(i),4).UpperCase() != "/DL=") )
     i++;
   if (i<=ParamCount())
   try {
     IndivDrawLimit = StrToInt(ParamStr(i).SubString(5,ParamStr(i).Length()));
   }
   catch (...) {};
} //GetIndivDrawLimitFromCommandline


AnsiString CleanFilename(AnsiString s, bool CheckPeriod)
//replace problematic characters by "_"
{ int i;
  for (i=1; i<=s.Length(); i++)
    if (s[i]=='\\' || s[i]=='/' || s[i]==':' ||
        s[i]=='|' || s[i]=='?' || s[i]=='*' ||
        ( CheckPeriod && s[i]=='.') )
      s[i]='_';
  return s;
}

__fastcall TMainForm::TMainForm(TComponent* Owner)
  : TForm(Owner)
{ PPI = GetPPI();
  PrPPI = GetPrPPI(); if (PrPPI==0) PrPPI=PPI;
  LinePx = (63+PrPPI) / 127; //integer division; approx. 0.2 mm
  //Today=Now();
  ProgDir = ExtractFilePath(ParamStr(0));
  DataDir = ProgDir;
  TempDir=MyGetEnvironmentVariable("TMP");
  if (TempDir=="") TempDir=MyGetEnvironmentVariable("TEMP");
  if (TempDir=="") TempDir="C:\\";
  if (TempDir[TempDir.Length()] != "\\"[0])
     TempDir = TempDir + "\\";
  //WarningDate=EncodeDate(2010,12,31);

  //save old scrollbox scroll routine (will be called in new one):
  ScrollBoxOldWndProc = ViewScrollbox->WindowProc;
  //set new scrollbox scroll routine (calls old one):
  ViewScrollbox->WindowProc = ScrollBoxNewWndProc;
  //initialize other things:
  SettingScrollbarRange=false;
  #ifdef VISAREA
    VisArea = new Graphics::TBitmap;
    VisArea->Transparent=true;
  #else
    ZoomBM = new Graphics::TBitmap;
    ZoomBM->Transparent=true;
  #endif

  PagePanel->Width=0; PagePanel->Height=0;
  ViewImage->Width=0; ViewImage->Height=0;
  ViewScrollboxResize(NULL);
  ProjectSaved=true;
  FileClearExecute(NULL);

} //TNwMainForm::TNwMainForm
//---------------------------------------------------------------------------
void __fastcall TMainForm::ScrollBoxNewWndProc(TMessage &Message)
{ /*  if (Message.Msg == WM_VSCROLL || Message.Msg == WM_HSCROLL)
      {  here: things to do BEFORE scrolling, with
         ScrollBox1->HorzScrollBar->Position and
         ScrollBox1->VertScrollBar->Position reporting old position
      } */

  ScrollBoxOldWndProc(Message);
  //now both scrollbar positions are updated

  #ifdef VISAREA
  if (Message.Msg == WM_VSCROLL || Message.Msg == WM_HSCROLL)
  { /* here: things to do AFTER scrolling, with
         ScrollBox1->HorzScrollBar->Position and
         ScrollBox1->VertScrollBar->Position reporting new position  */
    DrawVisArea();   //GetDataClick(NULL);
    //PositionShapes();
  }
  #endif
} //TNwMainForm::ScrollBoxNewWndProc
//---------------------------------------------------------------------------

#ifdef VISAREA
void TMainForm::DrawVisArea(void)
{
/*NB VisArea nu Metafile gemaakt ipv Bitmap vw transparantie,
maar wordt nu toch niet de complete chart bitmap gegenereerd??
Shapes nog niet te zien -> worden ze wel goed gemaakt?*/
  //VisArea=TBitmap of whole visible area of the (zoomed) chart
  //where chart is the whole picture.
  //the chart itself includes a margin.
  //It is positioned such that there is an "empty" (gray) border all around
  //the sides of the page,
  //of Bor pixels if chart+2*Bor bigger than the client area,
  //but chart centered in client area if smaller
     TView *V;
    //TMetafileCanvas *CV;
    TRect R;
    int LeftBor,TopBor, //actual border: > Bor if whole chart smaller than client area
        LeftClient,TopClient, //position in client area where VisArea must be placed
                              //Note: (0,0) is the topleft corner of the currently
                              //visible part of the schrollbox range
        i,VPI;

    V=CurrentViewPop->VW[CurrentViewPop->CurrVW];

    ZoomCombx->ItemIndex=V->GetZoomValIndex();
    ViewScrollbox->HorzScrollBar->Range=V->ZoomedChartWidth+2*Bor;
    ViewScrollbox->VertScrollBar->Range=V->ZoomedChartHeight+2*Bor;

    //second, set size of VisArea and left and top empty border:
    LeftBor=Bor;
    if (ViewScrollbox->ClientWidth > V->ZoomedChartWidth)
    { //zoomed chart smaller than scrollbox area
      VisArea->Width = V->ZoomedChartWidth;
      if (ViewScrollbox->ClientWidth > VisArea->Width+2*Bor)
      {  //if small enough, adjust borders to center in client area:
         LeftBor=(ViewScrollbox->ClientWidth-VisArea->Width)/2;
      }
    }
    else //zoomed chart larger than scrollbox area
    { if ( (i=ViewScrollbox->HorzScrollBar->Range - LeftBor - ViewScrollbox->HorzScrollBar->Position)
            < (ViewScrollbox->ClientWidth) )
      VisArea->Width=i;
      else VisArea->Width=ViewScrollbox->ClientWidth;
    }
    TopBor=Bor;
    if (ViewScrollbox->ClientHeight > V->ZoomedChartHeight)
    { //zoomed chart smaller than scrollbox area
      VisArea->Height = V->ZoomedChartHeight;
      if (ViewScrollbox->ClientHeight > VisArea->Height+2*Bor)
      { TopBor=(ViewScrollbox->ClientHeight-VisArea->Height)/2; }
    }
    else
    { if ( (i=ViewScrollbox->VertScrollBar->Range - TopBor - ViewScrollbox->VertScrollBar->Position)
            < (ViewScrollbox->ClientHeight) )
      VisArea->Height=i;
      else VisArea->Height=ViewScrollbox->ClientHeight;
    }
    //next, clear VisArea:
    //CV = new TMetafileCanvas(VisArea,0); //screen canvas
    R=TRect(0,0,VisArea->Width+1,VisArea->Height+1);
    VisArea->Canvas->Brush->Style=bsSolid;
    VisArea->Canvas->Brush->Color=TranspColor;
    VisArea->Canvas->FillRect(R);

    //next, calculate zoomed-chart pixels corresponding to left and top of VisArea
    //and the position of VisArea in the current client area:
    if ( (i=ViewScrollbox->HorzScrollBar->Position-LeftBor)<=0 )
       { V->ZoomedLeftVis=0; LeftClient=-i; }
    else { V->ZoomedLeftVis=i; //double(i)*V->ZoomedChartWidth/PrChartWidth;
           LeftClient=0; }
    if ( (i=ViewScrollbox->VertScrollBar->Position-TopBor)<=0 )
       { V->ZoomedTopVis=0; TopClient=-i; }
    else { V->ZoomedTopVis=i; //i/ZoomFact;
           TopClient=0; }

    //next, draw part of Chart on VisArea:
    VisArea->Canvas->StretchDraw(Rect(-V->ZoomedLeftVis,-V->ZoomedTopVis,
                                      -V->ZoomedLeftVis+V->ZoomedChartWidth,
                                      -V->ZoomedTopVis+V->ZoomedChartHeight),
                                 V->ChartMF);

    //set the bottom-left pixel of VisArea to the transparent color:
    VisArea->Canvas->Pixels[0][VisArea->Height-1]=TranspColor;
    //delete CV;
    ViewImage->Picture->Graphic=VisArea;

    //finally, position VisArea in scrollbox:
    PagePanel->Left=LeftClient; PagePanel->Top=TopClient;
    PagePanel->Width=VisArea->Width; PagePanel->Height=VisArea->Height;
    PagePanel->Color=V->Opt.PageCol;
    ViewImage->Left=0; ViewImage->Top=0;
    ViewImage->Width=VisArea->Width; ViewImage->Height=VisArea->Height;
    VisArea->SaveToFile("VisArea.bmp");
    //finally, redraw the selection and focus rectangles:
    for (VPI=0; VPI<CurrentViewPop->VPIndCount; VPI++)
    { if (CurrentViewPop->Selected[VPI]) SetShapeRect(VPI,Sel,(TShape*)(IndShapeList->Items[VPI]));  }
    if (CurrentViewPop->FocusInd>=0)
    { SetShapeRect(CurrentViewPop->FocusInd,Foc,FocusShape);
      //FocusShape->BringToFront(); not: else indiv not clickable
    }
    // recalc the zoom values and show in ZoomCombx
    // seems unnecessary, but by setting scrollbar ranges the scrollbox wndproc
    // called with confusing results
    //Herstel: als er in de laagste vergroting geen scrollbars nodig zijn
    //is de client area groter, dus voor full-page view een grotere (bijv 50% ipv 48%)
    //zoom factor mogelijk. Dat wisselt dus!
    //oplossing: bereken de client area zonder scrollbars, en daarmee de
    // (constante) zoom factor voor full page.
    //V->FillZoomvalues(ViewScrollbox->ClientWidth,ViewScrollbox->ClientHeight);
    //ZoomCombx->Items->Assign(V->ZoomValues);
    //ZoomCombx->ItemIndex=V->GetZoomValIndex();
} //TNwMainForm::DrawVisArea
#endif    //ifdef VISAREA

//---------------------------------------------------------------------------

/* old version TNwMainForm::ShowCurrentView with BackPanel and PagePanel:
  waarschijnlijk:
  BackPanel was the complete page and showed the background color;
  PagePanel was the size of the Chart (without margins) and
  was used as a container for the ViewImage and the Shapes
void TNwMainForm::ShowCurrentView(TViewPop *VP)
{ TView *V;
  V=VP->VW[VP->CurrVW];
  if (V->ChartMF==NULL) DrawCurrentView(VP);
  MainTabctrl->TabIndex=VP->CurrVW+OverviewTab;
  MainListview->Visible=false;
  ViewScrollbox->Visible=true;
  BackPanel->Width  = V->ChartMF->Width+30;
  BackPanel->Height = V->ChartMF->Height+30;
  PagePanel->Width  = V->ChartMF->Width+2;
  PagePanel->Height = V->ChartMF->Height+2;
  PagePanel->Left   =14;
  PagePanel->Top    =14;
  ViewImage->Left   =1;
  ViewImage->Top    =1;
  ViewImage->Picture->Assign(V->ChartMF);
  if (ViewScrollbox->ClientWidth > BackPanel->Width)
    BackPanel->Left=
      (ViewScrollbox->ClientWidth - BackPanel->Width) /2;
  else BackPanel->Left=0;
  if (ViewScrollbox->ClientHeight > BackPanel->Height)
    BackPanel->Top=
      (ViewScrollbox->ClientHeight - BackPanel->Height) /2;
  else BackPanel->Top=0;
  FillIndShapeList();
  delete FocusShape;
  if (VP->FocusInd >= 0) FocusShape = CreateIndShape(VP->FocusInd,Foc);
  else if (VP->FocusCross != NULL) FocusShape = CreateCrossShape(VP->FocusCross);
  else FocusShape=NULL;
  if (FocusShape != NULL) FocusShape->SendToBack();
  ViewScrollboxResize(NULL); //to set scrollbars
} //ShowCurrentView
*/

//new version August 2004:
void TMainForm::ShowCurrentView(TViewPop *VP)
/*
   When is chart re-displayed?
   - ToolsCreateView: new ChartMF calculated, Zoomcombx filled,
                      Zoom & ZoomIndex set to 100%,
                      positions of Scrollbars set to 0
   - ToolsOptionsExecute: ChartMF recalculated, Zoomcombx filled,
                          Zoom left unchanged, ZoomIndex updated,
                          positions of Scrollbars set to 0
   - MainTabctrlChange: only if ChartMF=NULL recalculated, Zoomcombx filled,
                        Zoom set to stored View value, ZoomIndex updated,
                        positions of Scrollbars set to 0
   - ZoomCombxChange:   only if ChartMF=NULL recalculated, Zoomcombx unchanged,
                        Zoom set to new value,
                        positions of Scrollbars set to 0
   - ViewScrollboxResize: only if ChartMF=NULL recalculated, Zoomcombx filled,
                          Zoom unchanged, ZoomIndex updated,
                          positions of Scrollbars unchanged
   - ViewScrollbox->OnScroll: nothing changed

   How is display of chart implemented?
   * if VISAREA defined:
     - in each case the VisArea bitmap is drawn again, and the Scrollbars
       Range and Position explicitly set
   * if VisArea not set:
     - a new ZoomMF is calculated:
       - always after a new ChartMF is calculated;
       - on MainTabctrlChange;
       - when Zoom is changed.
     - only if a new ZoomMF is calculated, Scrollbar range and position
       are updated by program, else automatic behaviour.

ShowCurrentView is called when a new chart must be shown onscreen:
in ToolsOptionsExecute, ToolsCreateView and MainTabctrlChange.
if no ChartMF exists, it draws the metafile and sets the zoombox.
then it shows the chart (or VisArea) onscreen,
and sets the selection and focus rectangles
*/
{ TView *V; int VPI,i;
  V=VP->VW[VP->CurrVW];
  TStringList* coord = new TStringList(); //not used here but needed for DrawChartMF
  #ifdef VISAREA
  //VISAREA defined: only the displayed part of the chart is drawn
  //I think this didn't really solve the memory problem, VISAREA currently undefined
  if (V->ChartMF==NULL)
  { //if (V->PrIndWidth<=0) V->CalcInitialChart();
    if ( !V->ChartCalculated ) V->CalcInitialChart();
    V->ChartMF = V->DrawChartMF();
    V->Zoom=100;
  }
  V->FillZoomvalues(ViewScrollbox->Width-ScrollboxBorders,
                    ViewScrollbox->Height-ScrollboxBorders);
  ZoomCombx->Items->Assign(V->ZoomValues);
  V->CalcZoomData();
  ZoomCombx->ItemIndex=V->GetZoomValIndex();
  //ZoomCombxChange(NULL);
  DrawVisArea();

  #else
  //VISAREA undefined: draw the entire zoomed chart on bitmap ZoomBM
  if (V->ChartMF==NULL)
  { //if (V->PrIndWidth<=0) V->CalcInitialChart();
    if ( !V->ChartCalculated ) V->CalcInitialChart();
    V->ChartMF = V->DrawChartMF(false,coord);
    //V->Zoom=100; problem: out of memory errors for big charts
    V->Zoom=1; //1%: take the smallest Zoomvalue i.e. full page view
    ZoomBM->Width=0; //must be redrawn
  }
  if (ZoomBM->Width==0)
  { //this indicates that
    // - a ChartMF is available
    // - new Zoomvalues must be calculated (based on size of ChartMF and ViewScrollbox)
    // - the ZoomCbx must be filled with these values
    // - a zoomfactor must be chosen (based on the current V->Zoom and the list of Zoomvalues)
    // - ZoomBM must be sized and drawn
    V->FillZoomvalues(ViewScrollbox->Width-ScrollboxBorders,
                      ViewScrollbox->Height-ScrollboxBorders);
    ZoomCombx->Items->Assign(V->ZoomValues); //doesn't trigger ZoomCombxChange
    i=V->GetBestZoomValIndex();
    V->Zoom = StrToInt(V->ZoomValues->Strings[i]);
    V->CalcZoomData();
    ZoomCombx->ItemIndex=i; //V->GetZoomValIndex(); //doesn't trigger ZoomCombxChange
    //ZoomCombx->ItemIndex=0;
    //of ZoomCombxChange(NULL);

    try
    { ZoomBM->Width  = V->ZoomedChartWidth;
      ZoomBM->Height = V->ZoomedChartHeight;
      ZoomBM->Canvas->Brush->Style=bsSolid;
      ZoomBM->Canvas->Brush->Color = V->Opt.PageCol; // TranspColor;
      ZoomBM->Canvas->FillRect(Rect(0,0,ZoomBM->Width+1,ZoomBM->Height+1));
      ZoomBM->Canvas->StretchDraw(Rect(0,0,ZoomBM->Width,ZoomBM->Height),
                                 V->ChartMF);
         /*debugging rectangles:               setmapmode
             TRect R; int VPI; Graphics::TMetafile *MF; Graphics::TMetafileCanvas *MC;
             MF = new Graphics::TMetafile;
             MF->Width=V->ChartMF->Width;
             MF->Height=V->ChartMF->Height;
             ZoomBM->Canvas->Pen->Color=clLime;
             ZoomBM->Canvas->Brush->Style=bsClear;
             for (VPI=0; VPI<CurrentViewPop->VPIndCount; VPI++)
             { R=V->CalcIRect(VPI);
               //R=V->GetZoomedRect(R);
               //ZoomBM->Canvas->Rectangle(R);
               MC = new Graphics::TMetafileCanvas(MF,NULL);
               MC->Pen->Color=clLime;
               MC->Pen->Width=(63+PrPPI) / 127  ;
               MC->Brush->Style=bsClear;
               MC->Rectangle(R);
               delete MC;
               ZoomBM->Canvas->StretchDraw(Rect(0,0,ZoomBM->Width,ZoomBM->Height),MF);

             }
             delete MF;  */
       if (VP->VPIndCount<=IndivDrawLimit) {
             //draw all frames in correct (selected / focused) colors:
             TRect R;  Graphics::TMetafile *MF; Graphics::TMetafileCanvas *MC;
             MF = new Graphics::TMetafile;
             MF->Width=V->ChartMF->Width;
             MF->Height=V->ChartMF->Height;
             MC = new Graphics::TMetafileCanvas(MF,NULL);
             for (VPI=0; VPI<VP->VPIndCount; VPI++) V->DrawIndFrame(VPI,MC);
             delete MC;
             ZoomBM->Canvas->StretchDraw(Rect(0,0,ZoomBM->Width,ZoomBM->Height),MF);
             delete MF;
      }
      //set the bottom-left pixel of ZoomBM to the transparent color:
      //ZoomBM->Canvas->Pixels[0][ZoomBM->Height-1]=TranspColor;
      PagePanel->Width=ZoomBM->Width;
      PagePanel->Height=ZoomBM->Height;
      ViewScrollbox->HorzScrollBar->Position=0;
      ViewScrollbox->VertScrollBar->Position=0;
      if (ZoomBM->Width+2*Bor > ViewScrollbox->Width-ScrollboxBorders)
         PagePanel->Left=Bor;
      else PagePanel->Left=(ViewScrollbox->Width-ScrollboxBorders-ZoomBM->Width)/2;
      if (ZoomBM->Height+2*Bor > ViewScrollbox->Height-ScrollboxBorders)
         PagePanel->Top=Bor;
      else PagePanel->Top=(ViewScrollbox->Height-ScrollboxBorders-ZoomBM->Height)/2;
      PagePanel->Color=V->Opt.PageCol;
      ViewImage->Left   =0;
      ViewImage->Top    =0;
      ViewImage->Picture->Assign(ZoomBM);
      //ViewScrollbox->HorzScrollBar->Position=0;
      //ViewScrollbox->VertScrollBar->Position=0;
      SettingScrollbarRange=true;
      ViewScrollbox->HorzScrollBar->Range=V->ZoomedChartWidth+2*Bor;
      ViewScrollbox->VertScrollBar->Range=V->ZoomedChartHeight+2*Bor;
      SettingScrollbarRange=false;
      coord->Clear();
    }
    catch (...)
    { ShowMessage("The chart cannot be displayed at this zoom level");
      ZoomBM->Width=0;
    }
  } //ZoomBM->Width==0

  #endif

  //MainTabctrl->TabIndex=VP->CurrVW+OverviewTab;
  //MainListview->Visible=false;
  //ViewScrollbox->Visible=true;
  /*FillIndShapeList();
  delete FocusShape; FocusShape=NULL;
  if (VP->FocusInd >= 0)
  { FocusShape = CreateIndShape(VP->FocusInd,Foc);
    FocusShape->SendToBack();
  } */
  //ViewScrollboxResize(NULL); //to set scrollbars
} //ShowCurrentView

/*
void TNwMainForm::DeleteAllShapes(void)
{ int i;
  if (CurrentViewPop==NULL) return;
  for (i=0; i<IndShapeList->Count; i++)
      delete (TShape*) IndShapeList->Items[i];
  IndShapeList->Clear();
} //TNwMainForm::DeleteAllShapes

void TNwMainForm::FillIndShapeList(void)
//fills IndShapeList with VPInd elements: shapes and NULLs
{ int VPI;
  if (CurrentViewPop==NULL) return;
  DeleteAllShapes();
  /* Als de shapelist volgens de laatste Sort order van de VP geordend moet zijn
     moet het als volgt:
  for (i=0; i<CurrentViewPop->VPIndCount; i++) ShapeList->Add(NULL);
  for (i=0; i<CurrentViewPop->VPIndCount; i++)
  { VPI=CurrentViewPop->IL->Ind[i]->VPIndIndex;
    if (CurrentViewPop->IL->Ind[i]->Selected) AddShape(VPI,Sel);
    //else ShapeList[VPI]=NULL; not needed, done above
  }
  Maar logischer is om de shapelist volgens VPI te ordenen, dus: */
  /*for (VPI=0; VPI<CurrentViewPop->VPIndCount; VPI++) IndShapeList->Add(NULL);
  for (VPI=0; VPI<CurrentViewPop->VPIndCount; VPI++)
  { if (CurrentViewPop->Selected[VPI]) AddIndShape(VPI,Sel);
    //else ShapeList[VPI]=NULL; not needed, done above
  }

} //TNwMainForm::FillIndShapeList

void TNwMainForm::DeleteIndShape(int VPI)
{ if (CurrentViewPop==NULL || VPI<0 || VPI>=CurrentViewPop->VPIndCount) return;
  delete (TShape*) IndShapeList->Items[VPI];
  IndShapeList->Items[VPI]=NULL;
} //TNwMainForm::DeleteIndShape

void TNwMainForm::SetShapeRect(int VPI, TRecType RT, TShape *Sh)
{ //no error checking, Sh must be assigned and VPI valid
  TRect R;
  TView *V;

  V=CurrentViewPop->VW[CurrentViewPop->CurrVW];
  R=V->CalcIRect(VPI);
  R=V->GetZoomedRect(R);
  switch (RT) {
    case Sel: {
                Sh->Left  = R.Left - V->SelBorderWidth;
                Sh->Top   = R.Top - V->SelBorderWidth;
                Sh->Width = R.Right-R.Left + 2* V->SelBorderWidth;
                Sh->Height= R.Bottom-R.Top + 2* V->SelBorderWidth;
              } break;
    case Foc: {
                Sh->Left  = R.Left - V->SelBorderWidth -1;
                Sh->Top   = R.Top - V->SelBorderWidth -1;
                Sh->Width = R.Right-R.Left + 2* V->SelBorderWidth +2;
                Sh->Height= R.Bottom-R.Top + 2* V->SelBorderWidth +2;
              } break;
  }
} //TNwMainForm::SetShapeRect

TShape* TNwMainForm::CreateIndShape(int VPI, TRecType RT)
{ TShape* Sh;
  TRect R;
  if (CurrentViewPop==NULL || VPI<0 || VPI>=CurrentViewPop->VPIndCount) return NULL;
  Sh = new TShape(this);
  Sh->Parent=PagePanel;

  switch (RT) {
    case Sel: {
                Sh->Brush->Color=clBlue;
                Sh->Pen->Color=clBlue;
              } break;
    case Foc: {
                Sh->Brush->Style=bsClear;
                Sh->Pen->Color=clBlack;
                Sh->Pen->Style=psDot;
              } break;
  }

  SetShapeRect(VPI,RT,Sh);
  return Sh;
} //CreateIndShape

TShape* TNwMainForm::CreateCrossShape(TCrossing* Cro)
{ return NULL; } //for focussing Crossing; not implemented

void TNwMainForm::AddIndShape(int VPI, TRecType RT)
{
  if (CurrentViewPop==NULL || VPI<0 || VPI>=CurrentViewPop->VPIndCount) return;
  if (RT==Sel)
  { delete (TShape*) IndShapeList->Items[VPI];
    IndShapeList->Items[VPI]=CreateIndShape(VPI,Sel);
  }
  else
  { delete FocusShape;
    FocusShape = CreateIndShape(VPI,Foc);
    //FocusShape->SendToBack();
  }
  ViewImage->BringToFront();
    /*this causes bad flicker, with first the rectangles (if any) visible. Idea :
      draw the bitmaps including the rectangles. Will work if a solution is used
      where only the VisArea is drawn at any moment. If so, the Shapes can be
      omitted completely, only their positions/sizes/types(color&outline)
      must be stored. But then the VisArea must be drawn using the Metafile
      and not by copying parts from a large Bitmap.

} //TNwMainForm::AddIndShape
*/

void __fastcall TMainForm::ViewScrollboxResize(TObject *Sender)
{ TView *V;

  if (SettingScrollbarRange || ClearingTabs) return;

  if (CurrentViewPop!=NULL &&
      CurrentViewPop->CurrVW>=0 &&
      CurrentViewPop->CurrVW<CurrentViewPop->VWCount &&
      (V=CurrentViewPop->VW[CurrentViewPop->CurrVW])!=NULL &&
      V->ChartMF!=NULL)


  { if ( ViewScrollbox->Width !=ScrollboxLastWidth ||
         ViewScrollbox->Height!=ScrollboxLastHeight )
    { ScrollboxLastWidth =ViewScrollbox->Width;
      ScrollboxLastHeight=ViewScrollbox->Height;
      V->FillZoomvalues(ViewScrollbox->Width -ScrollboxBorders,
                        ViewScrollbox->Height-ScrollboxBorders);
      ZoomCombx->Items->Assign(V->ZoomValues);
    }
    ZoomCombx->ItemIndex=V->GetZoomValIndex();
    #ifdef VISAREA
      DrawVisArea();
      /*ShowMessage("Width="+IntToStr(ViewScrollbox->Width)+
                "ClientW="+IntToStr(ViewScrollbox->ClientWidth)+
                "Height="+IntToStr(ViewScrollbox->Height)+
                "ClientH="+IntToStr(ViewScrollbox->ClientHeight)+
                "ScrBarSize="+IntToStr(ViewScrollbox->HorzScrollBar->Size));*/
    #else
      if (PagePanel->Width+2*Bor > ViewScrollbox->Width-ScrollboxBorders)
         PagePanel->Left=Bor;
      else PagePanel->Left=(ViewScrollbox->Width-ScrollboxBorders-ZoomBM->Width)/2;
      if (PagePanel->Height+2*Bor > ViewScrollbox->Height-ScrollboxBorders)
         PagePanel->Top=Bor;
      else PagePanel->Top=(ViewScrollbox->Height-ScrollboxBorders-ZoomBM->Height)/2;
      SettingScrollbarRange=true;
      ViewScrollbox->HorzScrollBar->Range = PagePanel->Width+2*Bor;
      ViewScrollbox->VertScrollBar->Range = PagePanel->Height+2*Bor;
      SettingScrollbarRange=false;
    #endif


    /*old version:
    ViewScrollbox->HorzScrollBar->Position=0;
    ViewScrollbox->VertScrollBar->Position=0;
    if (ViewScrollbox->ClientWidth > BackPanel->Width)
    { BackPanel->Left= (ViewScrollbox->ClientWidth - BackPanel->Width) /2;
      ViewScrollbox->HorzScrollBar->Range = ViewScrollbox->ClientWidth;
    }
    else
    { BackPanel->Left=0;
      ViewScrollbox->HorzScrollBar->Range = BackPanel->Width;;
    }
    if (ViewScrollbox->ClientHeight > BackPanel->Height)
    { BackPanel->Top= (ViewScrollbox->ClientHeight - BackPanel->Height) /2;
      ViewScrollbox->VertScrollBar->Range = ViewScrollbox->ClientHeight;
    }
    else
    { BackPanel->Top=0;
      ViewScrollbox->VertScrollBar->Range = BackPanel->Height;
    }  */
  }
} //TNwMainForm::ViewScrollboxResize
//---------------------------------------------------------------------------


void __fastcall TMainForm::FilePrintAccept(TObject *Sender)
{ TPrinter *Ptr = Printer();
  //int p;
  TView *V;
  TRect Marg;
  //TMetafile *MF;
  //ShowMessage("PrinterPPI = "+IntToStr(GetDeviceCaps(Ptr->Handle, LOGPIXELSX)));
  if ( CurrentViewPop==NULL ||
       CurrentViewPop->CurrVW<0 ||
       (V=CurrentViewPop->VW[CurrentViewPop->CurrVW])==NULL )
    ShowMessage("Nothing to print!");
  /*else
  { p=GetDeviceCaps(Ptr->Handle, LOGPIXELSX);
    if (p!=PrPPI)
    { //draw Ind metafiles at new resolution:
      PrPPI=p;                                           //fout bij 2x printen op versch. PPI
      V->DeleteIndMetafiles();
    }
    //MF=V->DrawMetafile();
    MF=V->DrawChartMF(); //V->CalcZoomData(); niet voor printer!
    // zet portrait/landscape op printer ...
    //Marg=V->CalcMargRect(PrPPI);
    Marg.Left=Marg.Left-GetDeviceCaps(Ptr->Handle, PHYSICALOFFSETX);
    Marg.Left = Marg.Left<0 ? 0 : Marg.Left;
    Marg.Top=Marg.Top-GetDeviceCaps(Ptr->Handle, PHYSICALOFFSETY);
    Marg.Top = Marg.Top<0 ? 0 : Marg.Top;
    Ptr->BeginDoc();
    Ptr->Canvas->Draw(Marg.Left,Marg.Top,MF);
    Ptr->EndDoc();
  }
  */
} //TNwMainForm::FilePrintAccept
//---------------------------------------------------------------------------


void __fastcall TMainForm::ToolsCreateViewExecute(TObject *Sender)
{ int i,j; //,PrMinWidth, PrMinHeight; //min chart size (within margins) in printer pixels
  AnsiString s;
  if (CurrentViewPop!=NULL)
  { //get a new ViewName:
    i=1;
    do
    { s="View"+IntToStr(i++);
      for (j=0; j<CurrentViewPop->VWCount; j++)
      { if (s==CurrentViewPop->VW[j]->ViewName) break; }
    }
    while (j<CurrentViewPop->VWCount);

    CurrentViewPop->AddVW(s,false); //no overview
    CurrentViewPop->CurrVW=CurrentViewPop->VWCount-1;
    CurrentViewPop->VW[CurrentViewPop->CurrVW]->CalcInitialChart();
    SetTabs();
    #ifndef VISAREA
      ZoomBM->Width=0; //must be redrawn
    #endif
    MainTabctrl->TabIndex=CurrentViewPop->CurrVW+OverviewTab;
    MainTabctrlChange(NULL); //not automatically
    //ShowCurrentView(CurrentViewPop); called by MainTabctrlChange
    ProjectSaved=false;
  }
} //TNwMainForm::ToolsCreateViewExecute
//---------------------------------------------------------------------------

void __fastcall TMainForm::ViewImageMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{ int VPI,g,i,SelCount;
  TView* V;
  TCrossing* Cro;
  AnsiString s;
  TRect R;  Graphics::TMetafile *MF; Graphics::TMetafileCanvas *MC;

  if ( (Button != mbLeft) ||
       Shift.Contains(ssShift) || Shift.Contains(ssAlt) )
     return;

  V=CurrentViewPop->VW[CurrentViewPop->CurrVW];
  V->GetUnzoomedPoint(X,Y,X,Y);
  V->GetObjAtPoint(X,Y,g,VPI,Cro);
  if (g>=0)
  { if (VPI>=0) //clicked in Individual
    { //ShowMessage ("Indiv="+Indiv[CurrentViewPop->VPInd[VPI]].IName);
      i=CurrentViewPop->IL->GetIndex(VPI);
      SelCount=0;
      for (i=0; i<CurrentViewPop->VPIndCount; i++)
          if (CurrentViewPop->Selected[i]) SelCount++;
      try
      { MF = new Graphics::TMetafile;
        MF->Width=V->ChartMF->Width;
        MF->Height=V->ChartMF->Height;
        MC = new Graphics::TMetafileCanvas(MF,NULL);

        if ( Shift.Contains(ssCtrl) )
        { //change selection status of clicked indiv only,
          //and check if selection empty:
          if (CurrentViewPop->Selected[VPI] = !CurrentViewPop->Selected[VPI]) //assignment is correct!
          { //if deleting indiv from selection, perhaps clear SelScript:
            SelCount=0;
            for (i=0; i<CurrentViewPop->VPIndCount; i++)
                if (CurrentViewPop->Selected[i]) SelCount++;
            if (SelCount == 0) CurrentViewPop->ClearSelection();
          }
          //change FocusInd and retain last focussed:
          g=CurrentViewPop->FocusInd;
          CurrentViewPop->FocusInd=VPI;
          //draw new FocusInd frame:
          V->DrawIndFrame(VPI,MC);
          //if last FocusInd different, draw its frame too:
          if (g>=0 && g!=VPI)
            V->DrawIndFrame(g,MC);
        }
        else //plain mouseclick without Ctrl: erase previous selection, select this one
        { CurrentViewPop->ClearSelection();
          CurrentViewPop->Selected[VPI]=true;
          CurrentViewPop->FocusInd=VPI;
          //draw all frames:
          for (VPI=0; VPI<CurrentViewPop->VPIndCount; VPI++) V->DrawIndFrame(VPI,MC);
        }
        delete MC;
        ZoomBM->Canvas->StretchDraw(Rect(0,0,ZoomBM->Width,ZoomBM->Height),MF);
        ViewImage->Picture->Assign(ZoomBM);
        ViewImageMouseMove(Sender,Shift,X,Y);
      }
      __finally { delete MF; }
    } //(VPI>=0)
  } //(g>=0)
} //TNwMainForm::ViewImageMouseUp
//---------------------------------------------------------------------------

void __fastcall TMainForm::ViewImageMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{ int VPI,g,I,d;
  TView* V;
  TCrossing* Cro;
  AnsiString s;

  InfoMemo->Clear();
  V=CurrentViewPop->VW[CurrentViewPop->CurrVW];
  V->GetUnzoomedPoint(X,Y,X,Y); //convert zoomed(X,Y) to ChartMF(X,Y)

  V->GetObjAtPoint(X,Y,g,VPI,Cro);
  if (g>=0)
  { if (VPI>=0)
    { I=CurrentViewPop->VPInd[VPI];
      s=Indiv[I].IName;
      if (V->Opt.InfoSelected)
      { if (CurrentViewPop->Selected[VPI])
          s=s+" (selected)";
        else s=s+" (not selected)";
      } //InfoSelected
      InfoMemo->Lines->Add(s);
      if (V->Opt.InfoParents)
      { Cro=Indiv[I].ParentCross;
        if (Cro==NULL) s="Founder";
        else if (Cro->CType==ctCross)
        { s=ParentWithString(0,"parent")+": ";
          if (Cro->Parent[0] >= 0)
             s=s+Indiv[Cro->Parent[0]].IName;
          else s=s+"?";
          InfoMemo->Lines->Add(s);
          s=s=ParentWithString(1,"parent")+": ";
          
          if (Cro->Parent[1] >= 0)
             s=s+Indiv[Cro->Parent[1]].IName;
          else s=s+"?";
        }
        else
        { s="Parent : ";
          if (Cro->Parent[0] >= 0)
             s=s+Indiv[Cro->Parent[0]].IName;
          else s=s+"?";
        }
        InfoMemo->Lines->Add(s);
      } //InfoParents
      InfoMemo->Lines->Add("");
      for (d=0; d<IndivDataCount; d++)
      { if (V->Opt.InfoTrait[d])
        { s=IndivData[d].Caption+": "+DataToStr(d,I,strUnknown[0]);
          InfoMemo->Lines->Add(s); }
      }
      // show top line of InfoMemo:
      InfoMemo->SelStart=0;
      // scroll caret into view,
      // the wParam, lParam parameters are ignored
      SendMessage(InfoMemo->Handle, EM_SCROLLCARET,0,0);
    }
    else if (Cro!=NULL)
    { switch (Cro->CType)
      { case ctCross: s="Cross"; break;
        case ctSelfing: s="Selfing"; break;
        case ctDH: s="DHproduction"; break;
        case ctMutation: s="Mutation"; break;
        case ctVegProp: s="Cloning"; break;
      }
      InfoMemo->Lines->Add(s);
      if (Cro->CType == ctCross)
      { s=s=ParentWithString(0,"parent")+": ";
        if (Cro->Parent[0] >= 0)
           s=s+Indiv[Cro->Parent[0]].IName;
        else s=s+"?";
        InfoMemo->Lines->Add(s);
        s=s=ParentWithString(1,"parent")+": ";
        if (Cro->Parent[1] >= 0)
           s=s+Indiv[Cro->Parent[1]].IName;
        else s=s+"?";
        InfoMemo->Lines->Add(s);
      }
      else
      { s="Parent : ";
        if (Cro->Parent[0] >= 0)
           s=s+Indiv[Cro->Parent[0]].IName;
        else s=s+"?";
        InfoMemo->Lines->Add(s);
      }
      InfoMemo->Lines->Add("Offspring:");
      for (d=0; d<Cro->OCount; d++)
        InfoMemo->Lines->Add(Indiv[Cro->Offsp[d]].IName);
      // show top line of InfoMemo:
      InfoMemo->SelStart=0;
      // scroll caret into view,
      // the wParam, lParam parameters are ignored
      SendMessage(InfoMemo->Handle, EM_SCROLLCARET,0,0);
    }
  }

} //TNwMainForm::ViewImageMouseMove
//---------------------------------------------------------------------------

void __fastcall TMainForm::ViewScrollboxExit(TObject *Sender)
{
  InfoMemo->Clear();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::ToolsCreateSubpopExecute(TObject *Sender)
{ TViewPop *LastViewPop, *TempVP;;
  int i,j;
  AnsiString s,t;
  TTreeNode *Node;

  if (CurrentViewPop==NULL)
  { ShowMessage("Error: CurrentViewPop==NULL in CreateSubpop"); return; }

  //Count number of selected VPInd in CurrentViewPop:
  if (MainTabctrl->TabIndex < OverviewTab) MainListviewExit(NULL);

  MainListview->Items->Clear();
  MainListview->Columns->Clear();

  j=0;
  for (i=0; i<CurrentViewPop->VPIndCount; i++)
      if (CurrentViewPop->Selected[i]) j++;
  if (j==0)
  { ShowMessage("No individuals selected"); return; }


  LastViewPop=CurrentViewPop;
  CurrentViewPop=NULL;
  Node=PopTreeView->Items->AddChild(LastViewPop->VPNode,
                LastViewPop->VPNode->Text+"-"+
                IntToStr(++LastViewPop->LastSubpopNumber));
  Node->ImageIndex=57;
  TempVP = new TViewPop(Node,-1); //also makes Node->Data refer to new VP

  //first add evt. manual lines to SelScript and update LastSelection:
  LastViewPop->UpdateSelection(-1);
  //add selected individuals to new ViewPop:
  for (i=0; i<LastViewPop->VPIndCount; i++)
      if (LastViewPop->Selected[i])
         TempVP->AddVPInd(LastViewPop->VPInd[i]);
  TempVP->OrigScript.Copy(LastViewPop->SelScript);
  TempVP->AddVW("&Overview",true); //first view:Overview
  TempVP->VW[TempVP->CurrVW]->CalcInitialChart();
  PopTreeView->Selected=Node;
  //PopTreeViewChange(NULL,NULL); not needed?
  ProjectSaved=false;
} //TNwMainForm::ToolsCreateSubpopExecute
//---------------------------------------------------------------------------


void TMainForm::SetTabs(void)
{ int i;
  MainTabctrl->Tabs->Clear();
  if (CurrentViewPop!=NULL)
  { MainTabctrl->Tabs->Add("&Individuals");
    for (i=0; i<CurrentViewPop->VWCount; i++)
       MainTabctrl->Tabs->Add(CurrentViewPop->VW[i]->ViewName);
  }
} //TNwMainForm::SetTabs

void TMainForm::DataListWidths(void)
//calculates the width of IndivData columns in MainListView
{ int i,d,e,W=0,X;
  for (d=0; d<IndivDataCount; d++)
  { W=MainListview->Canvas->TextWidth(IndivData[d].Caption);
    e=IndivData[d].TypeIndex;
    switch (IndivData[d].DataType)
    { case dtC: ; break;
      case dtI: X=MainListview->Canvas->TextWidth(IntToStr(Iint[e][IndivData[d].V.OV.LargInd]));
                if (X>W) W=X;
                X=MainListview->Canvas->TextWidth(IntToStr(Iint[e][IndivData[d].V.OV.SmallInd]));
                if (X>W) W=X;
                break;
      case dtF: X=MainListview->Canvas->TextWidth(FloatToStrF(Ifloat[e][IndivData[d].V.OV.LargInd],
                        ffFixed,100,IndivData[d].V.OV.Decimals));
                if (X>W) W=X;
                X=MainListview->Canvas->TextWidth(FloatToStrF(Ifloat[e][IndivData[d].V.OV.SmallInd],
                        ffFixed,100,IndivData[d].V.OV.Decimals));
                if (X>W) W=X;
                break;
      case dtS: for (i=0; i<IndivData[d].V.SV.LabelCount; i++)
                { X=MainListview->Canvas->TextWidth(IndivData[d].V.SV.Labels[i]);
                  if (X>W) W=X;
                } break;
    }
    IndivData[d].ListPx=W;
  }
} //TNwMainForm::DataListWidths

void SetLegendColumns(void)
//find maximum widths for Locusname, Linkage group name and Position
//assumes that all relevant canvases have same font as LegendForm itself
//only needed to fill once after importing datafile
{ int i,p,lg;
  double MaxPos,MinPos,ps;

  LegendForm->LocnameWidth=LegendForm->Canvas->TextWidth("(Start)");
  LegendForm->LGnameWidth=0;
  MaxPos=0.0; MinPos=0.0;
  for (lg=0; lg<LinkGrpCount;lg++)
  { if ((i=LegendForm->Canvas->TextWidth(LinkGrp[lg]->LGName)) > LegendForm->LGnameWidth)
      LegendForm->LGnameWidth=i;
    if (IBDAlleleCount>0)
    { if ((ps=LinkGrp[lg]->IBDpos[0]->cM)<MinPos) MinPos=ps;
      if ((ps=LinkGrp[lg]->IBDpos[LinkGrp[lg]->IBDCount-1]->cM)>MaxPos) MaxPos=ps;
    }
    if (LinkGrp[lg]->LocCount>0)
    { if ((ps=LinkGrp[lg]->Locus[0]->cM)<MinPos) MinPos=ps;
      if ((ps=LinkGrp[lg]->Locus[LinkGrp[lg]->LocCount-1]->cM)>MaxPos) MaxPos=ps;
      for (p=0; p<LinkGrp[lg]->LocCount; p++)
      { if ( (i=LegendForm->Canvas->TextWidth(LinkGrp[lg]->Locus[p]->LocName)) >
             LegendForm->LocnameWidth )
          LegendForm->LocnameWidth=i;
      }
    }
  }
  LegendForm->PositionWidth=LegendForm->Canvas->TextWidth(FloatToStrF(MinPos,ffFixed,8,1));
  if ( (i=LegendForm->Canvas->TextWidth(FloatToStrF(MaxPos,ffFixed,8,1))) > LegendForm->PositionWidth)
    LegendForm->PositionWidth=i;
} //SetLegendColumns


void TMainForm::FillLegend(TView *VW)
{ //Adjust LegendForm:
  if (CurrentViewPop->CurrVW<0)
  { LegendForm->ContinuousTraitColSheet->TabVisible = false;
    LegendForm->DiscreteTraitColSheet->TabVisible = false;
    LegendForm->LGMapSheet->TabVisible = false;
    LegendForm->IBDColSheet->TabVisible = false;
    LegendForm->MarkerSheet->TabVisible = false;
  }
  else
  { LegendForm->ContinuousTraitColSheet->TabVisible =
      !VW->Opt.IFixedColor && VW->Opt.ColorTrait>=0 &&
      ( IndivData[VW->Opt.ColorTrait].DataType==dtI ||
        IndivData[VW->Opt.ColorTrait].DataType==dtF );
    if (LegendForm->ContinuousTraitColSheet->TabVisible)
    { LegendForm->ContTraitLbl->Caption="Trait: "+IndivData[VW->Opt.ColorTrait].Caption;
      LegendForm->MVshape->Brush->Color=VW->Opt.MVCol;
      if (IndivData[VW->Opt.ColorTrait].DataType==dtI)
      { LegendForm->MaxVallbl->Caption=
          IntToStr(Iint[IndivData[VW->Opt.ColorTrait].TypeIndex]
                         [IndivData[VW->Opt.ColorTrait].V.OV.LargInd]);
        LegendForm->MinVallbl->Caption=
          IntToStr(Iint[IndivData[VW->Opt.ColorTrait].TypeIndex]
                         [IndivData[VW->Opt.ColorTrait].V.OV.SmallInd]);
      }
      else
      { LegendForm->MaxVallbl->Caption=
          FloatToStrF( Ifloat[IndivData[VW->Opt.ColorTrait].TypeIndex]
                                [IndivData[VW->Opt.ColorTrait].V.OV.LargInd],
                       ffFixed,
                       IndivData[VW->Opt.ColorTrait].Fieldwidth,
                       IndivData[VW->Opt.ColorTrait].V.OV.Decimals );
        LegendForm->MinVallbl->Caption=
          FloatToStrF( Ifloat[IndivData[VW->Opt.ColorTrait].TypeIndex]
                                [IndivData[VW->Opt.ColorTrait].V.OV.SmallInd],
                       ffFixed,
                       IndivData[VW->Opt.ColorTrait].Fieldwidth,
                       IndivData[VW->Opt.ColorTrait].V.OV.Decimals );
      }
      LegendForm->LoCol = VW->Opt.LoCol;
      LegendForm->HiCol = VW->Opt.HiCol;
      LegendForm->ContColPaintBox->Invalidate();
      //LegendForm->MVCol = VW->Opt.MVCol;
    }

    LegendForm->DiscreteTraitColSheet->TabVisible =
      !VW->Opt.IFixedColor && VW->Opt.ColorTrait>=0 &&
      ( IndivData[VW->Opt.ColorTrait].DataType==dtC ||
        IndivData[VW->Opt.ColorTrait].DataType==dtS );
    if (LegendForm->DiscreteTraitColSheet->TabVisible)
    { LegendForm->DiscrTraitPanel->Caption="Trait: "+IndivData[VW->Opt.ColorTrait].Caption;
      LegendForm->DiscrColListbox->Count=VW->Opt.TraitColCount+1; //including one for missing value;
    }

    LegendForm->LGMapSheet->TabVisible =
      (VW->Opt.IndivContents==icIBDprob || VW->Opt.IndivContents==icIBDall)
      && VW->Opt.LG>=0;
    LegendForm->IBDColSheet->TabVisible = LegendForm->LGMapSheet->TabVisible;
    if (LegendForm->LGMapSheet->TabVisible)
    { LegendForm->LGmapPanel->Caption="Linkage group "+LinkGrp[VW->Opt.LG]->LGName;
      LegendForm->LGmapLbox->Items->Clear();
      LegendForm->LGmapLbox->Count=LinkGrp[VW->Opt.LG]->LocCount;
      LegendForm->LGstart =
         LinkGrp[VW->Opt.LG]->IBDpos[0]->cM < LinkGrp[VW->Opt.LG]->Locus[0]->cM;
      if (LegendForm->LGstart) LegendForm->LGmapLbox->Count++;
      if (LinkGrp[VW->Opt.LG]->IBDpos[LinkGrp[VW->Opt.LG]->IBDCount-1]->cM <
          LinkGrp[VW->Opt.LG]->Locus[LinkGrp[VW->Opt.LG]->LocCount-1]->cM)
        LegendForm->LGmapLbox->Count++;

      LegendForm->IBDColorListbox->Count=IBDAlleleCount;
    }



    LegendForm->MarkerSheet->TabVisible = VW->Opt.IndivContents==icMarkers;
    if (LegendForm->MarkerSheet->TabVisible)
    { LegendForm->MarkerLbox->Count=VW->Opt.MarkSelCount;
    }
  }
} //TNwMainForm::FillLegend

void __fastcall TMainForm::MainTabctrlChange(TObject *Sender)
{ //Beep(880,300);
  if (CurrentViewPop==NULL) return;
  InfoMemo->Clear();
  LegendForm->ContinuousTraitColSheet->TabVisible=false;
  LegendForm->LGMapSheet->TabVisible=false;
  LegendForm->IBDColSheet->TabVisible=false;
  LegendForm->MarkerSheet->TabVisible=false;
  LegendForm->DiscreteTraitColSheet->TabVisible=false;
  if (MainTabctrl->TabIndex < OverviewTab)
  { //show the Individuals list:
    ViewScrollbox->Visible=false;
    MainListview->OnData=NULL;
    //MainListview->Visible=true; in ShowIndivList
    if (MainTabctrl->TabIndex == IndividualsTab)
    { ShowIndivList();
      CurrentViewPop->CurrVW = -1; //to be changed if/when CrossingList added
    }
    //else ShowCrossingList(); not implemented yet
  }
  else
  { //show one of the Views:
    MainListview->Visible=false;
    ViewScrollbox->Visible=true;
    CurrentViewPop->CurrVW = MainTabctrl->TabIndex-OverviewTab;
    #ifndef VISAREA
      ZoomBM->Width=0; //ZoomBM must be redrawn
    #endif
    ShowCurrentView(CurrentViewPop);
    FillLegend(CurrentViewPop->VW[CurrentViewPop->CurrVW]); //20100111
  }
  /* deleted 20100111:
  if (CurrentViewPop->CurrVW >= 0 &&
      CurrentViewPop->CurrVW < CurrentViewPop->VWCount)
  FillLegend(CurrentViewPop->VW[CurrentViewPop->CurrVW]);
  */
} //TNwMainForm::MainTabctrlChange
//---------------------------------------------------------------------------

void __fastcall TMainForm::MainTabctrlChanging(TObject *Sender,
      bool &AllowChange)
{ if (MainTabctrl->TabIndex < OverviewTab) MainListviewExit(NULL);
}

void __fastcall TMainForm::ToolsViewOptionsExecute(TObject *Sender)
{ //int PrMinWidth, PrMinHeight; //min chart size (within margins) in printer pixels
  TView *V;
  if ( CurrentViewPop!=NULL &&
       CurrentViewPop->CurrVW>=0 &&
       (V=CurrentViewPop->VW[CurrentViewPop->CurrVW])!=NULL &&
       ! V->Overview )   //for overview, options may not be changed
  { OptionsForm->OwnOptions = V->Opt;
    if (OptionsForm->ShowModal() == mrOk)
    { V->Opt = OptionsForm->OwnOptions;
      V->DeleteIndMetafiles();
      delete V->ChartMF; V->ChartMF=NULL;
      if (!V->ManualAdjust)
      { //recalculate positions
        V->CalcInitialPositions();
      }
      ShowCurrentView(CurrentViewPop);  //redraws V->ChartMF and ZoomBM
      FillLegend(CurrentViewPop->VW[CurrentViewPop->CurrVW]);
      ProjectSaved=false;
    }
  }
} //TNwMainForm::ToolsOptionsExecute
//---------------------------------------------------------------------------

void TMainForm::ShowIndivList(void)
{ TListColumn *LC;
  //TVPIndList *IL;
  int d,i, ColWidth, Space4;

  //MainListview->Visible=false;
  MainListview->Visible=MainListviewVisible;
  //Beep(220,300);
  MainListview->OnData=NULL;
  MainListview->Items->Clear();
  MainListview->Columns->Clear();
  MainListview->OwnerData=true;
  MainListview->MultiSelect=true;
  MainListview->ItemIndex=-1;
  Space4= MainListview->Canvas->TextWidth("    ");
  ColWidth= max (MainListview->Canvas->TextWidth("Parent 0"), IndNameListWidth)
            + Space4;
  MainListview->Columns->Add();
  LC=MainListview->Columns->Items[0];
  LC->Caption = "Nr";
  LC->AutoSize=false;
  i=MainListview->Canvas->TextWidth(IntToStr(IndivCount));
  LC->Width = max (MainListview->Canvas->TextWidth("Nr"), i)+Space4;
  MainListview->Columns->Add();
  LC=MainListview->Columns->Items[1];
  LC->Caption = "Name";
  LC->AutoSize=false;
  LC->Width=ColWidth;
  MainListview->Columns->Add();
  LC=MainListview->Columns->Items[2];
  LC->Caption = ParentCaption(0);
  LC->AutoSize=false;
  LC->Width=ColWidth;
  MainListview->Columns->Add();
  LC=MainListview->Columns->Items[3];
  LC->Caption = ParentCaption(1);
  LC->AutoSize=false;
  LC->Width=ColWidth;

  //add columns for the data:
  for (d=0; d<IndivDataCount; d++)
  {
    MainListview->Columns->Add();
    LC=MainListview->Columns->Items[4+d];
    LC->Caption = IndivData[d].Caption;
    LC->AutoSize=false;
    LC->Width= IndivData[d].ListPx + Space4;
  }

  MainListview->Visible=true;
  if (CurrentViewPop==NULL) return;
  MainListview->Items->Count=CurrentViewPop->VPIndCount;
  //MainListview->OnData=MainListviewDataI; //in MainListviewEnter

  MainListview->SetFocus(); //calls MainListviewEnter if not already focused
  MainListviewEnter(NULL);  //must still be called in case it was already focused
} //TNwMainForm::ShowIndivList

void TMainForm::ShowCrossingList(void)
{
  MainListview->Items->Clear();
  MainListview->Columns->Clear();
} //TNwMainForm::ShowCrossingList

void __fastcall TMainForm::MainListviewDataI(TObject *Sender,
      TListItem *Item)
//OnData for Indiv list; analogous to TViewPop::MakeIndListSL
{ int I,d;
  TCrossing *Cro;

  if ( Item==NULL ||
       Item->Index < 0 ||
       Item->Index >= MainListview->Items->Count)
    return;
  if (CurrentViewPop==NULL) return;
  try
  { I=CurrentViewPop->VPInd[CurrentViewPop->IL->Ind[Item->Index]->VPIndIndex]; }
  catch (...) {return;}
  Item->Caption=IntToStr(I);
  Item->SubItems->Add(Indiv[I].IName);
  //Item->Selected=CurrentViewPop->IL->Ind[Item->Index]->Selected;
  Cro=Indiv[I].ParentCross;
  if (Cro==NULL)
  { Item->SubItems->Add(strUnknown[0]); Item->SubItems->Add(strUnknown[0]); }
  else
  { if (Cro->Parent[0]<0 || CurrentViewPop->GetVPIndex(Cro->Parent[0])<0)
           Item->SubItems->Add(strUnknown[0]);
    else Item->SubItems->Add(Indiv[Cro->Parent[0]].IName);
    if (Cro->CType==ctCross)
    { if (Cro->Parent[1]<0 || CurrentViewPop->GetVPIndex(Cro->Parent[1])<0)
           Item->SubItems->Add(strUnknown[0]);
      else Item->SubItems->Add(Indiv[Cro->Parent[1]].IName);
    }
    else
    { if (Cro->CType==ctSelfing)
         Item->SubItems->Add(Indiv[Cro->Parent[0]].IName);
      else Item->SubItems->Add(CrossTypeString[Cro->CType]);
    }
    /*
    if (Cro->CType<=ctSelfing)
    { if (Cro->Parent[1]<0) Item->SubItems->Add(strUnknown);
      else Item->SubItems->Add(Indiv[Cro->Parent[1]].IName);
    }
    else Item->SubItems->Add(strNoParent);
    */
  }
  for (d=0; d<IndivDataCount; d++) Item->SubItems->Add(DataToStr(d,I,strUnknown[0]));
} //TNwMainForm::MainListviewDataI

void __fastcall TMainForm::MainListviewEnter(TObject *Sender)
{ int i;
  if (MainTabctrl->TabIndex==IndividualsTab)
  { MainListview->OnData=NULL;
    MainListview->OnSelectItem=NULL;
    MainListview->ItemIndex=-1;
    if (CurrentViewPop != NULL)
    { for (i=0; i<CurrentViewPop->VPIndCount; i++)
      { MainListview->Items->Item[i]->Selected = CurrentViewPop->IL->Ind[i]->Selected();
        MainListview->Items->Item[i]->Focused = false;
      }
      if ( CurrentViewPop->FocusInd >= 0 &&
           CurrentViewPop->FocusInd < MainListview->Items->Count )
      { i=CurrentViewPop->IL->GetIndex(CurrentViewPop->FocusInd);
        //NB: setting the Focus through ItemIndex doesn't work
        MainListview->Items->Item[i]->Focused = true;
      }
      else MainListview->ItemIndex = -1;

      //MainListview->OnSelectItem=MainListviewSelectItem;
      MainListview->OnData=MainListviewDataI;
      MainListview->OnSelectItem=MainListviewSelectItem;
    }
  }
} //TNwMainForm::MainListviewEnter
//---------------------------------------------------------------------------

void __fastcall TMainForm::MainListviewExit(TObject *Sender)
{ int i;
  //Beep(440,1000);
  if (MainTabctrl->TabIndex==IndividualsTab)
  { if (CurrentViewPop != NULL)
    { if ( MainListview->ItemFocused != NULL  &&
           (i=MainListview->ItemFocused->Index) >= 0 )
         CurrentViewPop->FocusInd =
           CurrentViewPop->IL->Ind[i]->VPIndIndex;
           //NB ItemIndex seems always to be the first selected indiv,
           //-1 if none selected
      for (i=0; i<CurrentViewPop->VPIndCount; i++)
          CurrentViewPop->Selected[CurrentViewPop->IL->Ind[i]->VPIndIndex] =
            MainListview->Items->Item[i]->Selected;
    }
  }
} //TNwMainForm::MainListviewExit
//---------------------------------------------------------------------------



void __fastcall TMainForm::ToolsSelectRelativesExecute(TObject *Sender)
{ int Anc,Prg,VPFocInd,i,Parentline;
  bool Select;
  MainTabctrlChanging(NULL,Select); //Select is dummy here, not used
  if ( CurrentViewPop != NULL &&
       (VPFocInd=CurrentViewPop->FocusInd) >= 0 &&
       SelectDlgForm->ShowModal() == mrOk )
  { try
    { Anc = StrToInt(SelectDlgForm->AncestorEd->Text);
      Prg = StrToInt(SelectDlgForm->ProgenyEd->Text);
      Parentline = SelectDlgForm->ParentlineRgrp->ItemIndex;
      if (Parentline<0 ||Parentline>2) Parentline=2; //default both parents
      Select = SelectDlgForm->SelectRbtn->Checked;
    }
    catch (...) {return;}
                   /*for debugging:
                   AnsiString s;
                   s="A: FocInd="+IntToStr(CurrentViewPop->FocusInd)+" LastSelCount="+IntToStr(CurrentViewPop->LastSelCount)+": ";
                   for (int j=0;j<CurrentViewPop->LastSelCount;j++) s+=" "+IntToStr(CurrentViewPop->LastSelection[j]);
                   s +=CRLF+"Selection: ";
                   for (int j=0;j<CurrentViewPop->VPIndCount;j++)
                       if (CurrentViewPop->Selected[j]) s+=" "+IntToStr(j);
                   ShowMessage(s);
                   */
    //first, add manual selection lines to selscript:
    CurrentViewPop->UpdateSelection(VPFocInd);
    //do auto selection:
    CurrentViewPop->SelectVPAncestors(VPFocInd,Anc,Parentline,Select);
    CurrentViewPop->SelectVPProgeny(VPFocInd,Prg,Parentline,Select,SelectDlgForm->ProgParentsCbx->Checked);
    if (SelectDlgForm->SibsCbx->Checked)
      CurrentViewPop->SelectVPSibs(VPFocInd,Select);
    //add auto line to SelScript:
    CurrentViewPop->SelScript.AddSelScriptLine();
      i=CurrentViewPop->SelScript.LineCount-1;
    CurrentViewPop->SelScript.SSLine[i].BaseInd=CurrentViewPop->VPInd[VPFocInd];
    CurrentViewPop->SelScript.SSLine[i].Select=Select;
    CurrentViewPop->SelScript.SSLine[i].IndCount=0;
    CurrentViewPop->SelScript.SSLine[i].AncestorGen=Anc;
    CurrentViewPop->SelScript.SSLine[i].ProgenyGen=Prg;
    CurrentViewPop->SelScript.SSLine[i].Parentline=Parentline;
    CurrentViewPop->SelScript.SSLine[i].Sibs=SelectDlgForm->SibsCbx->Checked;
    CurrentViewPop->SelScript.SSLine[i].ColdParents=SelectDlgForm->ProgParentsCbx->Checked;
    //update LastSelection:
    delete[] CurrentViewPop->LastSelection; CurrentViewPop->LastSelection=NULL;
    CurrentViewPop->SelectedToSet(CurrentViewPop->LastSelection,CurrentViewPop->LastSelCount);
    MainListviewVisible=true;
    MainTabctrlChange(NULL);
    MainListviewVisible=false;
  }
} //TNwMainForm::ToolsSelectRelativesExecute
//---------------------------------------------------------------------------


void __fastcall TMainForm::MainListviewColumnClick(TObject *Sender,
      TListColumn *Column)
{ int f;
  if (CurrentViewPop != NULL)
  { if (MainTabctrl->TabIndex==IndividualsTab)
    { MainListviewExit(NULL); //Store Focused and all Selected VPInd
      CurrentViewPop->SortIL(Column->Index); //sort indiv list
      MainListviewEnter(NULL); //Set Focused and all Selected VPInd
      //scroll to focused item:
      f=CurrentViewPop->IL->GetIndex(CurrentViewPop->FocusInd);
      if (f>-1)
         MainListview->Items->Item[f]->MakeVisible(false);
    }
    /*else if (MainTabctrl->TabIndex==1) Crossing list: not implemented
      CurrentViewPop->SortCL(Column->Index); //sort crossing list */

    MainListview->Refresh();
  }
} //TNwMainForm::MainListviewColumnClick
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{ RegWriteMRUfiles();
  ClearData();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::MainListviewSelectItem(TObject *Sender,
      TListItem *Item, bool Selected)
{ if (MainListview->SelCount <= 0) CurrentViewPop->ClearSelection();
  //toevoegen: SelectAll
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::PopTreeViewDeletion(TObject *Sender,
      TTreeNode *Node)
{ delete (TViewPop*) Node->Data; }
//---------------------------------------------------------------------------

/*void __fastcall TNwMainForm::PopTreeViewClick(TObject *Sender)
//not called anymore as OnClick handler, but called from other places
//OnClick handler replaced by OnChange and OnMouseDown
{ if (PopTreeView->Selected==NULL)
  { if (CurrentViewPop != NULL) PopTreeView->Selected = CurrentViewPop->VPNode;
  }
  else
  { if ( (TViewPop*)PopTreeView->Selected->Data != CurrentViewPop)
      if (MainTabctrl->TabIndex<OverviewTab && CurrentViewPop!=NULL)
        MainListviewExit(NULL);
    MainListview->Items->Clear(); //else in SetTabs calls to OnData may be undefined
    CurrentViewPop = (TViewPop*) PopTreeView->Selected->Data;
    SetTabs();
    if (CurrentViewPop->CurrVW<-1 || CurrentViewPop->CurrVW>=CurrentViewPop->VWCount)
       CurrentViewPop->CurrVW=-OverviewTab; //-> TabIndex 0
    MainTabctrl->TabIndex=CurrentViewPop->CurrVW + OverviewTab;
    MainTabctrlChange(NULL);
  }
} //TNwMainForm::PopTreeViewClick    */
//---------------------------------------------------------------------------
void __fastcall TMainForm::PopTreeViewChange(TObject *Sender, TTreeNode *Node)
{ if (PopTreeView->Selected==NULL)
  { if (CurrentViewPop != NULL) PopTreeView->Selected = CurrentViewPop->VPNode;
  }
  else
  { if ( (TViewPop*)PopTreeView->Selected->Data != CurrentViewPop)
      if (MainTabctrl->TabIndex<OverviewTab && CurrentViewPop!=NULL)
        MainListviewExit(NULL);
    MainListview->Items->Clear(); //else in SetTabs calls to OnData may be undefined
    CurrentViewPop = (TViewPop*) PopTreeView->Selected->Data;
    SetTabs();
    if (CurrentViewPop->CurrVW<-1 || CurrentViewPop->CurrVW>=CurrentViewPop->VWCount)
       CurrentViewPop->CurrVW=-OverviewTab; //-> TabIndex 0
    MainTabctrl->TabIndex=CurrentViewPop->CurrVW + OverviewTab;
    MainTabctrlChange(NULL);
  }
} //TNwMainForm::PopTreeViewChange
//---------------------------------------------------------------------------

void __fastcall TMainForm::PopTreeViewMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{ TTreeNode *Node;
  //TRect R;
  Node=PopTreeView->GetNodeAt(X,Y);
  if (Node!=NULL)
  { PopTreeView->Select(Node);
    //if (Shift.Contains(ssRight))  doesn't work ??
    if (Button==mbRight)
    { //R=Node->DisplayRect(1);
      PopulationMenu->Popup(PopTreeView->ClientOrigin.x+X,
                            PopTreeView->ClientOrigin.y+Y);
    }
  }
} //TNwMainForm::PopTreeViewMouseUp
/*
void UnselectSubnodes(TTreeNode *Node)
{ int i;
  for (i=0; i<Node->Count; i++)
  { Node->Item[i]->Selected=false;
    UnselectSubnodes(Node->Item[i]);
  }
}
*/
void AddSelectedToList(TList *Lst, TTreeNode *Node)
{ int i;
  if (Node->Selected) Lst->Add(Node);
  for (i=0; i<Node->Count; i++) AddSelectedToList(Lst,Node->Item[i]);
}

void __fastcall TMainForm::PopDeleteExecute(TObject *Sender)
{ TTreeNode *Curr,*Parent,
            *LastSelected; //the focused selected node or, if any ancestors of it
                           //are also selected, the one closest to the root
  TList *Blacklist; //list with LastSelected and all selected nodes in its offspring;
                    //always includes the focused node.
                    //nodes on this list may not be deleted until all others have gone
                    //and then all nodes on this list are deleted by deleting LastSelected
  bool ready; int p;
  if (PopTreeView->Items->Item[0]->Selected)
    ShowMessage("Cannot delete base population");
  else if (!PopTreeView->Selected->Selected)
    //PopTreeView->Selected is the last clicked node, i.e. has focus, but is it selected?
    ShowMessage("Current population must be part of the selection");
  else
  { if ( ( (PopTreeView->SelectionCount > 1) &&
           (MessageDlg("Deleting multiple populations including any sub-populations; proceed?",
                    mtConfirmation,TMsgDlgButtons() << mbYes << mbNo,0)
            == mrYes )
         ) ||
         ( (PopTreeView->SelectionCount == 1) &&
           //(PopTreeView->Selections[0]->Parent != NULL) &&  //root cannot be deleted
           (MessageDlg("Deleting current population including any sub-populations; proceed?",
                    mtConfirmation,TMsgDlgButtons() << mbYes << mbNo,0)
             == mrYes )
         )
       )

    { //delete the selected populations one by one
      //deleting one may also delete selected subpopulations below it
      //When deleting the focused population, another is selected automatically
      //and the Selections are not reliable anymore.
      //Therefore the focused node must be deleted last.
      //But if the focussed node (selected or not)is a child (or further generation) of another
      //selected node that is not possible.
      //Also strange things seem to happen to the Selection if any node is deselected.
      //Therefore:
      //We find the selected node LastSelected that is the earliest parent of the
      //focused node. This node and all its Selected offspring are put in the
      //Blacklist (and this includes the focused node).
      //Then we delete first all the selected nodes that are not in Blacklist:
      //as these are not Focused, nothing strange happens to the Selections.
      //Last we delete LastSelected, which automatically deletes also all other
      //nodes in Blacklist.
      //Finally we et focus to the parent of LastSelected, which is the most
      //"advanced" surviving ancestor of the original focused node.

      //first, find LastSelected:
      ready=false; LastSelected=PopTreeView->Selected; //the focused node
      do
      { Curr=LastSelected;
        while (Curr!=NULL && Curr->Parent!=NULL && !Curr->Parent->Selected)
          Curr=Curr->Parent;
        if (Curr==NULL || Curr->Parent==NULL)
        { //root node; there is no selected ancestor of LastSelected
           ready=true;
        } else
        { //Curr->Parent is a selected subpopulation
          LastSelected=Curr->Parent;
        }
      } while (!ready);

      //next, fill Blacklist:
      Blacklist=new TList;
      try {
        AddSelectedToList(Blacklist,LastSelected); //recursive

        //delete all selected nodes not on the Blacklist:
        while (PopTreeView->SelectionCount > Blacklist->Count)
        { p=PopTreeView->SelectionCount-1;
          while (p>0 && Blacklist->IndexOf(PopTreeView->Selections[p])>=0)
            p--;
          PopTreeView->Selections[p]->Delete();
        }
        //delete nodes on the Blacklist:
        Parent=LastSelected->Parent;
        LastSelected->Delete();
        Parent->Focused=true; //doesn't work
        CurrentViewPop=(TViewPop*)(Parent->Data);
      }
      __finally { delete(Blacklist); }

      PopTreeViewChange(NULL,NULL); //not needed?
      ProjectSaved=false;
    }
  } //else
} //TNwMainForm::PopDeleteExecute
//---------------------------------------------------------------------------




void __fastcall TMainForm::PopCombinePopsExecute(TObject *Sender)
{ TTreeNode *Node,*Root;
  int i,p;
  AnsiString s;
  TViewPop *TempVP,*Source;
  if (PopTreeView->Items->Item[0]->Selected)
    ShowMessage("Cannot combine with base population");
  else if (!PopTreeView->Selected->Selected)
    //PopTreeView->Selected is the last clicked node, i.e. has focus, but is it selected?
    ShowMessage("Current population must be part of the selection");
  else if (PopTreeView->SelectionCount<2)
    ShowMessage("At least 2 populations must be selected");
  else
  { MainListview->Items->Clear();
    MainListview->Columns->Clear();
    CurrentViewPop=NULL;
    Root=PopTreeView->Items->Item[0];
    //Make unique name for new ViewPop:
    p=0;
    do {
      p++;
      s="Combination_"+IntToStr(p);
      i=Root->Count-1;
      while (i>=0 &&
             UpperCase(Root->Item[i]->Text) != UpperCase(s))
        i--;
    } while (i>=0);
    //generate new node directly under root:
    Node=PopTreeView->Items->AddChild(Root,s);
    Node->ImageIndex=57;
    TempVP = new TViewPop(Node,-1); //also makes Node->Data refer to new VP
    TempVP->Notes->Add("This node combines all individuals from the following nodes:");
    for (p=0; p<PopTreeView->SelectionCount; p++)
    { Source=(TViewPop*)PopTreeView->Selections[p]->Data;
      for (i=0; i<Source->VPIndCount; i++)
      { if (TempVP->GetVPIndex(Source->VPInd[i])==-1)
          TempVP->AddVPInd(Source->VPInd[i]);
      }
      TempVP->Notes->Add(Source->VPNode->Text);
    }
   //Generate a new SelScript consisting of a manual selection of all
   //Indivs in the new VP:
   TempVP->ClearSelection();
   for (i=0;i<TempVP->VPIndCount; i++)
     TempVP->Selected[i]=true;
   TempVP->UpdateSelection(-1);
   TempVP->OrigScript.Copy(TempVP->SelScript);
   //Clear selection in combination VP and make IndList and Overview::
   TempVP->ClearSelection();
   TempVP->AddVW("&Overview",true); //first view:Overview
   TempVP->VW[TempVP->CurrVW]->CalcInitialChart();
   CurrentViewPop=TempVP;
  //PopTreeViewChange(NULL,NULL); not needed?
  ProjectSaved=false;

  } //else
} //TMainForm::PopCombinePopsExecute
//---------------------------------------------------------------------------

void __fastcall TMainForm::PopInfoExecute(TObject *Sender)
{ TSelectionScript *Scr; TTreeNode *Node;

  Node=PopTreeView->Selected;
  Scr=&CurrentViewPop->OrigScript;
  InfoForm->FlagCombx->ItemIndex=CurrentViewPop->Flag;
  InfoForm->SelMemo->Clear();
  InfoForm->SelMemo->Lines->Add("Selection information for (sub-)population "+Node->Text);
  InfoForm->SelMemo->Lines->Add("");
  if (Node->Parent != NULL)
   InfoForm->SelMemo->Lines->Add("Derived from (sub-)population "+Node->Parent->Text);
  Scr->WriteLines(true,InfoForm->SelMemo->Lines);
  InfoForm->NotesMemo->Lines->Assign(CurrentViewPop->Notes);
  InfoForm->NotesMemo->Modified=false;
  InfoForm->ShowModal();
  if (InfoForm->NotesMemo->Modified)
  { CurrentViewPop->Notes->Assign(InfoForm->NotesMemo->Lines);
    ProjectSaved=false;
  }
  if (InfoForm->FlagCombx->ItemIndex != CurrentViewPop->Flag)
  { CurrentViewPop->Flag=InfoForm->FlagCombx->ItemIndex;
    Node->ImageIndex=CurrentViewPop->Flag;
    Node->SelectedIndex=Node->ImageIndex;
    PopTreeView->Refresh();
    ProjectSaved=false;
  }
}

//******************************************************************
// File reading & writing routines
//******************************************************************

void TMainForm::StartProject(void)
// Creates a new project with the data already read
{ TTreeNode *Node;

  if (PopTreeView->Items->Count != 0)
  { ShowMessage("Error: Treeview not empty in StartProject"); return; }

  Node=PopTreeView->Items->Add(NULL,PopName);
  CurrentViewPop = MakeOverallViewPop(Node); //with 1 View: Overview
  DataListWidths();
  CurrentViewPop->VW[CurrentViewPop->CurrVW]->CalcInitialChart();
  //ShowCurrentView(CurrentViewPop);
  SetTabs();
  MainTabctrl->TabIndex=CurrentViewPop->CurrVW + OverviewTab;
  MainTabctrlChange(NULL);
} //TNwMainForm::StartProject


//---------------------------------------------------------------------------
void TMainForm::SaveProject(AnsiString SaveDir, AnsiString SaveName)
{ TStringList *SL;
  NodesWritten=0;
  if (PopTreeView->Items->Count > 0)
  {
    SL = new TStringList;
    SL->Add("PedimapFileVersion=10");
    if (DataDir==SaveDir) SL->Add("DataFile "+DQ+DataFileName+DataFileExt+DQ);
    else SL->Add("DataFile "+DQ+DataDir+DataFileName+DataFileExt+DQ);
    SL->Add("IBDAlleles "+IntToStr(IBDAlleleCount));

    NodesWritten=0;
    WriteNode(PopTreeView->Items->Item[0],SL);
    SL->Add(""); SL->Add("[FILE_END]");
    SL->SaveToFile(SaveDir+SaveName+ProjectExt);
    delete SL;
    ProjectSaved=true;
  }
} //TNwMainForm::SaveProject
//---------------------------------------------------------------------------

bool TMainForm::OpenProjectFile(AnsiString FileName, bool ReadData)
//Reads the project file and creates the ViewPop tree-structure and
//Viewpop contents.
//if ReadData=false, the data currently loaded are used, else the datafile
//specified in the project file is used.
//result=true indicates success
//OpenProjectFile resets DataDir, DataFileName and DataFileExt if ReadData=true,
//but not ProjectDir and ProjectFileName
{ TStringList *SL;
  AnsiString woord, TempFileName;
  int r,p,PedimapFileVersion,
      OldIBDAlleleCount;
         //IBDAlleles of the datafile that was loaded when this project was
         //saved; datafile may have changed since then
  bool DataOK,LicOK;

  SL=new TStringList; DataOK=true;

  try
  { //ClearData();
    SL->LoadFromFile(FileName); //the project file
    r=0;p=1;
    ReadWoordSL(r,p,woord,SL);
    if (woord.Length()!=21 || woord.SubString(1,19)!="PedimapFileVersion=")
       throw Exception(DQ + "PedimapFileVersion" + DQ + " not found");
    try {
        PedimapFileVersion=StrToInt(woord.SubString(20,2));
    }
    catch (Exception &e)
    { throw Exception("Invalid PedimapFileVersion number"); }
    if (PedimapFileVersion>10)
      throw Exception ("Project file saved by newer version of Pedimap");

    //  if (woord!="PedimapFileVersion=10")
    //  throw Exception(DQ + "PedimapFileVersion" + DQ + " not found");
    r++; p=1;
    ReadWoordSL(r,p,woord,SL);
      if (woord!="DataFile")
      throw Exception(DQ+"DataFile"+DQ+" not found");
    ReadWoordSL(r,p,TempFileName,SL); //Tempfilename is name of Datafile read from Projectfile
    r++; p=1;
    ReadWoordSL(r,p,woord,SL);
      if (woord!="IBDAlleles")
      throw Exception(DQ+"IBDAlleles"+DQ+" not found");
    ReadWoordSL(r,p,woord,SL); OldIBDAlleleCount=StrToInt(woord);
    //Projectfile is valid, keep path and name:
    ProjectDir=ExtractFilePath(FileName);
    ProjectFileName=ExtractFileName(FileName);
    ProjectFileName=ProjectFileName.SubString(1,ProjectFileName.Length()-ProjectExt.Length());

    if (ReadData)
    { ClearData();
      DataFileName=TempFileName;
      if (ExtractFilePath(TempFileName)=="")
         TempFileName=ProjectDir+TempFileName;
      //DataOK=ReadDataFile(DataFileName);
      DataOK=ReadDataFile(TempFileName);  //17-01-2007
      if (DataOK)
      { /*moved to ReadDataFile 17-01-2007:
        DataDir=ExtractFilePath(TempFileName);
        DataFileName=ExtractFileName(DataFileName);
        DataFileExt=ExtractFileExt(DataFileName);
        DataFileName=DataFileName.SubString(1, DataFileName.Length()
                                               - DataFileExt.Length());
        */
        woord="";
        SelectDlgForm->SetParentlineRgrpItems(FemaleParent);
        OptionsForm->SetParentColorCaptions(FemaleParent);
      } //DataOK
    } //ReadData
    if (DataOK)
    { StartProject(); //creates first node: OverallViewPop
      LicOK=TestLicense();
      //ReadViewpopForm->Show();
      while (r<SL->Count && UpperCase(woord)!="[FILE_END]" &&
                (LicOK || TViewPop::VPCount<3) )
      { while (r<SL->Count && UpperCase(woord)!="[SUBPOP")
        { r++; p=1; ReadWoordSL(r,p,woord,SL); }
        if (UpperCase(woord)=="[SUBPOP")
        { ReadViewpopData(r,SL,OldIBDAlleleCount);
          woord="";
        }
      }
      //ReadViewpopForm->Hide();
      PopTreeView->FullExpand();
      PopTreeView->Selected=PopTreeView->Items->Item[0];
      //PopTreeViewChange(NULL,NULL); not needed?
      if (!LicOK) ShowUnlicensedMessage();
      return true;
    }
    else return false;
  }
  catch (Exception &e)
  { FileClearExecute(NULL);
    ShowMessage ("Error opening project "+FileName+":"+LF+e.Message);
    return false;
  }
} //TNwMainForm::OpenProjectFile

void TMainForm::WriteNode(TTreeNode *Node, TStringList *SL)
// writes itself and all its children, by calling itself recursively
{ int i;
  if (TestLicense() || NodesWritten<3) { //if not licensed, write only first two subpopulations
    ((TViewPop*) (Node->Data))->WriteData(SL);
    NodesWritten++;
    for (i=0; i<Node->Count; i++) WriteNode(Node->Item[i], SL);
  }
} //TNwMainForm::WriteNode

TTreeNode* TMainForm::FindNode(int VPnum)
{ int i;
  i=0;
  while (i<PopTreeView->Items->Count &&
          (PopTreeView->Items->Item[i]==NULL ||
           ((TViewPop*) (PopTreeView->Items->Item[i]->Data))->VPnum != VPnum))
  i++;
  if (i>=PopTreeView->Items->Count) return NULL;
  else return PopTreeView->Items->Item[i];
} //NwMainForm::FindNode

void TMainForm::ReadViewData(TViewPop *VP, int OldIBDAlleleCount, TStringList *SL)
{ int r,p,v; AnsiString woord;
  //TView *VW;
  r=0; p=1; ReadWoordSL(r,p,woord, SL);
  if (woord!="[View") throw Exception("ReadView: View not found");
  ReadWoordSL(r,p,woord,SL);
  if (woord=="") throw Exception("ReadView: View not found");
  v = VP->AddVW(woord,false); //not Overview
  VP->VW[--v]->Opt.ReadFromStringlist(SL,OldIBDAlleleCount,false);
  VP->VW[v]->CalcInitialChart();
} //TNwMainForm::ReadViewData

void TMainForm::ReadViewpopData(int &r, TStringList *SL, int OldIBDAlleleCount)
// really required data that are missing generate Exceptions that are passed
// to calling routine.
// SelScripts referring to non-existing individuals just abort creation of the
// new node and ViewPop
// Similar (in future) with Views referring to non-existing traits, loci,
// alleles etc: these views are not created
{ //r points to line with [SUBPOP n]
  int p,i,n,VPnumber,ParentNumber,LastSub, Flag, Ordering, CurrView;
  AnsiString woord,NodeText;
  TTreeNode *ParentNode, *Node;
  TViewPop *VP, *ParentVP;
  TSelectionScript Scr;
  TStringList *ViewSL;
  //bool VPok; //true if (ParentVP exists or VPnumber==0) and >0 indivs in VP
  p=1; ReadWoordSL(r,p,woord,SL);
  if (woord != "[SUBPOP")
     throw Exception("ReadData: SUBPOP not found");
  ReadWoordSL(r,p,woord,SL);
  VPnumber=StrToInt(woord);
  r++; p=1; ReadWoordSL(r,p,woord,SL);
  if (UpperCase(woord) != "NAME")
     throw Exception("ReadData: Name not found");
  ReadWoordSL(r,p,NodeText,SL);
  //ReadViewpopForm->ViewpopLbl->Caption=NodeText;
  //Application->ProcessMessages();
  //VPNode = new TTreeNode(NwMainForm->PopTreeView->Items);
  //VPNode->Text = SL->Strings[r].SubString(p,SL->Strings[r].Length());
  r++; p=1; ReadWoordSL(r,p,woord,SL);
  if (UpperCase(woord) != "ORDERING")
     throw Exception("ReadData: Ordering not found");
  ReadWoordSL(r,p,woord,SL); Ordering=StrToInt(woord);
  if (Ordering==ordManual)
  { //fill in; check if all listed Indivs exist,
    //and if all Indivs in VP are listed
  }
  r++; p=1; ReadWoordSL(r,p,woord,SL);
  if (UpperCase(woord) != "LASTSUB")
     throw Exception("ReadData: LastSub not found");
  ReadWoordSL(r,p,woord,SL); LastSub=StrToInt(woord);
  r++; p=1; ReadWoordSL(r,p,woord,SL);
  if (UpperCase(woord) != "FLAG")
  { Flag=0; r--; }
  else { ReadWoordSL(r,p,woord,SL); Flag=StrToInt(woord); }
  r++; p=1; ReadWoordSL(r,p,woord,SL);
  if (UpperCase(woord) != "ORIGIN")
     throw Exception("ReadData: Origin not found");
  ReadWoordSL(r,p,woord,SL); ParentNumber=StrToInt(woord);
  if (ParentNumber<0) ParentNode=NULL;
  else ParentNode=FindNode(ParentNumber); //may be NULL if ParentNode not created
  if (ParentNode==NULL) ParentVP=NULL;
  else ParentVP = (TViewPop*) (ParentNode->Data);

  Node=NULL; VP=NULL; 
  if (Scr.ReadLines(++r,SL,ParentVP))
  { //all individuals in Scr really exist
    if (VPnumber==0)
    { // root node and OverallViewPop (with IndivList and Overview) already created
      if (PopTreeView->Items->Count !=1)
        throw Exception("ReadData: NodeCount error");
      Node = PopTreeView->Items->Item[0]; VP=(TViewPop*) (Node->Data);
      if (VP->Ordering != Ordering) VP->ChangeOrdering(Ordering);
    }
    else
    { // not root node: check if not empty:
      n=0;
      if (ParentVP!=NULL)
      { Scr.Run(ParentVP);
        for (i=0; i<ParentVP->VPIndCount; i++)
           if (ParentVP->Selected[i]) n++;
      }
      if (n>0)
      { //ParentVP exists and VP not empty: create Node and VP
        Node = PopTreeView->Items->AddChild(ParentNode,NodeText);
        (TViewPop*) (Node->Data) = VP = new TViewPop(Node,VPnumber);
        for (i=0; i<ParentVP->VPIndCount; i++)
          if (ParentVP->Selected[i]) VP->AddVPInd(ParentVP->VPInd[i]);
        ParentVP->ClearSelection();
        VP->AddVW("&Overview",true); //first view:Overview
        VP->CurrVW=VP->VWCount-1;
        VP->Ordering=Ordering;
        //VP->VW[VP->CurrVW]->CalcInitialChart();
      }
    }
  }

  if (VP != NULL)
  { //Node and VP have been created: either root or Subpop,
    //and Overview added and chart calculated.
    //Now set remaining ViewPop attributes:
    VP->OrigScript.Copy(Scr); Scr.Clear();
    Node->Text=NodeText;
    VP->LastSubpopNumber=LastSub;
    VP->Flag=Flag;
    Node->ImageIndex=Flag;
    Node->SelectedIndex=Node->ImageIndex;
  }

  //now, read remaining lines of SUBPOP including VIEWs
  //but only process if VPok

  //read Notes
  //skip empty lines:
  do { p=1; ReadWoordSL(++r,p,woord,SL); }
  while (r<SL->Count && woord=="");

  if (woord=="[Notes")
  { ReadWoordSL(r,p,woord,SL); n=StrToInt(woord);
    for (i=0; i<n; i++)
    { r++;
      if (VP!=NULL) VP->Notes->Add(SL->Strings[r]);
    }
    p=1; ReadWoordSL(++r,p,woord,SL);
    if (woord != "[Notes_end]")
       throw Exception("ReadData: Notes_End not found");
  }

  //read current view number:
  //skip empty lines:
  do { p=1; ReadWoordSL(++r,p,woord,SL); }
  while (r<SL->Count && woord=="");
  if (woord=="CurrentView")
  {  try { ReadWoordSL(r,p,woord,SL);
           CurrView=StrToInt(woord); }
     catch (...) { CurrView=0; } //Overview
  }
  else //older project file (before 01-06-2005) without CurrentView:
  {  CurrView=0; //Overview
     r--; //read this line again
  }

  //read Views
  do
  { //skip empty lines:
    do { p=1; ReadWoordSL(++r,p,woord,SL); }
    while (r<SL->Count && woord=="");

    if (woord=="[View")
    { ViewSL = new TStringList();
      do {
         ViewSL->Add(SL->Strings[r]);
         p=1; ReadWoordSL(++r,p,woord,SL);
      } while (r<SL->Count && woord!="[View_end]");
      if (woord=="[View_end]" && VP!=NULL)
        ReadViewData(VP,OldIBDAlleleCount,ViewSL);
      delete ViewSL; ViewSL=NULL;
    }
  } while (woord=="[View_end]");

  if (VP!=NULL)
  { if (VP->VWCount<=CurrView) VP->CurrVW=0; //OverView
    else VP->CurrVW=CurrView;
  }

} //TNwMainForm::ReadViewpopData

//---------------------------------------------------------------------------

void __fastcall TMainForm::FileImportExecute(TObject *Sender)
{ bool ReOpen;
  AnsiString TempProjectName, TempProjectDir;

  ReOpen=false;
  if (PopTreeView->Items->Count>0)
  { //currently project open:
    if ( ProjectSaved &&
         MessageDlg("Replace data in current project ?", mtConfirmation,
                   TMsgDlgButtons() << mbOK << mbCancel,0)
           != mrOk )
        return;
    if (!ProjectSaved &&
         MessageDlg("Replace data in current project ?"+CRLF+CRLF+
                    "Note: current project not saved!", mtConfirmation,
                   TMsgDlgButtons() << mbOK << mbCancel,0)
           != mrOk )
        return;
    else { TempProjectName=ProjectFileName; TempProjectDir=ProjectDir;
           SaveProject(TempDir,TempProjectName);
           ReOpen=true;
           ClearData();
         }
  }
  // Now: open new datafile:
  //OpenDialog->InitialDir=DataDir;
  OpenDialog->FileName="";
  OpenDialog->Filter="";
  if ( OpenDialog->Execute() &&
       ReadDataFile(OpenDialog->FileName) )
  { SelectDlgForm->SetParentlineRgrpItems(FemaleParent);
    OptionsForm->SetParentColorCaptions(FemaleParent);
    DataDir = ExtractFilePath(OpenDialog->FileName);
    DataFileName = ExtractFileName(OpenDialog->FileName);
    DataFileExt= ExtractFileExt(DataFileName);
    DataFileName = DataFileName.SubString(1,DataFileName.Length()
                   - DataFileExt.Length()); //including DataDir, excluding Extension


    if (ReOpen)
    { OpenProjectFile(TempDir+TempProjectName+ProjectExt,false);
                                   //false: don't read datafile
      ProjectFileName=TempProjectName;
      ProjectDir=TempProjectDir;
      DeleteFile(TempDir+TempProjectName+ProjectExt);
      ProjectSaved=false;
    }
    else
    { StartProject();
      ProjectSaved=false; //needed for AddSemifounderParents, ImputS
      Caption="Pedimap - NONAME";
    }
    SetLegendColumns();
  }
} //TNwMainForm::FileImportExecute
//---------------------------------------------------------------------------

void TMainForm::DoFileOpen(const AnsiString FName)
{ TCursor Save_Cursor;

    Save_Cursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;    // Show hourglass cursor
    try
    { if ( OpenProjectFile(FName, true) )
      {  //moved to OpenProjectFile 17-01-2007:
         //still needed! 14-02-2010
         ProjectDir=ExtractFilePath(FName);
         ProjectFileName=ExtractFileName(FName);
         ProjectFileName=ProjectFileName.SubString(1,ProjectFileName.Length()-ProjectExt.Length());
         
         Caption="Pedimap - "+ProjectFileName;
         UpdateMRUfiles(FName);
      }
    }
    __finally { Screen->Cursor = Save_Cursor; } // always restore the cursor
} //TNwMainForm::DoFileOpen

void __fastcall TMainForm::FileOpenExecute(TObject *Sender)
{ //TCursor Save_Cursor;
  FileClearExecute(NULL);
  //OpenDialog->InitialDir=ProjectDir;
  OpenDialog->FileName="";
  OpenDialog->Filter="Pedimap projects (*"+ProjectExt+")|*"+ProjectExt;
  if ( OpenDialog->Execute() )
  { DoFileOpen(OpenDialog->FileName); }
  SetLegendColumns();
} //TNwMainForm::FileOpenExecute

void __fastcall TMainForm::MRUClick(TObject *Sender)
//as FileOpenExecute without dialog
{ int MRUindex;
  //if (typeid(Sender) == typeid(TMenuItem)) doesn't work
  try
  { MRUindex = StrToInt( ((TMenuItem*)Sender)->Caption.SubString(2,1) ) -1 ; }
  catch (...)
  { MRUindex=MaxMRU; }
  if (MRUindex<MaxMRU)
  { FileClearExecute(NULL);
    DoFileOpen(MRUfile[MRUindex]);
    SetLegendColumns();
  }
  //     ShowMessage("Clicked "+MRUfile[StrToInt(((TMenuItem*)(Sender))->Caption[2])-1]);
} //TMainForm::MRUClick


//---------------------------------------------------------------------------

void __fastcall TMainForm::FileSaveExecute(TObject *Sender)
{ if (ProjectFileName=="") FileSaveAsExecute(NULL);
  else SaveProject(ProjectDir,ProjectFileName);
} //TNwMainForm::FileSaveExecute
//---------------------------------------------------------------------------

void __fastcall TMainForm::FileSaveAsExecute(TObject *Sender)
{ if (ProjectDir!="") SaveDialog->InitialDir=ProjectDir;
  else if (DataDir!="") SaveDialog->InitialDir=DataDir;
  else SaveDialog->InitialDir=OpenDialog->InitialDir;
  if (ProjectFileName != "") SaveDialog->FileName=ProjectFileName;
  else if (DataFileName != "") SaveDialog->FileName=DataFileName;
  else SaveDialog->FileName="";
  if (SaveDialog->FileName != "" &&
      (UpperCase(ExtractFileExt(SaveDialog->FileName)) != UpperCase(ProjectExt)) )
    SaveDialog->FileName = SaveDialog->FileName + ProjectExt;
  SaveDialog->DefaultExt="";
  SaveDialog->Filter="Pedimap projects (*"+ProjectExt+")|*"+ProjectExt;
  if (SaveDialog->Execute())
  { if (UpperCase(ExtractFileExt(SaveDialog->FileName)) != UpperCase(ProjectExt))
       SaveDialog->FileName = SaveDialog->FileName+ProjectExt;
    ProjectDir=ExtractFilePath(SaveDialog->FileName);
    ProjectFileName=ExtractFileName(SaveDialog->FileName);
    ProjectFileName=ProjectFileName.SubString(1,ProjectFileName.Length()-ProjectExt.Length());
    Caption="Pedimap - "+ProjectFileName;
    SaveProject(ProjectDir,ProjectFileName);
    UpdateMRUfiles(SaveDialog->FileName);
  }
} //TNwMainForm::FileSaveAsExecute
//---------------------------------------------------------------------------

void __fastcall TMainForm::FileClearExecute(TObject *Sender)
{ TModalResult mr;

  if ( !ProjectSaved )
  { mr = MessageDlg("Save project ?", mtConfirmation,
                   TMsgDlgButtons() << mbYes <<mbNo << mbCancel, 0);
    if (mr==mrCancel) return;
    if (mr==mrYes) FileSaveExecute(NULL);
  }
  ClearData();
  MainListview->Clear();
  InfoMemo->Lines->Clear();
  //delete ViewImage->Picture;
  //BackPanel->Width=0; BackPanel->Height=0;
  PagePanel->Width=0; PagePanel->Height=0;
  ViewScrollboxResize(NULL);
} //TNwMainForm::FileClearExecute
//---------------------------------------------------------------------------

void __fastcall TMainForm::ZoomCombxChange(TObject *Sender)
{  TView *V;
  if (CurrentViewPop!=NULL && CurrentViewPop->CurrVW>=0 &&
      CurrentViewPop->CurrVW<CurrentViewPop->VWCount &&
      (V=CurrentViewPop->VW[CurrentViewPop->CurrVW]) !=NULL &&
      V->ChartMF!=NULL)
  { try
    { V->Zoom=StrToInt(ZoomCombx->Items->Strings[ZoomCombx->ItemIndex]); //in % of true size
      //ZoomFact = V->Zoom/100.0 * Screen->PixelsPerInch / double(PrPPI); //in pix
      //V->ZoomedChartWidth  = ZoomFact*V->ChartMF->Width+1;
      //V->ZoomedChartHeight = ZoomFact*V->ChartMF->Height+1;
      //V->CalcZoomData();
      //ViewScrollbox->HorzScrollBar->Range=V->ZoomedChartWidth+2*Bor;
      //ViewScrollbox->VertScrollBar->Range=V->ZoomedChartHeight+2*Bor;
      ViewScrollbox->HorzScrollBar->Position=0;
      ViewScrollbox->VertScrollBar->Position=0;
      #ifdef VISAREA
        //DrawVisArea();
        ShowCurrentView(CurrentViewPop);
      #else
        ZoomBM->Width=0; //must be redrawn
        ShowCurrentView(CurrentViewPop);
      #endif
      MainTabctrl->SetFocus();
    }
    catch (...) { ; } //if ZoomCombx empty or non-numeric, nothing happens
  }
} //TNwMainForm::ZoomCombxChange
//---------------------------------------------------------------------------

void __fastcall TMainForm::ChartCopyExecute(TObject *Sender)
{  TView *V;
   Graphics::TMetafile *MF;
  if (CurrentViewPop!=NULL && CurrentViewPop->CurrVW>=0 &&
      CurrentViewPop->CurrVW<CurrentViewPop->VWCount &&
      (V=CurrentViewPop->VW[CurrentViewPop->CurrVW]) !=NULL &&
      V->ChartMF!=NULL)
  { try
    { MF=NULL;
      TStringList* coord = new TStringList(); //needed for DrawChartExportMF
      try
      {
        MF = V->DrawChartExportMF(coord);
        Clipboard()->Assign(MF);
      }
      __finally {delete MF; coord->Clear();}
    }
    catch (...) { ; }
  }
  else ShowMessage("No chart to copy");
} //TNwMainForm::ChartCopyExecute

//---------------------------------------------------------------------------

void __fastcall TMainForm::ChartSaveAsExecute(TObject *Sender)
{  TView *V;
   AnsiString fname;
   Graphics::TMetafile *MF = NULL;
   Graphics::TBitmap *BM = NULL;
  if (CurrentViewPop!=NULL && CurrentViewPop->CurrVW>=0 &&
      CurrentViewPop->CurrVW<CurrentViewPop->VWCount &&
      (V=CurrentViewPop->VW[CurrentViewPop->CurrVW]) !=NULL &&
      V->ChartMF!=NULL)
  { SaveDialog->InitialDir=ProjectDir;
    SaveDialog->FileName = CleanFilename(CurrentViewPop->VPNode->Text + "-"
                           + MainTabctrl->Tabs->Strings[MainTabctrl->TabIndex], true);
    SaveDialog->DefaultExt=""; //".emf";
    SaveDialog->Filter="Enhanced metafile (*.emf)|*.emf|Bitmap (*.bmp)!*.bmp";
    SaveDialog->FilterIndex=1; //emf
    if (SaveDialog->Execute())
    { try
      { if (ExtractFileExt(SaveDialog->FileName)=="")
        { if (SaveDialog->FilterIndex==1)
             SaveDialog->FileName = SaveDialog->FileName+".emf";
          else if (SaveDialog->FilterIndex==2)
             SaveDialog->FileName = SaveDialog->FileName+".bmp";
        }
        TStringList* coord = new TStringList(); //not used here
        try
        { MF=NULL; BM=NULL;
          MF = V->DrawChartExportMF(coord);
          if (LowerCase(ExtractFileExt(SaveDialog->FileName))==".emf")
              MF->SaveToFile(SaveDialog->FileName);
          else if (LowerCase(ExtractFileExt(SaveDialog->FileName))==".bmp")
          { BM = new Graphics::TBitmap;
            BM->Width=MF->Width;
            BM->Height=MF->Height;
            BM->Transparent=false;
            BM->Canvas->Brush->Color=V->Opt.PageCol;
            BM->Canvas->Brush->Style=bsSolid;
            BM->Canvas->FillRect(Rect(0,0,BM->Width+1,BM->Height+1));
            BM->Canvas->Draw(0,0,MF);
            BM->SaveToFile(SaveDialog->FileName);
          }
          else ShowMessage ("Only *.emf or *.bmp filetypes supported");
        }
        __finally { delete MF; delete BM; coord->Clear();}
      }
      catch (...) { ShowMessage("Error saving file "+SaveDialog->FileName); }
    }
    SaveDialog->DefaultExt="";
  }
  else ShowMessage("No chart to export");
} //TNwMainForm::ChartSaveAsExecute
//---------------------------------------------------------------------------

__int32 rotateLeft(__int32 x, int count, TStringList *SL) {
        AnsiString s;
        if (count<=0) return x;
        count = count % 32;
        //s="count="+IntToStr(count)+" mask1="+IntToHex(0xFFFFFFFF << count)+
        __int32 mask = ~(0xFFFFFFFF << count); //mask possible 1's shifted in from left:
        s=("count="+IntToStr(count)+" x="+IntToHex(x,8)+" mask="+IntToHex(mask,8)+" so1="+IntToHex(x >> (32-count),8)
                +" so2="+IntToHex((x >> (32-count)) & mask,8)+" res="+
                IntToHex((x << count) | ((x >> (32-count)) & mask),8) );
        __int32 shiftout = (x >> (32-count)) & mask; //0's shifted in from left
        SL->Add(s);
        return (x << count) | shiftout;
}

AnsiString Crypt(AnsiString s) {
  const char CryptFact = 0xAD; //10101101, converts the non-char (0x8F) into " in IWindows-1252
  for (int i=1; i<=s.Length(); i++)
    s[i] = s[i] ^ CryptFact; //xor
  return(s);
}

//The following routine generates a list of encrypted AnsiString declarations
//so that these cannot be found in the executable.
//Comment out the #define Encrypt to have the regular functioning Pedimap
//#define Encrypt
#ifdef Encrypt
void SaveEncryptedStrings(void) {
  AnsiString Q="p";
  Q[1]='"';
  AnsiString s = "Pedimap";
  TStringList *SL = new TStringList();
  SL->Add("AnsiString PedimapStr="+Q+Crypt("Pedimap")+Q+";");
  SL->Add("AnsiString Pedimap_CommandlineStr="+Q+Crypt("Pedimap-commandline")+Q+";");
  SL->Add("AnsiString PedimapVersionStr="+Q+Crypt("1.2")+Q+";");
  SL->Add("int PedimapMainversion=1;");
  SL->Add("AnsiString AuthorStr="+Q+Crypt("Roeland E. Voorrips")+Q+";");
  SL->Add("AnsiString CopyrightStr="+Q+Crypt("(C) Plant Research International 2004-2011")+Q+";");
  SL->Add("AnsiString LicensedtoStr="+Q+Crypt("Licensed to :")+Q+";");
  SL->Add("AnsiString NotlicensedStr="+Q+Crypt("Not licensed")+Q+";");
  SL->Add("AnsiString LicensevalidStr="+Q+Crypt("License valid until ")+Q+";");
  SL->Add("AnsiString LicenseexpiredStr="+Q+Crypt("License expired on ")+Q+";");
  SL->Add("AnsiString UnlicMessageStr="+Q+
    Crypt("Not licensed; restricted to reading and writing only 2 subpopulations.\n")+Q+";");
  SL->Add("AnsiString SeeWebsiteStr="+Q+
    Crypt("See wwww.plantbreeding.wur.nl/UK/software.html to obtain a license")+Q+";");
  SL->Add("AnsiString CheckWebsite1Str="+Q+
    Crypt("http://www.plantbreeding.wur.nl/UK/software.html")+Q+";");
  SL->Add("AnsiString CheckWebsite2Str="+Q+
    Crypt("For current version and licenses check")+Q+";");


  //SL->Add("Q converted="+Crypt(Q)); //just to show that the double quote is not
                                    //translated to something common and v.v.
  /*this prints out a list of the Windows-1252 character set:
  char C=' ';
  for (int i=32; i<=255; i++) {
    //Q[1]=char( i & 0x0F);
    SL->Add(IntToHex(i,4)+" "+AnsiString(C++));
  }
  */ 
  SL->SaveToFile("EncryptedCode.txt");

  ShowMessage("File EncryptedCode.txt saved");
  Application->Terminate();
}
#endif

//The encrypted strings are inserted here:
//NOTE: use ANSI, not (default) ASCII encoding when copying from Lister;
//Notepad and other editors are OK

AnsiString PedimapStr="ýÈÉÄÀÌÝ";
AnsiString Pedimap_CommandlineStr="ýÈÉÄÀÌÝ€ÎÂÀÀÌÃÉÁÄÃÈ";
AnsiString PedimapVersionStr="œƒŸ";
int PedimapMainversion=1;
AnsiString AuthorStr="ÿÂÈÁÌÃÉèƒûÂÂßßÄÝÞ";
AnsiString CopyrightStr="…î„ýÁÌÃÙÿÈÞÈÌßÎÅäÃÙÈßÃÌÙÄÂÃÌÁŸ™€Ÿœœ";
AnsiString LicensedtoStr="áÄÎÈÃÞÈÉÙÂ—";
AnsiString NotlicensedStr="ãÂÙÁÄÎÈÃÞÈÉ";
AnsiString LicensevalidStr="áÄÎÈÃÞÈÛÌÁÄÉØÃÙÄÁ";
AnsiString LicenseexpiredStr="áÄÎÈÃÞÈÈÕÝÄßÈÉÂÃ";
AnsiString UnlicMessageStr="ãÂÙÁÄÎÈÃÞÈÉ–ßÈÞÙßÄÎÙÈÉÙÂßÈÌÉÄÃÊÌÃÉÚßÄÙÄÃÊÂÃÁÔŸÞØÏÝÂÝØÁÌÙÄÂÃÞƒ§";
AnsiString SeeWebsiteStr="þÈÈÚÚÚÚƒÝÁÌÃÙÏßÈÈÉÄÃÊƒÚØßƒÃÁ‚øæ‚ÞÂËÙÚÌßÈƒÅÙÀÁÙÂÂÏÙÌÄÃÌÁÄÎÈÃÞÈ";
AnsiString CheckWebsite1Str="ÅÙÙÝ—‚‚ÚÚÚƒÝÁÌÃÙÏßÈÈÉÄÃÊƒÚØßƒÃÁ‚øæ‚ÞÂËÙÚÌßÈƒÅÙÀÁ";
AnsiString CheckWebsite2Str="ëÂßÎØßßÈÃÙÛÈßÞÄÂÃÌÃÉÁÄÎÈÃÞÈÞÎÅÈÎÆ";

TLicenseInfo GetLicenseInfo(void) {
    TLicenseInfo LicInfo;
#ifdef Encrypt
    SaveEncryptedStrings();
#else
    AnsiString s,CodeStr,RandomStr16,RandomStr2;//,g;
    char NineProof,licNineProof;
    int i,sum;
    unsigned short year,month,day;
    TStringList *SL = new TStringList(); //, *Log = new TStringList();
    const Margin=15; //in license file Compiler message "is declared but never used" is incorrect
    const __int32 xorFactor=0xA97E6CF3;
    __int64 code,liccode;
    //get license file name:
    s=ParamStr(0);
    i=s.Length();
    s[i]='c';
    s[i-2]='l';
    s[i-1]='i';  // .../Pedimap.lic without showing in the compiled code
    try {
        SL->LoadFromFile(s);
        if (SL->Count < 6)
           throw new Exception("Invalid license file");
        for (i=6; i<SL->Count; i++) {
            //we allow extra blank lines at the end (eg. introduced by email transfer)
            //but not extra text
            if (SL->Strings[i]!="")
               throw new Exception("Invalid license file");
        }
        //get LicInfo:
        for (i=0; i<6; i++) SL->Strings[i].Trim();
        LicInfo.UserName = Crypt(SL->Strings[0].SubString(Margin,SL->Strings[0].Length()));
        LicInfo.UserAffiliation = Crypt(SL->Strings[1].SubString(Margin,SL->Strings[1].Length()));
        LicInfo.Software = Crypt(SL->Strings[2].SubString(Margin,SL->Strings[2].Length()));
        LicInfo.Version = StrToInt(SL->Strings[3].SubString(Margin,1)); //only the 1-digit main version number
        //get the date:
        s=SL->Strings[4].SubString(Margin,SL->Strings[4].Length());
        if (s=="indefinite")
           LicInfo.ExpiryDate = TDateTime(9999,12,31);
        else {
             i=s.Pos("-"); year=StrToInt(LeftStr(s,i-1));
             s=RightStr(s,s.Length()-i); //after first '-'
             i=s.Pos("-"); month=StrToInt(LeftStr(s,i-1));
             s=RightStr(s,s.Length()-i); //after second '-'
             day=StrToInt(s);
             LicInfo.ExpiryDate = TDateTime(year,month,day);
        }
        //check code:
        CodeStr=SL->Strings[5];
        RandomStr16 = RightStr(CodeStr,11); //the last 11 of which the middle one must be removed
        //for(i=1;i<=RandomStr16.Length();i++)
        //        Log->Add("i="+IntToStr(i)+" Ran16[i]="+RandomStr16[i]);
        licNineProof = RandomStr16[6];
        RandomStr16 = LeftStr(RandomStr16,5)+RightStr(RandomStr16,5);
        RandomStr16 = LeftStr(CodeStr,3) +
                    CodeStr.SubString(6,3) +
                    RandomStr16; //now all 16 digits combined
        RandomStr2 = CodeStr.SubString(4,2);
        //Log->Add("CodeStr="+CodeStr);
        //Log->Add("RandomStr2="+RandomStr2);
        CodeStr = CodeStr.SubString(9,CodeStr.Length()-19);
        liccode = StrToInt64(CodeStr);
        //combine all strings into contents:
        s=LeftStr(RandomStr16,10);
        for (i=0; i<5;i++) s += SL->Strings[i];
        s += RightStr(RandomStr16,6);
        //convert s to set of (4-byte) integers:
        i = s.Length() % 4;
        s += AnsiString("PRI ").SubString(0,4-i); //now multiple of 4
        int intarrLength = s.Length() / 4;
        __int32 *intarr = new __int32 [intarrLength];
        for (i=0; i<intarrLength; i++) {
            //g="i="+IntToStr(i)+" chars: "+IntToStr(s[4*i+1]);
            intarr[i] = (unsigned char)s[4*i+1];
            for (int j=1; j<4; j++) {
                //g=g+" "+IntToStr(s[4*i+j+1]);
                intarr[i] = (intarr[i]<<8)+(unsigned char)s[4*i+j+1];
            }
            //g=g+" intarr[i]="+IntToStr(intarr[i]);
            //Log->Add(g);
        }
        //for (i=0;i<40;i++)
        //    Log->Add("i="+IntToStr(i)+" rotate(1048576,i)="+IntToStr(rotateLeft(1048576,i)));
        //encode intarr:
        __int32 xorFactor = 0xA97E6CF3; //compiler message "never used" is incorrect
        //Log->Add("xorFactor set to "+IntToStr(xorFactor));
        //for (i=0;i<40;i++)
        //    Log->Add("i="+IntToStr(i)+" rotate(xorFactor,i)="+IntToStr(rotateLeft(xorFactor,i,SL)));
        //__int32 y=xorFactor;
        //for (i=0;i<40;i+=2)
        //    Log->Add("i="+IntToStr(i)+" rotate(y,2)="+IntToStr(y=rotateLeft(y,2,SL)));
        //Log->Add("i xorFactora rotBy xorFactorb intarr[i]a intarr[i]b");
        for (i=0; i<intarrLength; i++) {
            //g = IntToStr(i)+" "+IntToStr(xorFactor)+" "+IntToStr(intarr[i] % 32 + i);
            xorFactor = rotateLeft(xorFactor, (intarr[i] % 32 + i), SL);
            //g += " "+IntToStr(xorFactor)+" "+IntToStr(intarr[i])+" ";
            intarr[i] = intarr[i] ^ xorFactor; //xor-symbol is ^
            //g += IntToStr(intarr[i]);
            //Log->Add(g);
        }
        code = 0;
        for (i=0; i<intarrLength; i++) {
            code += intarr[i];
        }
        delete[] intarr;
        code = code<0 ? -code : code;
        //Log->Add("liccode="+IntToStr(liccode)+" code="+IntToStr(code));
        if (liccode!=code)
                throw new Exception("Invalid license file");
        //Log->Add("RandomStr16="+RandomStr16);
        //Log->Add("RandomStr2="+RandomStr2);
        sum = 0;
        //s="";
        for (i=1; i<=2; i++) sum += RandomStr2[i]-'0'; // s+=" "+AnsiString(RandomStr2[i])+" "+IntToStr(sum);}
        for (i=2; i<=3; i++) sum += RandomStr16[i]-'0'; // s+=" "+AnsiString(RandomStr16[i])+" "+IntToStr(sum);}
        for (i=12; i<=15; i++) sum += RandomStr16[i]-'0'; // s+=" "+AnsiString(RandomStr16[i])+" "+IntToStr(sum);}
        //Log->Add(s);
        NineProof='0';
        NineProof += (char) (sum % 10);
        //Log->Add("sum="+IntToStr(sum)+" licNine="+AnsiString(licNineProof)+" Nine="+AnsiString(NineProof));
        if (licNineProof!=NineProof)
                throw new Exception("Invalid license file");
    }
    catch (...) {
        //Log->Add("Exception!");
        LicInfo.UserName=""; //signals invalid license file
    }
    //Log->Add("LicInfo UserName="+LicInfo.UserName);
    //Log->Add("LicInfo.UserAffiliation="+LicInfo.UserAffiliation);
    //Log->Add("LicInfo.ExpiryDate="+LicInfo.ExpiryDate->DateString());
    //Log->SaveToFile("Lic.log");
#endif
    return (LicInfo);
} //GetLicenseInfo

void __fastcall TMainForm::FormShow(TObject *Sender)
{   AnsiString FName;
    int i;
    TMenuItem *NewItem;

    ScrollboxBorders = ViewScrollbox->Width - ViewScrollbox->ClientWidth;
                     //without scrollbars; always 4 pixels?
    ScrollboxLastWidth=  ViewScrollbox->Width;
    ScrollboxLastHeight= ViewScrollbox->Height;
    ViewScrollbox->HorzScrollBar->Tracking=false;
    ViewScrollbox->VertScrollBar->Tracking=false;
    //if (Today>WarningDate)
    //{ ShowMessage("This Pedimap version may be outdated\nCheck www.plantbreeding.wur.nl/UK/software.html for the current version");
    //  Application->Terminate();
    //}
    ClearingTabs=false;
    OpenDialog->InitialDir="";
    OptionsForm->PageControl1->ActivePage=OptionsForm->IndivSheet;
    OptionsForm->ActiveControl=OptionsForm->OKbtn;
    CheckPedimapKey();
    //add MRUitems to file menu:
    for (i=0; i<MaxMRU; i++)
    { NewItem = new TMenuItem(this);
      NewItem->Caption = "&"+IntToStr(i+1); //temporary
      NewItem->OnClick = MRUClick;
      //MainMenu1->Items->Add(NewItem);
      FileMenu->Add(NewItem);
    }
    RegReadMRUfiles();
    GetIndivDrawLimitFromCommandline();
    FName=GetFileFromCommandLine();
    LicInfo = GetLicenseInfo();
    if (FName!="")
    { DoFileOpen(FName);
      SetLegendColumns();
    }
} //TMainForm::FormShow
//---------------------------------------------------------------------------

void __fastcall TMainForm::FileMenuClick(TObject *Sender)
//sets the MRU files in the File menu
{ int i,MRUbaseIndex;

   MRUbaseIndex=MRUbreak->MenuIndex;
   //MainMenu1->Items[0].Items[MRUbaseIndex].Visible:=MRUfile[1]>'';
   MRUbreak->Visible=MRUfile[1]>"";
   for (i=0; i<MaxMRU; i++)
   { FileMenu->Items[MRUbaseIndex+1+i]->Visible = MRUfile[i]>"";
     FileMenu->Items[MRUbaseIndex+1+i]->Caption =
            "&"+IntToStr(i+1)+" "+MRUfile[i];
   }
} //FileMenuClick

void UpdateMRUfiles(const AnsiString Fname)
{ int i,j;
  i=0;
  while (i<MaxMRU-1 && MRUfile[i]!=Fname) i++;
  for (j=i; j>=1; j--) MRUfile[j]=MRUfile[j-1];
  MRUfile[0]=Fname;
} //UpdateMRUfiles


void __fastcall TMainForm::ListSaveAsExecute(TObject *Sender)
{  TStringList *SL;
  if (CurrentViewPop!=NULL && MainTabctrl->TabIndex==0)
  { SaveDialog->InitialDir=ProjectDir;
    SaveDialog->FileName = CleanFilename(CurrentViewPop->VPNode->Text + "-"
                           + MainTabctrl->Tabs->Strings[MainTabctrl->TabIndex],true);
    SaveDialog->DefaultExt=".txt";
    SaveDialog->Filter="Text files (*.txt)|*.txt|All files (*.*)!*.*";
    SaveDialog->FilterIndex=1; //txt
    if (SaveDialog->Execute())
    { try
      { if (ExtractFileExt(SaveDialog->FileName)=="")
          SaveDialog->FileName = SaveDialog->FileName+".txt";
        SL=CurrentViewPop->MakeIndListSL(true);
        SL->SaveToFile(SaveDialog->FileName);
        delete SL;
      }
      catch (...) { ShowMessage("Error saving file "+SaveDialog->FileName); }
    }
    SaveDialog->DefaultExt="";
  }
  else ShowMessage("No list to export");
} //TNwMainForm::ListSaveAsExecute
//---------------------------------------------------------------------------

void __fastcall TMainForm::ListCopyExecute(TObject *Sender)
{  TStringList *SL; AnsiString s;
  if (CurrentViewPop!=NULL && MainTabctrl->TabIndex==0)
  { SL = CurrentViewPop->MakeIndListSL(true);
    s = SL->Text;
    //Clipboard()->Assign(s.c_str());
    Clipboard()->SetTextBuf(s.c_str());
    delete SL;
  }
  else ShowMessage("No list to export");
} //TNwMainForm::ListCopyExecute
//---------------------------------------------------------------------------

void TraitColWidths(int* ColWidth)
//copied from TNwMainForm::DataListWidths but calculates
//ascii lengths, and Quoted Stringwidth for captions
{ int i,d,e,W=0,X;
  for (d=0; d<IndivDataCount; d++)
  { W=QuotedString(IndivData[d].Caption).Length();
    e=IndivData[d].TypeIndex;
    switch (IndivData[d].DataType)
    { case dtC: ; break;
      case dtI: X=IntToStr(Iint[e][IndivData[d].V.OV.LargInd]).Length();
                if (X>W) W=X;
                X=IntToStr(Iint[e][IndivData[d].V.OV.SmallInd]).Length();
                if (X>W) W=X;
                break;
      case dtF: X=FloatToStrF(Ifloat[e][IndivData[d].V.OV.LargInd],
                        ffFixed,100,IndivData[d].V.OV.Decimals).Length();
                if (X>W) W=X;
                X=FloatToStrF(Ifloat[e][IndivData[d].V.OV.SmallInd],
                        ffFixed,100,IndivData[d].V.OV.Decimals).Length();
                if (X>W) W=X;
                break;
      case dtS: for (i=0; i<IndivData[d].V.SV.LabelCount; i++)
                { X=IndivData[d].V.SV.Labels[i].Length();
                  if (X>W) W=X;
                } break;
    }
    ColWidth[d+3]=W; //Col 0,1,2 = name + parents
  }
} //TraitColWidths

void AlleleColWidths (int *AllColWidth)
//sets column widths for all allele columns;
{ int loc, lg, p, AlleleWidth, LocNameWidth;
  //determine length of longest locus and allele name
  LocNameWidth=0;
  AlleleWidth=strUnknown[0].Length();
  if (strNullHomoz.Length()>AlleleWidth) AlleleWidth=strNullHomoz.Length();
  if (strConfirmedNull.Length()>AlleleWidth) AlleleWidth=strConfirmedNull.Length();
  if (IntToStr(MarkCodeCount).Length()>AlleleWidth) AlleleWidth=IntToStr(MarkCodeCount).Length();
  for (lg=0; lg<LinkGrpCount; lg++)
  { for (loc=0; loc<LinkGrp[lg]->LocCount; loc++)
    { if (QuotedString(LinkGrp[lg]->Locus[loc]->LocName).Length() > LocNameWidth)
        LocNameWidth=QuotedString(LinkGrp[lg]->Locus[loc]->LocName).Length();
      for (p=0; p<LinkGrp[lg]->Locus[loc]->AllNameCount; p++)
        if (LinkGrp[lg]->Locus[loc]->AllName[p].Length()>AlleleWidth)
          AlleleWidth=LinkGrp[lg]->Locus[loc]->AllName[p].Length();
    }
    if (LinkGrp[lg]->LocCount==1 && //then for export an extra locys "dummyNN" is exported
        LocNameWidth<7)
      LocNameWidth=7;
  }
  //set column widths
  for (p=0; p<Ploidy-1; p++)
    AllColWidth[p] = AlleleWidth;
  if (Ploidy*AlleleWidth+Ploidy-1 < LocNameWidth)
    AllColWidth[Ploidy-1] = LocNameWidth-(Ploidy-1)*AlleleWidth-(Ploidy-1);
  else AllColWidth[Ploidy-1] = AlleleWidth;
} //AlleleColWidths

AnsiString FillString(AnsiString S, int L)
// adds spaces to S until length is L
{ while (S.Length()<L)
    S=S+' ';
  return S;  
}

/* old version: fixed column width
AnsiString IndivExportLine(int I, int* ColWidth, int*AllColWidth)
//copied from TNwMainForm::MainListviewDataI (OnData for Indiv list);
//analogous to TViewPop::MakeIndListSL
//this routine only used for ExportData.
//Note: exports extra "dummy" locus with all alleles missing for linkage groups
//with only one true locus
{ int d, lg, loc, p, n;
  AnsiString s,ns;

  s="";
  for (d=0; d<IndivDataCount; d++)
    s=s+" "+FillString(DataToStr(d,I),ColWidth[d+3]);

  for (lg=0; lg<LinkGrpCount; lg++)
  { for (loc=0; loc<LinkGrp[lg]->LocCount; loc++)
    { for (p=0; p<Ploidy; p++)
      { n=LinkGrp[lg]->Locus[loc]->AllIx[I][p];
        if (n>=0) ns=LinkGrp[lg]->Locus[loc]->AllName[n];
        else if (n==-1) ns=mv_marker; //strUnknown[0];
        else ns=strNullHomoz;
        s=s + " " + FillString(ns,AllColWidth[p]);
      }
    }
    if (LinkGrp[lg]->LocCount==1) //write dummy locus alleles (*)
    { for (p=0; p<Ploidy; p++)
      { ns=mv_marker; //strUnknown[0];
        s=s + " " + FillString(ns,AllColWidth[p]);
      }
    }
  }
  return s;
} //IndivExportLine
*/

//new version: tab-separated
AnsiString IndivExportLine(int I)
//copied from TNwMainForm::MainListviewDataI (OnData for Indiv list);
//analogous to TViewPop::MakeIndListSL
//this routine only used for ExportData.
//Note: exports extra "dummy" locus with all alleles missing for linkage groups
//with only one true locus
{ int d, lg, loc, p, n;
  AnsiString s,ns;
  
  s="";
  for (d=0; d<IndivDataCount; d++)
    s=s+Tb+DataToStr(d,I,mv_trait);

  for (lg=0; lg<LinkGrpCount; lg++)
  { for (loc=0; loc<LinkGrp[lg]->LocCount; loc++)
    { for (p=0; p<Ploidy; p++)
      { n=LinkGrp[lg]->Locus[loc]->AllIx[I][p];
        if (n>=0) ns=LinkGrp[lg]->Locus[loc]->AllName[n];
        else if (n==-1) ns=mv_marker;
        else if (n==-2) ns=strNullHomoz;
        else ns=strConfirmedNull;
        s=s + Tb + ns;
      }
    }
    if (LinkGrp[lg]->LocCount==1) //write dummy locus alleles (*)
    { for (p=0; p<Ploidy; p++)
      { ns=mv_marker; //strUnknown[0];
        s=s + Tb + ns;
      }
    }
  }
  return s;
} //IndivExportLine

void __fastcall TMainForm::PopExportAllDataExecute(TObject *Sender)
{ if (IndivCount>0) {
    TViewPop *VP;
    VP = (TViewPop*) PopTreeView->Items->Item[0]->Data;
    ExportData(VP);
  }
} //TMainForm::PopExportAllDataExecute

void __fastcall TMainForm::PopExportPopDataExecute(TObject *Sender)
{ if (CurrentViewPop!=NULL)
    ExportData(CurrentViewPop);
} //TMainForm::PopExportPopDataExecute

void TMainForm::ExportData(TViewPop *VP)
//exports for subpop VP, in FlexQTL format + block with color codes below
//column widths are calculated on full population, so different subpopulations
//get the same layout
//Note: tries to avoid exporting semi-founders by including the second parent
//if available in the base population;
//also exports extra dummy locus with all alleles missing for linkage groups
//with only one true locus
{
  int //*ColWidth, *AllColWidth,
      i, vpi, p, lg, loc, LocNameWidth, SemifCount;
  //bool *Semif;
  TStringList *SL;
  AnsiString s,ns;
  TTreeNode *TN;
  TCrossing *Cro;
  TViewPop *NP; //New population: all individuals of VP plus the missing
                //parent of individuals with only one cross parent,
                //if present in base population (otherwise some semi-founders will
                //still be exported as such)
                //Needed for FlexQTL: cannot handle semi-founders
                //Needs to be done repeatedly, as adding a new individual
                //might convert a "founder" of a subpop to a semi-founder
  if (ProjectDir!="") SaveDialog->InitialDir=ProjectDir;
  else if (DataDir!="") SaveDialog->InitialDir=DataDir;
  else SaveDialog->InitialDir=ProgDir;
  if (ProjectFileName != "") SaveDialog->FileName=ProjectFileName;
  else if (DataFileName != "") SaveDialog->FileName=DataFileName;
  else SaveDialog->FileName="";
  if (SaveDialog->FileName!="")
  { s=ExtractFileName(SaveDialog->FileName);
    SaveDialog->FileName=s.SubString(1,s.Length()-ExtractFileExt(s).Length())+"_Export.txt";
  }
  SaveDialog->Filter="Text files (*.txt)|*.txt|All files (*.*)|*.*";
  SaveDialog->DefaultExt="txt";
  if (SaveDialog->Execute())
  {
    NP = new TViewPop(); //creates temporary VP without node
    SL = new TStringList;
    //ColWidth=AllColWidth=NULL;
    try
    { SL->Add(";"+Tb+"Pedimap export file"); SL->Add(";");
      SL->Add(";"+Tb+"Source data:");
      SL->Add(";"+Tb+"Pedimap project file: " + ProjectFileName+ProjectExt);
      SL->Add(";"+Tb+"Data file           : " + DataFileName+DataFileExt);
      TN = VP->VPNode;
      if (TN==PopTreeView->Items->Item[0])
        SL->Add(";"+Tb+"Data of full population");
      else
      { s="";
        while (TN!=PopTreeView->Items->Item[0])
        { s=" -> " + TN->Text + s;
          TN = TN->Parent;
        }
        s=TN->Text + s;
        SL->Add(";"+Tb+"Data from subpopulation "+s);
      }
      SL->Add(";");
      /*
      //calculate column widths of pedigree and phenotypic data:
      ColWidth = new int [3+IndivDataCount];
      ColWidth[0]=4; //"Name"
      for (i=0; i<IndivCount; i++)
        if (QuotedString(Indiv[i].IName).Length() > ColWidth[0])
          ColWidth[0]=QuotedString(Indiv[i].IName).Length();
      ColWidth[1]=ColWidth[0];
      if (ColWidth[1]<7) ColWidth[1]=7; //"Parent1" or "Female" or "Mother"
      ColWidth[2]=ColWidth[1];
      TraitColWidths(ColWidth);
      //calculate column widths of allele columns:
      if (TotLocCount>0)
      { AllColWidth =  new int [Ploidy];
        aanpassen AlleleColWidths(AllColWidth); //also reckons with "dummyNN"
        LocNameWidth=Ploidy-1; //spaces between allele columns of one locus
        for (p=0; p<Ploidy; p++)
          LocNameWidth=LocNameWidth+AllColWidth[p];
      }
      */
      //Fill NP and Pedigree & Phenotypes table:
      if ( (SemifCount=NP->CopyIndivs(VP,true))!=0 )  //SemifCount here: number of added indivs, negative if not successful
      { if (SemifCount>0)
        { ShowMessage("Warning: "+IntToStr(SemifCount)+" individuals added to avoid exporting semi-founders");
          SemifCount=0;
        }
        else
        { //SemifCount<0: semifounders present in NP
          SemifCount=-SemifCount;
          if (SemifCount==MAXINT) //semi-founders present but no ancestors to add
             ShowMessage("Warning: exported data contain semi-founders and may be incompatible with FlexQTL"+CRLF+
                         "(see list in export file)");
          else ShowMessage("Warning: "+IntToStr(SemifCount)+" individuals added trying to avoid exporting semi-founders but not successful,"+CRLF+
                         "may be incompatible with FlexQTL"+CRLF+
                         "(see list in export file)");
        }
      }
      //now: SemifCount=0 if no semifounders, else negative

      //write caption of pedigree - traits - alleles table:
      /* old version
      s="; PT "+       //PopType
        FillString("Name",ColWidth[0])+" "+
        FillString(ParentCaption(0),ColWidth[1])+" "+
        FillString(ParentCaption(1),ColWidth[2]);
      for (p=0; p<IndivDataCount; p++)
        s=s+" "+FillString(QuotedString(IndivData[p].Caption),ColWidth[p+3]);
      for (lg=0; lg<LinkGrpCount; lg++)
      { for (loc=0; loc<LinkGrp[lg]->LocCount; loc++)
        aanpassen: tab-separated, 4* de naam   s=s+" "+FillString(QuotedString(LinkGrp[lg]->Locus[loc]->LocName),LocNameWidth);
        if (LinkGrp[lg]->LocCount==1) //add one dummy locus if there is only one true locus
           s=s+" "+FillString("dummy"+IntToStr(LinkGrp[lg]->LGNum+1),LocNameWidth);
      }
      SL->Add(s);
      */
      //new version: with tabs and extra header columns for (ploidy) alleles per locus
      s=";"+Tb+"PT"+Tb+       //PopType
        "Name"+Tb+
        ParentCaption(0)+Tb+
        ParentCaption(1);
      for (p=0; p<IndivDataCount; p++)
        s=s+Tb+QuotedString(IndivData[p].Caption);
      for (lg=0; lg<LinkGrpCount; lg++)
      { for (loc=0; loc<LinkGrp[lg]->LocCount; loc++) {
            s=s+Tb+QuotedString(LinkGrp[lg]->Locus[loc]->LocName);
            for (p=1; p<Ploidy; p++) s=s+Tb;
        }
        if (LinkGrp[lg]->LocCount==1) { //add one dummy locus if there is only one true locus
           s=s+Tb+"dummy"+IntToStr(LinkGrp[lg]->LGNum+1);
           for (p=1; p<Ploidy; p++) s=s+Tb;
        }
      }
      SL->Add(s);
      //fill pedigree part of table line:
      for (vpi=0; vpi<NP->VPIndCount; vpi++)
      { i=NP->VPInd[vpi];
        s=Tb+"1"+Tb+QuotedString(Indiv[i].IName)+Tb;
        Cro = Indiv[i].ParentCross;
        if (Cro==NULL)
           s=s+mv_parent+Tb+mv_parent;
        else
        { //first parent
          if ( Cro->Parent[0]<0 || NP->GetVPIndex(Cro->Parent[0])<0 )
             s=s+mv_parent+Tb;
          else s=s+QuotedString(Indiv[Cro->Parent[0]].IName)+Tb;
          //second parent
          if (Cro->CType==ctCross)
          { if ( Cro->Parent[1]<0 || NP->GetVPIndex(Cro->Parent[1])<0 )
               s=s+mv_parent;
            else s=s+QuotedString(Indiv[Cro->Parent[1]].IName);
          }
          else
          { if (Cro->CType==ctSelfing)
            { if ( Cro->Parent[0]<0 || NP->GetVPIndex(Cro->Parent[0])<0 )
                 s=s+mv_parent;
              else s=s+QuotedString(Indiv[Cro->Parent[0]].IName);
            }
            else { //not Cross or Selfing; give cross type instead of second parent
                   s=s+CrossTypeString[Cro->CType];
                 }
          }
        }
        SL->Add(s+IndivExportLine(i));
      }

      //next: export color codes for each allele
      SL->Add(";"); SL->Add(";"+Tb+"Color codes per allele in same order:");
      //write caption of color codes table:
      s=";";
      for (p=0; p<IndivDataCount+4; p++) s=s+Tb;
      s=s+"Name";
      for (lg=0; lg<LinkGrpCount; lg++)
      { for (loc=0; loc<LinkGrp[lg]->LocCount; loc++) {
            s=s+Tb+QuotedString(LinkGrp[lg]->Locus[loc]->LocName);
            for (p=1; p<Ploidy; p++) s=s+Tb;
        }
        if (LinkGrp[lg]->LocCount==1) { //add one dummy locus if there is only one true locus
            s=s+Tb+"dummy"+IntToStr(LinkGrp[lg]->LGNum+1);
            for (p=1; p<Ploidy; p++) s=s+Tb;
        }
      }
      SL->Add(s);

      //write color code lines:
      for (vpi=0; vpi<NP->VPIndCount; vpi++)
      { i=NP->VPInd[vpi];
        s=";";
        for (p=0; p<IndivDataCount+4; p++) s=s+Tb;
        s=s+QuotedString(Indiv[i].IName);
        for (lg=0; lg<LinkGrpCount; lg++)
        { for (loc=0; loc<LinkGrp[lg]->LocCount; loc++)
            for (p=0; p<Ploidy; p++)
              s=s + Tb + IntToStr(LinkGrp[lg]->Locus[loc]->AllCode[i][p]);
          if (LinkGrp[lg]->LocCount==1) //write dummy locus color codes (0)
            for (p=0; p<Ploidy; p++)
              s=s + Tb + "0";
        }
        SL->Add(s);
      }

      //write linkage map:
      SL->Add(";"); SL->Add(";"+Tb+"Linkage map:");
      for (lg=0; lg<LinkGrpCount; lg++)
      { SL->Add(";"); SL->Add(";"+Tb+"GROUP "+QuotedString(LinkGrp[lg]->LGName));
        for (loc=0; loc<LinkGrp[lg]->LocCount; loc++)
           SL->Add(";"+Tb+QuotedString(LinkGrp[lg]->Locus[loc]->LocName) + Tb +
                    FloatToStrF(LinkGrp[lg]->Locus[loc]->cM,ffFixed,100,3));
        if (LinkGrp[lg]->LocCount==1) //add dummy locus 50 cM beyond true locus
           SL->Add(";"+Tb+"dummy"+IntToStr(LinkGrp[lg]->LGNum+1) + Tb +
                    FloatToStrF(LinkGrp[lg]->Locus[0]->cM+50,ffFixed,100,3));
      }

      //write exported semifounders, if any:
      if (SemifCount != 0)
      { SL->Add(";");
        SL->Add(";"+Tb+"Semi-founders exported:");
        SemifCount=0;
        for (vpi=0; vpi<NP->VPIndCount; vpi++) {
          //SL->Add(IntToStr(vpi)+" -> "+NP->VPInd[vpi]+": "+Indiv[NP->VPInd[vpi]].IName);
          if (NP->IsVPSemifounder(vpi))
          { SemifCount++;
            i=NP->VPInd[vpi];
            SL->Add(";"+Tb+Indiv[i].IName);
          }
        }
        SL->Add(";"+Tb+"Total: "+IntToStr(SemifCount)+" semi-founders");
      }

      SL->SaveToFile(SaveDialog->FileName);
    } //try

    __finally
    { delete SL; SL=NULL;
      delete NP; NP=NULL;
      //delete[] ColWidth; ColWidth=NULL;
      //delete[] AllColWidth; AllColWidth=NULL;
      SaveDialog->DefaultExt="";
    }
  }
} //TMainForm::ExportData

void __fastcall TMainForm::MainTabctrlMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{  int i;
   if (Button==mbRight)
   { i=MainTabctrl->IndexOfTabAt(X,Y);
     if (i==MainTabctrl->TabIndex && i>1)
        //ViewTabMenu->Popup(Left+MainPanel->Left+X,Top+MainPanel->Top+Y);
        ViewTabMenu->Popup(MainTabctrl->ClientOrigin.x+X,
                           MainTabctrl->ClientOrigin.y+Y);

        //ShowMessage("Right at tab"+IntToStr(i));
   }
}

void __fastcall TMainForm::ViewTabDeleteExecute(TObject *Sender)
{ TView *V;
  V=CurrentViewPop->VW[CurrentViewPop->CurrVW]; //to be deleted

  //adjust links of tabs to Views, delete last tab:
  //if (CurrentViewPop->VWCount<=CurrentViewPop->CurrVW)+1)
  //   CurrentViewPop->CurrVW--; //new view=previous, else next
  CurrentViewPop->DeleteVW(V->ViewName);
  SetTabs();
  MainTabctrl->TabIndex=CurrentViewPop->CurrVW + OverviewTab;
  MainTabctrlChange(NULL);
  ProjectSaved=false;
} //TMainForm::ViewTabDeleteExecute

void __fastcall TMainForm::ViewTabRenameExecute(TObject *Sender)
{ int v;
  bool present;
  RenameForm->RenameEd->Text=CurrentViewPop->VW[CurrentViewPop->CurrVW]->ViewName;
  if (RenameForm->ShowModal()==mrOk && RenameForm->RenameEd->Text.Trim()!="")
  { //test for duplicate names:
    present=false; v=0;
    while (!present && v<CurrentViewPop->VWCount)
    { present = (v!=CurrentViewPop->CurrVW) &&
                (CurrentViewPop->VW[v]->ViewName==RenameForm->RenameEd->Text.Trim());
      v++;
    }
    if (present)
      ShowMessage("Duplicate View names not allowed");
    else
    { CurrentViewPop->VW[CurrentViewPop->CurrVW]->ViewName =
        RenameForm->RenameEd->Text.Trim();
      MainTabctrl->Tabs->Strings[MainTabctrl->TabIndex] =
        CurrentViewPop->VW[CurrentViewPop->CurrVW]->ViewName;
      ProjectSaved=false;
    }
  }
} //TMainForm::ViewTabRenameExecute

void __fastcall TMainForm::ViewTabMenuPopup(TObject *Sender)
{ ViewTabPasteOptions->Enabled = CopiedViewOptions!=NULL;
}

void __fastcall TMainForm::ViewTabCopyOptionsExecute(TObject *Sender)
{ delete CopiedViewOptions; CopiedViewOptions=NULL;
  if (CurrentViewPop!=NULL && CurrentViewPop->CurrVW>0)
    CopiedViewOptions = new TViewOptions(CurrentViewPop->VW[CurrentViewPop->CurrVW]->Opt);
}

void __fastcall TMainForm::ViewTabPasteOptionsExecute(TObject *Sender)
{ TView *V;
  if (CopiedViewOptions!=NULL &&
      CurrentViewPop!=NULL && CurrentViewPop->CurrVW>0)
  { V=CurrentViewPop->VW[CurrentViewPop->CurrVW];
    V->Opt = *CopiedViewOptions;
    V->DeleteIndMetafiles();
    delete V->ChartMF; V->ChartMF=NULL;
    if (!V->ManualAdjust)
    { //recalculate positions
      V->CalcInitialPositions();
    }
    ShowCurrentView(CurrentViewPop);  //redraws V->ChartMF and ZoomBM
    ProjectSaved=false;
  }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::PopulationMenuPopup(TObject *Sender)
{ PopAddViews->Enabled = (SourceViewPop!=NULL) &&
                         (SourceViewPop!=CurrentViewPop);
  PopViewsSource->Enabled = (CurrentViewPop!=NULL) &&
                            (CurrentViewPop!=SourceViewPop);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::PopViewsSourceExecute(TObject *Sender)
{ SourceViewPop=CurrentViewPop;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::PopAddViewsExecute(TObject *Sender)
{ int vc, v1, v2, n;
  bool present;
  AnsiString Name;
  if ( SourceViewPop!=NULL &&
       SourceViewPop->VPCount>1 && //not only Overview
       CurrentViewPop!=NULL )
  { vc = CurrentViewPop->VWCount;
    // first: copy all views (except Overview = VW[0]) from source:
    for (v1=1; v1<SourceViewPop->VWCount; v1++)
    { v2=CurrentViewPop->AddVW(SourceViewPop->VW[v1]->ViewName, false);
      CurrentViewPop->VW[v2-1]->Opt = SourceViewPop->VW[v1]->Opt;
      //CurrentViewPop->VW[v2-1]->Opt.Copy(SourceViewPop->VW[v1]->Opt);
    }
    // next: check for duplicate names:
    for (v2=vc; v2<CurrentViewPop->VWCount; v2++)
    { n=2; Name=CurrentViewPop->VW[v2]->ViewName;
      do
      { present=false; v1=0;
        while (!present && v1<CurrentViewPop->VWCount)
        { present= ( v1!=v2 &&
                     ( CurrentViewPop->VW[v1]->ViewName ==
                       CurrentViewPop->VW[v2]->ViewName ) ) ;
          v1++;
        }
        if (present)
        { //create new name:
          CurrentViewPop->VW[v2]->ViewName = Name+"_"+IntToStr(n++);
        }
      } while (present);
    }
    n=MainTabctrl->TabIndex;
    SetTabs();
    MainTabctrl->TabIndex=n;
    if (n>0)
       CurrentViewPop->CurrVW = n-1;
    ProjectSaved=false;
  }
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::FileExitExecute(TObject *Sender)
{ Close(); }
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCloseQuery(TObject *Sender,
      bool &CanClose)
{ TModalResult mr;

  if (ProjectSaved) CanClose = true;
  else
  { mr = MessageDlg("Save project ?", mtConfirmation,
                   TMsgDlgButtons() << mbYes << mbNo << mbCancel, 0);
    CanClose = mr!=mrCancel;
    if (mr==mrYes) FileSaveExecute(NULL);
  }
} //TNwMainForm::FormCloseQuery
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void __fastcall TMainForm::ToolsShowLegendExecute(TObject *Sender)
{ LegendForm->Show(); }
//---------------------------------------------------------------------------

void __fastcall TMainForm::PopSortAlphaExecute(TObject *Sender)
{ if ( PopTreeView->Selected != NULL)
    PopTreeView->Selected->AlphaSort(false);
} //TNwMainForm::PopSortAlphaExecute
//---------------------------------------------------------------------------
int __stdcall PopSortOrigCompareFunc(TTreeNode *Node1,TTreeNode *Node2, int dummy)
{ return ((TViewPop*) (Node1->Data))->VPnum - ((TViewPop*) (Node2->Data))->VPnum ; }

void __fastcall TMainForm::PopSortOrigExecute(TObject *Sender)
{ if ( PopTreeView->Selected != NULL)
    PopTreeView->Selected->
      CustomSort((TTVCompare)PopSortOrigCompareFunc,0);
      /*NOTE: this will not compile because of an error in the CommCtrl.h file!
              there is an error in the treeview:
              there's a difference between the source and the headerfile.
              The function prototype (in CommCtrl.h):
                PNTVCOMPARE is defined as int(*)(long,long,long)
              in the Lib:
                PNTVCOMPARE is used as int(*)(int,int,int)
        Solve this by replacing CommCtrl.h line 5113
        typedef int (CALLBACK *PFNTVCOMPARE)(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
        with the line:
        typedef int (CALLBACK *PFNTVCOMPARE)(int lParam1, int lParam2, int lParamSort);
        (from post of Mike Harris <mhar.mneer@mcione.com> Date: Thu, 08 Oct 1998 10:36:42 -0500)
      */
} //TNwMainForm::PopSortOrigExecute
//---------------------------------------------------------------------------




void __fastcall TMainForm::LeftPanelResize(TObject *Sender)
{ /*if ( (LeftPanel->ClientHeight - Splitter1->Top) <
       (LeftMidPanel->Height + InfoMemo->Constraints->MinHeight) )
  Splitter1->Top = LeftPanel->ClientHeight - LeftMidPanel->Height -
                   InfoMemo->Constraints->MinHeight;   */
  if ( (LeftTopPanel->Height + PopTreeView->Height) >
       (LeftPanel->ClientHeight - Splitter1->Height - LeftMidPanel->Height -
        InfoMemo->Constraints->MinHeight) )
  PopTreeView->Height = LeftPanel->ClientHeight - LeftTopPanel->Height -
                        Splitter1->Height - LeftMidPanel->Height -
                        InfoMemo->Constraints->MinHeight;
} //TMainForm::LeftPanelResize
//---------------------------------------------------------------------------


void __fastcall TMainForm::HelpAboutExecute(TObject *Sender)
{ AboutboxForm->SoftwareLbl->Caption = Crypt(PedimapStr.SubString(2,6))+  //the P is already on the Aboutbox
                        " "+Crypt(PedimapVersionStr);
  AboutboxForm->AuthorLbl->Caption = Crypt(AuthorStr);
  AboutboxForm->CopyrightLbl->Caption = Crypt(CopyrightStr);
  AboutboxForm->LicensedtoLbl->Caption = Crypt(LicensedtoStr);
  AboutboxForm->Website1Lbl->Caption = Crypt(CheckWebsite1Str);
  AboutboxForm->Website2Lbl->Caption = Crypt(CheckWebsite2Str);
  AboutboxForm->Color = MakeColor(255,255,180); //pale yellow
  AboutboxForm->SoftwarePanel->Color = MakeColor(255,255,180); //pale yellow
  AboutboxForm->ButtonPanel->Color = MakeColor(255,255,180); //pale yellow
  AboutboxForm->OkPanel->Color = MakeColor(255,255,150); //less pale yellow
  if (LicInfo.UserName == "") {
     AboutboxForm->NameLbl->Caption = Crypt(NotlicensedStr);
     AboutboxForm->NameLbl->Font->Color = clRed;
     AboutboxForm->NameLbl->Font->Style = TFontStyles()<< fsBold;
     AboutboxForm->AffiliationLbl->Caption = "";
     AboutboxForm->ExpiryLbl->Caption = "";
  } else {
     AboutboxForm->NameLbl->Caption = Crypt(LicInfo.UserName);
     AboutboxForm->NameLbl->Font->Color = clBlack;
     AboutboxForm->NameLbl->Font->Style = TFontStyles();
     AboutboxForm->AffiliationLbl->Caption = Crypt(LicInfo.UserAffiliation);
     TDateTime date = TDateTime(9999,12,31);
     if (LicInfo.ExpiryDate < date) {
         date = Date();
         if (date > (LicInfo.ExpiryDate+1)) {
             AboutboxForm->ExpiryLbl->Caption = Crypt(LicenseexpiredStr)+LicInfo.ExpiryDate.DateString();
             AboutboxForm->ExpiryLbl->Font->Color = clRed;
             AboutboxForm->ExpiryLbl->Font->Style = TFontStyles()<< fsBold;
         } else {
             AboutboxForm->ExpiryLbl->Caption = Crypt(LicensevalidStr)+LicInfo.ExpiryDate.DateString();
             AboutboxForm->ExpiryLbl->Font->Color = clBlack;
             AboutboxForm->ExpiryLbl->Font->Style = TFontStyles();
         }
     }
     else AboutboxForm->ExpiryLbl->Caption = "";
  }

  AboutboxForm->ShowModal();
  AboutboxForm->SoftwareLbl->Caption = "";
  AboutboxForm->AuthorLbl->Caption = "";
  AboutboxForm->CopyrightLbl->Caption = "";
  AboutboxForm->LicensedtoLbl->Caption = "";
  AboutboxForm->NameLbl->Caption = "";
  AboutboxForm->AffiliationLbl->Caption = "";
  AboutboxForm->ExpiryLbl->Caption = "";

} //TMainForm::HelpAboutExecute
//---------------------------------------------------------------------------
bool TestLicense(void) {
  return (LicInfo.UserName != "" &&
          ( LicInfo.Software == PedimapStr ||
            LicInfo.Software == Pedimap_CommandlineStr ) &&
          LicInfo.Version >= PedimapMainversion &&
          LicInfo.ExpiryDate >= Date());
}

void ShowUnlicensedMessage(void) {
  ShowMessage(Crypt(UnlicMessageStr)+Crypt(SeeWebsiteStr));
}
void __fastcall TMainForm::HelpManualExecute(TObject *Sender)
{ ShellExecute(Application->Handle,
               "open",
               "PedimapManual.pdf",
               NULL,
               NULL,
               SW_SHOWNORMAL);

} //TMainForm::HelpManualExecute
//---------------------------------------------------------------------------

void __fastcall TMainForm::ToolsAddSFparentsExecute(TObject *Sender)
{ bool ok=false, cancel;
  if (IndivCount>0) {
    if (!ProjectSaved) {\
      ShowMessage("Project must be saved first");
    }
    else {
      TViewPop* VP = (TViewPop*) PopTreeView->Items->Item[0]->Data;
      int i=0;
      while (i<VP->VPIndCount && !VP->IsVPSemifounder(i)) i++;
      if (i>=VP->VPIndCount) {
        ShowMessage("No semi-founders in population");
      }
      else {
        ImputeSFparentAllelesForm->SaveDialog1->InitialDir = DataDir;
        ImputeSFparentAllelesForm->SaveDialog1->FileName = DataFileName+"_sfparents"+DataFileExt;
        AddSFparentsForm->FilenameEd->Text =
          ImputeSFparentAllelesForm->SaveDialog1->InitialDir + "\\" +
          ImputeSFparentAllelesForm->SaveDialog1->FileName;
        TModalResult result;
        do { result = AddSFparentsForm->ShowModal(); }
        while (result==mrNo);
        if (result==mrOk) {
          AddSFparents(AddSFparentsForm->FilenameEd->Text);
        }
      }
    } //Project saved
  } //IndivCount>0
} //ToolsAddSFparentsExecute

void __fastcall TMainForm::ToolsImputeSFparentAllelesExecute(
      TObject *Sender)
{ bool ok=false, cancel;
  if (IndivCount>0) {
    if (!ProjectSaved) {\
      ShowMessage("Project must be saved first");
    }
    else {
      TViewPop* VP = (TViewPop*) PopTreeView->Items->Item[0]->Data;
      int i=0;
      while (i<VP->VPIndCount && !VP->IsVPSemifounder(i)) i++;
      if (i<VP->VPIndCount) {
        ShowMessage("Semifounders found in population; first run 'Add semifounder parents'");
      }
      else {
        ImputeSFparentAllelesForm->SaveDialog1->InitialDir = DataDir;
        ImputeSFparentAllelesForm->SaveDialog1->FileName = DataFileName+"_imputed"+DataFileExt;
        ImputeSFparentAllelesForm->FilenameEd->Text =
          ImputeSFparentAllelesForm->SaveDialog1->InitialDir + "\\" +
          ImputeSFparentAllelesForm->SaveDialog1->FileName;
        TModalResult result;
        do { result = ImputeSFparentAllelesForm->ShowModal(); }
        while (result==mrNo);
        if (result==mrOk) {
          ImputeSFparentAlleles(ImputeSFparentAllelesForm->FilenameEd->Text,
                                ImputeSFparentAllelesForm->AlleleEd->Text,
                                ImputeSFparentAllelesForm->ColorCodeCombx->ItemIndex);
        }
      }
    } //Project saved
  } //IndivCount>0
} //TMainForm::ToolsImputeSFparentAllelesExecute

//---------------------------------------------------------------------------
//Here follow the routines specific for command line mode

AnsiString GetDataFromCommandline(void) {
   AnsiString result = "";
   int i=1;
   while ( (i<=ParamCount()) &&
           (LeftStr(ParamStr(i),6).UpperCase() != "/DATA=") )
     i++;
   if (i<=ParamCount()) {
     result = ParamStr(i).SubString(7,ParamStr(i).Length());
     result = ExpandFileName( result );
   }
   return result;
} //GetDataFromCommandline

AnsiString GetTraitFromCommandline(void) {
   AnsiString result = "";
   int i=1;
   while ( (i<=ParamCount()) &&
           (LeftStr(ParamStr(i),7).UpperCase() != "/TRAIT=") )
     i++;
   if (i<=ParamCount()) {
     result = ParamStr(i).SubString(8,ParamStr(i).Length());
   }
   return result;
} //GetTraitFromCommandline

int* GetIndivFromCommandline(void) {
   int* SelInd = new int[1];
   int SelCount=0;
   SelInd[0] = MAXINT;
   int i=1;
   while ( (i<=ParamCount()) &&
           (LeftStr(ParamStr(i),7).UpperCase() != "/INDIV=") )
     i++;
   if (i<=ParamCount()) {
     AnsiString filename = ParamStr(i).SubString(8,ParamStr(i).Length());
     if (FileExists(filename)) {
       TStringList* SL = new TStringList();
       try {
         SL->LoadFromFile(filename);
         for (int line=0; line<SL->Count; line++) {
           AnsiString iname = SL->Strings[line].Trim();
           if (iname!="") {
             int ind = 0; //index of iname in Indiv array
             while (ind<IndivCount && Indiv[ind].IName!=iname)
               ind++;
             if (ind<IndivCount) {
                int ix = 0; //index of new ind in SelInd
                while (SelInd[ix]<ind) ix++;
                if (SelInd[ix]!=ind) { //avoid duplicates
                  SelCount++;
                  int* tmpind = new int[SelCount+1];
                  for (int j=0; j<ix; j++)
                    tmpind[j] = SelInd[j];
                  tmpind[ix] = ind;
                  for (int j=ix+1; j<=SelCount; j++)
                    tmpind[j] = SelInd[j-1];
                  delete[] SelInd;
                  SelInd = tmpind; tmpind=NULL;
                } //!=ind, not a duplicate
             } //ind<IndivCount, name exists
           } //inname!=""
         } //for line
       }
       catch (const Exception& e) {}
     }
   }
   return SelInd;
} //GetIndivFromCommandline


void CommandlineProcessing(AnsiString datafile) {
//#define CommandlineEnabled  //comment out for the normal version
  AnsiString dir = ExtractFilePath(datafile);
  TStringList* SL = new TStringList();
  //Application->Terminate();
  Indiv = NULL; IndivCount=0;
  bool success = FileExists(datafile) &&
                 ReadDataFile(datafile) &&
                 Indiv != NULL &&
                 IndivCount>0;

#ifdef CommandlineEnabled
  LicInfo = GetLicenseInfo();
  if (success) {
    //TODO: we should test a special form of license file; then ...

    try {
      success = false;
      PPI = GetPPI();
      PrPPI = PPI; //drawing now at screen resolution also for exported images
      LinePx = (63+PrPPI) / 127; //integer division; approx. 0.2 mm
      CurrentViewPop = MakeOverallViewPop(NULL);
      int* SelInd = GetIndivFromCommandline();
      if (SelInd[0]<MAXINT) {
      }
      CurrentViewPop->AddVW("newView",false); //no overview
      CurrentViewPop->CurrVW=CurrentViewPop->VWCount-1;
      TView* V = CurrentViewPop->VW[CurrentViewPop->CurrVW];
      GetIndivDrawLimitFromCommandline();
      if (IndivCount>IndivDrawLimit) {
        V->DrawNoImageMessageMF();
      } else {
        AnsiString traitname = GetTraitFromCommandline();
        int trait = IndivDataCount-1;
        while (trait>=0 && IndivData[trait].Caption!=traitname)
          trait--;
        if (trait>=0) {
          V->Opt.IFixedColor = false;
          V->Opt.ColorTrait = trait;
          V->Opt.SetDefTraitColors();
        }
        if (TestLicense() && LicInfo.Software==Pedimap_CommandlineStr) {
          V->CalcInitialChart();
          Graphics::TMetafile *MF = V->DrawChartExportMF(SL);
          if (MF!=NULL) {
            Graphics::TBitmap *BM = NULL;
            //MF = V->DrawChartExportMF();
            BM = new Graphics::TBitmap;
            BM->Width=MF->Width;
            BM->Height=MF->Height;
            BM->Transparent=false;
            BM->Canvas->Brush->Color=V->Opt.PageCol;
            BM->Canvas->Brush->Style=bsSolid;
            BM->Canvas->FillRect(Rect(0,0,BM->Width+1,BM->Height+1));
            BM->Canvas->Draw(0,0,MF);
            AnsiString ext = ExtractFileExt(datafile);
            AnsiString imgfile = datafile.SubString(1,datafile.Length()-ext.Length())+".bmp";
            BM->SaveToFile(imgfile);
            success = true;
          }
        } else SL->Add(Crypt(NotlicensedStr));
      } //IndivDrawLimit ok
    }
    catch (const Exception& e) {}
  }
  if (success) {
    SL->Insert(0,"ok");
    SL->Insert(1,"coordinates:");
    SL->SaveToFile(datafile+".log");
  } else {
    SL->Insert(0,"failed");
    if (FileExists(datafile)) SL->SaveToFile(datafile+".log");
    else SL->SaveToFile(dir+"Pedimap.log");
  }
#else
  //not defined CommandlineEnabled
  SL->Add("To obtain a commandline-enabled version of Pedimap");
  SL->Add("contact Roeland Voorrips at Wageningen-UR Plant Breeding");
  SL->Add("P.O. Box 16, 6700 AA Wageninegn, the Netherlands");
  SL->Add("e-mail: roeland.voorrips@wur.nl");
  if (success) {
    SL->SaveToFile(datafile+".log");
  } else {
    SL->SaveToFile(dir+"Pedimap.log");
  }
#endif
} //CommandlineProcessing


