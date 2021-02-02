#include "PCH.h"
//#include <vcl.h>
#pragma hdrstop

#include "ParseUnit.h"
#include "Main.h"
#include "OpenProgressUnit.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include "DataUnit.h"
#include "InTextfileUnit.h"
#include "BinfileUnit.h"


float *AProbs;
//short int *FrBase; deleted January 2007
bool ParsingFirstLocus,      //first locus determines if Founderalleles en Allelescores
     ParsingFirstLinkGroup; //first linkage group determines if IBD probabilities

const Word BFversion=10001; //version number in binary file (first two bytes)
                      //must be equal to this
const BFnone=0, BFread=1, BFwrite=2; 
const char CommentChar = ';' ;

bool IsCrosstypeString(AnsiString s)
{ int i;
  s=UpperCase(s);
  i=CrossTypeCount-1;
  while (i>=0 && CrossTypeString[i]!=s) i--;
  return i>=0;
}

bool IsUnknown(AnsiString s)
//checks if s occurs in strUnknown list
{ int i;
  i=0;
  while (i<UnknownCount && s!=strUnknown[i])
        i++;
  return i<UnknownCount;
}

bool IsKeyword(AnsiString s)
{ bool Result;
  Result= (IsUnknown(s) || s==strNullHomoz || s==strConfirmedNull); //not converted to uppercase
  s=UpperCase(s);
  Result = Result ||
           s=="POPULATION" ||
           s=="UNKNOWN" || s=="NULLHOMOZ" || s=="CONFIRMEDNULL" ||
           s=="NALLELES" || s=="PLOIDY" ||
           IsCrosstypeString(s) ||
           s=="PEDIGREE" || s=="NAME" ||
           s=="GROUP" || s=="LINKAGEGROUP" || s=="MAP" ||
           s=="LOCUS" || s=="ALLELENAMES" || s=="FOUNDERALLELES" ||
           s=="ALLELES" || 
           s=="IBDPOSITION" || s=="IBDPOSITIONS" ;
  return Result;
} //IsKeyword

int AddIndiv (AnsiString &SelfName, AnsiString &Par0Name, AnsiString &Par1Name)
{ //returns index of this Indiv
  TCrossing *Cro;
  int i, P0, P1, CrType;
  AnsiString s,t,u;

  //Find index i of new TIndiv (note: Indiv array already created and filled with
  //empty TIndivs before starting to read the Indivs)
  i=0;
  while (i<IndivCount && Indiv[i].IName!="" && Indiv[i].IName!=SelfName) i++;
  if (i==IndivCount) throw Exception("Error in AddIndiv: i=IndivCount");
  if (Indiv[i].IName==SelfName) throw Exception ("Duplicate Indiv "+SelfName);
  //Indiv[i].IName=SelfName; fill in after finding parents, to avoid errors with parentname==selfname

  //Find or create parental TCrossing and add Indiv to it:
  if (IsCrosstypeString(Par0Name))
    throw Exception ("First parent of individual "+SelfName+" is a cross type indicator");
  s=UpperCase(Par0Name); t=UpperCase(Par1Name);
  if (!IsUnknown(s) || !IsUnknown(t))
  //else no parents, so no parental cross
  { //1: determine crosstype
    if (s==t) CrType=ctSelfing;
    else
    { u=UpperCase(t); CrType=ctVegProp;
      while (CrType>ctCross && CrossTypeString[CrType]!=u) CrType--;
    }

    //2: check for errors:
    P0=FindIndName(Par0Name);
    if (P0==-1 && !IsUnknown(s))
    { throw Exception ("Parent 1 of " + SelfName + " doesn't appear before " + SelfName);
    }
    if (CrType==ctCross)
    { P1=FindIndName(Par1Name);
      if (P1==-1 && !IsUnknown(t))
      { throw Exception ("Parent 2 of " + SelfName + " doesn't appear before " + SelfName);
      }
    }
    else //not ctCross
    { if (P0==-1)
      { throw Exception ("Parent 1 of " + SelfName + " is unknown"
                         " and cannot be part of uniparental inheritance");
      }
      else P1=P0;
    }

    //3: find or create TCrossing:
    Cro=NULL;
    if (P0!=-1 && P1!=-1) //if not both parents known a new crossing must be created
      Cro=FindCrossParents(P0,P1,CrType);
    if (Cro==NULL)
    { //create new TCrossing and add to parent(s):
      Cro= new TCrossing(P0,P1,CrType);
      if (FirstCrossing==NULL) FirstCrossing=Cro;
      if (P0!=-1) Indiv[P0].AddMating(Cro);
      if (CrType==ctCross && P1!=-1) Indiv[P1].AddMating(Cro);
    }

    //4: Add Self to Cro, and SelfName to Self:
    Cro->AddOffspring(i);
    Indiv[i].ParentCross=Cro;
  } //not both parents unknown
  //finally enter Selfname (not earlier, because Selfname must be unknown while searching parents)
  Indiv[i].IName=SelfName;
  return i;
} // AddIndiv


//still used by SortPedigree
void SkipBlNComm ( int &r, int &p, TStringList *L)
// when called: not in comment
// returns with r=L->Count or r,p at start first non-comment word
{ //const char CommentChar = ';' ;
  if ( r<L->Count )
  { //first work through current line:
    while (p <= L->Strings[r].Length() && L->Strings[r][p]<=' ') p++;
    while ( r<L->Count &&
            (p>L->Strings[r].Length() || L->Strings[r][p]==CommentChar) )
    { //work through following lines:
      if (++r < L->Count)
      { p=1;
        while (p <= L->Strings[r].Length() && L->Strings[r][p]<=' ') p++;
      }
    }
  }
} //SkipBlNComm  Compiler warning about CommentChar never used incorrect!
/*
void SkipPastChar (int &p, const AnsiString &s, char *c)
{ while (p<=s.Length() && s[p]!=*c) p++;  p++; }
*/

 //SortPedigree items:

  struct TInd
       { AnsiString Name, Par1, Par2;
         int LineNr;
       };
  TInd *Ind;

bool Present(int i, const AnsiString S)
//true if Indiv with name S occurs before Ind[i]
//(by setting i to IndivCount the whole Ind array is checked)
{ int j; bool Found;
  Found= IsUnknown(S);
  j=i-1;
  while (j>=0 && !Found)
  { Found=Ind[j].Name == S; j--; }
  return Found;
}

TInd* PruneRemainingInd(TInd* Ind, int &Count) {
// deletes all individuals that are not parents; repeat until none left
  TStringList *Parents;
  int i,p,x;
  bool changed;
  do {
    changed=false;
    //fill the Parents array: may include Crosstypes
    Parents = new TStringList;
    Parents->Sorted = true;
    Parents->CaseSensitive = true;
    for (i=0; i<Count; i++) {
      if (!Parents->Find(Ind[i].Par1,x)) Parents->Add(Ind[i].Par1);
      if (!Parents->Find(Ind[i].Par2,x)) Parents->Add(Ind[i].Par2);
    }
    //delete all individuals that are not parents:
    i = Count-1;
    while (i>=0) {
      if (!Parents->Find(Ind[i].Name,x)) {
        //delete this Ind:
        for (int j=i; j<Count-1; j++) {
          Ind[j] = Ind[j+1];
        }  
        Count--;
        changed=true;
      }
      i--;
    }
  } while (changed);
  return(Ind);
} //PruneRemaining

TStringList* SortPedigree(TStringList *L)
//on entering r points to first line in L after Pedigree & header line
//on exit r points to first line after pedigree
//return value are the lines of the pedigree in correct order

//L contains the lines of the unsorted pedigree (without blank lines or comment lines)
{
  const AnsiString newl=AnsiString(char(13));
  int r,p,j,I,pass;
  TStringList *Result=NULL;
  bool ok;
#undef ReportChanges
#ifdef ReportChanges
  bool Changes;
#endif

  try
  { r=0;
    Ind = new TInd [2*IndivCount]; //room at end needed for sorting
    for (I=0; I<IndivCount; I++)
    //read all following Indivs:
    { p=1; SkipBlNComm(r,p,L);
      ReadWoordSL(r,p, Ind[I].Name, L);
      ReadWoordSL(r,p, Ind[I].Par1, L);
      ReadWoordSL(r,p, Ind[I].Par2, L);
      Ind[I].LineNr=r;
      if ( Ind[I].Par1.Length()==0 || Ind[I].Par1[1]==CommentChar ||
           Ind[I].Par2.Length()==0 || Ind[I].Par2[1]==CommentChar )
      throw Exception("Parent(s) missing for individual "+Ind[I].Name);
      r++;
    }
    /*Sort the Indiv array in ICount-1 passes. In each pass i, an Indiv[i]
      is placed of which both parents (if not unknown) are already placed before.
      This Indiv is searched from position i down. If it is found at position j,
      all the Indivs at position i to j-1 are moved to the end of the array,
      and the gap is filled by moving all Indivs j-i positions up.
      In this way, the order of e.g. sibs placed as one consecutive block
      is conserved.
    */
    //first check if any Indiv is it's own parent:
    I=0;
    while (I<IndivCount && Ind[I].Par1!=Ind[I].Name && Ind[I].Par2!=Ind[I].Name)
    I++;
    if (I<IndivCount)
    { throw Exception("Individual '"+Ind[I].Name+" is it's own parent!");
    }
    // next check if any first parent is a CrosstypeString:
    I=0;
    while (I<IndivCount && !IsCrosstypeString(Ind[I].Par1))
    I++;
    if (I<IndivCount)
    { throw Exception("First parent of individual "+Ind[I].Name+" is a cross type indicator");
    }

    ok=true;
#ifdef ReportChanges
    Changes=false;
#endif
    for (pass=0; pass<IndivCount; pass++)
       //for sorting, pass<ICount-1 would be sufficient, but in that case
       //the last Indiv might have a non-existing parent which would not be noticed
    { I=pass;
      //find first Indiv i that can be placed at position pass:
      while ( I<IndivCount &&
              (!Present(pass,Ind[I].Par1) ||
               (!IsCrosstypeString(Ind[I].Par2) && !Present(pass,Ind[I].Par2))) )
        I++;
      if (I>=IndivCount)
      { //no Indiv remains with both parents already placed;
        //report the first where one parent lacks:
        ok=false;
        I=pass;
        while ( I<IndivCount &&
                (Present(IndivCount,Ind[I].Par1) &&
                 (IsCrosstypeString(Ind[I].Par2) || Present(IndivCount,Ind[I].Par2)))  )
        I++;
        if (I<IndivCount)
        { if (!Present(IndivCount,Ind[I].Par1))
          throw Exception(Ind[I].Par1+" is listed as parent of "+Ind[I].Name+
                          ", but it does not occur in the population");
          else
          throw Exception(Ind[I].Par2+" is listed as parent of "+Ind[I].Name+
                          ", but it does not occur in the population");
        }
        else {
          //export list of unplaced individuals
          TInd *RemainingInd;
          RemainingInd = new TInd[IndivCount-pass];
          for (I=pass; I<IndivCount; I++)
              RemainingInd[I-pass] = Ind[I];
          //prune all individuals that are not parents (repeat until none left)
          int NumRemaining = IndivCount-pass;
          RemainingInd = PruneRemainingInd(RemainingInd,NumRemaining);
          //export remaining individuals
          AnsiString Errorfile = DataDir+"\\"+DataFileName+"_PedigreeErrors.txt";
          TStringList *Errorlist = new TStringList;
          Errorlist->Add("The following "+IntToStr(NumRemaining)+" individuals could not be sorted;");
          Errorlist->Add("presumably because they are part of one or more circular pedigree references");
          Errorlist->Add("");
          Errorlist->Add("Name\tParent1\tParent2");
          for (I=0; I<NumRemaining; I++) {
            Errorlist->Add(RemainingInd[I].Name+"\t"+RemainingInd[I].Par1+"\t"+RemainingInd[I].Par2);
          }
          Errorlist->SaveToFile(Errorfile);
          delete[] RemainingInd;
          Errorlist->Clear();
          throw Exception("Pedigree cannot be sorted, probably due to circular references"+CRLF+
                "see file "+Errorfile);
        }
        //break; not needed, as already achieved through Exceptions
      }
      if (I>pass)
      { //move all Indivs from pass to I-1 to end:
#ifdef ReportChanges
        Changes=true;
#endif        
        for (j=pass; j<I; j++)
          Ind[IndivCount+j-pass] = Ind[j];
        //move all Indivs from I to new end up to close gap:
        for (j=0; j<IndivCount-pass; j++)
          Ind[pass+j]=Ind[I+j];
      }
    }
    if (ok)
    { //sort Memo in correct order
      Result = new TStringList;
      for (I=0; I<IndivCount; I++)
          Result->Add(L->Strings[Ind[I].LineNr]);
#ifdef ReportChanges
          if (!Changes) ShowMessage("No sorting was needed");
#endif
    }
    else throw Exception("Error sorting pedigree");
  }
  __finally { delete[] Ind; }
  return Result;
#undef ReportChanges
} //SortPedigree  Compiler warning about CommentChar never used incorrect!

