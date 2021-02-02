#include "PCH.h"
//#include <vcl.h>
#pragma hdrstop

#include "OptionsUnit.h"
#include "FontUnit.h"
#include "ContinuousColorUnit.h"
#include "DiscreteColorUnit.h"
#include "IBDMarkerUnit.h"
#include "DefaultsUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TOptionsForm *OptionsForm;
//---------------------------------------------------------------------------

bool NowFormToSettings;

__fastcall TOptionsForm::TOptionsForm(TComponent* Owner)
        : TForm(Owner)
{ //OwnOptions = new TViewOptions;
}
//---------------------------------------------------------------------------



void __fastcall TOptionsForm::INameFontbtnClick(TObject *Sender)
{ FontForm->Caption = "Individual names font";
  FntAttToFont(OwnOptions.INameFont,FontForm->FontLbl->Font);
  if (FontForm->ShowModal() == mrOk)
  { FontToFntAtt(FontForm->FontLbl->Font,OwnOptions.INameFont); }
}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::AlleleFontbtnClick(TObject *Sender)
{ FontForm->Caption = "Allele font";
  FntAttToFont(OwnOptions.AlleleFont,FontForm->FontLbl->Font);
  if (FontForm->ShowModal() == mrOk)
  { FontToFntAtt(FontForm->FontLbl->Font,OwnOptions.AlleleFont); }
}
//---------------------------------------------------------------------------

void ColCombxDrawItem(TComboBox *Combx, TColor Col, AnsiString Text,
  TRect Rect, TOwnerDrawState State)
{
   Combx->Canvas->Pen->Color=clWhite;
   Combx->Canvas->Pen->Style=psSolid;
   Combx->Canvas->Brush->Color=clWhite;
   Combx->Canvas->Brush->Style=bsSolid;
   Combx->Canvas->Rectangle(Rect.Left,Rect.Top,Rect.Right,Rect.Bottom);
   Combx->Canvas->Pen->Color=clBlack;
   Combx->Canvas->Brush->Color=Col; //Opt.ACol[Index];
   Combx->Canvas->Rectangle(Rect.Left+2,Rect.Top+2,Rect.Left+22,Rect.Bottom-2);
   Combx->Canvas->Brush->Color=clWhite;
   Combx->Canvas->Font->Color=clBlack;
   Combx->Canvas->Font->Height=Rect.Bottom-Rect.Top; //Height fits in Rect.Height
   Combx->Canvas->TextOut(Rect.Left+25,Rect.Top,Text); //IntToStr(Index));
} //ColCombxDrawItem

void __fastcall TOptionsForm::FormShow(TObject *Sender)
{ TWinControl *ActCont;
  TTabSheet *ActPg;
  ActCont = ActiveControl;
  ActPg = PageControl1->ActivePage;
  OptionsToForm();
  LinkGrpCombxChange(NULL); //to update IBDMarkerForm
  //if non-relevant tab selected, go to IndivSheet:
  if ( ActPg==MarkerSheet &&
       !(OwnOptions.IndivContents==icMarkers)
     )
  { PageControl1->ActivePage=IndivSheet;
    ActiveControl=OKbtn;
  }
  else if ( ActPg==IBDSheet &&
            !(OwnOptions.IndivContents==icIBDprob || OwnOptions.IndivContents==icIBDall)
          )
  { PageControl1->ActivePage=IndivSheet;
    ActiveControl=OKbtn;
  }
  else if (ActPg->TabVisible) PageControl1->ActivePage=ActPg;
  if (ActPg==PageControl1->ActivePage && ActCont->Enabled && ActCont->Visible)
    ActiveControl=ActCont;
  else ActiveControl=OKbtn;
} //TOptionsForm::FormShow

void TOptionsForm::ShowMarkerLview(void)
{ int i, LocNum;
  TLocus *Loc;
  MarkerLview->Clear();
     for (i=0; i<TotLocCount; i++)
     { MarkerLview->Items->Add();
       if (OwnOptions.MarkByName)
       { Loc=(TLocus*) LocusList->Items[i];
         LocNum = LinkGrp[Loc->LGnr]->StartLocNr + Loc->LocNum;
         MarkerLview->Items->Item[i]->Checked = OwnOptions.LocSelected(LocNum);
       }
       else
       { Loc=GetLocusByNr(i);
         MarkerLview->Items->Item[i]->Checked = OwnOptions.LocSelected(i);
       }
       MarkerLview->Items->Item[i]->Caption = Loc->LocName;
       MarkerLview->Items->Item[i]->SubItems->Add(LinkGrp[Loc->LGnr]->LGName);
       MarkerLview->Items->Item[i]->SubItems->Add(FloatToStrF(Loc->cM,ffFixed,7,1));
     }
     MarkerLview->ItemIndex=0;

}


