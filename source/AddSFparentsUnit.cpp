//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "AddSFparentsUnit.h"
#include "DataUnit.h"
#include "ParseUnit.h"
#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAddSFparentsForm *AddSFparentsForm;
//---------------------------------------------------------------------------
__fastcall TAddSFparentsForm::TAddSFparentsForm(TComponent* Owner)
 : TForm(Owner)
{
}
//---------------------------------------------------------------------------

AnsiString MakePedigreeLine(int Ind, AnsiString NewParent) {
  //NewParent only used to fill in new parent of semifounder
  AnsiString s = QuotedString(Indiv[Ind].IName)+Tb;
  TCrossing* Cro = Indiv[Ind].ParentCross;
  if (Cro==NULL)
     s=s+strUnknown[0]+Tb+strUnknown[0];
  else
  { //first parent
    if ( Cro->Parent[0]<0 ) {
       if (NewParent==NULL || NewParent.Length()==0) s=s+strUnknown[0]+Tb;
       else s=s+QuotedString(NewParent)+Tb;
    }
    else s=s+QuotedString(Indiv[Cro->Parent[0]].IName)+Tb;
    //second parent
    if (Cro->CType==ctCross)
    {  if ( Cro->Parent[1]<0 ) {
           if (NewParent==NULL || NewParent.Length()==0) s=s+strUnknown[0]+Tb;
           else s=s+QuotedString(NewParent)+Tb;
       }
       else s=s+QuotedString(Indiv[Cro->Parent[1]].IName);
    }
    else
    { if (Cro->CType==ctSelfing)
      { if ( Cro->Parent[0]<0 )
           s=s+strUnknown[0];
        else s=s+QuotedString(Indiv[Cro->Parent[0]].IName);
      }
      else { //not Cross or Selfing; give cross type instead of second parent
             s=s+CrossTypeString[Cro->CType];
      }
    }
  }
  //fill trait part of pedigree line:
  for (int d=0; d<IndivDataCount; d++)
      s=s+Tb+DataToStr(d,Ind,strUnknown[0]);
  return s;
} //MakePedigreeLine

void writeFileHeader(TStringList* SL, bool withIBD) {
    //continues to write to existing SL
    AnsiString s;

    //write header lines
    SL->Add("POPULATION ="+Tb+QuotedString(PopName));
    s ="UNKNOWN =";
    for (int i=0; i<UnknownCount; i++) s=s+Tb+strUnknown[i];
    SL->Add(s);
    SL->Add("NULLHOMOZ ="+Tb+strNullHomoz);
    SL->Add("CONFIRMEDNULL ="+Tb+strConfirmedNull);
    SL->Add("PLOIDY ="+Tb+IntToStr(Ploidy));
    if (withIBD && IBDAlleleCount>0)
      SL->Add("NALLELES ="+Tb+IntToStr(IBDAlleleCount));
} //writeFileHeader

void writePedigree(TStringList* SL, bool withIBD) {

    //write caption of pedigree - traits table:
    SL->Add("");
    SL->Add("PEDIGREE");
    AnsiString s="Name"+Tb+ParentCaption(0)+Tb+ParentCaption(1);
    for (int p=0; p<IndivDataCount; p++)
      s=s+Tb+QuotedString(IndivData[p].Caption);
    SL->Add(s);
    //write pedigree & traits lines:
    for (int i=0; i<IndivCount; i++) {
      SL->Add(MakePedigreeLine(i,NULL));
    } //for i
}

void writeLinkgrps(TStringList* SL, bool withIBD) {
    //add the linkage groups:
    AnsiString s;
    for (int lg=0; lg<LinkGrpCount; lg++) {
      TLinkGrp* lgrp = LinkGrp[lg];
      SL->Add("");
      SL->Add("LINKAGEGROUP "+lgrp->LGName);
      SL->Add("");
      //write MAP:
      SL->Add("MAP");
      for (int loc=0; loc<lgrp->LocCount; loc++) {
        TLocus* locus = lgrp->Locus[loc];
        SL->Add(locus->LocName + Tb + FloatToStrF(locus->cM,ffFixed,8,2));
      }
      //write each LOCUS:
      for (int loc=0; loc<lgrp->LocCount; loc++) {
        TLocus* locus = lgrp->Locus[loc];
        SL->Add("");
        SL->Add("LOCUS "+locus->LocName);
        s="ALLELENAMES";
        for (int a=0; a<locus->AllNameCount; a++)
          s=s+Tb+locus->AllName[a];
        SL->Add(s);
        if (withIBD && locus->FounderAllele!=NULL) {
          s="FOUNDERALLELES";
          for (int a=0; a<IBDAlleleCount; a++) {
            s=s+Tb+locus->AllName[locus->FounderAllele[a]];
          }
          SL->Add(s);
        }
      } //for loc
      if (withIBD && IBDAlleleCount>0) {
        SL->Add("");
        s="IBDPOSITIONS";
        for (int p=0; p<lgrp->IBDCount; p++) {
          s=s+Tb+FloatToStrF(lgrp->IBDpos[p]->cM,ffFixed,8,2);
        }
        SL->Add(s);
      }
    } //for lg
} //writePedigreeandLinkgrps