void ReadPedigree (TInTextfile *Dat)
//Current line in Dat is first line if pedigree (after "PEDIGREE")
//reads pedigree data and further (phenotypic) datacolumns
{ int p,r1,i,d,e;
  bool ok,allUnknown;
  TCanvas* CV = NULL;
  AnsiString SelfName, Par0Name, Par1Name, woord;
  TIndivDatum* TempID=NULL; //temporary for holding global IndivData
  AnsiString** TextData=NULL; //all data in the form of strings
  TStringList *SortSL; //contains sorted pedigree
  TStringList *L; //contains all non-comment lines of original pedigree
  //bool mv; //missing value

  char* TempC=NULL; int* TempI=NULL; double* TempF=NULL; AnsiString* TempS;
        //one datacolumn of each type
  char** TempIchar; int** TempIint; double** TempIfloat;
  int** TempIlabel; //array of indices into Labels (AnsiStrings) arrays
         //arrays of datacolumns

  //if (CommandlineModeInt==1) {
  //  CV = new TCanvas();
  //} else CV=MainForm->MainListview->Canvas;
  if (CommandlineModeInt!=1) {
    CV=MainForm->MainListview->Canvas;
  }
  //check if there are column names (if so, there can be datafields)
  IndivData=NULL; IndivDataCount=0;
  Dat->P=1; Dat->SkipBlNComm();
  //24-1-2010: read first line into StringList L
  L = new TStringList;
  do {
      SelfName=Dat->ReadWoord();
      if (SelfName == "" || SelfName[1]==CommentChar)
          break;
      L->Add(SelfName);
  } while (true);
  //Check if this is a caption line:
  if (L->Count<3)
    throw Exception("Error in first line of pedigree: "+Dat->Line);
  else {
    SelfName = UpperCase(L->Strings[0]);
    Par0Name = UpperCase(L->Strings[1]);
    Par1Name = UpperCase(L->Strings[2]);
    if (SelfName=="NAME" &&
        ( (Par0Name=="PARENT1" && Par1Name=="PARENT2") ||
          (Par0Name=="MOTHER" && Par1Name=="FATHER") ||
          (Par0Name=="FATHER" && Par1Name=="MOTHER") ||
          (Par0Name=="FEMALE" && Par1Name=="MALE") ||
          (Par0Name=="MALE" && Par1Name=="FEMALE") ) )
    { //caption line found
       if (Par0Name=="MOTHER" || Par0Name=="FEMALE") FemaleParent=0;
       else if (Par1Name=="MOTHER" || Par1Name=="FEMALE") FemaleParent=1;
       else FemaleParent=2;
       //create the trait data fields, if any:
       for (i=3; i<L->Count; i++) {
               TempID=IndivData;
               IndivData = new TIndivDatum [++IndivDataCount];
               IndivData[IndivDataCount-1].Caption=L->Strings[i];
               IndivData[IndivDataCount-1].DataType=dtI; //simplest, temporary
               IndivData[IndivDataCount-1].V.CV.Levels=NULL;
               IndivData[IndivDataCount-1].V.SV.Labels=NULL;
               for (d=0; d<IndivDataCount-1; d++)
                  IndivData[d]=TempID[d];
               delete[] TempID; TempID=NULL;
        }
        Dat->ReadLn(); //to next line
    }
    else { //line doesn't start with NAME etc, no caption line
      if (L->Count==3)
      { //no caption line, use Parent1 and Parent2 as parent captions
        FemaleParent=2;
        //do not go to next line, first line = first data line
      }
      else throw Exception("Error in first line of pedigree: "+Dat->Line);
    }
  }

  //first count individuals and store pedigree lines in L:
  L = new TStringList;
  Dat->P=1;
  do
  { Dat->SkipBlNComm();
    SelfName=Dat->ReadWoord();
    //i++; //r++;
    if ( SelfName!="" && !IsKeyword(SelfName) )
    { L->Add(Dat->Line);
      Dat->ReadLn();
    }
  //} while (SelfName!="" && !IsUnknown(SelfName) &&
  //         UpperCase(SelfName)!="LINKAGEGROUP" && UpperCase(SelfName)!="GROUP");
  } while ( SelfName!="" && !IsKeyword(SelfName) );
  //r--; //points to line where first non-NAME is found
  //Dat->Line is line where first non-NAME is found
  //if (IsKeyword(SelfName)) throw Exception("Error in Individual name in pedigree: "+SelfName);
  if ( SelfName!="" && UpperCase(SelfName)!="LINKAGEGROUP" && UpperCase(SelfName)!="GROUP" )
    throw Exception("Error in Individual name in pedigree: "+SelfName);
  if (L->Count==0) //(i==0)
    throw Exception("No individuals in pedigree");
  IndivCount=L->Count; //i;
  if (IndivCount>20 && IBDAlleleCount>20 && CommandlineModeInt==2)
  { OpenProgressForm->ReadLbl1->Caption = "Pedigree";
    OpenProgressForm->ReadLbl2->Caption = "";
    OpenProgressForm->ProgressBar1->Visible = false;
    OpenProgressForm->ProgressBar1->Min=0;
    OpenProgressForm->ProgressBar1->Max=IndivCount;
    OpenProgressForm->Position = poScreenCenter;
    OpenProgressForm->StopPressed = false;
    MainForm->Enabled=false;
    OpenProgressForm->Show();
  }
  SortSL=SortPedigree(L);
  delete L;
  if (CommandlineModeInt==2) {
  Application->ProcessMessages();
    if ( OpenProgressForm->StopPressed)
       throw Exception("Opening file aborted");
  }
  if (SortSL==NULL) exit;
  //Create the Indiv and TextData arrays:
  Indiv = new TIndiv [IndivCount];
  if (IndivDataCount>0)
  { TextData = new AnsiString* [IndivDataCount];
    for (d=0; d<IndivDataCount; d++)
    { TextData[d] = new AnsiString[IndivCount];
      for (i=0; i<IndivCount; i++) TextData[d][i]="";

    }
  }
  //Read Indivs from SortSL:
  if (CommandlineModeInt==1) {
     IndNameListWidth=80;
  } else {
     IndNameListWidth=CV->TextWidth(strUnknown[0]);
  }
  r1=0; //start of pedigree in SortSL
  for (i=0; i<IndivCount; i++) {
    p=1; SkipBlNComm(r1,p,SortSL);
    ReadWoordQ(p, SelfName, SortSL->Strings[r1]);
    if (CommandlineModeInt==1) {
       IndNameListWidth=80;
    } else {
       IndNameListWidth = max (IndNameListWidth, CV->TextWidth(SelfName));
    }
    ReadWoordQ(p, Par0Name, SortSL->Strings[r1]);
    ReadWoordQ(p, Par1Name, SortSL->Strings[r1]);
    AddIndiv (SelfName, Par0Name, Par1Name);  //may raise an Exception
    //read the trait data as text strings:
    d=0; ok=true;
    while (d<=IndivDataCount && ok) { //also = to test for too many data
        ReadWoordQ(p, woord, SortSL->Strings[r1]);
        if (d<IndivDataCount) {
          ok = woord!="";  //if false, not enough data
          if (ok)
              TextData[d][i]=woord;
        }
        else {
          ok= woord=="" || woord[1]==';';  //if false, too many data
        }
        d++;
    } 
    r1++;
    //test if the correct number of data present:
    if (!ok) {
      if (woord!="") {
          throw Exception("too many data at individual " + Indiv[i].IName);
      }
      else {
          throw Exception("not enough data at individual " + Indiv[i].IName);
      }
    }  
  } //for i

  //are all individual names numbers (integers)?
  try {
    for (i=0; i<IndivCount; i++) {
        Indiv[i].INameNum = StrToInt(Indiv[i].IName);
    }
    INameNumbers=true;
  }
  catch (...) {
    INameNumbers=false;
  }
  
  //determine datatypes
  for (d=0; d<IndivDataCount; d++) {
    //ichar-type or all missing?
    allUnknown=true;
    IndivData[d].Fieldwidth=strUnknown[0].Length(); //max length
    IndivData[d].V.OV.LargInd=-1; //ind with longest string; used in case this will prove to be string data
    for (i=0; i<IndivCount; i++) {
      allUnknown = allUnknown && IsUnknown(TextData[d][i]);
      if (TextData[d][i].Length()>IndivData[d].Fieldwidth) {
        IndivData[d].Fieldwidth=TextData[d][i].Length();
      }
    }
    if (IndivData[d].Fieldwidth==1 || allUnknown) {
      //datatype char, may be all missing values
      TempIchar= new char* [++DataTypeCount[dtC]];
      for (e=0; e<DataTypeCount[dtC]-1; e++) TempIchar[e]=Ichar[e];
      e=DataTypeCount[dtC]-1;
      TempIchar[e] = new char [IndivCount];
      for (i=0; i<IndivCount; i++)
      { if (IsUnknown(TextData[d][i])) TempIchar[e][i]=NULLCHAR;
        else TempIchar[e][i]=TextData[d][i][1];
      }
      delete[] Ichar;
      Ichar=TempIchar; TempIchar=NULL;
      IndivData[d].DataType=dtC;
      IndivData[d].TypeIndex=DataTypeCount[dtC]-1;
      //get number and list of values
      if (allUnknown) {
        IndivData[d].V.CV.LevelCount=0;
        IndivData[d].V.CV.Levels=NULL; //should already be the case
      }
      else {
        i=0;
        while (Ichar[e][i]==NULLCHAR)
          i++; //note that i is always less than IndivCount as allUnknown is false
        IndivData[d].V.CV.LevelCount=1;
        IndivData[d].V.CV.Levels = new char[1];
        IndivData[d].V.CV.Levels[0] = Ichar[e][i];
        i++;
        while (i<IndivCount) {
          if (Ichar[e][i] != NULLCHAR) {
            p=0;
            while ( p<IndivData[d].V.CV.LevelCount &&
                  Ichar[e][i] > IndivData[d].V.CV.Levels[p] )
              p++;
            if ( p==IndivData[d].V.CV.LevelCount ||
               Ichar[e][i] != IndivData[d].V.CV.Levels[p] ) {
              //insert Ichar[e][i] at position p in Levels
              TempC = new char [++IndivData[d].V.CV.LevelCount];
              for (r1=0; r1<p; r1++) TempC[r1]=IndivData[d].V.CV.Levels[r1];
              TempC[p]=Ichar[e][i];
              for (r1=p+1; r1<IndivData[d].V.CV.LevelCount; r1++)
                  TempC[r1]=IndivData[d].V.CV.Levels[r1-1];
              delete[] IndivData[d].V.CV.Levels;
              IndivData[d].V.CV.Levels = TempC; TempC=NULL;
            }
          }
          i++;
        }
      }
    }
    else {
      //not char and not empty, test if int:
      i=0; ok=true;
      TempI = new int [IndivCount];
      while (i<IndivCount && ok)
      { try
        { if (IsUnknown(TextData[d][i])) TempI[i]=-MAXINT;
          else TempI[i]=StrToInt(TextData[d][i]);
        }
        catch (...) { ok=false; delete[] TempI; TempI=NULL; }
        //__except (EXCEPTION_EXECUTE_HANDLER) { ok=false; }
        i++;
      }
      if (ok)
      { //datatype int; new data now in TempI
        TempIint = new int* [++DataTypeCount[dtI]];
        for (e=0; e<DataTypeCount[dtI]-1; e++) TempIint[e]=Iint[e];
        TempIint[DataTypeCount[dtI]-1]=TempI; TempI=NULL;
        delete[] Iint;
        Iint=TempIint; TempIint=NULL;
        IndivData[d].DataType=dtI;
        IndivData[d].TypeIndex=DataTypeCount[dtI]-1;
        //find indiv with min and max values, and Fieldwidth
        //skip all leading missing values:
        i=0; while (i<IndivCount && Iint[e][i]==-MAXINT) i++;
        //ignore the possibility that all non-missing are -MAXINT;
        //find smallest and largest:
        IndivData[d].V.OV.SmallInd=i; IndivData[d].V.OV.LargInd=i;
        i++;
        while (i<IndivCount)
        { if ( Iint[e][i] != -MAXINT )
          { if ( Iint[e][i] < Iint[e][IndivData[d].V.OV.SmallInd] )
               IndivData[d].V.OV.SmallInd = i;
            else if ( Iint[e][i] > Iint[e][IndivData[d].V.OV.LargInd] )
               IndivData[d].V.OV.LargInd = i;
          }
          i++;
        }
        IndivData[d].Fieldwidth= IntToStr(Iint[e][IndivData[d].V.OV.LargInd]).Length();
        if ( IntToStr(Iint[e][IndivData[d].V.OV.SmallInd]).Length() > IndivData[d].Fieldwidth )
           IndivData[d].Fieldwidth=IntToStr(Iint[e][IndivData[d].V.OV.SmallInd]).Length();
        IndivData[d].V.OV.Decimals=0;
      }
      else //not empty, char, int; test if float:
      { i=0; ok=true;
        TempF = new double [IndivCount];
        while (i<IndivCount && ok)
        { try
          { if (IsUnknown(TextData[d][i]))
               TempF[i]=-MAXDOUBLE;
            else TempF[i]=StrToFloatGen(TextData[d][i]);
          }
          catch (...) { ok=false; delete[] TempF; TempF=NULL; }
          i++;
        } // while i
        if (ok)
        { //datatype float; new data now in TempF
          TempIfloat = new double* [++DataTypeCount[dtF]];
          for (e=0; e<DataTypeCount[dtF]-1; e++) TempIfloat[e]=Ifloat[e];
          TempIfloat[DataTypeCount[dtF]-1]=TempF; TempF=NULL;
          delete[] Ifloat;
          Ifloat=TempIfloat; TempIfloat=NULL;
          IndivData[d].DataType=dtF;
          IndivData[d].TypeIndex=DataTypeCount[dtF]-1;
          IndivData[d].V.OV.Decimals=0;
          //find indiv with min and max values, and Fieldwidth
          //skip all leading missing values:
          i=0; while (i<IndivCount && Ifloat[e][i]==-MAXDOUBLE) i++;
          //ignore the possibility that all non-missing are -MAXDOUBLE;
          //find smallest and largest:
          IndivData[d].V.OV.SmallInd=i; IndivData[d].V.OV.LargInd=i;
          i++;
          while (i<IndivCount)
          { if ( Ifloat[e][i] != -MAXDOUBLE )
            //find indiv with min and max values, and Decimals and Fieldwidth
            { if ( Ifloat[e][i] < Ifloat[e][IndivData[d].V.OV.SmallInd] )
                 IndivData[d].V.OV.SmallInd = i;
              else if ( Ifloat[e][i] > Ifloat[e][IndivData[d].V.OV.LargInd] )
                 IndivData[d].V.OV.LargInd = i;
              p=TextData[d][i].Length();
              while ( (p>0) &&
                      (TextData[d][i][p]!=*(".")) &&
                      (TextData[d][i][p]!=*(",")) )
              { p--; }
              if (p>0) //else no decimals separator found
              { p=TextData[d][i].Length()-p; //num. decimals
                if (p>IndivData[d].V.OV.Decimals) IndivData[d].V.OV.Decimals=p;
              }
            }
            i++;
          } //while i
          //calculate Fieldwidth:
          IndivData[d].Fieldwidth =
            FloatToStrF(Ifloat[e][IndivData[d].V.OV.LargInd],
                        ffFixed, 100, IndivData[d].V.OV.Decimals).Length();
          p=FloatToStrF(Ifloat[e][IndivData[d].V.OV.SmallInd],
                        ffFixed, 100, IndivData[d].V.OV.Decimals).Length();
          if (p>IndivData[d].Fieldwidth) IndivData[d].Fieldwidth=p;
        } //if ok
        else //datatype label:
        { //first: get number and list of label values and sort alphabetically
          i=0;
          while (i<IndivCount && IsUnknown(TextData[d][i])) i++;
          if (i>=IndivCount) ShowMessage("Error: only missing labels");
          else
          { IndivData[d].V.SV.LabelCount=1;
            IndivData[d].V.SV.Labels = new AnsiString[1];
            IndivData[d].V.SV.Labels[0] = TextData[d][i];
            IndivData[d].Fieldwidth = IndivData[d].V.SV.Labels[0].Length();
            i++;
            while (i<IndivCount)
            { if (!IsUnknown(TextData[d][i]))
              { //to be replaced by binary search
                p=0;
                while (p<IndivData[d].V.SV.LabelCount &&
                       IndivData[d].V.SV.Labels[p]<TextData[d][i])
                p++;
                if (p==IndivData[d].V.SV.LabelCount ||
                       TextData[d][i]!=IndivData[d].V.SV.Labels[p])
                { //not found; add new label at position p:
                  TempS = new AnsiString[++IndivData[d].V.SV.LabelCount];
                  for (r1=0; r1<p; r1++) TempS[r1]=IndivData[d].V.SV.Labels[r1];
                  TempS[p]=TextData[d][i];
                  for (r1=p+1; r1<IndivData[d].V.SV.LabelCount; r1++)
                    TempS[r1]=IndivData[d].V.SV.Labels[r1-1];
                  delete[] IndivData[d].V.SV.Labels;
                  IndivData[d].V.SV.Labels=TempS;
                  if (IndivData[d].Fieldwidth < TempS[p].Length() )
                    IndivData[d].Fieldwidth = TempS[p].Length();
                }
              }
              i++;
            }
            //all labels read and sorted; assign indices for each indiv:
            TempI = new int[IndivCount];
            for (i=0; i<IndivCount; i++)
            { if (IsUnknown(TextData[d][i])) TempI[i]=-MAXINT;
              else
              { //to be replaced by binary search)
                p=0;
                while (TextData[d][i]>IndivData[d].V.SV.Labels[p]) p++;
                TempI[i]=p;
              }
            }
            TempIlabel = new int* [++DataTypeCount[dtS]];
            for (e=0; e<DataTypeCount[dtS]-1; e++) TempIlabel[e]=Ilabel[e];
            TempIlabel[DataTypeCount[dtS]-1]=TempI; TempI=NULL;
            delete[] Ilabel;
            Ilabel=TempIlabel; TempIlabel=NULL;
            IndivData[d].DataType=dtS;
            IndivData[d].TypeIndex=DataTypeCount[dtS]-1;
          } //only missing labels else
        } //if float else
      } //if int else
    } //if char else
  } //for d
  for (d=0; d<IndivDataCount; d++)
      delete[] TextData[d];
  delete[] TextData;
} //ReadPedigree


