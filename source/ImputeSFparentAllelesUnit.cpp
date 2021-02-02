//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "ImputeSFparentAllelesUnit.h"
#include "DataUnit.h"
#include "ParseUnit.h"
#include "main.h"
#include "AddSFparentsUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TImputeSFparentAllelesForm *ImputeSFparentAllelesForm;
//---------------------------------------------------------------------------
__fastcall TImputeSFparentAllelesForm::TImputeSFparentAllelesForm(TComponent* Owner)
 : TForm(Owner)
{
}
//---------------------------------------------------------------------------

bool isIn(int target, int* array) {
  //array is sorted, has at least one element, bounded by MAXINT
  int i=0;
  while (array[i]<target) i++;
  return array[i]==target;
}

bool allMissing(int* array) {
  //array is sorted, has at least one element, bounded by MAXINT
  int i=0;
  while (array[i]==-1) i++;
  return array[i]==MAXINT;
}

int numval(int* array) {
  //array is sorted, has at least one element, bounded by MAXINT
  int i=0;
  while (array[i]<MAXINT) i++;
  return i;
}

int* GetAllPossibleAlleles(int ind, TLocus* locus) {
   int* indall = new int[Ploidy+1]; //store a MAXINT in the last used element
   indall[0] = MAXINT;
   for (int p=0; p<Ploidy; p++) {
     //store all elements sorted and non-duplicated in indall
     int a = locus->AllIx[ind][p];
     int j=0; while (indall[j]<a) j++;
     if (indall[j]>a) {
       do {
         int tmp = indall[j]; indall[j] = a; a=tmp; j++;
       } while (a<MAXINT);
       indall[j] = MAXINT;
     }
     //else equal, value not stored
   }
   //indall[Ploidy] = MAXINT;
   if (indall[0]==-2) indall[0]=-3; //if nullhomoz, null is a possible allele
   if (allMissing(indall)) { //if missing, go to ancestors
     TCrossing* Cro = Indiv[ind].ParentCross;
     if (Cro!=NULL) {
       if (Cro->Parent[0]>=0 &&
           (Cro->CType!=ctCross || Cro->Parent[1]>=0)) {
         //no missing parents, i.e. ind is not a founder or semi-founder
         //(else ind could have any allele, so the missing score must not be changed)
         delete[] indall; indall=NULL;
         indall = GetAllPossibleAlleles(Cro->Parent[0],locus);
         if (!isIn(-1,indall) && Cro->CType==ctCross) {
           //only check parent 1 if parent 0 does not pass on unknown alleles
           int* parall = GetAllPossibleAlleles(Cro->Parent[1],locus);
           if (parall[0]==-1) indall[0]=-1;
           else {
             //both parents do not pass on unknown alleles, so combine the two lists:
             int* tmpall = new int[numval(indall)+numval(parall)+1];
             int i=0; int j=0; int k=0;
             int ai = indall[0]; int aj = parall[0];
             while (ai!=MAXINT || aj!=MAXINT ) {
               if (ai<aj) {
                 tmpall[k] = ai;
                 do {ai = indall[++i];}
                 while (ai==tmpall[k]);
                 k++;
               } else if (aj<ai) {
                 tmpall[k] = aj;
                 do {aj = parall[++j];}
                 while (aj==tmpall[k]);
                 k++;
               } else {
                 //ai==aj, not both MAXINT
                 tmpall[k] = ai;
                 do {ai = indall[++i];}
                 while (ai==tmpall[k]);
                 do {aj = parall[++j];}
                 while (aj==tmpall[k]);
                 k++;
               }
             } //while
             tmpall[k] = MAXINT;
             delete[] indall; delete[] parall;
             indall = new int[k+1];
             for (int n=0; n<=k; n++)
               indall[n] = tmpall[n];
             delete[] tmpall;
           } //else (both parents non-missing alleles)
         } //ctCross
       } //not (semi-)founder
     } //not Cro==NULL
   } //ind has missing allele scores
   return indall;
} //GetAllPossibleAlleles

AnsiString Errorline(int ind, TLocus* locus) {
  //produce an error line with the information
  return "error: ind="+Indiv[ind].IName+Tb+"LG="+LinkGrp[locus->LGnr]->LGName+Tb+"locus="+locus->LocName;
}