void TOptionsForm::OptionsToForm(void)
{  int i; //TLocus *Loc;

   //options from Page setup page:
   //PapWidthEd->Text=IntToStr(OwnOptions.PgWidthMM / 100);
   //PapHeightEd->Text=IntToStr(OwnOptions.PgHeightMM / 100);
   //LeftMargEd->Text=IntToStr(OwnOptions.MargMM[0] / 100);
   //RightMargEd->Text=IntToStr(OwnOptions.MargMM[2] / 100);
   //TopMargEd->Text=IntToStr(OwnOptions.MargMM[1] / 100);
   //BottomMargEd->Text=IntToStr(OwnOptions.MargMM[3] / 100);

   //PgTranspCbx->Checked=OwnOptions.PgTransp;
   //ShowFrameCbx->Checked=OwnOptions.ShowFrame;

   //options from Layout page
   ShowCrossCbx->Checked=OwnOptions.ShowCrossings;
   CrossSizeEd->Text=FloatToStrF(OwnOptions.CrossSizeMM / 100.0, ffFixed,4,1);
   PedOrientRGrp->ItemIndex= OwnOptions.LeftRight ? 1 : 0;
   BetwGenerEd->Text=IntToStr(OwnOptions.GenerDist);
   BetwFamEd->Text=IntToStr(OwnOptions.BetwFamSpace);
   BetwSibEd->Text=IntToStr(OwnOptions.BetwSibSpace);

   //options from Individuals page:
   //TraitsRbtn->Enabled = IndivDataCount>0; TraitSheet->TabVisible=TraitsRbtn->Enabled;
   // (not implemented yet)
   IBDprobRbtn->Enabled = IBDAlleleCount>0; //IBDprobsPresent;
   IBDallRbtn->Enabled = FounderAllelesPresent && IBDprobRbtn->Enabled; //IBDprobsPresent;
   IBDSheet->TabVisible = IBDprobRbtn->Enabled || IBDallRbtn->Enabled;
   MarkRbtn->Enabled = TotLocCount>0; //AlleleObsPresent;
   MarkerSheet->TabVisible = MarkRbtn->Enabled;
   FixedColorPanel->Color = OwnOptions.IBackCol;
   TraitCombx->Items->Clear();
   if (IndivDataCount>0)
   { TraitColbtn->Enabled = true;
     for (i=0; i<IndivDataCount; i++)
         TraitCombx->Items->Add(IndivData[i].Caption);
     if (OwnOptions.ColorTrait<0 || OwnOptions.ColorTrait>=IndivDataCount)
     { OwnOptions.ColorTrait=-1; //to force new colors in DiscreteColorForm
       TraitCombx->ItemIndex=0;
       OwnOptions.IFixedColor=true;
     }
     else TraitCombx->ItemIndex=OwnOptions.ColorTrait;
     FixedColbtn->Checked = OwnOptions.IFixedColor;
     TraitColbtn->Checked = ! OwnOptions.IFixedColor;
   }
   else
   { TraitColbtn->Enabled = false;
     OwnOptions.IFixedColor=true;
     FixedColbtn->Checked=true;
     TraitCombx->Text="";
     OwnOptions.ColorTrait=-1;
   }
   IBackColbtnClick(NULL);


   switch (OwnOptions.IndivContents)
   { //case icTraits : TraitsRbtn->Checked=true; break;  not implemented yet
     case icIBDprob : IBDprobRbtn->Checked=true; break;
     case icIBDall : IBDallRbtn->Checked=true; break;
     case icMarkers : MarkRbtn->Checked=true; break;
     default : NameRbtn->Checked=true;
   }

   //set active page; must be done after Individuals page
   if ((PageControl1->ActivePage==TraitSheet && !TraitSheet->TabVisible) ||
       (PageControl1->ActivePage==MarkerSheet && !MarkerSheet->TabVisible) ||
       (PageControl1->ActivePage==IBDSheet && !IBDSheet->TabVisible))
     PageControl1->ActivePage = IndivSheet;

   //options from IBD page:
   if (IBDSheet->TabVisible)
   { LinkGrpCombx->Items->Clear();
     //LinkGrpCombx->Items->Add("(none)");
     for (i=0; i<LinkGrpCount; i++)
     { LinkGrpCombx->Items->Add(LinkGrp[i]->LGName); }
     if (OwnOptions.LG<0 || OwnOptions.LG>=LinkGrpCount)
     { LinkGrpCombx->ItemIndex=0; }
     else LinkGrpCombx->ItemIndex=OwnOptions.LG;
     //ShowProbcbx->Checked=OwnOptions.ShowProb;
     ProbWidthEd->Text=IntToStr(OwnOptions.ProbWidthMM / 100);
     ProbHeightEd->Text=IntToStr(OwnOptions.ProbHeightMM / 100);
     BoldEd->Text=FloatToStrF(0.0001*OwnOptions.ProbBig,ffGeneral,1,2);
     ItalEd->Text=FloatToStrF(0.0001*OwnOptions.ProbSmall,ffGeneral,1,2);
     AlleleCombx->Items->Clear();
     for (i=0; i<IBDAlleleCount; i++)
     { AlleleCombx->Items->Add(""); }
      AlleleCombx->ItemIndex=0;
   }

   //options from Markers page
   if (MarkerSheet->TabVisible)
   { MrkNameRbtn->Checked=OwnOptions.MarkByName;
     ShowMarkerLview();
     MarkColorCombx->Items->Clear();
     for (i=0; i<MarkCodeCount; i++)
     { MarkColorCombx->Items->Add(""); }
     MarkColorCombx->ItemIndex=0;
   }

   //options from Colors page
   PageColPanel->Color      = OwnOptions.PageCol;
   CrossColPanel->Color     = OwnOptions.CrossCol;
   Par1ColPanel->Color      = OwnOptions.Par1Col;
   Par2ColPanel->Color      = OwnOptions.Par2Col;
   ParSelfedColPanel->Color = OwnOptions.ParSelfedCol;

   //options from Info page
   InfoSelChbx->Checked     = OwnOptions.InfoSelected;
   InfoParentsChbx->Checked = OwnOptions.InfoParents;
   InfoTraitChlist->Clear();
   for (i=0; i<IndivDataCount; i++)
   { InfoTraitChlist->AddItem(IndivData[i].Caption,NULL);
     InfoTraitChlist->Checked[i]=OwnOptions.InfoTrait[i];
   }
}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::ColorbtnClick(TObject *Sender)
{ TColor* Col;
  TPanel* Panel;
  Col=NULL; Panel=NULL;
  if (Sender==SetFixedColBtn)
    { Col=&(OwnOptions.IBackCol); Panel=FixedColorPanel; } else
  if (Sender==PageColBtn)
    { Col=&(OwnOptions.PageCol); Panel=PageColPanel; } else
  //if (Sender==FrameColorbtn) Col=&(OwnOptions.FrameCol); else
  //if (Sender==IndFrameColor) Col=&(OwnOptions.IFrameCol); else
  //if (Sender==INameColorbtn) Col=&(OwnOptions.INameCol); else
  if (Sender==CrossColBtn)
    { Col=&(OwnOptions.CrossCol); Panel=CrossColPanel; } else
  if (Sender==Par1ColBtn)
    { Col=&(OwnOptions.Par1Col); Panel=Par1ColPanel; } else
  if (Sender==Par2ColBtn)
    { Col=&(OwnOptions.Par2Col); Panel=Par2ColPanel; } else
  if (Sender==ParSelfedColBtn)
    { Col=&(OwnOptions.ParSelfedCol); Panel=ParSelfedColPanel; }
  if (Col==NULL) ShowMessage ("ColorbtnClick called from invalid source");
  else
  { ColorDialog1->Color=*Col;
    if (ColorDialog1->Execute() )
    { *Col=ColorDialog1->Color;
      if (Panel != NULL) Panel->Color=*Col;
    }
  }
} //TOptionsForm::ColorbtnClick
//---------------------------------------------------------------------------