void ReadMap(TInTextfile *Dat, int LG)
//r,p point to first woord after line with "MAP"
{ int p,r,i;
  AnsiString Name, woord;
  float CM;
  TStringList *SL; //contains map lines

  //first count loci in LG map:
  //r1=r; i=-1;
  SL = new TStringList;
  //add non-comment, non-blank lines of map to SL:
  Dat->P=1;
  do
  { Dat->SkipBlNComm(); Name=Dat->ReadWoord();
    //i++; r++;
    if (Name!="" && UpperCase(Name)!="LOCUS")
    { SL->Add(Dat->Line);
      Dat->ReadLn();
    }
  } while (Name!="" && UpperCase(Name)!="LOCUS");
  if (SL->Count==0) throw Exception("No loci in map of linkage group "+LinkGrp[LG]->LGName);
  //Create the Locus array:
  LinkGrp[LG]->LocCount=SL->Count;
  LinkGrp[LG]->Locus = new TLocus* [SL->Count];
  for (i=0; i<LinkGrp[LG]->LocCount; i++) LinkGrp[LG]->Locus[i]=NULL;
  //Read Loci names and positions from SL:
  r=0; i=0;
  do
  { p=1; SkipBlNComm(r,p,SL);
    ReadWoordSL(r,p,Name,SL);
    if (UpperCase(LinkGrp[LG]->LGName)=="UNMAPPED") CM=i;
    else
    { ReadWoordSL(r,p,woord,SL);
      try {CM=StrToFloatGen(woord);}
      catch (...) { throw Exception("Error in position of locus "+Name); }
      if (i>0 && CM<LinkGrp[LG]->Locus[i-1]->cM)
      { throw Exception("Loci in map of linkage group "+LinkGrp[LG]->LGName +
              "not in ascending order of position");
      }
    }
    LinkGrp[LG]->Locus [i] = new TLocus (i,LG,Name,CM); //compile warning because i and LG
                                                        //normal int, not short int
    r++; i++;
  } while (i<LinkGrp[LG]->LocCount);
  TotLocCount += LinkGrp[LG]->LocCount;
  delete SL;
} //ReadMap

void ReadAlleleProbs(int p, AnsiString Line, int LG, int ip, int I)
//LG=index to linkage group, ip=index to IBDposition, I=index to Indiv
//slightly faster with parameter AnsiString Line than with const AnsiString &Line
// January 2007: called with p instead of &p to avoid compiler warning;
//doesn't matter as whole line is processed before leaving
{ int a, pl;
  AnsiString woord;
  float Tot;
  TIBDposition *IBDp;

  for (pl=0; pl<Ploidy; pl++)
  { Tot=0.0;
    for (a=0; a<IBDAlleleCount; a++)
    { ReadWoord(p,woord,Line);
      try
      { Tot += AProbs[a] = StrToFloatGen(woord);
         //not slower than StrToFloat(woord);
      }
      catch (...) {throw Exception("Found '"+woord+"' while reading allele probabilities");}
    }
    //ConvertToBase(AProbs, FrBase, IBDAlleleCount, 240);
    //for (a=0; a<IBDAlleleCount; a++)
    //{ LinkGrp[LG]->IBDpos[ip]->AllProb[I][pl][a]= FrBase[a]; }
    //simpler conversion just very slightly better (max 5%):
    //for (a=0; a<IBDAlleleCount; a++)
    //   LinkGrp[LG]->IBDpos[ip]->AllProb[I][pl][a] = AProbs[a]/Tot*240 + 0.5;
    //speed improvement by avoiding repeated array lookups: ~7%
    IBDp=LinkGrp[LG]->IBDpos[ip];
    for (a=0; a<IBDAlleleCount; a++)
       IBDp->AllProb[I][pl][a] = AProbs[a]/Tot*240 + 0.5;
       //rounded values; in DrawIndMF filled out to 100% of width of IBD rectangle
  }
  ReadWoord(p,woord,Line);
  if (woord!="" && woord[1]!=';') throw Exception("Found '"+woord+"' but expected end of line while reading allele probabilities");
} //ReadAlleleProbs

int WoordIndex(AnsiString woord, AnsiString *List, int Count)
//returns -1 if woord not present, else index (0-based)
{ int i;
  i=Count-1;
  while (i>=0 && woord!=List[i]) i--;
  return i;
} //WoordIndex

int AllelenameIndex(const AnsiString &woord, AnsiString &Message, int LG, int loc,
                   bool AlleleNames, bool NullHomAllowed)
//returns the index of woord in LinkGrp[LG]->Locus[loc]->AllName,
//or -1 if StrUnknown, or -2 if strNullHomoz, or -3 if strConfirmedNull.
//Message=="" if no error.
//AlleleNames indicates if a list of allowed allele names was given.
//if AlleleNames: message if name not present, else adds name to end of list
//  Format January 2007: AlleleNames must always be true,
//  code here left unchanged, but call always specifies true
//Also message if woord==strNullHomoz and not NullHomozAllowed,
//or if woord is keyword
{ int a,Result;
  AnsiString *TempS;

  Result=-1; Message="";
  if (IsUnknown(woord)) Result=-1;
  else if (woord==strNullHomoz)
  { if (NullHomAllowed) Result=-2;
    else Message=woord+" is not an allowed allele name";
  }
  else if (woord==strConfirmedNull)
    Result=-3;
  else if (IsKeyword(woord))
    Message=woord+" is not an allowed allele name";
  else
  { a=WoordIndex(woord, LinkGrp[LG]->Locus[loc]->AllName,
                 LinkGrp[LG]->Locus[loc]->AllNameCount);
    if (a==-1)
    { if (AlleleNames)
        Message=woord+" does not occur in ALLELENAMES";
      else
      { //add woord to list of allele names:
        TempS = new AnsiString[++LinkGrp[LG]->Locus[loc]->AllNameCount];
        for (a=0; a<LinkGrp[LG]->Locus[loc]->AllNameCount-1; a++)
            TempS[a]=LinkGrp[LG]->Locus[loc]->AllName[a];
        TempS[LinkGrp[LG]->Locus[loc]->AllNameCount-1]=woord;
        delete[] LinkGrp[LG]->Locus[loc]->AllName;
        LinkGrp[LG]->Locus[loc]->AllName=TempS; TempS=NULL;
        a=LinkGrp[LG]->Locus[loc]->AllNameCount-1;
      }
    }
    Result=a; //real index if added or already present, -1 if not present and AlleleNames
  }
  return Result;
} //AllelenameIndex

