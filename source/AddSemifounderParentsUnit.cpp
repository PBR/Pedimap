//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "AddSemifounderParentsUnit.h"
#include "DataUnit.h"
#include "ParseUnit.h"
#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAddSemifounderParentsForm *AddSemifounderParentsForm;
//---------------------------------------------------------------------------
__fastcall TAddSemifounderParentsForm::TAddSemifounderParentsForm(TComponent* Owner)
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
      s=s+Tb+DataToStr(d,Ind);
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
    SL->Add("NULL ="+Tb+strConfirmedNull);
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

int* GetAllPossibleAlleles(int ind, TLocus* locus) {
   int* indall = new int[Ploidy+1]; //store a MAXINT in the last used element
   for (int p=0; p<Ploidy; p++) {
     //store all elements sorted in indall
     int a = locus->AllIx[ind][p];
     int j=0; while (j<p && indall[j]<a) j++;
     while (j<p) {
       int tmp = indall[j]; indall[j] = a; a=tmp; j++;
     }
     indall[p] = a;
   }
   indall[Ploidy] = MAXINT;
   if (indall[0]==-2) indall[0]=-3; //if nullhomoz, null is a possible allele
   if (indall[0]==-1) { //if missing, go to ancestors
     TCrossing* Cro = Indiv[ind].ParentCross;
     if (Cro!=NULL) {
       if (Cro->Parent[0]>=0 &&
           (Cro->CType!=ctCross || Cro->Parent[1]>=0)) {
         //no missing parents, i.e. ind is not a founder or semi-founder
         //(else ind could have any allele, so the missing score must not be changed)
         indall = GetAllPossibleAlleles(Cro->Parent[0],locus);
         if (indall[0]!=-1 && Cro->CType==ctCross) {
           //only check parent 1 if parent 0 does not pass on unknown alleles
           int* parall = GetAllPossibleAlleles(Cro->Parent[1],locus);
           if (parall[0]==-1) indall[0]=-1;
           else {
             //both parents do not pass on unknown alleles, so combine the two lists:
             int* tmpall = new int[(sizeof(indall)+sizeof(parall)) / sizeof(int)];
             int i=0; int j=0; int k=0;
             int ai = indall[0]; int aj = parall[0];
             while (ai!=MAXINT || aj!=MAXINT ) {
               if (ai<aj) {
                 tmpall[k++] = ai;
                 ai = indall[++i];
               } else if (aj<ai) {
                 tmpall[k++] = aj;
                 aj = parall[++j];
               } else {
                 //ai==aj, not both MAXINT
                 tmpall[k++] = ai;
                 ai = indall[++i];
                 aj = parall[++j];
               }
             } //while
             tmpall[k] = MAXINT;
             indall = tmpall;
           } //else (both parents non-missing alleles)
         } //ctCross
       } //not (semi-)founder
     } //not Cro==NULL
   } //ind has missing allele scores
   return indall;
} //GetAllPossibleAlleles

bool isIn(int target, int* array) {
  //array is sorted, has at least one element, bounded by MAXINT
  int i=0;
  while (array[i]<target) i++;
  return array[i]==target;
}