void __fastcall TOptionsForm::AlleleCombxDrawItem(TWinControl *Control,
      int Index, TRect &Rect, TOwnerDrawState State)
{ if (IBDSheet->TabVisible)
  { if (IBDalleleFounder==NULL)
       ColCombxDrawItem((TComboBox*)(Control),OwnOptions.ACol[Index],
           IntToStr(Index), Rect,State);
    //else if (IBDalleleFounder[Index]<0)
    else if (IBDalleleFounder[IBDalleleIndex[Index]]<0)
       ColCombxDrawItem((TComboBox*)(Control),OwnOptions.ACol[IBDalleleIndex[Index]],
           "("+IntToStr(IBDalleleIndex[Index])+": no founder)",
           Rect,State);
    else
       ColCombxDrawItem(
           (TComboBox*)(Control),
           OwnOptions.ACol[IBDalleleIndex[Index]],
           //IntToStr(Index)+" "+
             Indiv[IBDalleleFounder[IBDalleleIndex[Index]]].IName+"-"+
             IntToStr(IBDalleleFounderHom[IBDalleleIndex[Index]]+1),
           Rect,State);
  }
}

void __fastcall TOptionsForm::MarkColorCombxDrawItem(TWinControl *Control,
      int Index, TRect &Rect, TOwnerDrawState State)
{ if (MarkerSheet->TabVisible)
    ColCombxDrawItem((TComboBox*)(Control),OwnOptions.MarkCodeCol[Index],IntToStr(Index),
                     Rect,State);
}