void CalcSemifounderParentAlleles(int SemifounderParentIndex, AnsiString sfpAllele,
       int colorCode, TStringList* logSL) {
  /*test:
  int* tstall = GetAllPossibleAlleles(2,LinkGrp[0]->Locus[4]);
  AnsiString as = "tstall:";
  int i=0;
  while (i<10 && tstall[i]<MAXINT)
    as = as + " " + IntToStr(tstall[i++]);
  if (i>10) ShowMessage("i>10");  
  //end test */
  TIndiv* np = &Indiv[SemifounderParentIndex]; //the new semifounder parent;
  int sfi = Indiv[SemifounderParentIndex].Mating[0]->Offsp[0]; //index of semifounder
  TIndiv* sf = &Indiv[sfi]; //the original semifounder
  TIndiv* op; //the "other parent" of the original semifounder
  TCrossing* Cro = sf->ParentCross;
  int opi = Cro->Parent[0]==SemifounderParentIndex ? Cro->Parent[1] : Cro->Parent[0];
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
      int* opall = NULL; //array containing the two (or more) possible alleles of op "other parent"
      int* sfall = NULL; //array containing the two possible alleles of sf "semifounder"
      int* npall = NULL; //array containing the two possible alleles of np "new parent"
      opall = GetAllPossibleAlleles(opi,locus); //last used element is MAXINT
      /* for debugging:
      AnsiString s ="sfp="+Indiv[SemifounderParentIndex].IName+" OP, locus="+locus->LocName+":";
      int a=0;
      while (opall[a]<MAXINT) {
        s=s+" "+IntToStr(opall[a]); a++;
      }
      logSL->Add(s);
      */

      //fill sfall:
      sfall = new int[2];
      for (int p=0; p<Ploidy; p++) sfall[p] = locus->AllIx[sfi][p];
      //sort:
      if (sfall[0]>sfall[1]) {
        int tmp=sfall[0]; sfall[0]=sfall[1]; sfall[1]=tmp;
      }
      //fill npall:
      npall = new int[2];
      for (int p=0; p<Ploidy; p++) npall[p] = locus->AllIx[SemifounderParentIndex][p];
      //sort:
      if (npall[0]>npall[1]) {
        int tmp=npall[0]; npall[0]=npall[1]; npall[1]=tmp;
      }

      //if both np alleles missing, do the imputation:
      //if (MarkCodeCount<colorCode+1)
      //   MarkCodeCount = colorCode+1;
      if (npall[0]==-1 && npall[1]==-1) {
        if (sfall[0]>=0) { //semifounder has no missing, nullhomoz or null allele
          if (sfall[1]==sfall[0]) {
            //sf is homozygous
            if (opall[0]==-1 || isIn(sfall[0],opall)) { //else impossible
              locus->AllIx[SemifounderParentIndex][0] = sfall[0];
              locus->AllCode[SemifounderParentIndex][0] = colorCode;
              locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
              locus->AllCode[SemifounderParentIndex][1] = colorCode;
            } else logSL->Add(Errorline(sfi,locus));
          }
          else if (opall[0]>=0) { //sf heterozygous, other parent has no missing, nullhomoz or null allele
              if (isIn(sfall[0],opall) || isIn(sfall[1],opall)) { //else impossible
                if (isIn(sfall[0],opall) && isIn(sfall[1],opall)) {
                  //both alleles can be inherited from op and sfp
                  locus->AllIx[SemifounderParentIndex][0] = sfall[0];
                  locus->AllCode[SemifounderParentIndex][0] = colorCode;
                  locus->AllIx[SemifounderParentIndex][1] = sfall[1];
                  locus->AllCode[SemifounderParentIndex][1] = colorCode;
                }
                else { //only one allele can be inherited from op
                  if (isIn(sfall[0],opall)) {
                    //first allele comes from op, so second from sfp:
                    locus->AllIx[SemifounderParentIndex][0] = sfall[1];
                    locus->AllCode[SemifounderParentIndex][0] = colorCode;
                  }
                  else { //vice versa:
                    locus->AllIx[SemifounderParentIndex][0] = sfall[0];
                    locus->AllCode[SemifounderParentIndex][0] = colorCode;
                  }
                  locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
                  locus->AllCode[SemifounderParentIndex][1] = colorCode;
                }
              } else logSL->Add(Errorline(sfi,locus));
          }
          else if (opall[0]==-1) { //other parent has missing allele info
            //both alleles can be inherited from op and sfp, no error
            locus->AllIx[SemifounderParentIndex][0] = sfall[0];
            locus->AllCode[SemifounderParentIndex][0] = colorCode;
            locus->AllIx[SemifounderParentIndex][1] = sfall[1];
            locus->AllCode[SemifounderParentIndex][1] = colorCode;
          }
          else { //other parent is homozygous or has null allele,
                 //or it has null allele + unknown allele
            if (!isIn(-1,opall)) {
              //no missing in opall
              //op is homozygous or has null allele,
              //so only one allele can be inherited from op
              if (isIn(sfall[0],opall) || isIn(sfall[1],opall)) { //else impossible
                if (isIn(sfall[0],opall)) {
                  //first allele comes from op, so second from sfp:
                  locus->AllIx[SemifounderParentIndex][0] = sfall[1];
                  locus->AllCode[SemifounderParentIndex][0] = colorCode;
                }
                else { //vice versa:
                  locus->AllIx[SemifounderParentIndex][0] = sfall[0];
                  locus->AllCode[SemifounderParentIndex][0] = colorCode;
                }
                locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
                locus->AllCode[SemifounderParentIndex][1] = colorCode;
              } else logSL->Add(Errorline(sfi,locus));
            } //not isIn(-1,opall)
            else { //op has missing allele
              //both alleles can be inherited from op and sfp, no error
              locus->AllIx[SemifounderParentIndex][0] = sfall[0];
              locus->AllCode[SemifounderParentIndex][0] = colorCode;
              locus->AllIx[SemifounderParentIndex][1] = sfall[1];
              locus->AllCode[SemifounderParentIndex][1] = colorCode;
            }
          }
        } //sfall[0]>=0
        else if (sfall[0]==-1) { //semifounder has missing allele info
          if (sfall[1]>=0 && !isIn(sfall[1],opall)) {
            //the known allele must come from np:
            locus->AllIx[SemifounderParentIndex][0] = sfall[1];
            locus->AllCode[SemifounderParentIndex][0] = colorCode;
            locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
            locus->AllCode[SemifounderParentIndex][1] = colorCode;
          }
          // else sfp remains unknown, nothing changes, no error
        }
        else if (sfall[0]==-2) { //semifounder is homozygous or has null allele
          if (sfall[1]==-1) logSL->Add(Errorline(sfi,locus)); //nonsense score
          else if (sfall[1]==-2) {
            //only interpretation is double null: adjust score and continue there
            locus->AllIx[sfi][0] = -3;
            //locus->AllCode[sfi][0] = colorCode;
            locus->AllIx[sfi][1] = -3;
            //locus->AllCode[sfi][1] = colorCode;
            //we don't set the colorcode as the score was not missing
            sfall[0] = -3;
            sfall[1] = -3;
            goto doublenull;
          }
          else {
            //sfall[1] is a normal, non-null allele
            if (opall[0]>=0) { //op no missing, nullhomoz or null
              if (isIn(sfall[1],opall)) {
                // the non-null sf allele can be inherited from op; sfp can have a null and/or sfall[1]:
                locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
                locus->AllCode[SemifounderParentIndex][0] = colorCode;
                locus->AllIx[SemifounderParentIndex][1] = sfall[1];
                locus->AllCode[SemifounderParentIndex][1] = colorCode;
              }
              else logSL->Add(Errorline(sfi,locus)); //no allele from op can be passed to sf!
            }
            else if (opall[0]==-1) { //other parent has missing allele info, no error
              // both sf alleles can be inherited from op; sfp can have a null and/or sfall[1]:
              locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
              locus->AllCode[SemifounderParentIndex][0] = colorCode;
              locus->AllIx[SemifounderParentIndex][1] = sfall[1];
              locus->AllCode[SemifounderParentIndex][1] = colorCode;
            }
            else { //other parent is homozygous or has null allele
                   //or has missing allele
              if (!isIn(-1,opall)) {
                // op has no missing allele
                if (isIn(sfall[1],opall)) {
                  //sfall[1] can be inherited from op; sfp can have a null and/or sfall[1]:
                  locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
                  locus->AllCode[SemifounderParentIndex][0] = colorCode;
                  locus->AllIx[SemifounderParentIndex][1] = sfall[1]; //is not a null allele
                  locus->AllCode[SemifounderParentIndex][1] = colorCode;
                }
                else {
                  // the sfall[1] must come from sfp:
                  locus->AllIx[SemifounderParentIndex][0] = sfall[1];
                  locus->AllCode[SemifounderParentIndex][0] = colorCode;
                  locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
                  locus->AllCode[SemifounderParentIndex][1] = colorCode;
                  //but we know also that the semifounder itself is not homozygous
                  //but has a null allele:
                  if (locus->AllIx[sfi][0]==-2) {
                     locus->AllIx[sfi][0]=-3;
                     locus->AllCode[sfi][0] = colorCode;
                  }
                  if (locus->AllIx[sfi][1]==-2) {
                     locus->AllIx[sfi][1]=-3;
                     locus->AllCode[sfi][1] = colorCode;
                  }
                  //we set the colorCode, although the original score was not actually missing
                  //We also know that the op therefore must also have a null allele,
                  //but as there are multiple situations (op missing, nullhomoz or confirmednull)
                  //we leave that to FlexQTL for the moment
                }
              }
              else {
                //op has missing allele, sfp can have a null and/or sfall[1]:
                locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
                locus->AllCode[SemifounderParentIndex][0] = colorCode;
                locus->AllIx[SemifounderParentIndex][1] = sfall[1]; //is not a null allele
                locus->AllCode[SemifounderParentIndex][1] = colorCode;
              }
            }
          }// sfall[0]==-2, sfall[1]>=0
        } //sfall[0]==-2
        else { //sfall[0]==-3
          //semifounder has one (or two) confirmed null alleles and the other allele is also known
          //different situation from sf nullhomoz!
          if (sfall[1]==-1) {
            //sf has one true null and one unknown allele
            if (opall[0]>=0) {
              // the true null cannot come from op so must come from np:
              locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
              locus->AllCode[SemifounderParentIndex][0] = colorCode;
              locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
              locus->AllCode[SemifounderParentIndex][1] = colorCode;
            }
            // else np remains unknown
          }
          else if (sfall[1]==-2) {
            // then both sf alleles must be true null alleles:
            locus->AllIx[sfi][0] = -3;
            //locus->AllCode[sfi][0] = colorCode;
            locus->AllIx[sfi][1] = -3;
            //locus->AllCode[sfi][1] = colorCode;
            //we don't set the colorCode as the score was not missing
            sfall[1] = -3;
            //and we continue in the next situation: two true null alleles
          }
          //no else here: we continue
          if (sfall[1]==-3) { //semifounder is homozygous for the null allele
            doublenull:  //label for goto!
            if (opall[0]<0) { // else impossible
              locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
              locus->AllCode[SemifounderParentIndex][0] = colorCode;
              locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
              locus->AllCode[SemifounderParentIndex][1] = colorCode;
            } else logSL->Add(Errorline(sfi,locus));
          }
          else if (sfall[1]>=0) { //second allele of sf is a known, non-null allele
            if (opall[0]>=0) { //other parent has no missing, nullhomoz or null allele,
              //therefore the null must come from sfp:
              if (isIn(sfall[1],opall)) { // else impossible
                locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
                locus->AllCode[SemifounderParentIndex][0] = colorCode;
                locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
                locus->AllCode[SemifounderParentIndex][1] = colorCode;
              } else logSL->Add(Errorline(sfi,locus));
            }
            else if (opall[0]==-1) { //other parent has missing allele info
              //sfp can have a null and/or sfall[1]:
              locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
              locus->AllCode[SemifounderParentIndex][0] = colorCode;
              locus->AllIx[SemifounderParentIndex][1] = sfall[1]; //is not a null allele
              locus->AllCode[SemifounderParentIndex][1] = colorCode;
            }
            else { //op is homozygous or has null allele
                   // or op has a missing allele
              if (!isIn(-1, opall)) { // op has no missing allele
                if (isIn(sfall[1],opall)) {
                  //sfall[1] is not null and can be inherited from op; sfp can have a null and/or sfall[1]:
                  locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
                  locus->AllCode[SemifounderParentIndex][0] = colorCode;
                  locus->AllIx[SemifounderParentIndex][1] = sfall[1]; //is not a null allele
                  locus->AllCode[SemifounderParentIndex][1] = colorCode;
                }
                else {
                  // the sfall[1] is not null and must come from sfp;
                  // the null can come from op, so no error
                  locus->AllIx[SemifounderParentIndex][0] = sfall[1];
                  locus->AllCode[SemifounderParentIndex][0] = colorCode;
                  locus->AllIx[SemifounderParentIndex][1] = sfpall; sfpAlleleAdded=true;  //new allele!
                  locus->AllCode[SemifounderParentIndex][1] = colorCode;
                }
              }
              else {
                //op has missing allele, sfp can have a null and/or sfall[1]:
                locus->AllIx[SemifounderParentIndex][0] = -3; //confirmed null
                locus->AllCode[SemifounderParentIndex][0] = colorCode;
                locus->AllIx[SemifounderParentIndex][1] = sfall[1]; //is not a null allele
                locus->AllCode[SemifounderParentIndex][1] = colorCode;
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
      } // np alleles missing
    } // for loc
  } //for lg
} //CalcSemifounderParentAlleles

void ImputeSFparentAlleles(AnsiString NewFileName, AnsiString sfpAllele,
          int colorCode) {
  TStringList* ImputationLog = new TStringList();
  AnsiString tempfile = "tempdata.tmp";
  //store the current projectfile and patafile names:
  AnsiString OldProjDir, OldProjFile;
  OldProjDir = ProjectDir;
  OldProjFile = ProjectFileName;

  //TViewPop *VP;
  //VP = (TViewPop*) MainForm->PopTreeView->Items->Item[0]->Data; //whole population
  //fill in the marker data for the ex-semifounder parents and similar founders:
  if (Ploidy==2 && LinkGrpCount>0) { //currently only implemented for diploids
    for (int i=0; i<IndivCount;i++) {
      if (/*VP->IsVPFounder(i)*/ Indiv[i].ParentCross==NULL && Indiv[i].MCount==1 &&
          Indiv[i].Mating[0]->OCount==1) {
        //Indiv[i] is a founder with exactly 1 direct descendant,
        //e.g. an added semifounder parent
        ImputationLog->Add("Founder: "+Indiv[i].IName);
        CalcSemifounderParentAlleles(i, sfpAllele, colorCode, ImputationLog);
      }
    }
  } //Ploidy==2
  writePedimapDatafile(NewFileName,false);
  if (ImputationLog->Count>0)
    ImputationLog->SaveToFile(ExtractFilePath(NewFileName)+"Pedimap imputation.log");
  //ShowMessage("File created: "+NewFileName);
  //finally, re-run the current project so that all changes are updated
  //in the subpopulations and views;
  ProjectDir = OldProjDir;
  ProjectFileName = OldProjFile;
  if (ImputeSFparentAllelesForm->Importcbx->Checked) {
    //set the new datafile:
    DataDir = ExtractFilePath(NewFileName);
    DataFileName = ExtractFileName(NewFileName);
    DataFileExt= ExtractFileExt(DataFileName);
    DataFileName = DataFileName.SubString(1,DataFileName.Length()
                   - DataFileExt.Length()); //excluding Extension
    //copied from ClearData:
    ProjectSaved=false;
    MainForm->MainListview->Visible=false;
    MainForm->ViewScrollbox->Visible=false;
    MainForm->MainTabctrl->Tabs->Clear();
    if (MainForm->PopTreeView->Items->Count > 0)
       delete MainForm->PopTreeView->Items->Item[0]; //root, clears all nodes and automatically ViewPops
    //NwMainForm->PopTreeView->Clear(); automatically
    TViewPop::VPCount=0;
    CurrentViewPop=NULL; SourceViewPop=NULL;
    MainForm->MainTabctrl->Tabs->Clear();
    //now we need to set MarkCodeCount to colorCode+1, else if
    //MarkCodeCount is smaller in the original project it is not reset:
    MarkCodeCount = colorCode+1;
    //TODO: there may be an issue here if the original file contained
    //IBD values; to check later
    MainForm->OpenProjectFile(ProjectDir+ProjectFileName+ProjectExt,false); //keep the data now loaded
    ProjectSaved = false;
  }
  else { //ImportCbx not checked
    MainForm->OpenProjectFile(ProjectDir+ProjectFileName+ProjectExt,true);
  }
} //ImputeSFparentAlleles


void __fastcall TImputeSFparentAllelesForm::BrowsebtnClick(TObject *Sender)
{
   if (SaveDialog1->Execute())
     FilenameEd->Text = SaveDialog1->FileName;
}
//---------------------------------------------------------------------------

bool TestMissingAlleleString() {
//return true if ok, false if invalid
  AnsiString s = ImputeSFparentAllelesForm->AlleleEd->Text;
  return (!IsUnknown(s) && s != strNullHomoz) && s != strConfirmedNull;
}

void __fastcall TImputeSFparentAllelesForm::OKbtnClick(TObject *Sender)
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
} //TImputeSFparentAllelesForm::OKbtnClick
//---------------------------------------------------------------------------