void CalcSemifounderParentAlleles(int SemifounderParentIndex, AnsiString sfpAllele) {
  TIndiv* np = &Indiv[SemifounderParentIndex]; //the new semifounder parent;
  //ShowMessage("np="+np->IName);
  TIndiv* sf = &Indiv[SemifounderParentIndex+1]; //the original semifounder
  //ShowMessage("sf="+sf->IName);
  TIndiv* op; //the "other parent" of the original semifounder
  TCrossing* Cro = sf->ParentCross;
  int opi = Cro->Parent[0]==SemifounderParentIndex ? Cro->Parent[1] : Cro->Parent[0];
  if (opi<0 || opi>=SemifounderParentIndex) throw Exception("Error finding semifounder parents");
  op = &Indiv[opi];
  for (int lg=0; lg<LinkGrpCount; lg++) {
    TLinkGrp* lgrp = LinkGrp[lg];
    for (int loc=0; loc<lgrp->LocCount; loc++) {
      TLocus* locus = lgrp->Locus[loc];
      // get the allele index of the sfpAllele name:
      int sfpall = 0;
      while (sfpall<locus->AllNameCount && locus->AllName[sfpall]!=sfpAllele)
        sfpall++;
      //now sfpall is the allele index to add; if this actually occurs then sfpAllele
      //will be added to AllName
      bool sfpAlleleAdded=false;
      //get the two alleles for np and possibly sf
      int* opall = NULL; //array containing the two (or more) possible alleles of op
      int* sfall = NULL; //array containing the two possible alleles of sf
      //fill opall: TODO fill in from op ancestors if genotype op op unknown
      //opall = new int[2];  //TODO: change to collect all possible alleles if op itself unknown
      //for (int p=0; p<Ploidy; p++) opall[p] = locus->AllIx[opi][p];
      //sort:
      //if (opall[0]>opall[1]) {
      //  int tmp=opall[0]; opall[0]=opall[1]; opall[1]=tmp;
      //}
      opall = GetAllPossibleAlleles(opi,locus); //last used element is MAXINT
      //fill sfall:
      sfall = new int[2];
      for (int p=0; p<Ploidy; p++) sfall[p] = locus->AllIx[SemifounderParentIndex+1][p];
      //sort:
      if (sfall[0]>sfall[1]) {
        int tmp=sfall[0]; sfall[0]=sfall[1]; sfall[1]=tmp;
      }
      //do the analysis
      if (sfall[0]>=0) { //semifounder has no missing, nullhomoz or null allele
        if (sfall[1]==sfall[0]) { //semifounder is homozygous
          locus->AllIx[SemifounderParentIndex][0] = sfall[0];
          locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
        }
        else if (opall[0]>=0) { //other parent has no missing, nullhomoz or null allele
          if (isIn(sfall[0],opall) && isIn(sfall[1],opall)) {
            //both alleles can be inherited from op and sfp
            locus->AllIx[SemifounderParentIndex][0] = sfall[0];
            locus->AllIx[SemifounderParentIndex][1] = sfall[1];
          }
          else { //only one allele can be inherited from op
            if (isIn(sfall[0],opall)) {
              //first allele comes from op, so second from sfp:
              locus->AllIx[SemifounderParentIndex][0] = sfall[1];
            }
            else { //vice versa:
              locus->AllIx[SemifounderParentIndex][0] = sfall[0];
            }
            locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
          }
        }
        else if (opall[0]==-1) { //other parent has missing allele info
          //both alleles can be inherited from op and sfp
          locus->AllIx[SemifounderParentIndex][0] = sfall[0];
          locus->AllIx[SemifounderParentIndex][1] = sfall[1];
        }
        else { //other parent is homozygous or has null allele,
               //so only one allele can be inherited from op
          if (isIn(sfall[0],opall)) {
            //first allele comes from op, so second from sfp:
            locus->AllIx[SemifounderParentIndex][0] = sfall[1];
          }
          else { //vice versa:
            locus->AllIx[SemifounderParentIndex][0] = sfall[0];
          }
          locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
        }
      }
      else if (sfall[0]==-1) { //semifounder has missing allele info
        //sfp remains unknown, nothing changes
      }
      else if (sfall[0]==-2) { //semifounder is homozygous or has null allele
        if (opall[0]>=0) { //op no missing, nullhomoz or null
          if (isIn(sfall[1],opall)) {
            // the non-null sf allele can be inherited from op; sfp can have a null and/or sfall[1]:
            locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
            locus->AllIx[SemifounderParentIndex][1] = sfall[1];
          }
          else {
            // the sfall[1] must come from sfp:
            locus->AllIx[SemifounderParentIndex][0] = sfall[1];
            locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
          }
        }
        else if (opall[0]==-1) { //other parent has missing allele info
          // both sf alleles can be inherited from op; sfp can have a null and/or sfall[1]:
          locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
          locus->AllIx[SemifounderParentIndex][1] = sfall[1];
        }
        else { //other parent is homozygous or has null allele
          if (isIn(sfall[1],opall)) {
            //sfall[1] can be inherited from op; sfp can have a null and/or sfall[1]:
            locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
            locus->AllIx[SemifounderParentIndex][1] = sfall[1]; //is not a null allele
          }
          else {
            // the sfall[1] must come from sfp:
            locus->AllIx[SemifounderParentIndex][0] = sfall[1];
            locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
          }
        }
      }
      else { //semifounder has one (or two) confirmed null alleles and the other allele is also known
        //different situation from sf nullhomoz!
        if (sfall[1]==-3) { //semifounder is homozygous for the null allele
          locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
          locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
        }
        else { //second allele of sf is a known, non-null allele
          if (opall[0]>=0) { //other parent has no missing, nullhomoz or null allele
            //therefore the null must come from sfp:
            locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
            locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
          }
          else if (opall[0]==-1) { //other parent has missing allele info
            //sfp can have a null and/or sfall[1]:
            locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
            locus->AllIx[SemifounderParentIndex][1] = sfall[1]; //is not a null allele
          }
          else { //other parent is homozygous or has null allele
            if (isIn(sfall[1],opall)) {
              //sfall[1] is not null and can be inherited from op; sfp can have a null and/or sfall[1]:
              locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
              locus->AllIx[SemifounderParentIndex][1] = sfall[1]; //is not a null allele
            }
            else {
              // the sfall[1] is not null and must come from sfp:
              locus->AllIx[SemifounderParentIndex][0] = sfall[1];
              locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
            }
          }
        }
      }
      //finally, if sfpAllele is a new allele name and it has been used,
      //add it to the AllName array
      if (sfpAlleleAdded && sfpall>=locus->AllNameCount) {
        locus->AllNameCount++;
        AnsiString* tmp = locus->AllName;
        locus->AllName = new AnsiString[locus->AllNameCount];
        for (int a=0; a<locus->AllNameCount-1; a++) locus->AllName[a]=tmp[a];
        locus->AllName[locus->AllNameCount-1] = sfpAllele;
        delete[] tmp;
      }  
    } // for loc
  } //for lg
} //CalcSemifounderParentAlleles