//---------------------------------------------------------------------------

void TOptionsForm::GetSelectedLoci(void)
{ int i,j;
  TLocus *Loc;

  OwnOptions.MarkSelCount=0;
  for (i=0; i<MarkerLview->Items->Count; i++)
    if (MarkerLview->Items->Item[i]->Checked) OwnOptions.MarkSelCount++;
  delete[] OwnOptions.MarkSelected;
  if (OwnOptions.MarkSelCount==0) OwnOptions.MarkSelected=NULL;
  else
  { OwnOptions.MarkSelected = new int [OwnOptions.MarkSelCount];
    if (OwnOptions.MarkByName)
    { j=0;
      for (i=0; i<MarkerLview->Items->Count; i++)
      { if (MarkerLview->Items->Item[i]->Checked)
        { Loc=(TLocus*) LocusList->Items[i];
          OwnOptions.MarkSelected[j++]=
             LinkGrp[Loc->LGnr]->StartLocNr + Loc->LocNum;
        }
      }
    }
    else //markers by position
    { j=0;
      for (i=0; i<MarkerLview->Items->Count; i++)
      { if (MarkerLview->Items->Item[i]->Checked)
          OwnOptions.MarkSelected[j++]=i;
      }
    }
  } //MarkSelCount>0
} //GetSelectedLoci

double GetFloat(TEdit *Edit, double min, double max)
{ double Result;
  try
  { try {Result=StrToFloatGen(Edit->Text); }
    catch (...) {throw Exception ("Invalid number format"); }
    if (Result<min || Result>max)
    throw Exception("Number out of range");
    return Result;
  }
  catch ( Exception& e)
  { Edit->SelectAll();
    Edit->SetFocus();
    throw; //rethrow same exception
  }
} //GetFloat