void ReadAlleleObs(int p, const AnsiString &Line, int LG, int loc, int I)
// January 2007: new format (and only used if no binary file)
// only change: Allelenames must now be specified before reading alleles
//reads only the current line; may throw an exception
//p is position in Line past Individual name
{ int a, pl;
  AnsiString woord,Message;

  //check how many data on line:
  //if Ploidy, then no color codes are present,
  //else if 2*Ploidy then every allele is followed by a color code
  //else error
  a=0; pl=p;
  do
  { ReadWoordQ(p,woord,Line);
    if (woord=="" || woord[1]==';') break;
         //this will cause errors if quoted allele names start with ';'
         //just leave for the moment
    a++;
  } while (true);
  p=pl; //back to startpoint

  if (a!=Ploidy && a!=2*Ploidy)
  { throw Exception("Error reading ALLELES of individual "+
       IntToStr(I) + ": " + Indiv[I].IName + " in locus " +
       LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
       LinkGrp[LG]->LGName + ":\n" +
       "incorrect number of data");
  }
  if (a==Ploidy)
  { //read alleles without color codes
    for (pl=0; pl<Ploidy; pl++)
    { ReadWoordQ(p,woord,Line);
      a=AllelenameIndex(woord,Message,LG,loc,true,true);
      if (Message!="")
      { throw Exception("Error reading allele observations of individual "+
           IntToStr(I) + ": " + Indiv[I].IName + " in locus " +
           LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
           LinkGrp[LG]->LGName + ":\n" + Message);
      }
      LinkGrp[LG]->Locus[loc]->AllIx[I][pl] = a;
    }
  }
  else //a==2*Ploidy)
  { //read alleles with color codes
    for (pl=0; pl<Ploidy; pl++)
    { ReadWoordQ(p,woord,Line);
      a=AllelenameIndex(woord,Message,LG,loc,true,true);
      if (Message!="")
      { throw Exception("Error reading allele observations of individual "+
           IntToStr(I) + ": " + Indiv[I].IName + " in locus " +
           LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
           LinkGrp[LG]->LGName + ":\n" + Message);
      }
      LinkGrp[LG]->Locus[loc]->AllIx[I][pl] = a;
      //next read color code:
      ReadWoord(p,woord,Line); //color code: number, unquoted
      try { a=StrToInt(woord); }
      catch (...)
      { throw Exception("Error reading allele observations of individual "+
           IntToStr(I) + ": " + Indiv[I].IName + " in locus " +
           LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
           LinkGrp[LG]->LGName + ":\n" +
           "'"+woord+"' is not a valid color code");
      }
      if (a<0 || a>254)
      { throw Exception("Error reading allele observations of individual "+
           IntToStr(I) + ": " + Indiv[I].IName + " in locus " +
           LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
           LinkGrp[LG]->LGName + ":\n" +
           "'"+woord+"' is not a valid color code");
      }
      LinkGrp[LG]->Locus[loc]->AllCode[I][pl]=a; //i always in byte range, ignore compiler warning
      if (MarkCodeCount<=a) MarkCodeCount=a+1; //Count=max.code +1, includes unused codes
    }
  } //a==2*Ploidy
} //ReadAlleleObs

/*
void ReadLocus_OldFormat(TInTextfile *Dat, int LG, int loc)
{ int a,i;
  AnsiString woord, Uwoord, *TempS, Line;
  bool AlleleNames, //present in THIS locus
       AlleleObs,   //present in THIS locus (AlleleObsPresent: in ALL loci)
       *IndRead; //array: has this individual already been read?

  // search for next "LOCUS":
  Dat->P=1; Dat->SkipBlNComm(); woord=Dat->ReadWoord();
  if (UpperCase(woord)!="LOCUS")
  { if (woord=="") woord="end of file";
    else woord="'"+woord+"'";
    throw Exception("'LOCUS' expected but "+woord+" found when expecting locus " +
       LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
       LinkGrp[LG]->LGName);
  }
  //verify if expected locus found:
  woord=Dat->ReadWoord();
  if (woord!="" && woord!=LinkGrp[LG]->Locus[loc]->LocName)
  { throw Exception("Found locus name '" + woord +
       "' when expecting locus name '" +
       LinkGrp[LG]->Locus[loc]->LocName + "' in linkage group " +
       LinkGrp[LG]->LGName);
  }

  OpenProgressForm->ReadLbl2->Caption = "Locus "+LinkGrp[LG]->Locus[loc]->LocName;
  OpenProgressForm->ProgressBar1->Position=0;
  OpenProgressForm->ProgressBar1->Visible=true;
  Application->ProcessMessages();

  Dat->ReadLn();
  Dat->SkipBlNComm(); woord=Dat->ReadWoord();
  AlleleObs=AlleleNames=false;
  if (UpperCase(woord)=="ALLELENAMES")
  { AlleleNames=true; //Line=L->Strings[r];
    do {woord=Dat->ReadWoordQ();
        if (woord=="" || woord.SubString(1,1)==";")
           //will cause error if quoted allelename starts with ';',
           //leave for the moment
          break;
        if (IsKeyword(woord))
          { throw Exception("Error reading ALLELENAMES in locus " +
                 LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
                 LinkGrp[LG]->LGName + ":  "+ woord + " is not an allowed allele name");
          }
        if (WoordIndex(woord, LinkGrp[LG]->Locus[loc]->AllName,
                               LinkGrp[LG]->Locus[loc]->AllNameCount) == -1)
        { TempS = new AnsiString[++LinkGrp[LG]->Locus[loc]->AllNameCount];
          for (a=0; a<LinkGrp[LG]->Locus[loc]->AllNameCount-1; a++)
          { TempS[a]=LinkGrp[LG]->Locus[loc]->AllName[a]; }
          TempS[LinkGrp[LG]->Locus[loc]->AllNameCount-1]=woord;
          delete[] LinkGrp[LG]->Locus[loc]->AllName;
          LinkGrp[LG]->Locus[loc]->AllName=TempS; TempS=NULL;
        }
    } while (true);
    //AllNameCountIsIBDAllCount &= LinkGrp[LG]->Locus[loc]->AllNameCount == IBDAlleleCount;
    //r++; p=1;
    Dat->ReadLn();
    Dat->SkipBlNComm(); woord=Dat->ReadWoord();
  }

  if (UpperCase(woord)=="FOUNDERALLELES")
  { if (ParsingFirstLocus) FounderAllelesPresent=true;
    else if (!FounderAllelesPresent)
      throw Exception("FOUNDERALLELES should be present at all loci or at none");
    //Line=L->Strings[r];
    LinkGrp[LG]->Locus[loc]->FounderAllele = new signed char [IBDAlleleCount];
    i=0;
    do {woord=Dat->ReadWoordQ();
        if (woord=="" || woord.SubString(1,1)==";")
           //will cause error if quoted allelename starts with ';',
           //leave for the moment
          break;
        a=AllelenameIndex(woord,Uwoord,LG,loc,AlleleNames,false);
        if (Uwoord!="")
        { throw Exception("Error reading FOUNDERALLELES in locus " +
                 LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
                 LinkGrp[LG]->LGName + ":  "+ Uwoord);
        }
        LinkGrp[LG]->Locus[loc]->FounderAllele[i]=a;
        i++;
    } while (i<IBDAlleleCount);
    if (i<IBDAlleleCount)
      throw Exception("Error reading FOUNDERALLELES in locus " +
               LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
               LinkGrp[LG]->LGName + ": only " + IntToStr(i)+ " names read, but " +
               IntToStr(IBDAlleleCount) + " expected");
    //test if more names present than allowed:
    woord=Dat->ReadWoord();
    if (woord!="" && woord[1]!=';')
       throw Exception("Error reading FOUNDERALLELES in locus " +
               LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
               LinkGrp[LG]->LGName + ": more than " +
               IntToStr(IBDAlleleCount) + " alleles found");
    //r++; p=1;
    Dat->ReadLn();
    Dat->SkipBlNComm(); woord=Dat->ReadWoord();
  }

  if (UpperCase(woord)=="ALLELES" || UpperCase(woord)=="ALLELEOBS")
  { AlleleObs=true;
    // AlleleObsPresent doesn't exist any more
    //if (ParsingFirstLocus) AlleleObsPresent=true;
    //else if (!AlleleObsPresent) throw Exception("ALLELES should be present at all loci or at none");
    //
    //at least marker code color MarkCodeCol[0] must be created; therefore:
    if (MarkCodeCount<1) MarkCodeCount=1; //
    //read observed scores:
    //doesn't exist any more: LinkGrp[LG]->Locus[loc]->CreateObsArrays(); //filled with missing values: -1
    IndRead = new bool[IndivCount];
    for (i=0; i<IndivCount; i++) IndRead[i]=false;
    try
    { do //loop: read one line of observations; stop when keyword or end found
      { //r++; p=1;
        Dat->ReadLn(); Dat->SkipBlNComm();
        //read first word on line (should be individual name) and check:
        woord=Dat->ReadWoord();
        if ( woord=="" || IsKeyword(woord) )
          break;
        i=0; while (i<IndivCount && Indiv[i].IName!=woord) i++;
        if (i>=IndivCount)
        { throw Exception("Error reading allele observations of locus " +
             LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
             LinkGrp[LG]->LGName + ":\n" +
             "individual name "+woord+" unknown");
        }
        if (IndRead[i])
        { throw Exception("Error reading allele observations of locus " +
             LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
             LinkGrp[LG]->LGName + ":\n" +
             "individual "+woord+" occurs more than once");
        }
        OpenProgressForm->ProgressBar1->StepIt();
        Application->ProcessMessages();
        if (OpenProgressForm->StopPressed)
           throw Exception("Opening file aborted");
        //read observed allels from rest of line:
        ReadAlleleObs(Dat->P,Dat->Line,LG,loc,i); //reads line from p, may throw exception
        IndRead[i]=true;
      } while (true);
    }
    __finally { delete[] IndRead; IndRead=NULL; }
  } //if ALLELES

  if (!ParsingFirstLocus && FounderAllelesPresent &&
      LinkGrp[LG]->Locus[loc]->FounderAllele==NULL)
     throw Exception("FOUNDERALLELES should be present in all loci or in none");
  // AlleleObsPresent doesn't exist any more
  //if (!ParsingFirstLocus && AlleleObsPresent && !AlleleObs)
  //   throw Exception("ALLELES should be present in all loci or in none");
  //
  Dat->P=1;
  //ends at start of line after AlleleNames or Alleles
  //ends with first word on first non-empty line after AlleleNames or Alleles in woord
} //ReadLocus_OldFormat
*/