void writeAllelesAndIBDs(TStringList* SL, bool withIBD) {
    //continues to write tot existing SL
    AnsiString s;

    //write the allele scores and color codes per locus:
    for (int lg=0; lg<LinkGrpCount; lg++) {
      TLinkGrp* lgrp = LinkGrp[lg];
      for (int loc=0; loc<lgrp->LocCount; loc++) {
        TLocus* locus = lgrp->Locus[loc];
        SL->Add("");
        SL->Add("ALLELES "+locus->LocName+" ; LG "+lgrp->LGName);
        for (int i=0; i<IndivCount; i++) {
          s=QuotedString(Indiv[i].IName);
          for (int p=0; p<Ploidy; p++) {
            int all=locus->AllIx[i][p];
            if (all>=0)
              s=s+Tb+locus->AllName[all];
            else if (all==-1) s=s+Tb+strUnknown[0];
            else if(all==-2) s=s+Tb+strNullHomoz;
            else s=s+Tb+strConfirmedNull;
            s=s+Tb+IntToStr(locus->AllCode[i][p]);
          }
          SL->Add(s);
        } // for i
      } // for loc
    } // for lg

    //finally write the IBD probabilities:
    if (withIBD && IBDAlleleCount>0) {
      for (int lg=0; lg<LinkGrpCount; lg++) {
        TLinkGrp* lgrp = LinkGrp[lg];
        for (int ib=0; ib<lgrp->IBDCount ; ib++) {
          TIBDposition* ibd = lgrp->IBDpos[ib];
          SL->Add("");
          SL->Add("IBDPOSITION "+FloatToStrF(ibd->cM,ffFixed,8,2)+" ; LG "+lgrp->LGName);
          for (int i=0; i<IndivCount; i++) {
            s=QuotedString(Indiv[i].IName);
            for (int p=0; p<Ploidy; p++) {
              if (p>0) s=s+Tb; //extra separation between each block
              for (int j=0; j<IBDAlleleCount; j++) {
                s=s+Tb+FloatToStrF(ibd->AllProb[i][p][j]/240.0,ffFixed,4,2);
              }
            }
            SL->Add(s);
          } //for i
        } // for ib
      } // for lg
    } //if withIBD
} //writeAllelesAndIBDs

bool writePedimapDatafile(AnsiString filename, bool withIBD) {
    bool result=false;
    try {
      TStringList* SL = new TStringList();
      SL->Add(";"+Tb+"Pedimap datafile exported by Pedimap");
      SL->Add(";"+Tb+"Original datafile: " + DataFileName+DataFileExt);
      SL->Add("");
      writeFileHeader(SL, withIBD);
      writePedigree(SL, withIBD);
      writeLinkgrps(SL, withIBD);
      SL->Add("");
      SL->Add("; *************************************");
      writeAllelesAndIBDs(SL, withIBD);
      SL->SaveToFile(filename);
      result=true;
    }
    catch (...) { result=false; }
    return result;
} //writePedimapDatafile

AnsiString MakeNewPedigreeLine(AnsiString name) {
  AnsiString s = QuotedString(name) + Tb + strUnknown[0] + Tb + strUnknown[0];
  for (int d=0; d<IndivDataCount; d++)
      s=s+Tb+strUnknown[0];
  return(s);
}