bool TOptionsForm::FormToOptions(void)
//update OwnOptions with data from form; returns true if no errors
//HomFounders, fonts and most colors already updated when dialog closed
{ bool Result; double d;
  int i;
  TTabSheet *OrigSheet;

  Result=true;
  NowFormToSettings=true;
  OrigSheet=PageControl1->ActivePage;
  //{$R+Q+ ; cause range and overflow errors to raise exception}

  /*options from Page setup page
  PageControl1->ActivePage=PageSheet; //go to offending page
  try
  { //OwnOptions.PgWidthMM=100*GetFloat(PapWidthEd,20.0,1999.9);
    //OwnOptions.PgHeightMM=100*GetFloat(PapHeightEd,20.0,1999.9);
    //OwnOptions.MargMM[0]=100*GetFloat(LeftMargEd,0.0,999.9);
    //OwnOptions.MargMM[1]=100*GetFloat(TopMargEd,0.0,999.9);
    //OwnOptions.MargMM[2]=100*GetFloat(RightMargEd,0.0,999.9);
    //OwnOptions.MargMM[3]=100*GetFloat(BottomMargEd,0.0,999.9);
    //if (OwnOptions.PgWidthMM-OwnOptions.MargMM[0]-OwnOptions.MargMM[2]<200 ||
    //    OwnOptions.PgHeightMM-OwnOptions.MargMM[1]-OwnOptions.MargMM[3]<200)
    //{ throw Exception("Page area within marging too small"); }
    //OwnOptions.PgTransp=PgTranspCbx->Checked;
    //OwnOptions.ShowFrame=ShowFrameCbx->Checked;
  }
  catch (Exception &e)
  { Result=false;
    ShowMessage("Error in Page layout options:/n"+e.Message);
  } */

  if (Result)
  //options from Layout page
  { PageControl1->ActivePage=LayoutSheet;
    try
    { OwnOptions.ShowCrossings=ShowCrossCbx->Checked;
      OwnOptions.CrossSizeMM=100*GetFloat(CrossSizeEd,0.0,999.9);
      OwnOptions.LeftRight=PedOrientRGrp->ItemIndex==1;
      OwnOptions.GenerDist=GetFloat(BetwGenerEd,0.0,100.0);
      OwnOptions.BetwFamSpace=GetFloat(BetwFamEd,0.0,10000.0);
      OwnOptions.BetwSibSpace=GetFloat(BetwSibEd,0.0,100.0);
    }
    catch (const Exception& e)
    { Result=false;
      ShowMessage("Error in Other options:\n"+e.Message);
    }
  }

  if (Result)
  //options from Individuals page
  { PageControl1->ActivePage=IndivSheet;
    try
    { /*if (TraitsRbtn->Checked) OwnOptions.IndivContents=icTraits;
      else*/ if (IBDprobRbtn->Checked) OwnOptions.IndivContents=icIBDprob;
      else if (IBDallRbtn->Checked) OwnOptions.IndivContents=icIBDall;
      else if (MarkRbtn->Checked) OwnOptions.IndivContents=icMarkers;
      else OwnOptions.IndivContents=icNone;

      OwnOptions.IFixedColor = FixedColbtn->Checked;
      OwnOptions.ColorTrait = TraitCombx->ItemIndex;
      CheckDiscreteColors();
    }
    catch (const Exception& e)
    { Result=false;
      ShowMessage("Error in Individuals options:/n"+e.Message);
    }
  }

  if (Result && MarkerSheet->TabVisible)
  //options from Markers page
  { PageControl1->ActivePage=MarkerSheet;
    try
    { GetSelectedLoci();
    }
    catch (const Exception& e)
    { Result=false;
      ShowMessage("Error in Markers options:/n"+e.Message);
    }
  }

  if (Result && IBDSheet->TabVisible)
  //options from IBD page
  { PageControl1->ActivePage=IBDSheet;
    try
    { OwnOptions.LG=LinkGrpCombx->ItemIndex;
      //OwnOptions.ShowProb=ShowProbcbx->Checked != false;
      OwnOptions.ProbWidthMM=100*GetFloat(ProbWidthEd,0.0,999.9);
      OwnOptions.ProbHeightMM=100*GetFloat(ProbHeightEd,0.0,999.9);
      //OwnOptions.MinAllProb=1000*GetFloat(ProbHeightEd,-99999.9,99999.9);
      //if (OwnOptions.MinAllProb<0 || OwnOptions.MinAllProb>1000)
      //OwnOptions.MinAllProb=2000;
      d=GetFloat(BoldEd,0.0,1.1);
      OwnOptions.ProbBig=10000*d;
      OwnOptions.ProbSmall=10000*GetFloat(ItalEd,0.0,d);
    }
    catch (const Exception& e)
    { Result=false;
      ShowMessage("Error in IBD options:/n"+e.Message);
    }
  }

  if (Result)
  //options from Colors page
  { PageControl1->ActivePage=ColorsSheet;
    try
    { //all colors already set
    }
    catch (const Exception& e)
    { Result=false;
      ShowMessage("Error in Colors options:/n"+e.Message);
    }
  }

  if (Result)
  //options from Info page
  { PageControl1->ActivePage=InfoSheet;
    try
    { OwnOptions.InfoSelected=InfoSelChbx->Checked;
      OwnOptions.InfoParents=InfoParentsChbx->Checked;
      for (i=0; i<IndivDataCount; i++)
        OwnOptions.InfoTrait[i]=InfoTraitChlist->Checked[i];
    }
    catch (const Exception& e)
    { Result=false;
      ShowMessage("Error in Info options:/n"+e.Message);
    }
  }

  if (Result) PageControl1->ActivePage=OrigSheet;

  return Result;
} //TOptionsForm::FormToOptions


//---------------------------------------------------------------------------

void __fastcall TOptionsForm::FormCloseQuery(TObject *Sender,
      bool &CanClose)
{ CanClose= (ModalResult==mrCancel) || FormToOptions();
}
//---------------------------------------------------------------------------