void ReadLocus(TInTextfile *Dat, int LG, int loc)
// January 2007, new format
{ int a,i;
  AnsiString woord, Uwoord, *TempS, Line;
  bool AlleleNames; //present in THIS locus
       //AlleleObs;   //present in THIS locus (AlleleObsPresent: in ALL loci)

  // search for next "LOCUS":
  Dat->P=1; Dat->SkipBlNComm(); woord=Dat->ReadWoord();
  if (UpperCase(woord)!="LOCUS")
  { if (woord=="") woord="end of file";
    else woord="'"+woord+"'";
    throw Exception("'LOCUS' expected but "+woord+" found when expecting locus " +
       LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
       LinkGrp[LG]->LGName);
  }
  //verify if expected locus found:
  woord=Dat->ReadWoord();
  if (woord!="" && woord!=LinkGrp[LG]->Locus[loc]->LocName)
  { throw Exception("Found locus name '" + woord +
       "' when expecting locus name '" +
       LinkGrp[LG]->Locus[loc]->LocName + "' in linkage group " +
       LinkGrp[LG]->LGName);
  }

  Dat->ReadLn();
  Dat->SkipBlNComm(); woord=Dat->ReadWoord();
  AlleleNames=false;
  if (UpperCase(woord)!="ALLELENAMES")
    throw Exception("ALLELENAMES not found in locus " +
                 LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
                 LinkGrp[LG]->LGName);
  do  //read allowed allelenames from rest of line
  { woord=Dat->ReadWoordQ();
    if (woord=="" || woord.SubString(1,1)==";")
       //will cause error if quoted allelename starts with ';',
       //leave for the moment
      break;
    if (UpperCase(woord) != UpperCase(strConfirmedNull)) {
      //for compatibility with older versions the strConfirmedNull is tolerated
      //among the allele names, but we don't store it as allele name
      if (IsKeyword(woord))
        { throw Exception("Error reading ALLELENAMES in locus " +
               LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
               LinkGrp[LG]->LGName + ":  "+ woord + " is not an allowed allele name");
        }
      if (WoordIndex(woord, LinkGrp[LG]->Locus[loc]->AllName,
                             LinkGrp[LG]->Locus[loc]->AllNameCount) == -1)
                // double names are ignored
      { TempS = new AnsiString[++LinkGrp[LG]->Locus[loc]->AllNameCount];
        for (a=0; a<LinkGrp[LG]->Locus[loc]->AllNameCount-1; a++)
        { TempS[a]=LinkGrp[LG]->Locus[loc]->AllName[a]; }
        TempS[LinkGrp[LG]->Locus[loc]->AllNameCount-1]=woord;
        delete[] LinkGrp[LG]->Locus[loc]->AllName;
        LinkGrp[LG]->Locus[loc]->AllName=TempS; TempS=NULL;
      }
    }
  } while (true);
  if (LinkGrp[LG]->Locus[loc]->AllNameCount > 128)
    throw Exception("Too many (>128) ALLELENAMES in locus " +
             LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
             LinkGrp[LG]->LGName);
  Dat->ReadLn();
  Dat->SkipBlNComm(); woord=Dat->ReadWoord();

  if (UpperCase(woord)=="FOUNDERALLELES")
  { if (ParsingFirstLocus) FounderAllelesPresent=true;
    else if (!FounderAllelesPresent)
      throw Exception("FOUNDERALLELES should be present at all loci or at none");
    LinkGrp[LG]->Locus[loc]->FounderAllele = new signed char [IBDAlleleCount];
    i=0;
    do
    { woord=Dat->ReadWoordQ();
      if (woord=="" || woord.SubString(1,1)==";")
         //will cause error if quoted allelename starts with ';',
         //leave for the moment
        break;
      a=AllelenameIndex(woord,Uwoord,LG,loc,AlleleNames,false);
      if (Uwoord!="")  //error message from AllelenameIndex
      { throw Exception("Error reading FOUNDERALLELES in locus " +
               LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
               LinkGrp[LG]->LGName + ":  "+ Uwoord);
      }
      LinkGrp[LG]->Locus[loc]->FounderAllele[i]=a;
      i++;
    } while (i<IBDAlleleCount);
    if (i<IBDAlleleCount)
      throw Exception("Error reading FOUNDERALLELES in locus " +
               LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
               LinkGrp[LG]->LGName + ": only " + IntToStr(i)+ " names read, but " +
               IntToStr(IBDAlleleCount) + " expected");
    //test if more names present than allowed:
    woord=Dat->ReadWoord();
    if (woord!="" && woord[1]!=';')
       throw Exception("Error reading FOUNDERALLELES in locus " +
               LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
               LinkGrp[LG]->LGName + ": more than " +
               IntToStr(IBDAlleleCount) + " alleles found");
    Dat->ReadLn();
  }
  else
  { //FOUNDERALLELES not found
    if (!ParsingFirstLocus && FounderAllelesPresent)
       throw Exception("FOUNDERALLELES should be present in all loci or in none");
  }
} //ReadLocus

void ReadAlleles(TInTextfile *Dat, int LG, int loc)
// January 2007, new format; previously part of ReadLocus
// used only if no binary file available
{ int i;
  AnsiString woord, Uwoord, Line; 
  bool *IndRead; //array: has this individual already been read?

  // search for next "ALLELES":
  Dat->P=1; Dat->SkipBlNComm(); woord=Dat->ReadWoord();
  if (UpperCase(woord)!="ALLELES")
    throw Exception("Looking for 'ALLELES "+LinkGrp[LG]->Locus[loc]->LocName+"'\n"+
                    "but found: '"+Dat->Line+"'");
  woord=Dat->ReadWoord();
  if (woord!=LinkGrp[LG]->Locus[loc]->LocName)
    throw Exception("Looking for 'ALLELES "+LinkGrp[LG]->Locus[loc]->LocName+"'\n"+
                    "but found: '"+Dat->Line+"'");
  //at least marker code color MarkCodeCol[0] must be created; therefore:
  if (MarkCodeCount<1) MarkCodeCount=1; //
  //read observed scores:
  if (CommandlineModeInt==2) {
    OpenProgressForm->ProgressBar1->Position=0;
    OpenProgressForm->ProgressBar1->Visible=true;
  }
  IndRead = new bool[IndivCount];
  for (i=0; i<IndivCount; i++) IndRead[i]=false;
  try
  { do //loop: read one line of observations; stop when keyword or end found
    { Dat->ReadLn(); Dat->SkipBlNComm();
      //read first word on line (should be individual name) and check:
      woord=Dat->ReadWoordQ();
      if ( woord=="" || IsKeyword(woord) )
        break;
      i=0; while (i<IndivCount && Indiv[i].IName!=woord) i++;
      if (i>=IndivCount)
      { throw Exception("Error reading ALLELES of locus " +
           LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
           LinkGrp[LG]->LGName + ":\n" +
             "individual name "+woord+" unknown");
      }
      if (IndRead[i])
      { throw Exception("Error reading ALLELES of locus " +
           LinkGrp[LG]->Locus[loc]->LocName + " in linkage group " +
           LinkGrp[LG]->LGName + ":\n" +
           "individual "+woord+" occurs more than once");
      }
      if (CommandlineModeInt==2) {
           OpenProgressForm->ProgressBar1->StepIt();
           Application->ProcessMessages();
           if (OpenProgressForm->StopPressed)
              throw Exception("Opening file aborted");
      }        
      //read observed alleles from rest of line:
      ReadAlleleObs(Dat->P,Dat->Line,LG,loc,i); //reads line from p, may throw exception
      IndRead[i]=true;
    } while (true);
  } //try
  __finally { delete[] IndRead; IndRead=NULL; }

  Dat->P=1;
  //ends at start of line after current ALLELES section
} //ReadAlleles

/*
void ReadIBDpos_OldFormat(TInTextfile *Dat, int LG, int ip, TBinaryfile *BF, int BFmode)
//LG=index to linkage group, ip to IBDposition
{ int a,i;
  AnsiString woord, Uwoord;
  float f;
  bool *IndRead; //array: has this individual already been read?
  TIBDposition **TempIBD;

    //called with woord="IBDPOSITION"
    woord=Dat->ReadWoord();
    f=StrToFloatGen(woord);
    if (ip>0 && LinkGrp[LG]->IBDpos[ip-1]->cM>=f)
       throw Exception("IBD positions of linkage group "+LinkGrp[LG]->LGName+
                        " not in ascending order");

    OpenProgressForm->ReadLbl2->Caption = "IBD Position "+woord;
    OpenProgressForm->ProgressBar1->Position=0;
    OpenProgressForm->ProgressBar1->Visible=true;
    Application->ProcessMessages();

    TempIBD = new TIBDposition* [++LinkGrp[LG]->IBDCount];
    for (a=0; a<LinkGrp[LG]->IBDCount-1; a++)
        TempIBD[a]=LinkGrp[LG]->IBDpos[a];
    TempIBD[LinkGrp[LG]->IBDCount-1] = new TIBDposition(LG,f); //compiler warning because LG is short int
    delete[] LinkGrp[LG]->IBDpos;
    LinkGrp[LG]->IBDpos = TempIBD; TempIBD=NULL;

    if (IBDAlleleCount<=0)
    { throw Exception("'(IBD)Position' found but not expected when reading linkage group " +
         LinkGrp[LG]->LGName);
    }
    //individuals may be entered in any sequence, missing individuals not allowed:
    IndRead = new bool[IndivCount];
    for (i=0; i<IndivCount; i++)
      IndRead[i]=false;
    try
    { do //loop: read one line of IBDprobs; stop when keyword or end found
      { Dat->ReadLn(); Dat->SkipBlNComm();
        woord=Dat->ReadWoord(); Uwoord=UpperCase(woord);
        if ( woord=="" || IsKeyword(woord) ) //individual names are never keywords
             //Uwoord=="LOCUS" ||
             //Uwoord=="POSITION" ||
             //Uwoord=="IBDPOSITION" ||
             //Uwoord=="GROUP" ||
             //Uwoord=="LINKAGEGROUP" )
          break;
        i=0; while (i<IndivCount && Indiv[i].IName!=woord) i++;
        if (i>=IndivCount)
        { throw Exception("Error reading IBD probabilities of position " +
             FloatToStrF(LinkGrp[LG]->IBDpos[ip]->cM,ffGeneral,4,0)+ " in linkage group " +
             LinkGrp[LG]->LGName + ":\n" +
             "individual name "+woord+" unknown");
        }
        if (IndRead[i])
        { throw Exception("Error reading IBD probabilities of position " +
             FloatToStrF(LinkGrp[LG]->IBDpos[ip]->cM,ffGeneral,4,0)+ " in linkage group " +
             LinkGrp[LG]->LGName + ":\n" +
             "individual "+woord+" occurs more than once");
        }
        // now i is the valif Indiv for which we will read the IBDs
        OpenProgressForm->ProgressBar1->StepIt();
        Application->ProcessMessages();
        if ( OpenProgressForm->StopPressed)
           throw Exception("Opening file aborted");
        //temporary solution, while IBDprobs still between other info in file:
        if (BFmode!=BFread)
        { //read IBD from datafile line
          ReadAlleleProbs(Dat->P,Dat->Line,LG,ip,i); //ends at line r, with p at end
        }
        IndRead[i]=true;
      } while (true);
      //all individuals read?
      a=0;
      for (i=0; i<IndivCount; i++)
        if (IndRead[i]) a++;
      if ( a!=IndivCount )
      { throw Exception("Error reading IBD probabilities of position " +
             FloatToStrF(LinkGrp[LG]->IBDpos[ip]->cM,ffGeneral,4,0)+ " in linkage group " +
             LinkGrp[LG]->LGName + ":\n" +
             IntToStr(a)+" individuals read but "+IntToStr(IndivCount)+" expected");
      }

      //read binary file:
      if (BFmode==BFread)
      { BF->seekg(16+   //header: 16 bytes
                  (LinkGrp[LG]->StartIBDNr+ip)*IndivCount*Ploidy*IBDAlleleCount); //start current IBD
        BF->read(LinkGrp[LG]->IBDpos[ip]->IndArray(0), IndivCount*Ploidy*IBDAlleleCount);
      }

      //write binary file:
      if (BFmode==BFwrite)
      { for (i=0; i<IndivCount; i++)
          BF->write(LinkGrp[LG]->IBDpos[ip]->IndArray(i), Ploidy*IBDAlleleCount);
      }
    }
    __finally { delete[] IndRead; IndRead=NULL; }
} //ReadIBDpos_OldFormat
*/

void ReadIBDpos(TInTextfile *Dat, int LG, int ip)
// January 2007 new format
//LG=index to linkage group, ip to IBDposition
{ int a,i;
  AnsiString woord, Uwoord;
  float f;
  bool *IndRead; //array: has this individual already been read?
  //TIBDposition **TempIBD;

  // search for next "IBDPOSITION":
  Dat->P=1; Dat->SkipBlNComm(); woord=Dat->ReadWoord();
  if (UpperCase(woord)!="IBDPOSITION")
    throw Exception("Looking for 'IBDPOSITION "+
                    FloatToStrF(LinkGrp[LG]->IBDpos[ip]->cM,ffFixed,8,3)+
                    "'\nbut found: '"+Dat->Line+"'");
  woord=Dat->ReadWoord();
  f=StrToFloatGen(woord);
  if (LinkGrp[LG]->IBDpos[ip]->cM!=f)
    throw Exception("Looking for 'IBDPOSITION "+
                    FloatToStrF(LinkGrp[LG]->IBDpos[ip]->cM,ffFixed,8,3)+
                    "'\nbut found: '"+Dat->Line+"'");
  if (CommandlineModeInt==2) {
    OpenProgressForm->ProgressBar1->Position=0;
    OpenProgressForm->ProgressBar1->Visible=true;
    Application->ProcessMessages();
  }
  //individuals may be entered in any sequence, missing individuals not allowed:
  IndRead = new bool[IndivCount];
  for (i=0; i<IndivCount; i++)
    IndRead[i]=false;
  try
  { do //loop: read one line of IBDprobs; stop when keyword or end found
    { Dat->ReadLn(); Dat->SkipBlNComm();
      woord=Dat->ReadWoordQ(); Uwoord=UpperCase(woord);
      if ( woord=="" || IsKeyword(woord) ) //individual names are never keywords
        break;
      i=0; while (i<IndivCount && Indiv[i].IName!=woord) i++;
      if (i>=IndivCount)
      { throw Exception("Error reading IBD probabilities of position " +
           FloatToStrF(LinkGrp[LG]->IBDpos[ip]->cM,ffGeneral,4,0)+ " in linkage group " +
           LinkGrp[LG]->LGName + ":\n" +
           "individual name "+woord+" unknown");
      }
      if (IndRead[i])
      { throw Exception("Error reading IBD probabilities of position " +
           FloatToStrF(LinkGrp[LG]->IBDpos[ip]->cM,ffGeneral,4,0)+ " in linkage group " +
           LinkGrp[LG]->LGName + ":\n" +
           "individual "+woord+" occurs more than once");
      }
      // now i is the valid Indiv for which we will read the IBDs
      if (CommandlineModeInt==2) {
            OpenProgressForm->ProgressBar1->StepIt();
            Application->ProcessMessages();
            if ( OpenProgressForm->StopPressed)
               throw Exception("Opening file aborted");
      }         
      ReadAlleleProbs(Dat->P,Dat->Line,LG,ip,i);
      IndRead[i]=true;
    } while (true);
    //all individuals read?
    a=0;
    for (i=0; i<IndivCount; i++)
      if (IndRead[i]) a++;
    if ( a!=IndivCount )
    { throw Exception("Error reading IBD probabilities of position " +
           FloatToStrF(LinkGrp[LG]->IBDpos[ip]->cM,ffGeneral,4,0)+ " in linkage group " +
           LinkGrp[LG]->LGName + ":\n" +
           IntToStr(a)+" individuals read but "+IntToStr(IndivCount)+" expected");
    }
  }
  __finally { delete[] IndRead; IndRead=NULL; }
} //ReadIBDpos