void AddSemifounderParents(AnsiString NewFileName, AnsiString sfpAllele) {
  AnsiString tempfile = "tempdata.tmp";
  //writePedimapDatafile("testwithibd.txt", true);
  //writePedimapDatafile("testwithoutibd.txt", false);
  //store the current projectfile and patafile names:
  AnsiString OldProjDir, OldProjFile; //, OldDataDir, OldDataFile;
  OldProjDir = ProjectDir;
  OldProjFile = ProjectFileName;
  //OldDataDir = DataDir;
  //OldDataFile = DataFileName;

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
      AnsiString prefix;
      if ( Cro->Parent[0]<0 ) {
        //first parent missing
        prefix = FemaleParent==0 ? "M_" :
                   FemaleParent==1 ? "F_" : "P_";
      }
      else {
        //second parent missing
        prefix = FemaleParent==0 ? "F_" :
                   FemaleParent==1 ? "M_" : "P_";
      }
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
    MainForm->SaveProject("",tempfile+".pmp");
    ClearData();
    ReadDataFile(tempfile);
    //ShowMessage("File read: "+tempfile);
    //next, fill in the marker data for the new individuals
    if (Ploidy==2 && LinkGrpCount>0) { //currently only implemented for diploids
      int j=0;
      for (int i=0; i<IndivCount;i++) {
        if (j<NewParentCount && i==NewParents[j]) {
          CalcSemifounderParentAlleles(i, sfpAllele);
          j++;
        }
      }
    } //Ploidy==2
    writePedimapDatafile(NewFileName,false);
    //ShowMessage("File created: "+NewFileName);
    //finally, re-run the current project so that all changes are updated
    //in the subpopulations and views;
    //save a fog-file with the names of the new individuals
    //and display a message with the name of this file, the number of new individuals
    ProjectDir = OldProjDir;
    ProjectFileName = OldProjFile;
    if (AddSemifounderParentsForm->Importcbx->Checked) {
      //set the new datafile:
      DataDir = ExtractFilePath(NewFileName);
      DataFileName = ExtractFileName(NewFileName);
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
} //AddSemifounderParents


void __fastcall TAddSemifounderParentsForm::BrowsebtnClick(TObject *Sender)
{
   if (SaveDialog1->Execute())
     FilenameEd->Text = SaveDialog1->FileName;
}
//---------------------------------------------------------------------------

bool TestMissingAlleleString() {
//return true if ok, false if invalid
  AnsiString s = AddSemifounderParentsForm->AlleleEd->Text;
  return (!IsUnknown(s) && s != strNullHomoz) && s != strConfirmedNull;
}

void __fastcall TAddSemifounderParentsForm::OKbtnClick(TObject *Sender)
{  //Note: we return ModalResult=mrNo to signal invalid input
   AlleleEd->Text = AlleleEd->Text.Trim();
   if (AlleleEd->Text.IsEmpty() || IsUnknown(AlleleEd->Text) ||
       AlleleEd->Text==strNullHomoz || AlleleEd->Text==strConfirmedNull) {
     ShowMessage("Non-transmitted allele invalid");
     ModalResult = mrNo;
   }
   else {
     FilenameEd->Text = FilenameEd->Text.Trim();
     if (FileExists(FilenameEd->Text) &&
         (MessageDlg("Overwrite existing file ?",
                    mtConfirmation,TMsgDlgButtons() << mbYes << mbNo,0)
            == mrNo )) {
       ModalResult = mrNo;
     }
     else ModalResult=mrOk;
   }
} //TAddSemifounderParentsForm::OKbtnClick
//---------------------------------------------------------------------------