void __fastcall TOptionsForm::HomFounderBtnClick(TObject *Sender)
{ OwnOptions.HomFounders=true;
  OwnOptions.SetDefAlleleColors();
  AlleleCombx->Refresh();
}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::HeterzFounderBtnClick(TObject *Sender)
{ OwnOptions.HomFounders=false;
  OwnOptions.SetDefAlleleColors();
  AlleleCombx->Refresh();
}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::AllSamebtnClick(TObject *Sender)
{ int c;
  OwnOptions.HomFounders=false;
  ColorDialog1->Color=clBlack;
  if (ColorDialog1->Execute() )
  { for (c=0; c<IBDAlleleCount; c++)
      OwnOptions.ACol[c]=ColorDialog1->Color;
  }
  AlleleCombx->Refresh();
}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::AlleleColbtnClick(TObject *Sender)
{ ColorDialog1->Color=OwnOptions.ACol[IBDalleleIndex[AlleleCombx->ItemIndex]];
  if (ColorDialog1->Execute() )
  { OwnOptions.ACol[IBDalleleIndex[AlleleCombx->ItemIndex]]=ColorDialog1->Color;
    AlleleCombx->Refresh();
  }
} //TOptionsForm::AlleleColbtnClick
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::CodeColbtnClick(TObject *Sender)
{ ColorDialog1->Color=OwnOptions.MarkCodeCol[MarkColorCombx->ItemIndex];
  if (ColorDialog1->Execute() )
  { OwnOptions.MarkCodeCol[MarkColorCombx->ItemIndex]=ColorDialog1->Color;
    MarkColorCombx->Refresh();
  }
}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::CheckAllMarkersbtnClick(TObject *Sender)
{ for (int i=0; i<TotLocCount; i++)
    MarkerLview->Items->Item[i]->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::ClearAllMarkersbtnClick(TObject *Sender)
{ for (int i=0; i<TotLocCount; i++)
    MarkerLview->Items->Item[i]->Checked = false;
}
//---------------------------------------------------------------------------


void __fastcall TOptionsForm::IBackColbtnClick(TObject *Sender)
{ if (FixedColbtn->Checked)
    { SetFixedColBtn->Enabled=true;
      TraitCombx->Enabled=false;
      SetTraitColbtn->Enabled=false;
    }
   else
    { SetFixedColBtn->Enabled=false;
      TraitCombx->Enabled=true;
      SetTraitColbtn->Enabled=true;
    }
}
//---------------------------------------------------------------------------
void TOptionsForm::CheckDiscreteColors(void)
{ if (FixedColbtn->Checked) return;
  //if OwnOptions.TraitColCount incorrect, generate new TraitColors:
  //Note: we don't use TViewOptions::SefDefTraitColors because if TraitColCount
  //is unchanged we want to retain the set colors.
    if (IndivData[TraitCombx->ItemIndex].DataType==dtC &&
        IndivData[TraitCombx->ItemIndex].V.CV.LevelCount!=OwnOptions.TraitColCount)
    { //reset OwnOptions colors for all Levels:
      OwnOptions.TraitColCount=IndivData[TraitCombx->ItemIndex].V.CV.LevelCount;
      delete[] OwnOptions.TraitCol;
      OwnOptions.TraitCol = new TColor[OwnOptions.TraitColCount];
      FillColors(OwnOptions.TraitColCount, OwnOptions.TraitCol,
          OwnOptions.LoCol, OwnOptions.HiCol, TraitCombx->ItemIndex);
    }
    else if (IndivData[TraitCombx->ItemIndex].DataType==dtS &&
             IndivData[TraitCombx->ItemIndex].V.SV.LabelCount!=OwnOptions.TraitColCount)
    { //reset OwnOptions colors for all Labels:
      OwnOptions.TraitColCount=IndivData[TraitCombx->ItemIndex].V.SV.LabelCount;
      delete[] OwnOptions.TraitCol;
      OwnOptions.TraitCol = new TColor[OwnOptions.TraitColCount];
      FillColors(OwnOptions.TraitColCount, OwnOptions.TraitCol,
          OwnOptions.LoCol, OwnOptions.HiCol, TraitCombx->ItemIndex);
    }
} //TViewOptions::CheckDiscreteColors


void __fastcall TOptionsForm::SetTraitColbtnClick(TObject *Sender)
{ int i;
  if (TraitCombx->ItemIndex != OwnOptions.ColorTrait)
  { delete[] OwnOptions.TraitCol; OwnOptions.TraitCol=NULL;
    OwnOptions.TraitColCount=0;
    OwnOptions.ColorTrait=TraitCombx->ItemIndex;
  }
  if (IndivData[TraitCombx->ItemIndex].DataType==dtI ||
      IndivData[TraitCombx->ItemIndex].DataType==dtF ||
        (IndivData[TraitCombx->ItemIndex].DataType==dtC &&
         IndivData[TraitCombx->ItemIndex].V.CV.LevelCount>MaxTraitColors) ||
        (IndivData[TraitCombx->ItemIndex].DataType==dtS &&
         IndivData[TraitCombx->ItemIndex].V.SV.LabelCount>MaxTraitColors)
     )
  { ContinuousColorForm->LoCol = OwnOptions.LoCol;
    ContinuousColorForm->HiCol = OwnOptions.HiCol;
    ContinuousColorForm->MVCol = OwnOptions.MVCol;
    if (ContinuousColorForm->ShowModal() == mrOk)
    { OwnOptions.LoCol = ContinuousColorForm->LoCol;
      OwnOptions.HiCol = ContinuousColorForm->HiCol;
      OwnOptions.MVCol = ContinuousColorForm->MVCol;
    }
  }
  else //discrete colors
  { //if OwnOptions.TraitColCount incorrect, generate new TraitColors:
    CheckDiscreteColors();
    /*if (IndivData[TraitCombx->ItemIndex].DataType==dtC &&
        IndivData[TraitCombx->ItemIndex].V.CV.LevelCount!=OwnOptions.TraitColCount)
    { //reset OwnOptions colors for all Levels:
      OwnOptions.TraitColCount=IndivData[TraitCombx->ItemIndex].V.CV.LevelCount;
      delete[] OwnOptions.TraitCol;
      OwnOptions.TraitCol = new TColor[OwnOptions.TraitColCount];
      DiscreteColorForm->FillColors(OwnOptions.TraitColCount, OwnOptions.TraitCol,
          OwnOptions.LoCol, OwnOptions.HiCol, TraitCombx->ItemIndex);
    }
    else if (IndivData[TraitCombx->ItemIndex].DataType==dtS &&
             IndivData[TraitCombx->ItemIndex].V.SV.LabelCount!=OwnOptions.TraitColCount)
    { //reset OwnOptions colors for all Labels:
      OwnOptions.TraitColCount=IndivData[TraitCombx->ItemIndex].V.SV.LabelCount;
      delete[] OwnOptions.TraitCol;
      OwnOptions.TraitCol = new TColor[OwnOptions.TraitColCount];
      DiscreteColorForm->FillColors(OwnOptions.TraitColCount, OwnOptions.TraitCol,
          OwnOptions.LoCol, OwnOptions.HiCol, TraitCombx->ItemIndex);
    } */

    // set the colors for DiscreteColorForm:
    DiscreteColorForm->ColCount=OwnOptions.TraitColCount;
    delete[] DiscreteColorForm->Col;
    DiscreteColorForm->Col = new TColor[OwnOptions.TraitColCount];
    for (i=0; i<OwnOptions.TraitColCount; i++)
      DiscreteColorForm->Col[i] = OwnOptions.TraitCol[i];
    DiscreteColorForm->MVCol=OwnOptions.MVCol;
    DiscreteColorForm->InDat=TraitCombx->ItemIndex;
    // take new OwnOptions colors from DiscreteColorForm:
    if (DiscreteColorForm->ShowModal()==mrOk)
    { for (i=0; i<OwnOptions.TraitColCount; i++)
        OwnOptions.TraitCol[i] = DiscreteColorForm->Col[i];
      OwnOptions.MVCol=DiscreteColorForm->MVCol;
    }
  } // discrete colors
} //TOptionsForm::SetTraitColbtnClick
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::DefaultbtnClick(TObject *Sender)
{ const AnsiString DefFile="Options.ini";
  TStringList *SL;
  //TViewOptions *TempOpt=NULL;
{ if (DefaultsForm->ShowModal()==mrOk &&
      DefaultsForm->DefaultsRgroup->ItemIndex>-1)
  { SL = new TStringList;
    __try
    { if (DefaultsForm->DefaultsRgroup->ItemIndex==0)
      { OwnOptions.WriteData(SL,true);
        SL->SaveToFile(ProgDir+DefFile);
      }
      else if (DefaultsForm->DefaultsRgroup->ItemIndex==1)
      { OwnOptions.MakeBuiltinOpt();
        OptionsToForm();
      }
      else if (DefaultsForm->DefaultsRgroup->ItemIndex==2)
      { if (FileExists(ProgDir+DefFile))
        { SL->LoadFromFile(ProgDir+DefFile);
          OwnOptions.ReadFromStringlist(SL,IBDAlleleCount,true);
          OptionsToForm();
        }
        else ShowMessage("Saved options file "+ProgDir+DefFile+" not found");
      }
    }
    __finally { delete SL; }
  }
} //TOptionsForm::DefaultbtnClick

}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::IBDprobRbtnClick(TObject *Sender)
{ if (IBDprobRbtn->Checked) PageControl1->ActivePage=IBDSheet;
}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::IBDallRbtnClick(TObject *Sender)
{ if (IBDallRbtn->Checked) PageControl1->ActivePage=IBDSheet;

}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::MarkRbtnClick(TObject *Sender)
{ if (MarkRbtn->Checked) PageControl1->ActivePage=MarkerSheet;

}
//---------------------------------------------------------------------------


void __fastcall TOptionsForm::SelectMrkbtnClick(TObject *Sender)
{ TLinkGrp *LG;
  int i;

  if (LinkGrpCombx->ItemIndex <0 ) return;
  LG=LinkGrp[LinkGrpCombx->ItemIndex];
  if (IBDMarkerForm->ShowModal()==mrOk)
  { //check if all markers selected:
    i=0;
    while (i<IBDMarkerForm->IBDmrkLview->Items->Count &&
           IBDMarkerForm->IBDmrkLview->Items->Item[i]->Checked)
         i++;
    OwnOptions.AllIBDmrk = i==IBDMarkerForm->IBDmrkLview->Items->Count;
    if ( OwnOptions.AllIBDmrk )
    { delete[] OwnOptions.SelIBDmrk; OwnOptions.SelIBDmrk=NULL; }
    else
    { if (OwnOptions.SelIBDmrk==NULL) OwnOptions.SelIBDmrk = new bool [LG->LocCount];
      for (i=0; i<LG->LocCount; i++)
        OwnOptions.SelIBDmrk[i] = IBDMarkerForm->IBDmrkLview->Items->Item[i]->Checked;
    }
  }
} //TOptionsForm::SelectMrkbtnClick
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::LinkGrpCombxChange(TObject *Sender)
{ TLinkGrp *LG;
  int i;

  if (LinkGrpCombx->ItemIndex <0 )return;
  if ( OwnOptions.LG != LinkGrpCombx->ItemIndex )
  { //if LG changed, reset and clear SelIBDmrk:
    if (OwnOptions.SelIBDmrk!=NULL) {delete[] OwnOptions.SelIBDmrk; OwnOptions.SelIBDmrk=NULL;}
    OwnOptions.AllIBDmrk = true;
  }
  LG=LinkGrp[LinkGrpCombx->ItemIndex];
  IBDMarkerForm->LG = LG;
  IBDMarkerForm->LGnameEd->Text = LG->LGName;
  IBDMarkerForm->IBDmrkLview->Items->Clear();
  for (i=0; i<LG->LocCount; i++)
  { IBDMarkerForm->IBDmrkLview->Items->Add();
    IBDMarkerForm->IBDmrkLview->Items->Item[i]->Caption=LG->Locus[i]->LocName;
    IBDMarkerForm->IBDmrkLview->Items->Item[i]->SubItems->Add(FloatToStrF(LG->Locus[i]->cM,ffFixed,8,1));
    IBDMarkerForm->IBDmrkLview->Items->Item[i]->Checked =
         (OwnOptions.SelIBDmrk==NULL) || OwnOptions.SelIBDmrk[i];
         //so, if SelIBDmrk==NULL all markers are checked
  }
  //following line MUST appear after adding items
  //IBDMarkerForm->AllMrkRbtn->Checked = OwnOptions.AllIBDmrk; deleted 13-02-2010
} //TOptionsForm::LinkGrpCombxChange
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::MrkPosRbtnClick(TObject *Sender)
{ GetSelectedLoci(); //with old MarkByName setting
  OwnOptions.MarkByName=MrkNameRbtn->Checked;
  ShowMarkerLview(); //with new MarkByName setting
}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::InfAllbtnClick(TObject *Sender)
{ int i;
  for (i=0; i<InfoTraitChlist->Count; i++)
    InfoTraitChlist->Checked[i]=true;
}
//---------------------------------------------------------------------------

void __fastcall TOptionsForm::InfNonebtnClick(TObject *Sender)
{ int i;
  for (i=0; i<InfoTraitChlist->Count; i++)
    InfoTraitChlist->Checked[i]=false;
}
//---------------------------------------------------------------------------

void TOptionsForm::SetParentColorCaptions(int FemaleParent)
{         switch (FemaleParent) {
          case 0: { Par1ColBtn->Caption = "Line to female parent ...";
                    Par2ColBtn->Caption = "Line to male parent ...";
                    break; }
          case 1: { Par1ColBtn->Caption = "Line to male parent ...";
                    Par2ColBtn->Caption = "Line to female parent ...";
                    break; }
          default:{ Par1ColBtn->Caption = "Line to parent 1 ...";
                    Par2ColBtn->Caption = "Line to parent 2 ...";
                    break; }
        } //switch
        //caption of ParSelfedColBtn is always "Line to single parent ..."
} //SetParentColorCaptions