void AddSFparents(AnsiString NewFileName) {
  AnsiString tempfile = "tempdata.tmp";
  AnsiString temppmb = "tempdata.pmb"; //corresponding pmb file
  //store the current projectfile names:
  AnsiString OldProjDir, OldProjFile;
  OldProjDir = ProjectDir;
  OldProjFile = ProjectFileName;

  //first write the current allele data:
  TStringList* AlleleSL = new TStringList();
  writeAllelesAndIBDs(AlleleSL, false);
  AlleleSL->SaveToFile(tempfile);

  //next, check all indivs for semifoundership,
  //write out the complete pedigree with the new individuals
  //(with missing traits and no marker info) and store their indices:
  int NewParentCount=0;
  int *NewParents = NULL;
  TViewPop *VP;
  VP = (TViewPop*) MainForm->PopTreeView->Items->Item[0]->Data; //whole population
  TStringList* PedSL = new TStringList;
  PedSL->Add("");
  PedSL->Add("PEDIGREE");
  AnsiString s="Name"+Tb+ParentCaption(0)+Tb+ParentCaption(1);
  for (int p=0; p<IndivDataCount; p++)
      s=s+Tb+QuotedString(IndivData[p].Caption);
  PedSL->Add(s);
  //write pedigree & traits lines:
  for (int i=0; i<VP->VPIndCount; i++) {
    if (VP->IsVPSemifounder(i)) {
      //get the name of the new parent:
      TCrossing* Cro = Indiv[i].ParentCross;
      AnsiString prefix; //sfM_, sfF_, sfP_ for semifounder Mother, Father or Parent
      if ( Cro->Parent[0]<0 ) {
        //first parent missing
        prefix = FemaleParent==0 ? "sfM_" :
                   FemaleParent==1 ? "sfF_" : "sfP_";
      }
      else {
        //second parent missing
        prefix = FemaleParent==0 ? "sfF_" :
                   FemaleParent==1 ? "sfM_" : "sfP_";
      }
      //avoid duplicating an existing name by adding additional underscores:
      while (VP->GetVPIndex(prefix+Indiv[i].IName)!=-1)
        prefix=prefix+"_";
      //store the number of the new individual:
      int *tmp = NewParents;
      NewParents = new int[++NewParentCount];
      for (int j=0; j<NewParentCount-1; j++) NewParents[j]=tmp[j];
      delete[] tmp; tmp=NULL;
      NewParents[NewParentCount-1] = -3 + //3 headerlines before first individual
        PedSL->Add(MakeNewPedigreeLine(prefix+Indiv[i].IName));
      //finally write line for I with adapted parent:
      PedSL->Add(MakePedigreeLine(i,prefix+Indiv[i].IName));
    }
    else PedSL->Add(MakePedigreeLine(i,NULL));
  } // for i

  if (NewParentCount>0) {
    //next, write the pedigree & map info, combined with the original marker info
    //and write this to a new temp pedimap datafile
    //delete the temp datafile
    TFileStream* tempstream = new TFileStream(tempfile,fmCreate);
    TStringList* HeaderSL = new TStringList();
    writeFileHeader(HeaderSL,false);
    HeaderSL->SaveToStream(tempstream);
    PedSL->SaveToStream(tempstream);
    TStringList* LgSL = new TStringList();
    writeLinkgrps(LgSL,false);
    LgSL->SaveToStream(tempstream);
    AlleleSL->SaveToStream(tempstream);
    delete tempstream;
    //ShowMessage("File created: "+tempfile);

    //next, delete all data and open the temp datafile.
    //room for the markerdata for the new individuals will be made automatically
    //delete the temp datafile
    AnsiString TempProjectName, TempProjectDir;
    TempProjectName=ProjectFileName; TempProjectDir=ProjectDir;
    //MainForm->SaveProject("",tempfile+".pmp");
    ClearData();
    ReadDataFile(tempfile);
    DeleteFile(tempfile);
    DeleteFile(temppmb);
    //ShowMessage("File read: "+tempfile);
    writePedimapDatafile(NewFileName,false);
    //ShowMessage("File created: "+NewFileName);
    //finally, re-run the current project so that all changes are updated
    //in the subpopulations and views;
    //save a log-file with the names of the new individuals
    //and display a message with the name of this file, the number of new individuals
    ProjectDir = OldProjDir;
    ProjectFileName = OldProjFile;
    if (AddSFparentsForm->Importcbx->Checked) {
      //set the new datafile:
      DataDir = ExtractFilePath(NewFileName);
      DataFileName = ExtractFileName(NewFileName);
      DataFileExt= ExtractFileExt(DataFileName);
      DataFileName = DataFileName.SubString(1,DataFileName.Length()
                   - DataFileExt.Length()); //excluding Extension
      MainForm->OpenProjectFile(ProjectDir+ProjectFileName+ProjectExt,false); //keep the data now loaded
      ProjectSaved = false;
      //TODO: add a new ViewPop with the original semifounders and their ancestry
      /*
      TViewPop* VP = (TViewPop*) MainForm->PopTreeView->Items->Item[0]->Data;
      VP->ClearSelection();
      for (int sfp=0; sfp<NewParentCount; sfp++) {
        int sf = NewParents[sfp]+1;
        //do auto selection:
        VP->SelectVPAncestors(sf,9,2,true);
        //add auto line to SelScript:
        VP->SelScript.AddSelScriptLine();
        int i=VP->SelScript.LineCount-1;
        VP->SelScript.SSLine[i].BaseInd=sf;
        VP->SelScript.SSLine[i].Select=true;
        VP->SelScript.SSLine[i].IndCount=0;
        VP->SelScript.SSLine[i].AncestorGen=9;
        VP->SelScript.SSLine[i].ProgenyGen=0;
        VP->SelScript.SSLine[i].Parentline=2;
        VP->SelScript.SSLine[i].Sibs=false;
        VP->SelScript.SSLine[i].ColdParents=false;
        //update LastSelection:
        delete[] CurrentViewPop->LastSelection; CurrentViewPop->LastSelection=NULL;
        CurrentViewPop->SelectedToSet(CurrentViewPop->LastSelection,CurrentViewPop->LastSelCount);
      }
      */
    }
    else { //ImportCbx not checked
      MainForm->OpenProjectFile(ProjectDir+ProjectFileName+ProjectExt,true);
    }

  } //NewParentCount>0
  else ShowMessage("No semi-founder parents added"); //should not occur
} //AddSFparents