void InsertIBDatLoci(int LG)
//add an IBDpos for each locus position if not present within one cM
//(interpolate the probabilities)
//and set the Locus->IBDix to point to the corresponding IBDpos
{ int loc,ip,i,pl,a;//,i1,i2,loc1,loc2;
  float lpos,ipos,jpos,posprecision;
  TLinkGrp *LiGr;
  TLocus *Locus;
  TIBDposition **TempIBD;
  float *Prob;
  short int *BasePr;

  LiGr=LinkGrp[LG];
  if (LiGr->LocCount<=0 || LiGr->IBDCount<=0 || IBDAlleleCount<=0) return;

  Prob = new float[IBDAlleleCount];
  BasePr = new short int [IBDAlleleCount];

  //calculate posprecision = 1% of LinkGrp length:
  ipos=LiGr->Locus[0]->cM;
  if (LiGr->IBDpos[0]->cM<ipos) ipos=LiGr->IBDpos[0]->cM;
  jpos=LiGr->Locus[LiGr->LocCount-1]->cM;
  if (LiGr->IBDpos[LiGr->IBDCount-1]->cM>jpos) jpos=LiGr->IBDpos[LiGr->IBDCount-1]->cM;
  posprecision=(jpos-ipos)/100.0; if (posprecision<=0.0) posprecision=1.0;

  ip=0; loc=0;

  //first: if the first locus is before the first IBD position,
  //add an IBD position at the position of the first locus:
  Locus=LiGr->Locus[0];
  if ( Locus->cM < LiGr->IBDpos[0]->cM )
  { //add IBDposition at position of first locus, with same probabilities
    //as former first IBDposition:
    TempIBD = new TIBDposition* [++LiGr->IBDCount];
    for (i=0; i<LiGr->IBDCount-1; i++)
        TempIBD[i+1]=LiGr->IBDpos[i];
    TempIBD[0] = new TIBDposition(LG,Locus->cM); //compiler warning because LG is short int
    delete[] LiGr->IBDpos;
    LiGr->IBDpos=TempIBD; TempIBD=NULL;
    //fill IBDprobabilities at position [0] from previous first IBD position:
    for (i=0; i<IndivCount; i++)
      for (pl=0; pl<Ploidy; pl++)
        for (a=0; a<IBDAlleleCount; a++)
          LiGr->IBDpos[0]->AllProb[i][pl][a] =
            LiGr->IBDpos[1]->AllProb[i][pl][a];
    //let all loci before old first IBD position point to new first:
    while ( loc<LiGr->LocCount &&
            LiGr->Locus[loc]->cM < LiGr->IBDpos[1]->cM )
    { LiGr->Locus[loc]->IBDix=0;
      loc++;
    }
  }

  //next: find or calculate IBDpositions within one cM of locus positions:
  while ( loc < LiGr->LocCount )
  { Locus=LiGr->Locus[loc]; lpos=Locus->cM;
    while( ip < LiGr->IBDCount &&
           LiGr->IBDpos[ip]->cM < lpos )
      ip++;
    if ( ip < LiGr->IBDCount )
    { //IBDpos[ip] at or first after locus position
      jpos = LiGr->IBDpos[ip]->cM;
      if ( jpos == lpos )
        Locus->IBDix=ip; //at locus position, no need for new IBDpos
                         //compiler warning because IBDix is short int
      else //Locus between ip-1 and ip
      { ipos = LiGr->IBDpos[ip-1]->cM;
        //see if ipos or jpos within posprecision: then don't add new IBDpos
        if ( jpos-lpos < lpos-ipos )
        { if ( jpos-lpos < posprecision ) Locus->IBDix=ip; } //compiler warning because IBDix is short int
        else
        { if ( lpos-ipos < posprecision ) Locus->IBDix=ip-1; } //compiler warning because IBDix is short int
        if (Locus->IBDix==-1)
        { //create new IBDpos at locus position by interpolation:
          TempIBD = new TIBDposition* [++LiGr->IBDCount];
          for (i=0; i<ip; i++)
              TempIBD[i]=LiGr->IBDpos[i];
          for (i=ip+1; i<LiGr->IBDCount; i++)
              TempIBD[i]=LiGr->IBDpos[i-1];
          TempIBD[ip] = new TIBDposition(LG,lpos); //compiler warning because LG is short int
          Locus->IBDix = ip; //compiler warning because IBDix is short int
          //fill IBDprobabilities from flanking IBD positions:
          for (i=0; i<IndivCount; i++)
            for (pl=0; pl<Ploidy; pl++)     //gaat fout bij ip=14 en i=55 of 56
            { for (a=0; a<IBDAlleleCount; a++)
                Prob[a] = (TempIBD[ip-1]->AllProb[i][pl][a] +
                          (lpos*1.0-ipos)/(jpos-ipos) *
                          (TempIBD[ip+1]->AllProb[i][pl][a] - TempIBD[ip-1]->AllProb[i][pl][a]))
                          / 240.0; //all alleleprob as 0.0-1.0 instead of 0-240
              ConvertToBase(Prob,BasePr,IBDAlleleCount,240);
              for (a=0; a<IBDAlleleCount; a++)
                TempIBD[ip]->AllProb[i][pl][a] = BasePr[a]; //converts short int to signed char
            }
          delete[] LiGr->IBDpos;
          LiGr->IBDpos=TempIBD; TempIBD=NULL;
        }
      }
      loc++;
    }
    else
    { //last IBDpos before Locus:
      //add one IBDpos for last locus position, refer all loci from loc to last
      //to this IBDpos
      TempIBD = new TIBDposition* [++LiGr->IBDCount];
      for (i=0; i<LiGr->IBDCount-1; i++)
          TempIBD[i]=LiGr->IBDpos[i];
      TempIBD[LiGr->IBDCount-1] = new TIBDposition(LG,LiGr->Locus[LiGr->LocCount-1]->cM);
                                  //compiler warning because LG is short int
      //fill IBDprobabilities from previous IBD position:
      ip=LiGr->IBDCount-2; //previous last
      for (i=0; i<IndivCount; i++)
        for (pl=0; pl<Ploidy; pl++)
          for (a=0; a<IBDAlleleCount; a++)
            TempIBD[LiGr->IBDCount-1]->AllProb[i][pl][a] =
              TempIBD[ip]->AllProb[i][pl][a];
      delete[] LiGr->IBDpos;
      LiGr->IBDpos=TempIBD; TempIBD=NULL;

      for (loc=loc; loc<LiGr->LocCount; loc++)
          LiGr->Locus[loc]->IBDix=LiGr->IBDCount-1; //compiler warning because IBDix is short int
      loc=LiGr->LocCount; //breaks loop
    }
  } //while loc
  delete[] Prob; delete[] BasePr;
} //InsertIBDatLoci

/*
void ReadLinkageGroup_OldFormat(TInTextfile *Dat, TBinaryfile *BF, int BFmode)
//"LINKAGEGROUP" or "GROUP" just read when called
{ int LG,loc,ip;
  AnsiString woord,uwoord;

  woord=Dat->ReadWoord(); //linkage group name
  LG=AddLinkGrp(woord);
  OpenProgressForm->ReadLbl1->Caption = "Linkage group "+LinkGrp[LG]->LGName;
  Dat->ReadLn(); Dat->SkipBlNComm();
  woord=Dat->ReadWoord();
  if (UpperCase(woord)!="MAP")
  { throw Exception("'MAP' not found in linkage group "+LinkGrp[LG]->LGName); }
  //r++; p=1;
  Dat->ReadLn(); Dat->SkipBlNComm();
  ReadMap(Dat,LG); //ends with r on line below last locus in map

  loc=0;
  while (loc<LinkGrp[LG]->LocCount)
  { ReadLocus_OldFormat(Dat,LG,loc);
    ParsingFirstLocus=false;
    if (OpenProgressForm->StopPressed)
       throw Exception("Opening file aborted");
    loc++;
  }

  woord=Dat->ReadWoord();
  ip=0;
  while (UpperCase(woord)=="POSITION" || UpperCase(woord)=="IBDPOSITION")
  { ReadIBDpos_OldFormat(Dat,LG,ip,BF,BFmode);
    if (OpenProgressForm->StopPressed)
       throw Exception("Opening file aborted");
    Dat->P=1; woord=Dat->ReadWoord();
    ip++; TotIBDCount++;
  }

  //check if no IBD probs were found and if that is acceptable:
  if (IBDAlleleCount>0 && LinkGrp[LG]->IBDCount==0)
     throw Exception("No IBD probabilities found in linkage group "+LinkGrp[LG]->LGName+
                     " but NALLELES>0");

  //insert IBD probabilities at locus positions:
  //  (set all Locus->IBDix to point to the corresponding IBDpos;
  //   if necessary add IBDpos at locus positions)
  if ( LinkGrp[LG]->LocCount>0 &&
       LinkGrp[LG]->IBDCount>0 &&
       IBDAlleleCount>0 )
    InsertIBDatLoci(LG);

  Dat->P=1; //ends with start of line with first word after POSITIONS, with this word in woord
} //ReadLinkageGroup_OldFormat
*/

void ReadLinkageGroup(TInTextfile *Dat)
// new format January 2007
//"LINKAGEGROUP" or "GROUP" just read when called
{ int LG,loc,ip,i;
  double ipos;
  AnsiString woord,uwoord;
  TIBDposition **TempIBD;

  woord=Dat->ReadWoord(); //linkage group name
  LG=AddLinkGrp(woord);
  if (CommandlineModeInt==2) {
    OpenProgressForm->ReadLbl1->Caption = "Linkage group "+LinkGrp[LG]->LGName;
    OpenProgressForm->ReadLbl2->Caption = ""; //reading general part only
    OpenProgressForm->ProgressBar1->Position=0;
    OpenProgressForm->ProgressBar1->Visible=false;
    //ShowMessage("Linkage group "+LinkGrp[LG]->LGName);
  }  
  Dat->ReadLn(); Dat->SkipBlNComm();
  woord=Dat->ReadWoord();
  if (UpperCase(woord)=="MAP")
  { if (!ParsingFirstLinkGroup && LinkGrp[0]->LocCount==0)
      throw Exception("MAPs and Loci should occur in all linkage groups or in none");
    //read MAP and LOCUS sections:
    Dat->ReadLn(); Dat->SkipBlNComm();
    ReadMap(Dat,LG); //ends with Dat on line below last locus in map
    if (LinkGrp[LG]->LocCount==0)
      throw Exception("MAP contains no loci in linkage group "+
                      LinkGrp[LG]->LGName);
    loc=0;
    while (loc<LinkGrp[LG]->LocCount)
    { ReadLocus(Dat,LG,loc); //changed for new format!
      ParsingFirstLocus=false;
      loc++;
    }
    Dat->P=1; Dat->SkipBlNComm(); woord=Dat->ReadWoord();
  }
  if (!ParsingFirstLinkGroup && LinkGrp[LG]->LocCount==0 && LinkGrp[0]->LocCount!=0)
      throw Exception("MAPs and Loci should occur in all linkage groups or in none");

  if (UpperCase(woord)=="IBDPOSITIONS")
  { if (IBDAlleleCount<=0)
      throw Exception("IBDPOSITIONS statement found in linkage group "+
                      LinkGrp[LG]->LGName+" but NALLELES=0");
    // read the IBD positions from rest of line:
    ip=0;
    do
    { woord = Dat->ReadWoord();
      if (woord=="" || woord[1]==';') break;
      try {ipos=StrToFloatGen(woord);}
      catch (...) { throw Exception("Error in IBDPOSITIONS of linkage group "+
                                    LinkGrp[LG]->LGName); }
      if (ip>0 && LinkGrp[LG]->IBDpos[ip-1]->cM>=ipos)
        throw Exception("IBDPOSITIONS of linkage group "+LinkGrp[LG]->LGName+
                        " not in ascending order");
      TempIBD = new TIBDposition* [++LinkGrp[LG]->IBDCount];
      for (i=0; i<LinkGrp[LG]->IBDCount-1; i++)
        TempIBD[i]=LinkGrp[LG]->IBDpos[i];
      TempIBD[LinkGrp[LG]->IBDCount-1] = new TIBDposition(LG,ipos); //compiler warning because LG is short int
      delete[] LinkGrp[LG]->IBDpos;
      LinkGrp[LG]->IBDpos = TempIBD; TempIBD=NULL;
      ip++;
    } while (true);
    Dat->ReadLn(); Dat->SkipBlNComm();
  }
  else //woord!=IBDPOSITIONS
  { if (IBDAlleleCount>0)
      throw Exception("IBDPOSITIONS statement missing in linkage group "+
                      LinkGrp[LG]->LGName+" but NALLELES>0");
  }
} //ReadLinkageGroup

/* only used for debugging, not updates to present structures
void WriteData(void)
{ int lg,i,loc,par,a,p;
  TStringList *L;
  AnsiString s,n, spaces="      ";

  L=new TStringList;
  for (lg=0; lg<LinkGrpCount; lg++)
  { L->Add("");
    L->Add("Linkage group "+IntToStr(lg)+": "+LinkGrp[lg]->LGName);
    for (loc=0; loc<LinkGrp[lg]->LocCount; loc++)
    { L->Add("");
      L->Add("Locus "+IntToStr(loc)+": "+LinkGrp[lg]->Locus[loc]->LocName);
      for (i=0; i<IndivCount; i++)
      { s=(Indiv[i].IName+spaces).SubString(1,7);
        for (par=0; par<2; par++)
        { s=s+"   ";
          for (a=0; a<IBDAlleleCount; a++)
          { p=LinkGrp[lg]->Locus[loc]->AllProb[i][par][a];
            n=IntToStr(p);
            if (p>99) s=s+" "+n;
            else if (p>9) s=s+"  "+n;
            else s=s+"   "+n;
          }
        }
        L->Add(s);
      } //for i
    } //for loc
  } //for lg
  L->SaveToFile("Data.txt");
  delete L;
} //WriteData   */

//arrays used for ordering IBD alleles by founder:
AnsiString *IBDName=NULL;
int *IBDNameNum=NULL;

//Compare function for ordering IBDalleles if INameNumbers is true:
bool CompareFoundersbyNumber(int Item1, int Item2)
{ //int i, INum1, INum2;
  //INum1 = (i=IBDalleleFounder[Item1])>=0 ? Indiv[i].INameNum : -1;
  //INum2 = (i=IBDalleleFounder[Item2])>=0 ? Indiv[i].INameNum : -1;
  return
    IBDNameNum[Item1]<IBDNameNum[Item2] ? false :
    IBDNameNum[Item1]>IBDNameNum[Item2] ? true :
    (IBDalleleFounderHom[Item1] > IBDalleleFounderHom[Item2]) ;
}

//Compare function for ordering IBDalleles if INameNumbers is false:
bool CompareFoundersbyName(int Item1, int Item2)
{ //int i; AnsiString IName1, IName2;
  //IName1 = (i=IBDalleleFounder[Item1])>=0 ? Indiv[i].IName : AnsiString(NULL);
  //IName2 = (i=IBDalleleFounder[Item2])>=0 ? Indiv[i].IName : AnsiString(NULL);
  return
    IBDName[Item1]<IBDName[Item2] ? false :
    IBDName[Item1]>IBDName[Item2] ? true :
    (IBDalleleFounderHom[Item1] > IBDalleleFounderHom[Item2]) ;
}

//compare function for alphabetically sorting LocusList:
int __fastcall LocusCompare( void *Item1, void *Item2)
{ TLocus *L1, *L2;
  AnsiString N1, N2;
  L1=(TLocus*)(Item1); L2=(TLocus*)(Item2);
  return
    (N1=L1->LocName)<(N2=L2->LocName) ? -1 :
    N1>N2 ? 1 :
    L1->LGnr<L2->LGnr ? -1 :
    L1->LGnr>L2->LGnr ? 1 :
    L1->cM<L2->cM ? -1 :
    L2->cM>L2->cM ? 1 :
    0;
}

bool ReadDataFile (const AnsiString &FName)
// new format January 2007
{ TInTextfile *Dat;
  TBinaryfile *BF;
  AnsiString s,uw,woord,BFname,*tempUnknown;
  int i,t,a,b,p,r,iFileHandle, *ITemp,
      LG,loc,ip,
      FounderCount[3], *Founders[3];
        //index 0 and 1: individuals missing only Parent 0 or 1;
        //index 2: individuals missing both parents
  Word ver;
  bool Result,Found,
       BF_in_ok,  //valid BF available for reading
       BF_out_ok, //BF succesfully created for writing
       FileReadError;
  TCrossing *Cro;
  TDateTime DataDate,BinaryDate;
  TCursor Save_Cursor;



  FileReadError = true; //set to false when completed
  Save_Cursor= Screen->Cursor;  //Screen->Cursor;
  Screen->Cursor = crHourGlass;    // Show hourglass cursor
  Application->ProcessMessages();
  Result=false;
  for (i=0; i<3; i++) {Founders[i]=NULL; FounderCount[i]=0;}
  ITemp=NULL;
  if (CommandlineModeInt==2) {
    OpenProgressForm->FileLbl->Caption = FName; //doesn't show form yet
    OpenProgressForm->StopPressed = false;
  }  
  Dat=NULL;
  try
  {
    try
    { /*checks:
      - see if data file past expirydate
      - see if there is a binary file with
        - date later than that of datafile
        - binary fileversion same as executable
      */
      Dat=NULL; BF=NULL;
      /*
      FileHandle=FileOpen(FName,0x0); //cannot open files currently open in Excel
      if (FileHandle<0 || !FileExists(FName))
        throw Exception("File "+FName+" not found");
      DataDate=FileDateToDateTime(FileGetDate(FileHandle));
      Expired = DataDate>ExpiryDate;
      FileClose(FileHandle);
      */
      if (!FileExists(FName))
        throw Exception("File "+FName+" not found");
      iFileHandle = FileOpen(FName, fmOpenRead);
      if (iFileHandle<0) {
         //this is the case when reading a file open in Excel,
         //even with Mode = fmOpenRead or 0
         DataDate = Now(); //to force rewriting pmb file
         //ShowMessage("iFileHandle<0");
      }
      else {
         DataDate = FileDateToDateTime(FileGetDate(iFileHandle));
         FileClose(iFileHandle);
         //ShowMessage("iFileHandle="+IntToStr(iFileHandle));
      }   
      Dat = new TInTextfile(FName,Result);
      if (Result) //error opening file
        throw Exception("Cannot open file "+FName);
      //Read header lines
      /* Header lines may be omitted. If (some are) present they contain:
         a keyword (not case sensitive), "=", a value (spaces are ignored)
         The header lines that are present can occur in any order:
         POPULATION = population name ; if not present, the filename is used
         UNKNOWN = <strings> (multiple strings allowed); if not present, "*" is used
         NULLHOMOZ = <string> ; if not present, "$" is used
         CONFIRMEDNULL = <string> ; if not present, "null" is used
         PLOIDY = number>0 ; if not present, 2 is used
         NALLELES = <number> ; obligatory if allele probabilities are given, else ignored
      */
      //Datafile can be opened, keep path and name:
      //now Result==false
      DataDir=ExtractFilePath(FName);
      DataFileName=ExtractFileName(FName);
      DataFileExt=ExtractFileExt(DataFileName);
      DataFileName=DataFileName.SubString(1, DataFileName.Length()
                                               - DataFileExt.Length());
      //first set the default values:
      PopName=DataFileName;
      UnknownCount=1;
      strUnknown= new AnsiString[UnknownCount];
      strUnknown[0]=mv_trait; 
      strNullHomoz=strDefNullHomoz;
      strConfirmedNull=strDefConfirmedNull;
      Ploidy=2;
      IBDAlleleCount=0;

      //loop: read header lines in any order (if duplicated, the last one is used)
      for (;;)
      { Dat->SkipBlNComm();
        woord=Dat->ReadWoord(); uw=UpperCase(woord);
        if ( !(uw=="POPULATION" ||
               uw=="UNKNOWN" || uw=="NULLHOMOZ" ||uw=="CONFIRMEDNULL" ||
                uw=="PLOIDY" || uw=="NALLELES") )
          break; //end of loop

        if (uw=="POPULATION")
        { Dat->SkipPastChar("=");
          PopName=Dat->ReadWoordQ();
        }

        if (uw=="UNKNOWN")
        { Dat->SkipPastChar("=");
          UnknownCount=0; delete[] strUnknown; strUnknown=NULL;
          do
          { woord=Dat->ReadWoord();
            if (woord=="" || woord[1]==';')
            { Dat->P=Dat->Line.Length()+1; break; //past any comment
            }
            tempUnknown= new AnsiString[++UnknownCount];
            for (i=0; i<UnknownCount-1; i++) tempUnknown[i]=strUnknown[i];
            tempUnknown[UnknownCount-1]=woord;
            delete[] strUnknown; strUnknown=tempUnknown; tempUnknown=NULL;
          } while (true);
          if (UnknownCount==0) throw Exception("No valid Unknown symbols");
        }

        if (uw=="NULLHOMOZ")
        { Dat->SkipPastChar("=");
          woord=Dat->ReadWoord();
          if (woord.Length()==0) throw Exception("No valid NULLHOMOZ symbol");
          strNullHomoz=woord;
        }

        if (uw=="CONFIRMEDNULL")
        { Dat->SkipPastChar("=");
          woord=Dat->ReadWoord();
          if (woord.Length()==0) throw Exception("No valid CONFIRMEDNULL allele symbol");
          strConfirmedNull=woord;
        }

        if (uw=="PLOIDY")
        { Dat->SkipPastChar("=");
          woord=Dat->ReadWoord();
          Ploidy=StrToInt(woord);
          if (Ploidy<=0) throw Exception("Ploidy<0");
        }

        if (uw=="NALLELES")
        { Dat->SkipPastChar("=");
          woord=Dat->ReadWoord();
          IBDAlleleCount=StrToInt(woord);
          if (IBDAlleleCount<0) throw Exception("NALLELES<0");
          AProbs = new float [IBDAlleleCount];
          //FrBase = new short int [IBDAlleleCount];
        }
      } //for-loop

      //read pedigree (including phenotypic data); "PEDIGREE" not needed if IBDAlleleCount=0
      woord=UpperCase(woord);
      if (IBDAlleleCount==0 && woord!="PEDIGREE") Dat->P=1; //back to start of line
      else
      { if (IBDAlleleCount>0 && woord!="PEDIGREE")
        { throw Exception ("'PEDIGREE' not found"); }
        else Dat->ReadLn(); //to start of line after PEDIGREE
      }
      ReadPedigree(Dat);
      if (IndivCount<=0)
      { throw Exception ("no individuals found"); }

      if (CommandlineModeInt==2) {
        Application->ProcessMessages();
        if ( OpenProgressForm->StopPressed)
           throw Exception("Opening file aborted");
      }

      //read linkage groups, if IBDAlleleCount>0
      ParsingFirstLocus=true; FounderAllelesPresent=false;
      ParsingFirstLinkGroup=true;

      //Now Dat->Line contains GROUP or LINKAGEGROUP, or eof
      Dat->P=1; Dat->SkipBlNComm();
      woord=Dat->ReadWoord(); woord=UpperCase(woord);
      if ((IBDAlleleCount>0) &&
          (woord!="LINKAGEGROUP") && (woord!="GROUP"))
      throw Exception("'LINKAGEGROUP' not found but NALLELES>0");
      else if ((woord!="") && (woord!="LINKAGEGROUP") && (woord!="GROUP"))
           throw Exception("'LINKAGEGROUP' or end of file expected,\nbut '" +
                           Dat->Line + "' found");
      if (woord!="")
      { // further text after pedigree found:
        // - read linkage groups
        // - read alleles and/or ibds from datafile or binary file
        // - further procesing of alleles and/or ibds
        do
        { ReadLinkageGroup(Dat);
          ParsingFirstLinkGroup=false;
          if (CommandlineModeInt==2) {
            if (OpenProgressForm->StopPressed)
              throw Exception("Opening file aborted");
          }
          Dat->P=1; Dat->SkipBlNComm(); woord=Dat->ReadWoord();
          if (CommandlineModeInt==2) {
            Application->ProcessMessages();
            if ( OpenProgressForm->StopPressed)
              throw Exception("Opening file aborted");
          }    
        } while (UpperCase(woord)=="LINKAGEGROUP" || UpperCase(woord)=="GROUP");
        if (TotLocCount>0 && UpperCase(woord)!="ALLELES" )
        { throw Exception("'"+Dat->Line+"' found,\n" +
            "but 'ALLELES' expected");
        }
        else if (TotLocCount==0 && IBDAlleleCount>0 && woord!="IBDPOSITION" )
        { throw Exception("'"+Dat->Line+"' found,\n" +
            "but 'IBDPOSITION' expected");
        }
        else if (TotLocCount==0 && IBDAlleleCount==0)
        { //should never occur
          throw Exception("Linkage groups read but no Loci or IBD positions found");
        }

        /*
        after reading linkage groups:
        - check if loci or ibdpositions are declared
        - if so, check if binary file available or try to make one
        - read alleles and/or ibd probs from Dat or from BF
        - write to BF if needed
        */

        if (LinkGrpCount>0)     //should always be true after previous checks
        //check for presence of valid binary file:
        { BF_in_ok=false; BF_out_ok=false;
          BFname=DataDir+DataFileName+".pmb";
            //pedimap binary file has same name as datafile, with extension pmb (including path)
          iFileHandle=FileOpen(BFname,0);
          if (iFileHandle>0)
          { BinaryDate=FileDateToDateTime(FileGetDate(iFileHandle));
            FileClose(iFileHandle);
            BF_in_ok = BinaryDate>=DataDate;
            if (BF_in_ok)
            { BF = new TBinaryfile(BFname.c_str(), std::ios::in | std::ios::binary, BF_in_ok);
              BF_in_ok=!BF_in_ok;
              if (BF_in_ok)
              { BF->seekg(0);
                BF->read(&ver,1); //read 1 Word = binary file version
                BF_in_ok= ver==BFversion;
              }
            }
            if (BF_in_ok)
            { BF->read(&MarkCodeCount,1); // 1 byte
              BF->seekg(16); //skip header
            }
            else //binary file exists but not ok, delete
              DeleteFile(BFname);
          } //BF exists
          if (!BF_in_ok)
          { //try to create new binary file
            delete BF; BF=NULL;
            BF = new TBinaryfile(BFname.c_str(), std::ios::out | std::ios::binary, BF_out_ok);
            BF_out_ok=!BF_out_ok;
            if (BF_out_ok)
            { //write header: 16 bytes, currently only first 2 used for version:
              BF->write(&BFversion,1); //write 1 Word = 2 bytes
              ver=0; i=0; //used here to write 2 and 4 zero-bytes
              BF->write(&ver,1); //2 bytes (1st will later contain MarkCodeCount)
              BF->write(&i,1); //write 1 int = 4 bytes
              BF->write(&i,1); //write 1 int
              BF->write(&i,1); //write 1 int
            }
          }
          if (TotLocCount>0)
          { if (BF_in_ok)
            { if (CommandlineModeInt==2) {
                OpenProgressForm->ReadLbl2->Caption = ""; //binary reading fast, no need to show each locus name
              }
              for (LG=0; LG<LinkGrpCount;LG++)
              { if (CommandlineModeInt==2) {
                  OpenProgressForm->ReadLbl1->Caption = "Linkage group "+LinkGrp[LG]->LGName;
                  Application->ProcessMessages();
                }
                for (loc=0; loc<LinkGrp[LG]->LocCount; loc++)
                { BF->read(LinkGrp[LG]->Locus[loc]->IndAllIxArray(0), IndivCount*Ploidy);
                  BF->read(LinkGrp[LG]->Locus[loc]->IndAllCodeArray(0), IndivCount*Ploidy);
                }
              }
            }
            else //not BF_in_ok
            { for (LG=0; LG<LinkGrpCount;LG++)
              { if (CommandlineModeInt==2) {
                  OpenProgressForm->ReadLbl1->Caption = "Linkage group "+LinkGrp[LG]->LGName;
                }
                for (loc=0; loc<LinkGrp[LG]->LocCount; loc++)
                { if (CommandlineModeInt==2) {
                    OpenProgressForm->ReadLbl2->Caption = "Locus "+LinkGrp[LG]->Locus[loc]->LocName;
                    Application->ProcessMessages();
                    //ShowMessage("Linkage group "+IntToStr(LG)+" "+LinkGrp[LG]->LGName+
                    //            "\nLocus "+IntToStr(loc)+" "+LinkGrp[LG]->Locus[loc]->LocName);
                  }
                  ReadAlleles(Dat,LG,loc);
                  if (BF_out_ok)
                  { BF->write(LinkGrp[LG]->Locus[loc]->IndAllIxArray(0), IndivCount*Ploidy);
                    BF->write(LinkGrp[LG]->Locus[loc]->IndAllCodeArray(0), IndivCount*Ploidy);
                  }
                } //for loc
              } //for LG
            }
          }
          if (IBDAlleleCount>0)
          { //read IBDs from binary file:
            if (BF_in_ok)
            { for (LG=0; LG<LinkGrpCount;LG++)
              { if (CommandlineModeInt==2) {
                  OpenProgressForm->ReadLbl1->Caption = "Linkage group "+LinkGrp[LG]->LGName;
                  Application->ProcessMessages();
                }
                for (ip=0; ip<LinkGrp[LG]->IBDCount; ip++)
                { BF->read(LinkGrp[LG]->IBDpos[ip]->IndArray(0),
                           IndivCount*Ploidy*IBDAlleleCount);
                }
              }
            }
            else // not BF_in_ok
            { //read IBDs from Data file:
              for (LG=0; LG<LinkGrpCount;LG++)
              { if (CommandlineModeInt==2) {
                  OpenProgressForm->ReadLbl1->Caption = "Linkage group "+LinkGrp[LG]->LGName;
                }
                for (ip=0; ip<LinkGrp[LG]->IBDCount; ip++)
                { if (CommandlineModeInt==2) {
                    OpenProgressForm->ReadLbl2->Caption = "IBD Position "+
                       FloatToStrF(LinkGrp[LG]->IBDpos[ip]->cM,ffFixed,8,3);
                    Application->ProcessMessages();
                  }
                  ReadIBDpos(Dat,LG,ip);
                  //write binary file:
                  if (BF_out_ok)
                  { // seekp not needed
                    //BF->seekp(16+   //header: 16 bytes
                    //        (LinkGrp[LG]->StartIBDNr+ip)*IndivCount*Ploidy*IBDAlleleCount); //start current IBD
                    BF->write(LinkGrp[LG]->IBDpos[ip]->IndArray(0), IndivCount*Ploidy*IBDAlleleCount);
                  }
                } //for ip
              } //for LG
            }
            if (TotLocCount>0) //and we know IBDAlleleCount>0
            { //insert IBD probabilities at locus positions:
              for (LG=0; LG<LinkGrpCount; LG++)
                InsertIBDatLoci(LG);
            }
          }
          if (BF_out_ok && TotLocCount>0)
          { BF->seekp(2);
            BF->write(&MarkCodeCount,1); //1 byte
          }
        } //LinkGrpCount>0, alleles and ibs have now been read, BF written if needed

        /*then continue as before:
          with determining founder allele numbers if needed
          locuslist
          etc
        */
        
        if (IBDAlleleCount>0 && (Ploidy/2)*2==Ploidy)
        { //Assign each IBDallele index (each IBD color) to a Founder/Homolog
          //First: make lists of all founder individuals
          for (r=0; r<3; r++)
          { Founders[r]=NULL; FounderCount[r]=0; }
          for (i=0; i<IndivCount; i++)
          { Cro=Indiv[i].ParentCross;
            p = ( Cro==NULL || ( Cro->Parent[0]==-1 && Cro->CType!=ctCross ) ) ?
                2 :                                          //both parents unknown
                ( Cro->CType==ctCross && Cro->Parent[0]==-1 ) ?
                0 :                                          //only first parent unknown
                ( Cro->CType==ctCross && Cro->Parent[1]==-1 ) ?
                1 :                                          //only second parent unknown
                3 ;                                          //both parents known
            if (p<3)
            { ITemp = new int [++FounderCount[p]];
              for (r=0; r<FounderCount[p]-1; r++) ITemp[r]=Founders[p][r];
              ITemp[FounderCount[p]-1]=i;
              delete[] Founders[p]; Founders[p]=ITemp; ITemp=NULL;
            }
          }
          //now all Founders have been found
          //for all IBD alleles, find corresponding Founder/Homolog
          //based on first IBD position in first LG:
          if (LinkGrp[0]->IBDpos == NULL)
            throw Exception("NALLELES>0 but IBD probabilities missing in some or all linkage groups");
          IBDalleleFounder =    new int [IBDAlleleCount];
          IBDalleleFounderHom = new int [IBDAlleleCount];
          for (r=0; r<IBDAlleleCount; r++)
          { Found = false;
            t=2;
            while (!Found && t>=0)
            { //look first in full founders, then in those missing second parent, then first parent
              a = t==1 ? Ploidy/2 : 0 ;           //first homolog to check
              b = t==0 ? Ploidy/2-1 : Ploidy-1 ;  //last homolog  to check
              i=0;
              while (!Found && i<FounderCount[t])
              { p=a;
                while (!Found && p<=b)
                { Found = LinkGrp[0]->IBDpos[0]->AllProb[Founders[t][i]][p][r] >= 240-1;
                    //240-1 to allow for rounding error. NB: 0.99 in datafile is not sufficient!
                  p++;
                }
                i++;
              }
              t--;
            }
            if (Found)
            { IBDalleleFounder[r]=Founders[t+1][i-1];
              IBDalleleFounderHom[r]=p-1;
            }
            else
            { IBDalleleFounder[r]=-1;
              IBDalleleFounderHom[r]=-1;
            }
          } //for r
          for (r=0; r<3; r++)
          { delete[] Founders[r]; Founders[r]=NULL; }

        }
        // now order the IBD alleles by founder IName or INameNum and homolog:
        IBDalleleIndex = new int [IBDAlleleCount];
        for (r=0; r<IBDAlleleCount; r++) IBDalleleIndex[r] = r;
        //first check if any founders have been identified:
        r=IBDAlleleCount-1;
        while (r>=0 && IBDalleleFounder[r]<0) r--;
        if (r>=0)
        { //one or more founders found, re-order IBD alleles:
          if (INameNumbers)
          { IBDNameNum = new int [IBDAlleleCount];
            for (r=0; r<IBDAlleleCount; r++)
              IBDNameNum[r] = IBDalleleFounder[r]<0 ?
                            -1000000000 + r :
                            Indiv[IBDalleleFounder[r]].INameNum;
            MakeIndex(IBDAlleleCount, IBDalleleIndex, CompareFoundersbyNumber);
            delete[] IBDNameNum; IBDNameNum=NULL;
          }
          else
          { IBDName = new AnsiString [IBDAlleleCount];
            for (r=0; r<IBDAlleleCount; r++)
            { if (IBDalleleFounder[r]>=0)
                IBDName[r]=Indiv[IBDalleleFounder[r]].IName;
              else
              { s=AnsiString::StringOfChar(' ',20)+IntToStr(r);
                IBDName[r]=s.SubString(s.Length()-19,20);
              }
            }
            MakeIndex(IBDAlleleCount, IBDalleleIndex, CompareFoundersbyName);
            delete[] IBDName; IBDName=NULL;
          }
        }

        //fill and sort LocList:
        LocusList = new TList;
        for (i=0; i<LinkGrpCount; i++)
        { for (r=0; r<LinkGrp[i]->LocCount; r++)
            LocusList->Add(LinkGrp[i]->Locus[r]);
        }
        LocusList->Sort(LocusCompare);
      } //if further text after pedigree

      if (CommandlineModeInt==2) {
        MainForm->Enabled=true;
        OpenProgressForm->Hide();
      }
      FileReadError=false;
      Result=true;
    }
    catch (const Exception& e)
    { if (CommandlineModeInt==2) {
        MainForm->Enabled=true; OpenProgressForm->Hide();
        ShowMessage ("Error reading file " + FName +":\n" +
                   e.Message);     //"\n" +
      }
      for (i=0; i<3; i++)
        delete[] Founders[i];
      delete[] ITemp;

      ClearData();
      Result=false;
    }
  }
  __finally
  { if (CommandlineModeInt==2) {
      Screen->Cursor = Save_Cursor; // always restore the cursor
      MainForm->Enabled=true; OpenProgressForm->Hide();
    }
    delete Dat; Dat=NULL;
    delete[] AProbs; AProbs=NULL; //delete[] FrBase; FrBase=NULL;
    if (BF!=NULL)
    { delete BF; BF=NULL;
      if (BF_out_ok &&
        ((CommandlineModeInt==2 && OpenProgressForm->StopPressed) || FileReadError) )
        DeleteFile(BFname); //incomplete file written, delete
    }
    Application->ProcessMessages();
  }
  return Result;
} //ReadDataFile

