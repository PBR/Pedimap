#include "PCH.h"
//#include <vcl.h>
#pragma hdrstop

#include "DataUnit.h"
#include "Main.h"
#include <values.h> //MAXINT
#include <math.h> //sqrt
#include "EMFeditUnit.h"
#include "LegendUnit.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
int CommandlineModeInt=3;

const DefCodeColCount = 10;

TColor DefCodeCol[DefCodeColCount] = {
  clBlack,
  clRed,
  TColor(0x0088ff),  //orange
  clGreen,
  clAqua,
  clBlue,
  clTeal,
  clMaroon,
  clFuchsia,
  clPurple };

AnsiString CRLF=AnsiString(CR)+AnsiString(LF);
AnsiString DQ='"',
           Tb=AnsiString(cTb);
bool ProjectSaved;
//TDateTime Today;
AnsiString TempDir=""; //system temporary directory
AnsiString ProgDir=""; //path of Pedimap.exe
AnsiString DataDir=""; //path of datafile
AnsiString DataFileName=""; //filename of datafile without extension or path
AnsiString DataFileExt=""; //extension of datafile
AnsiString ProjectDir=""; //path of project file
AnsiString ProjectFileName=""; //filename of project file without extension or path
//const AnsiString ProjectExt=".pmp";

TIndiv* Indiv; //array with all Indiv
int IndivCount=0;
int Ploidy=2;
byte MarkCodeCount=0; //Marker allelecodes from 0 to MarkCodeCount=maximum code read +1 (max 255)
bool INameNumbers, //Indiv names are numbers
     FounderAllelesPresent; // are present or absent at all loci
     //AlleleObsPresent,      // id.
     //IBDprobsPresent;       // are present or absent in all linkage groups
int IndivDrawLimit=1000;

TCrossing* FirstCrossing=NULL;
TCrossing* TCrossing::Last=NULL;
int TCrossing::CCount=0;
int TViewPop::VPCount=0;

//the following keyword strings must be UPPERCASE !
AnsiString *strUnknown=NULL; //strMissVal is default, may be changed by user
int UnknownCount=0;
AnsiString strNullHomoz=strDefNullHomoz; //for reading Null-or-homozygote alleles
AnsiString strConfirmedNull=strDefConfirmedNull;
AnsiString CrossTypeString[CrossTypeCount] = {"*CROSS","*SELF","*DH","*MUT","*VP"};
int FemaleParent=2; //0 or 1: number of female parent; 2: no sex specified

AnsiString PopName=""; //Population name

int LinkGrpCount=0; //number of linkage groups
int TotLocCount=0;  //total number of loci over all LinkGrp
int TotIBDCount=0;  //total number of IBD positions over all LinkGrp
TLinkGrp **LinkGrp=NULL; //array of pointers to linkage groups
int IBDAlleleCount=0;  //number of different founder alleles; if >0, IBD probabilities are available
//bool AllNameCountIsIBDAllCount=false;
TLicenseInfo LicInfo;
TIndivDatum* IndivData=NULL;
int IndivDataCount=0;
int DataTypeCount[dtS-dtC+1]={0,0,0,0}; //number of data of each type
// arrays of array[0...IndCount-1] of each datatype:
char** Ichar=NULL;  //example: IChar[3,150] is fourth character datafield of 151-st Individual
int**  Iint=NULL;
double** Ifloat=NULL;
int** Ilabel=NULL; //indices into Labels array (Ansistrings) of IndivData[d]
int* IBDalleleFounder=NULL;
int* IBDalleleFounderHom=NULL;
int* IBDalleleIndex=NULL;

TList *LocusList=NULL; //alphabetically sorted list to all TLocus of all linkage groups

TViewPop *CurrentViewPop=NULL;
TViewPop *SourceViewPop=NULL; //for copying Views to Target ViewPop


TPrinter *Prntr = Printer();

int PPI ; // ScreenPixelsPerInch
int PrPPI; // PrinterPixelsPerInch
int LinePx; //width of all lines in pixels; = (63+PrPPI) / 127, approx. 0.2 mm

AnsiString IndivContentsTypes[5] = {"icNone", "icTraits", "icIBDprob", "icIBDall", "icMarkers"};

int IndNameListWidth; //width of longest Individual name in Listview pixels

TViewOptions *CopiedViewOptions=NULL; //after the first time View options are copied
          //they are stored here and used when View options are pasted
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define max(a, b)  (((a) > (b)) ? (a) : (b))

void ReadWoord(int &p, AnsiString &Woord, const AnsiString &Line)
//const AnsiString &Line is much faster than AnsiString Line
//does NOT read quoted words
{ int q,L;
  Woord=""; L=Line.Length(); //is just as fast as calling Line.Length() all the time
  while (p<=L && Line[p]<=' ') p++;
  q=p;
  while (p<=L && Line[p]>' ') p++;
  Woord=Line.SubString(q,p-q);
} //ReadWoord

void ReadWoordQ(int &p, AnsiString &Woord, const AnsiString &Line)
//as ReadWoord, but also reads quoted words
{ int q,L;
  Woord=""; L=Line.Length(); //is just as fast as calling Line.Length() all the time
  while (p<=L && Line[p]<=' ') p++;
  if (p<=L && Line[p]=='"')
  { //read quoted word until next '"' or EOLN
    // (may contain any characters including blanks, except '"')
    q=++p;
    while (p<=L && Line[p]!='"') p++;
    Woord=Line.SubString(q,p-q);
    p++; //skip past closing '"'
  }
  else
  { //read unquoted word until next blank or EOLN (may include '"' characters)
    q=p;
    while (p<=L && Line[p]>' ') p++;
    Woord=Line.SubString(q,p-q);
  }
} //ReadWoordQ

/*
void ReadWoordSL(int r, int &p, AnsiString &Woord, TStringList *SL)
//reads next woord on line r, but returns "" if r>=SL->Count
{ int q;
  Woord="";
  if (r<SL->Count)
  { while (p<=SL->Strings[r].Length() && SL->Strings[r][p]<=' ') p++;
    if (p<=SL->Strings[r].Length())
    { if (SL->Strings[r][p]==cDQ)
      { //double-quote: skip, and read until next DQ or EOLN:
        q=++p;
        while (p<=SL->Strings[r].Length() && SL->Strings[r][p]!=cDQ) p++;
        Woord=SL->Strings[r].SubString(q,p-q);
        p++; //skip final DQ or further past EOLN
      }
      else
      { //no double-quote: read until next blank or EOLN:
        q=p;
        while (p<=SL->Strings[r].Length() && SL->Strings[r][p]>*(" ")) {p++;}
        Woord=SL->Strings[r].SubString(q,p-q);
      }
    }
  }
} //ReadWoordSL
*/
void ReadWoordSL(int r, int &p, AnsiString &Woord, TStringList *SL)
//reads next woord on line r, but returns "" if r>=SL->Count
//reads quoted and unquoted words like ReadWoordQ
{ int q,L;
  if (r<SL->Count)
  { L=SL->Strings[r].Length();
    while (p<=L && SL->Strings[r][p]<=' ') p++;
    if (p<=L && SL->Strings[r][p]=='"')
    { //read quoted word until next '"' or EOLN
      // (may contain any characters including blanks, except '"')
      q=++p;
      while (p<=L && SL->Strings[r][p]!='"') p++;
      Woord=SL->Strings[r].SubString(q,p-q);
      p++; //skip past closing '"'
    }
    else
    { //read unquoted word until next blank or EOLN (may include '"' characters)
      q=p;
      while (p<=L && SL->Strings[r][p]>' ') p++;
      Woord=SL->Strings[r].SubString(q,p-q);
    }
  }
  else Woord=""; //if r>=SL->Count
} //ReadWoordSL

void CalcCircles(TPoint P1, TPoint P2, int L, bool Horz,
                 TPoint &C, int &r)
//Find circles tangent to Horz or Vert line L, through points P1 and P2
//Return only the circle with the contact point "between" P1 and P2
//(C=center of circle, r=radius). If no solution, r=0
{ //const maxint=0x7FFFFFFF;
  long double
    XL,                       //X or Y coord of line
    X1,Y1,X2,Y2,              //coord of P1 and P2
    Xa,Ya,Ra,Xb,Yb,Rb,        //two circle centers and radii
    AvX, AvY,                 //Average X- and Y coord of P1 and P2
    DX, DY, DDXY,             //Difference of X- and Y coord of P1 and P2, and DX/DY
    B, Det;                   //B and Determinant in A-B-C formula
  //if Horizontal line, exchange X and Y co-ordinates of P1 and P2:
  if (Horz) { X1=P1.y; Y1=P1.x; X2=P2.y; Y2=P2.x; }
  else { X1=P1.x; Y1=P1.y; X2=P2.x; Y2=P2.y; }
  XL=L;
  //rest of calculations with vertical tangent line, with X-coord = L

  //change coordinates to center points 1 and 2 around Origin:
  AvX=(X1+X2)/2.0; AvY=(Y1+Y2)/2.0; //Average X and Y
  X1-=AvX; X2-=AvX; Y1-=AvY; Y2-=AvY;
  XL-=AvX; //XL=X-coord of vertical line

  //calculate both circles (Ra=0 or Rb=0: circle not valid)
  if (Y1==Y2)
  { //points horizontally aligned)
    Det=XL*XL-X1*X1;
    if (Det<0) //no solutions
    { Ra=0; Rb=0; }
    else
    { Det=sqrtl(Det);
      Xa=0; Ya=Det; Ra=abs(XL);
      Xb=0; Yb=-Det; Rb=Ra;
    }
  }
  else //Y1<>Y2
  { DX=X1-X2; DY=Y1-Y2;
    if (X1==X2)
    { //points vertically aligned: max. one solution
      if (XL==0) Ra=0; //both points on tangent line: no solution
      else {Xa=(XL*XL-Y1*Y1)/2.0/XL; Ya=-DX/DY*Xa; Ra=abs(XL-Xa);}
      Rb=0;
    }
    else
    { //points diagonally positioned
      DDXY=DX/DY;
      B=2.0*(DDXY*Y1-X1+XL); //B in A-B-C formula
      Det=B*B - 4.0*DDXY*DDXY*(X1*X1+Y1*Y1-XL*XL); //Determinant of A-B-C formula
      if (Det<0) { Ra=0; Rb=0; } //no solution
      else
      { Det=sqrtl(Det);
        Xa=(-B+Det)/2.0/DDXY/DDXY; Ya=-DDXY*Xa; Ra=abs(XL-Xa);
        Xb=(-B-Det)/2.0/DDXY/DDXY; Yb=-DDXY*Xb; Rb=abs(XL-Xb);
      }
    }
  } //not Y1==Y2

  //convert back to original coordinates:
  X1+=AvX; X2+=AvX; Xa+=AvX; Xb+=AvX;
  Y1+=AvY; Y2+=AvY; Ya+=AvY; Yb+=AvY;

  //select the correct circle;
  //check for potential integer out-of-range errors::
  if (Ra>0 && Ya>=(min(Y1,Y2)-0.5) && Ya<=(max(Y1,Y2)+0.5) &&
      !( Xa-Ra<-MAXINT || Xa+Ra>MAXINT ||
         Ya-Ra<-MAXINT || Ya+Ra>MAXINT ) )
  { C.x=floor(Xa+0.5); C.y=floor(Ya+0.5); r=floor(Ra+0.5); }
  else if (Rb>0 && Yb>=(min(Y1,Y2)-0.5) && Yb<=(max(Y1,Y2)+0.5) &&
          !( Xb-Rb<-MAXINT || Xb+Rb>MAXINT ||
             Yb-Rb<-MAXINT || Yb+Rb>MAXINT ) )
  { C.x=floor(Xb+0.5); C.y=floor(Yb+0.5); r=floor(Rb+0.5); }
  else r=0;

  //if Horizontal line, exchange X- and Y co-ordinates of C:
  if (Horz) { L=C.x; C.x=C.y; C.y=L; }
} //CalcCircles



// ******** TIndiv ************

TIndiv::TIndiv(TCrossing *CParent)
{ IName=""; INameNum=0;
  MCount=0; Mating=NULL;
  ParentCross=CParent;
}

TIndiv::TIndiv(const AnsiString Name, TCrossing *CParent)
{ IName=Name; INameNum=0;
  MCount=0; Mating=NULL;
  ParentCross=CParent;
}

TIndiv::~TIndiv(void)
{ if (Mating!=NULL) {delete[] Mating;}
}

TIndiv::TIndiv(TIndiv &Source) //copy-constructor
{ ShowMessage("Warning: TIndiv copy-constructor not implemented"); }

void TIndiv::AddMating(TCrossing* Cro)
{ TCrossing* *Temp;
  int i;
  Temp = new TCrossing*[++MCount];
  for (i=0; i<MCount-1; i++) Temp[i]=Mating[i];
  delete[] Mating;
  Temp[MCount-1]=Cro;
  Mating=Temp;
} //TIndiv::AddMating

// ******** TCrossing ************

TCrossing::TCrossing(int P0, int P1, int CrType)
{ Prev=Last; Next=NULL;
  CNum=CCount++;
  CType=CrType;
  OCount=0;
  Offsp=NULL;
  Parent[0]=P0; Parent[1]=P1;
  if (Last!=NULL) {Last->Next=this;}
  Last=this;
}

TCrossing::~TCrossing(void)
{ if (Next!=NULL) {delete Next;}
  if (Offsp!=NULL) {delete[] Offsp;}
}

TCrossing::TCrossing(TCrossing &Source)
{ ShowMessage("Warning: TCrossing copy-constructor not implemented"); }

void TCrossing::AddOffspring(int Ind)
{ int *Temp;
  int i;
  Temp = new int [++OCount];
  for (i=0; i<OCount-1; i++) Temp[i]=Offsp[i];
  delete[] Offsp;
  Temp[OCount-1]=Ind;
  Offsp=Temp;
}

// ******** TGenStrip ************

TGenStrip::TGenStrip(TView *View)
{ VW=View;
  ICount=0; Ind=NULL;
  GPos=-1.0; IPos=NULL;
  IndMF=NULL;
}

TGenStrip::~TGenStrip(void)
{ int i;
  delete[] Ind; delete[] IPos;
  for (i=0; i<ICount; i++) delete IndMF[i];
  delete[] IndMF;
}

TGenStrip::TGenStrip(TGenStrip &Source)
{ ShowMessage("Warning: TGenStrip copy-constructor not implemented"); }

void TGenStrip::CountGaps(void)
{ TCrossing *Cro;
  TViewPop *VP;
  int i, VPI, Indv;
  VP=VW->VP;
  FamGapCount=SibGapCount=0;
  if (ICount>0)
  { Indv=VP->VPInd[Ind[0]];
    Cro=Indiv[Indv].ParentCross;
    for (i=1; i<ICount; i++) //not from i=0
    { VPI=Ind[i]; Indv=VP->VPInd[VPI];
      if (Indiv[Indv].ParentCross==Cro && !VP->IsVPFounder(VPI)) SibGapCount++;
      else { FamGapCount++; Cro=Indiv[Indv].ParentCross; }
    }
  }
} //TGenStrip::CountGaps

int TGenStrip::AddVPInd(int I, int index)
{ int *TempInd,j; int *TempIPos; TMetafile **TempMF;
  if (I==-1) return -1;
  else
  { TempInd = new int [++ICount];
    TempIPos = new int [ICount];
    TempMF = new TMetafile* [ICount];
    if (index<0 || index>=ICount) index=ICount-1; //add at end if not (correctly) specified
    for (j=0; j<index; j++)
    { TempInd[j]=Ind[j];
      TempIPos[j]=IPos[j];
      TempMF[j]=IndMF[j];
    }
    for (j=index+1; j<ICount; j++)
    { TempInd[j]=Ind[j-1];
      TempIPos[j]=IPos[j-1];
      TempMF[j]=IndMF[j-1];
    }
    delete[] Ind; delete[] IPos; delete[] IndMF;
    Ind=TempInd; Ind[index]=I;
    IPos=TempIPos; IPos[index]=-1.0;
    IndMF=TempMF; IndMF[index]=NULL;
    return index;
  }
} //TGenStrip::AddVPInd

int TGenStrip::RemoveIndIndex(int IndIndex)
{ //Removes Ind[IndIndex]; return ICount or -1 if IndIndex invalid
  int *TempInd; int *TempIPos; TMetafile **TempMF;
  int j;
  if (IndIndex<0 || IndIndex>=ICount) return -1;
  else
  { if (ICount==1)
    { ICount=0;
      delete[] Ind; Ind=NULL;
      delete[] IPos; IPos=NULL;
      delete IndMF[0]; delete[] IndMF; IndMF=NULL;
    }
    else
    { TempInd = new int[--ICount];
      TempIPos = new int[ICount];
      TempMF = new TMetafile* [ICount];
      for (j=0; j<IndIndex; j++)
      { TempInd[j]=Ind[j]; TempIPos[j]=IPos[j]; TempMF[j]=IndMF[j]; }
      for (j=IndIndex; j<ICount; j++)
      { TempInd[j]=Ind[j+1]; TempIPos[j]=IPos[j+1]; TempMF[j]=IndMF[j+1]; }
      delete[] Ind; delete[] IPos;
      delete IndMF[IndIndex]; delete[] IndMF;
      Ind=TempInd; IPos=TempIPos; IndMF=TempMF;
    }
    return ICount;
  }
} //TGenStrip::RemoveIndIndex

int TGenStrip::RemoveVPInd(int VPI)
//remove VPInd[VPI] form Ind-array; return old IndIndex(VPI), or -1 if not present
{ int j;
  j=GetIndIndex(VPI);
  RemoveIndIndex(j);
  return j;
} //TGenStrip::RemoveIndNum

inline int TGenStrip::GetIndIndex(int VPI)
{ int j;
  j=0;
  while (j<ICount && Ind[j]!=VPI) j++;
  if (j==ICount) j=-1;
  return j;
} //TGenStrip::GetIndIndex

// ******** TCrossStrip ************

TCrossStrip::TCrossStrip(void)
{ CCount=0; Cross=NULL;
  CGPos=-1.0; CIPos=NULL;
}

TCrossStrip::~TCrossStrip(void)
{ delete[] Cross; delete[] CIPos; }

TCrossStrip::TCrossStrip(TCrossStrip &Source)
{ ShowMessage("Warning: TCrossStrip copy-constructor not implemented"); }

int TCrossStrip::AddCross(TCrossing *Cro, int index)
{ TCrossing **TempCross; int j; int *TempCIPos;
  if (Cro==NULL) return -1;
  else
  { TempCross = new TCrossing* [++CCount];
    TempCIPos = new int [CCount];
    if (index<0 || index>=CCount) index=CCount-1; //add at end if not (correctly) specified
    for (j=0; j<index; j++)
    { TempCross[j]=Cross[j];
      TempCIPos[j]=CIPos[j];
    }
    for (j=index+1; j<CCount; j++)
    { TempCross[j]=Cross[j-1];
      TempCIPos[j]=CIPos[j-1];
    }
    delete[] Cross; delete[] CIPos;
    Cross=TempCross; Cross[index]=Cro; TempCross=NULL;
    CIPos=TempCIPos; CIPos[index]=-1.0; TempCIPos=NULL;
    return index;
  }
} //TCrossStrip::AddCross

int TCrossStrip::RemoveCrossIndex(int CrIndex)
{ //Removes Ind[IndIndex]; return ICount or -1 if IndIndex invalid
  TCrossing **TempCross; int *TempCIPos;
  int j;
  if (CrIndex<0 || CrIndex>=CCount) return -1;
  else
  { if (CCount==1)
    { CCount=0;
      delete[] Cross; Cross=NULL;
      delete[] CIPos; CIPos=NULL;
    }
    else
    { TempCross = new TCrossing* [--CCount];
      TempCIPos = new int[CCount];
      for (j=0; j<CrIndex; j++)
      { TempCross[j]=Cross[j]; TempCIPos[j]=CIPos[j]; }
      for (j=CrIndex; j<CCount; j++)
      { TempCross[j]=Cross[j+1]; TempCIPos[j]=CIPos[j+1]; }
      delete[] Cross; delete[] CIPos;
      Cross=TempCross; CIPos=TempCIPos;
    }
    return CCount;
  }
} //TCrossStrip::RemoveCrossIndex

int TCrossStrip::RemoveCross(TCrossing *Cro)
//remove *Cro from Cross-array; return old index, or -1 if not present
{ int j;
  j=GetCrossIndex(Cro);
  RemoveCrossIndex(j);
  return j;
} //TCrossStrip::RemoveCross

inline int TCrossStrip::GetCrossIndex(TCrossing *Cro)
{ int j;
  j=0;
  while (j<CCount && Cross[j]!=Cro) j++;
  if (j==CCount) j=-1;
  return j;
} //TCrossStrip::GetCrossIndex

// ******** TVPIndList, TVPCrossList ************

TVPIndListItem* TVPIndList::GetVPIndListItem(int Index)
{ return (TVPIndListItem*) Items[Index]; }

bool TVPIndListItem::Selected(void)
{ return CurrentViewPop->Selected[VPIndIndex]; }

int TVPIndList::GetIndex(int VPI)
//VPI is index into VPInd
//return current index in list where VPINdIndex==VPI, -1 if none
{ int i,VPj;
  //i=Count-1;
  //while ( i>=0 && Ind[i]->VPIndIndex != VPI ) i--;
  for (i=Count-1; i>=0; i--)
  { VPj=Ind[i]->VPIndIndex;
    if (VPj == VPI) break;
  }
  return i;
}

//void TVPIndList::SetVPIndListItem(int Index, TVPIndListItem* It)
//probably not useful: you can just pass a pointer to your own objecttype
//to the Items array, which is a list of void pointers
//{ Items[Index] = It; }

TVPCrossListItem* TVPCrossList::GetVPCrossListItem(int Index)
{ return (TVPCrossListItem*) Items[Index]; }

// ******** TViewPop ************

TViewPop::TViewPop(TTreeNode *Node, int VPnumber)
{  //if VPnumber<0 a unique number will be generated
   //ParentViewPop=ParentVP;
   //ParentViewPop->AddSubViewPop(this); 
   if (VPnumber<0) VPnum=VPCount++;
   else
   { VPnum=VPnumber;
     if (VPnum>=VPCount) VPCount=VPnum+1;
   }
   VPNode=Node;
   if (Node!=NULL) Node->Data=this;
   //SubViewPop=NULL; SubVPCount=0;
   LastSubpopNumber=0;
   Flag=0;
   VW=NULL; VWCount=0; CurrVW=-1;
   VPInd=NULL; Selected=NULL; VPIndCount=0; Sbt=NULL;
   LastSelCount=0; LastSelection=NULL;
   IL = new TVPIndList;   //Indivs are added to list in AddVPInd
   NumberSortUp=false; NameSortUp=false;
   ParentSortUp[0]=false; ParentSortUp[1]=false;
   DataSortUp = new bool[IndivDataCount];
   Ordering=ordSubtreesDown;
   for (int d=0; d<IndivDataCount; d++) DataSortUp[d]=false;
   CL = new TVPCrossList; //list is filled in first call to VW[x]->FillGenerations
   FocusInd = -1; //FocusCross = NULL;
   Notes = new TStringList;
} //TViewPop::TViewPop

TViewPop::TViewPop()
//Creates a temporary ViewPop unlinked to a Node and without increasing VPCount
{  VPnum=-2; VPNode=NULL; //signal for temp VP
   LastSubpopNumber=0;
   Flag=0;
   VW=NULL; VWCount=0; CurrVW=-1;
   VPInd=NULL; Selected=NULL; VPIndCount=0; Sbt=NULL;
   LastSelCount=0; LastSelection=NULL;
   IL = new TVPIndList;   //Indivs are added to list in AddVPInd
   NumberSortUp=false; NameSortUp=false;
   ParentSortUp[0]=false; ParentSortUp[1]=false;
   DataSortUp = new bool[IndivDataCount];
   Ordering=ordSubtreesDown;
   for (int d=0; d<IndivDataCount; d++) DataSortUp[d]=false;
   CL = new TVPCrossList; //list is filled in first call to VW[x]->FillGenerations
   FocusInd = -1; //FocusCross = NULL;
   Notes = NULL;
} //TViewPop::TViewPop

__fastcall TViewPop::~TViewPop(void)
{ int i;
  /*delete all Sub-populations:
  for (i=0; i<SubVPCount; i++) delete SubViewPop[i];
  delete[] SubViewPop; SubViewPop=NULL; SubVPCount=0;
  */
  //delete contents:
  for (i=VWCount-1; i>=0; i--) delete VW[i];
  delete[] VW; delete[] Sbt;
  for (i=0; i<IL->Count; i++) delete IL->Ind[i];
  delete IL;
  delete[] VPInd;
  for (int i=0; i<CL->Count; i++) delete CL->Cro[i];
  delete CL;
  delete[] DataSortUp;
  delete[] Selected;
  delete[] LastSelection;
  delete Notes;
  /*remove this from parent population's subpopulations:
  ParentViewPop->RemoveSubViewPop(this);
  //remove corresponding node in treeview:
  NwMainForm->PopTreeView->Items->Delete(VPNode);
  */
  if (CurrentViewPop==this) CurrentViewPop=NULL;
  if (SourceViewPop==this) SourceViewPop=NULL;
} //TViewPop::~TViewPop

TViewPop::TViewPop(TViewPop &Source)
{ ShowMessage("Warning: TViewPop copy-constructor not implemented"); }


/*void TViewPop::AddSubViewPop(TViewPop* SubVP)
//only adds SubVP to array SubViewPop; does not Create SubVP
{ /*int i;
  TViewPop **Temp;
  Temp=SubViewPop;
  SubViewPop = new TViewPop* [++SubVPCount];
  for (i=0; i<SubVPCount-1; i++) SubViewPop[i]=Temp[i];
  delete[] Temp;
  SubViewPop[SubVPCount-1] = SubVP;
} //TViewPop::AddSubViewPop


bool TViewPop::RemoveSubViewPop(TViewPop* SubVP)
//only removes SubVP from array SubViewPop; does not Destroy SubVP
{ /*int i,vp;
  TViewPop **Temp;
  vp=0;
  while (vp<SubVPCount && SubViewPop!=SubVP) vp++;
  if (vp>=SubVPCount) return false;
  Temp=SubViewPop;
  SubViewPop = new TViewPop* [--SubVPCount];
  for (i=0; i<vp; i++) SubViewPop[i]=Temp[i];
  for (i=vp; i<SubVPCount; i++) SubViewPop[i]=Temp[i+1];
  delete[] Temp;
} //TViewPop::DeleteSubViewPop
*/

int TViewPop::AddVW(AnsiString ViewName, bool Overview)
{ TView* *Temp;
  int g;
  Temp = new TView* [++VWCount];
  for (g=0; g<VWCount-1; g++) Temp[g]=VW[g];
  Temp[VWCount-1]=new TView(this, ViewName, Overview);
  delete[] VW;
  VW=Temp;
  CurrVW=VWCount-1; //the new View
  return VWCount;
} //TViewPop::AddVW

int TViewPop::DeleteVW(AnsiString ViewName)
{ TView* *Temp;
  int g,v;
  g=1; //never delete 0=Overview
  while (g<VWCount && VW[g]->ViewName!=ViewName) g++;
  if (g<VWCount)
  { if (CurrVW>=g) CurrVW--;
    delete VW[g];
    Temp = new TView* [--VWCount];
    for (v=0; v<g; v++) Temp[v]=VW[v];
    for (v=g; v<VWCount; v++) Temp[v]=VW[v+1];
    delete[] VW;
    VW=Temp; Temp=NULL;
  }
  return VWCount;
} //TViewPop::DeleteVW

int TViewPop::AddVPInd(int I)
// I is index of individual in global Indiv array
// I is inserted so that VPInd remains sorted in ascending order
{ int j,VPI;
  int *Temp;
  TVPIndListItem* ILI;

  if (I<0 || I>=IndivCount) throw Exception("TViewPop::AddVPInd : I not valid");
  Temp = new int [++VPIndCount];
  j=0;
  while (j<VPIndCount-1 && VPInd[j]<I) {Temp[j]=VPInd[j]; j++;}
  if (j<VPIndCount-1 && VPInd[j]==I) throw Exception("TViewPop::AddVPInd : Indiv["+IntToStr(I)+"] already in VPInd");
  Temp[j]=I; VPI=j;
  while (j<VPIndCount-1) Temp[j+1]=VPInd[j++];
  delete[] VPInd;
  VPInd=Temp;
  delete[] Selected;
  Selected = new bool [VPIndCount];

  //now the new I has been added to VPInd at position VPI;
  //it still must be added to the IndivList IL :
  ILI = new TVPIndListItem;
  ILI->VPIndIndex=VPI;
  IL->Add(ILI);
  //new, 8-2-2010:
  if (IL->Count != VPIndCount)
    throw new Exception("Error in AddVPInd");
  //re-fill the IL in order of VPInd:
  for (j=0; j<VPIndCount; j++) {
        IL->Ind[j]->VPIndIndex=j;
        IL->Ind[j]->SortRank=j;
        Selected[j]=false;
  }
  //TODO: how to know that the last sort was on IndivNumber,
  //and how to show that in the list view?
  //For the moment all seems to work well,
  //probably because individuals are never added into an already
  //existing (and already sorted) subpopulation
  //end 8-2-2010
  /* old version:
  //after adding, sort back to VPInd order:
  for (j=0; j<VPIndCount; j++)
  { Selected[j]=false;
    IL->Ind[j]->SortRank = j;
  }
  end old version */
  return VPI;
} //TViewPop::AddVPInd

int TViewPop::GetVPIndex(int I)
// I is index of individual in global Indiv array
// Return value is the index in the VP->VPInd array, or -1 if not in VP
{ int j;
  if (I<0 || I>=IndivCount) return -1;
  //to be changed to binary search !!
  j=0;
  while (j<VPIndCount && VPInd[j]<I) j++;
  return j<VPIndCount && VPInd[j]==I ? j : -1 ;
}

int TViewPop::GetVPIndex(AnsiString Name)
// Name is name of individual
// Return value is the index in the VP->VPInd array, or -1 if not in VP
{ int j;
  j=0;
  while (j<VPIndCount && Indiv[VPInd[j]].IName!=Name) j++;
  return (j<VPIndCount) ? j : -1 ;
}

bool TViewPop::Present(int I)
// I is index of individual in global Indiv array
{ return GetVPIndex(I) != -1 ; }

bool TViewPop::IsVPFounder(int VPI)
//VPI is index into VPInd
//Semi-founders are NOT VPfounders!
{ TCrossing *Cro;
  if (VPI<0 || VPI>=VPIndCount) throw Exception("TViewPop::IsVPFounder: invalid VPI");
  Cro=Indiv[VPInd[VPI]].ParentCross;
  if (Cro==NULL) return true;
  return !( (Cro->Parent[0]>=0 && Present(Cro->Parent[0])) ||
            (Cro->Parent[1]>=0 && Present(Cro->Parent[1])) ) ;
} //TViewPop::IsVPFounder

bool TViewPop::IsVPSemifounder(int VPI)
//VPI is index into VPInd
//Semi-founders are NOT VPfounders!
{ TCrossing *Cro;
  int ParentCount;
  if (VPI<0 || VPI>=VPIndCount) throw Exception("TViewPop::IsVPFounder: invalid VPI");
  Cro=Indiv[VPInd[VPI]].ParentCross;
  if (Cro==NULL || Cro->CType!=ctCross) return false; //if uniparental, semifounder impossible
  else
  { ParentCount=0;
    if (Cro->Parent[0]>=0 && Present(Cro->Parent[0])) ParentCount++;
    if (Cro->Parent[1]>=0 && Present(Cro->Parent[1])) ParentCount++;
    return ParentCount==1;
  }
} //TViewPop::IsVPSemiounder

void TViewPop::MarkFounders(int VPI)
//VPI is index into VPInd
//Marks all ancestors of VPI that are VPFounders (founders in this VP)
//Called only from TViewPop::MarkSubtree; no error checking, array Sbt already created
//recursively calls itself until VPI is founder
{ TCrossing *Cro;
  int vp0,vp1;

  Cro=Indiv[VPInd[VPI]].ParentCross;
  if (Cro==NULL) {Sbt[VPI]=true; return;} //VPI is founder of whole population
  vp0=GetVPIndex(Cro->Parent[0]); vp1=GetVPIndex(Cro->Parent[1]);
  if (vp0<0 && vp1<0) {Sbt[VPI]=true; return;} //VPI is founder in VPInd
  if (vp0>=0) MarkFounders(vp0);
  if (vp1>=0 && vp1!=vp0) MarkFounders(vp1);
} //TViewPop::MarkFounders

bool TViewPop::MarkSubtree(int VPI)
//Marks the whole subtree in VP of which VPI is a member (back to all ancestors
//of VPI that are VPFounders, and all other VPInd that are only descended from
//these VPFounders (and possibly from individuals outside VP)
//Uses the fact that progeny appears after parents in VPInd

{ int vi,vm,m,p,vp;
  TCrossing *Cro;

  if (Sbt==NULL) Sbt = new bool [VPIndCount];
  for (vi=0; vi<VPIndCount; vi++)
  { Sbt[vi]=false;
    Cro=Indiv[VPInd[vi]].ParentCross;
    if (Cro!=NULL) Cro->Marked=false;
  }
  if (VPI<0 || VPI>=VPIndCount) return false;
  MarkFounders(VPI);
  for (vi=0; vi<VPIndCount; vi++)
  { if (Sbt[vi])
    { for (m=0; m<Indiv[VPInd[vi]].MCount; m++)
      { Cro=Indiv[VPInd[vi]].Mating[m];
        //process this mating if
        //-uni-parental (selfing etc.); or
        //-other parent unknown, or known but not in VPInd; or
        //-other parent in Subtree
        //and in all cases only if not processed before (Marked=false) to save work
        //aim: mark all children of vi that are not already marked,
        //and which are part of the current subtree
        //(for which the other parent is not (in VP but outside subtree) )
        if ( !Cro->Marked )
        { vm=GetVPIndex(Cro->Parent[0]);
          if (vm==vi) vm=GetVPIndex(Cro->Parent[1]);
          if (vm<0 || Sbt[vm] )
          { for (p=0; p<Cro->OCount; p++)
            { Cro->Marked=true;
              if ( (vp=GetVPIndex(Cro->Offsp[p])) != -1 )
                 Sbt[vp]=true;
            }
          }
        }
      } //for m
    } //Sbt[vi] true
  } //for vi
  return true; //no error
} //TViewPop::MarkSubtree

int TViewPop::MinGenerDist(int *Gener)
//Calculates minimum generation distance between individuals in
//current Subtree and their offspring outside current subtree.
//uses the Gener array of the calling TView
{ int Dist=MAXINT;
  int vi,vm,m,p,vp,j;
  TCrossing *Cro;
  //go through VPInd in similar way as in MarkSubtree
  for (vi=0; vi<VPIndCount; vi++)
  { if (Sbt[vi])
    { for (m=0; m<Indiv[VPInd[vi]].MCount; m++)
      { Cro=Indiv[VPInd[vi]].Mating[m];
        //process this mating if other parent in VPInd but not in Sbt:
        //only then is progeny of this mating outside subtree
        vm=GetVPIndex(Cro->Parent[0]);
        if (vm==vi) vm=GetVPIndex(Cro->Parent[1]);
        if (vm>=0 && !Sbt[vm])
        { //if any progeny of this mating in VPInd: j is generation distance to this progeny
          //assumes that all VP-progeny of Cro is in same Generation,
          //so stop when first is found
          j=MAXINT; p=0;
          while (j==MAXINT && p<Cro->OCount)
          { if ( (vp=GetVPIndex(Cro->Offsp[p++])) != -1 )
               j=Gener[vp]-Gener[vi];
          }
          if (j<Dist) Dist=j;
        } //Mate outside subtree
      }
    } //for m
  } //for vi
  return Dist;
} //TViewPop::MinGenerDist

void TViewPop::SelectVPAncestors(int VPI, int Gen, int Parentline, bool Select)
// recursively finds all VPAncestors of VPInd[VPI] up to Gen generations back
// if Select=true, they will be selected, if false, deselected
{ int vp0,vp1;
  TCrossing* Cro;
  if ( VPI<0 || VPI>=VPIndCount || Gen<=0 ||
       (Cro=Indiv[VPInd[VPI]].ParentCross)==NULL) return; //VPI is founder of whole population
  vp0=GetVPIndex(Cro->Parent[0]); vp1=GetVPIndex(Cro->Parent[1]);
  if (vp0>=0 && vp1==vp0)
  { Selected[vp0]=Select;
    SelectVPAncestors(vp0,Gen-1,Parentline,Select);
  }
  else
  { if (vp0>=0 && Parentline!=1)
    { Selected[vp0]=Select;
      SelectVPAncestors(vp0,Gen-1,Parentline,Select);
    }
    if (vp1>=0 && Parentline!=0)
    { Selected[vp1]=Select;
      SelectVPAncestors(vp1,Gen-1,Parentline,Select);
    }
  }
  return;
    } //TViewPop::SelectVPAncestors

void TViewPop::SelectVPProgeny(int VPI, int Gen, int Parentline, bool Select, bool WithColdParent)
// recursively finds all VPprogeny of VPInd[VPI] up to Gen generations down
// if Select=true, they will be selected, if false, deselected
// if WithColdParents, then also the "other" or "cold" parent of each progeny
// is (de-)selected
{ int vp,vp0,vp1,m,p;
  TCrossing* Cro;
  bool VPprog;
  if ( Gen<=0 || VPI<0 || VPI>=VPIndCount ) return;
  for (m=0; m<Indiv[VPInd[VPI]].MCount; m++)
  { Cro=Indiv[VPInd[VPI]].Mating[m];
    vp0=GetVPIndex(Cro->Parent[0]); vp1=GetVPIndex(Cro->Parent[1]);
    if ( (vp0==VPI && Parentline!=1) ||
         (vp1==VPI && Parentline!=0) )
    {
       VPprog=false;
       for (p=0; p<Cro->OCount; p++)
       { vp = GetVPIndex(Cro->Offsp[p]);
         if (vp>=0)
         { VPprog=true; //there is progeny of this cross in VP, so cold parent must be done
           Selected[vp]=Select;
           SelectVPProgeny(vp, Gen-1, Parentline, Select, WithColdParent);
         }
       } //for p
       if (WithColdParent && VPprog)
       { if (vp0>=0 && vp0!=VPI) Selected[vp0]=Select;
         if (vp1>=0 && vp1!=VPI) Selected[vp1]=Select;
       }
    } //if (Parentline ok)
  } //for m
} //TViewPop::SelectVPProgeny

void TViewPop::SelectVPSibs(int VPI, bool Select)
// finds all VPSibs of VPInd[VPI]
// if Select=true, they will be selected, if false, deselected
{ int p,vp;
  TCrossing* Cro;
  if ( VPI<0 || VPI>=VPIndCount ||
       (Cro=Indiv[VPInd[VPI]].ParentCross)==NULL ) return; //VPI is founder of whole population
  for (p=0; p<Cro->OCount; p++)
  { if ( (vp=GetVPIndex(Cro->Offsp[p])) != -1 )
       Selected[vp]=Select;
  }
} //TViewPop::SelectVPSibs

void TViewPop::SelectedToSet(int* &Set, int &Count)
{ int i,j;
  delete[] Set; Set=NULL; Count=0;
  for (i=0; i<VPIndCount; i++)
    if (Selected[i]) Count++;
  Set = new int[Count];
  j=0;
  for (i=0; i<VPIndCount; i++)
    if (Selected[i]) Set[j++] = VPInd[i];
} //TViewPop::SelectedToSet

void TViewPop::UpdateSelection(int IgnoreVPInd)
// updates LastSelection, LastSelCount and SelScript with Manual lines;
// IgnoreVPInd is the BaseInd of an Auto selection, when called
// from ToolsSelectRelativesExecute;
// This individual is not included in the manual (de-) selection lines as it will
// be Auto (de-) selected anyway.
// In other cases, IgnoreVPInd must be -1: none will be ignored.
{ int NewSelCount=0, *NewSel=NULL, i,
      GainedCount,LostCount, *Gained=NULL, *Lost=NULL;

  SelectedToSet(NewSel,NewSelCount);
  CompareIntSets(LastSelection, LastSelCount, NewSel, NewSelCount,
                      Gained, GainedCount, Lost, LostCount);
    //make manual line if not only IgnoreVPInd lost:
    if (LostCount>1 ||
       (LostCount==1 && (IgnoreVPInd==-1 || Lost[0]!=VPInd[IgnoreVPInd])))
    { SelScript.AddSelScriptLine();
      i=SelScript.LineCount-1;
      SelScript.SSLine[i].Select=false;
      SelScript.SSLine[i].IndCount=LostCount;
      SelScript.SSLine[i].IndList=Lost; Lost=NULL;
    }
    //make manual line if not only IgnoreVPInd gained:
    if (GainedCount>1 ||
       (GainedCount==1 && (IgnoreVPInd==-1 ||Gained[0]!=VPInd[IgnoreVPInd])))
    { SelScript.AddSelScriptLine();
      i=SelScript.LineCount-1;
      SelScript.SSLine[i].Select=true;
      SelScript.SSLine[i].IndCount=GainedCount;
      SelScript.SSLine[i].IndList=Gained; Gained=NULL;
    }
    delete[] LastSelection; LastSelection=NewSel; NewSel=NULL;
    LastSelCount=NewSelCount;
    delete[] Gained; delete[] Lost;
} //TViewPop::UpdateSelection

void TViewPop::ClearSelection(void)
{ int i;
  for (i=0; i<VPIndCount; i++) Selected[i]=false;
  SelScript.Clear();
  delete[] LastSelection;
  LastSelection=NULL;
  LastSelCount=0;
} //TViewPop::ClearSelection

void TViewPop::WriteData(TStringList *SL)
{ AnsiString s; int i;
  SL->Add("");
  SL->Add("[SUBPOP "+IntToStr(VPnum)+" ]");
  if (VPNode->Parent==NULL) s="-1";
  else s= IntToStr( ((TViewPop*) (VPNode->Parent->Data))->VPnum);
  SL->Add("Name "+QuotedString(VPNode->Text));
  SL->Add("Ordering "+IntToStr(Ordering));
  SL->Add("LastSub "+IntToStr(LastSubpopNumber));
  SL->Add("Flag "+IntToStr(Flag));
  SL->Add("Origin "+s);
  OrigScript.WriteLines(false,SL);
  SL->Add(""); SL->Add("[Notes " + IntToStr(Notes->Count) +" lines]");
  for (i=0; i<Notes->Count; i++) SL->Add(Notes->Strings[i]);
  SL->Add("[Notes_end]");
  SL->Add("");
  SL->Add("CurrentView "+IntToStr(CurrVW));
  for (i=1; i<VWCount; i++) //not from 0: Overview not saved, recalculated from default Overview options
  { SL->Add(""); SL->Add("[View "+DQ+VW[i]->ViewName+DQ+"]"); //not with QuotedString: problems with reading final ']'
    VW[i]->Opt.WriteData(SL,false);
    SL->Add("[View_end]");
  }
  SL->Add(""); SL->Add("[SUBPOP_END]");
} //TViewPop::WriteData

void TViewPop::ChangeOrdering(int NewOrdering)
// parameter list must be extended to accommodate ordManual, with list of
// Generations with Indivs
{ if (NewOrdering != Ordering)
  { Ordering = NewOrdering;
    //eliminate all existing charts for all Views;
    //if currentView >0, recalculate chart immediately
  }
} //TViewPop::ChangeOrdering

TStringList* TViewPop::MakeIndListSL(bool Header)
//makes a tab-separated text version of the Indiv listview;
//analogous to TNwMainForm::MainListviewDataI
{ int I,d,n;
  TCrossing *Cro;
  TStringList *SL;
  AnsiString s;

  SL = new TStringList;

  if (Header)
  { s="Nr"+Tb+"Name"+Tb+ParentCaption(0)+Tb+ParentCaption(1);
    for (d=0; d<IndivDataCount; d++) s=s+Tb+IndivData[d].Caption;
    SL->Add(s);
  }

  for (n=0; n<VPIndCount; n++)
  { s="";
    try
    { I=VPInd[IL->Ind[n]->VPIndIndex];
      s=IntToStr(I)+Tb+Indiv[I].IName;
      Cro=Indiv[I].ParentCross;
      if (Cro==NULL) s=s+Tb+strUnknown[0]+Tb+strUnknown[0];
      else
      { if (Cro->Parent[0]<0 || GetVPIndex(Cro->Parent[0])<0)
           s=s+Tb+strUnknown[0];
        else s=s+Tb+Indiv[Cro->Parent[0]].IName;
        if (Cro->CType==ctCross)
        { if (Cro->Parent[1]<0 || GetVPIndex(Cro->Parent[1])<0)
             s=s+Tb+strUnknown[0];
          else s=s+Tb+Indiv[Cro->Parent[1]].IName;
        }
        else if (Cro->CType==ctSelfing) s=s+Tb+Indiv[Cro->Parent[0]].IName;
        else s=s+Tb+CrossTypeString[Cro->CType];
      }
      for (d=0; d<IndivDataCount; d++) s=s+Tb+DataToStr(d,I,strUnknown[0]);
    }
    catch (...) { s="Error at Indiv "+IntToStr(n); }
    SL->Add(s);
  }
  return SL;
} //TViewPop::MakeIndListSL

int TViewPop::CopyIndivs(TViewPop* VP, bool CompleteSemifounders)
//Copies all individuals from VP to the current ViewPop;
//if CompleteSemifounders==true the second parent of each semi-founder of VP is
//added, if it exists in the complete population.
//Returns the number of added ancestors; negative if not all semifounders
//could be completed
{ int vpi,p,Added;
  TCrossing *Cro;
  bool Success, NewAncestors;

  Added=0;
  for (vpi=0; vpi<VP->VPIndCount; vpi++)
    AddVPInd(VP->VPInd[vpi]);
  if (CompleteSemifounders==false) Success=true;
  else
  { do //outer loop: check for all individuals if they are semi-founder;
       //continue loop until no further ancestors have been added
    { NewAncestors=false; Success=true;
      vpi=0; //index in VPInd
      do //inner loop: check each individual for semi-founderness
      { if (IsVPSemifounder(vpi))
        { Cro=Indiv[VPInd[vpi]].ParentCross;
          //if (Cro==NULL || Cro->CType!=ctCross) Success=false; cannot occur!
          //which parent is lacking?
          if (Cro->Parent[0]<0 || !Present(Cro->Parent[0])) p=0;
          else p=1;
          //if exists in base population, add parent[p]:
          if (Cro->Parent[p]>=0)
          { AddVPInd(Cro->Parent[p]);
            NewAncestors=true;
            Added++;
          }
          else Success=false;
        }
        vpi++;
      }
      while (vpi<VPIndCount);
    }
    while (NewAncestors==true);
  }
  if (Added==0 && !Success) return -MAXINT;
  else if (!Success) return -Added;
  else return Added;
} //TViewPop::CopyIndivs

// ++++++++++ TViewPop: here follow Compare functions for SortIL ++++++++++

//#define COMPAR(a,b) (((a)>(b)) ? 1 : (((a)<(b)) ? -1 : 0))
inline int ComparChar (char a, char b)
       { return (a>b) ? 1 : ((a<b) ? -1 : 0); }
inline int ComparInt (int a, int b)
       { return (a>b) ? 1 : ((a<b) ? -1 : 0); }
inline int ComparFloat (double a, double b)
       { return (a>b) ? 1 : ((a<b) ? -1 : 0); }
inline int ComparStr (AnsiString a, AnsiString b)
       { return (a>b) ? 1 : ((a<b) ? -1 : 0); }

//Globals, set just before sorting:
int SortDir,     //sort up (+1) or down (-1)
    SortField;   //an index of the field, used in ParentSort and the DataSorts

int __fastcall CompareINumbers(void *Item1, void *Item2)
{ int i1,i2,a;
  i1=CurrentViewPop->VPInd [((TVPIndListItem *) Item1)->VPIndIndex];
  i2=CurrentViewPop->VPInd [((TVPIndListItem *) Item2)->VPIndIndex];
  a = ComparInt(i1,i2);
  if (a==0)
    a = ComparInt( ((TVPIndListItem *) Item1)->SortRank,
                   ((TVPIndListItem *) Item2)->SortRank  );
  return SortDir*a;
} //CompareINames

void TViewPop::SortINumber(void)
{ SortDir = (NumberSortUp) ? -1 : 1;
  IL->Sort(CompareINumbers);
  NumberSortUp = !NumberSortUp;
} //TViewPop::SortINumber

int __fastcall CompareINames(void *Item1, void *Item2)
{ int i1,i2,a;
  i1=CurrentViewPop->VPInd [((TVPIndListItem *) Item1)->VPIndIndex];
  i2=CurrentViewPop->VPInd [((TVPIndListItem *) Item2)->VPIndIndex];
  a = ComparStr(Indiv[i1].IName, Indiv[i2].IName);
  if (a==0)
    a = ComparInt( ((TVPIndListItem *) Item1)->SortRank,
                   ((TVPIndListItem *) Item2)->SortRank  );
  return SortDir*a;
} //CompareINames

int __fastcall CompareINameNums(void *Item1, void *Item2)
{ int i1,i2,a;
  i1=CurrentViewPop->VPInd [((TVPIndListItem *) Item1)->VPIndIndex];
  i2=CurrentViewPop->VPInd [((TVPIndListItem *) Item2)->VPIndIndex];
  a = ComparInt(Indiv[i1].INameNum, Indiv[i2].INameNum);
  if (a==0)
    a = ComparInt( ((TVPIndListItem *) Item1)->SortRank,
                   ((TVPIndListItem *) Item2)->SortRank  );
  return SortDir*a;
} //CompareINames

void TViewPop::SortIName(void)
{ SortDir = (NameSortUp) ? -1 : 1;
  if (INameNumbers) IL->Sort(CompareINameNums);
  else IL->Sort(CompareINames);
  NameSortUp = !NameSortUp;
}

AnsiString GetParName(int I)
{ TCrossing *Cro; int j;
  if ((Cro=Indiv[I].ParentCross)==NULL) return ""; //unknown parent
  else
  { if (SortField==1 && Cro->CType>ctSelfing) return " "; //no parent
    else
    { if ((j=Cro->Parent[SortField])<0) return ""; //unknown parent
      else return Indiv[j].IName;
    }
  }
} //GetParName

int GetParNameNum(int I)
{ TCrossing *Cro; int j;
  if ((Cro=Indiv[I].ParentCross)==NULL) return -MAXINT; //unknown parent
  else
  { if (SortField==1 && Cro->CType>ctSelfing) return -MAXINT+1; //no parent
    else
    { if ((j=Cro->Parent[SortField])<0) return -MAXINT; //unknown parent
      else return Indiv[j].INameNum;
    }
  }
} //GetParNameNum

int __fastcall CompareParNames(void *Item1, void *Item2)
{ int i1,i2,a;
  i1=CurrentViewPop->VPInd [((TVPIndListItem *) Item1)->VPIndIndex];
  i2=CurrentViewPop->VPInd [((TVPIndListItem *) Item2)->VPIndIndex];
  a = ComparStr(GetParName(i1), GetParName(i2));
  if (a==0)
    a = ComparInt( ((TVPIndListItem *) Item1)->SortRank,
                   ((TVPIndListItem *) Item2)->SortRank  );
  return SortDir*a;
} //CompareParNameNums

int __fastcall CompareParNameNums(void *Item1, void *Item2)
{ int i1,i2,a;
  i1=CurrentViewPop->VPInd [((TVPIndListItem *) Item1)->VPIndIndex];
  i2=CurrentViewPop->VPInd [((TVPIndListItem *) Item2)->VPIndIndex];
  a = ComparInt(GetParNameNum(i1), GetParNameNum(i2));
  if (a==0)
    a = ComparInt( ((TVPIndListItem *) Item1)->SortRank,
                   ((TVPIndListItem *) Item2)->SortRank  );
  return SortDir*a;
} //CompareParNameNums

void TViewPop::SortIParent(int Par)
{ SortDir = (ParentSortUp[Par]) ? -1 : 1;
  SortField = Par;
  if (INameNumbers) IL->Sort(CompareParNameNums);
  else IL->Sort(CompareParNames);
  ParentSortUp[Par] = !ParentSortUp[Par];
} //TViewPop::SortIParent

int __fastcall CompareChars(void *Item1, void *Item2)
{ int i1,i2,a;
  i1=CurrentViewPop->VPInd [((TVPIndListItem *) Item1)->VPIndIndex];
  i2=CurrentViewPop->VPInd [((TVPIndListItem *) Item2)->VPIndIndex];
  a = ComparChar(Ichar[SortField][i1],Ichar[SortField][i2]);
  if (a==0) return ComparInt( ((TVPIndListItem *) Item1)->SortRank,
                              ((TVPIndListItem *) Item2)->SortRank  );
  else return SortDir*a;
} //CompareChars

int __fastcall CompareInts(void *Item1, void *Item2)
{ int i1,i2,a;
  i1=CurrentViewPop->VPInd [((TVPIndListItem *) Item1)->VPIndIndex];
  i2=CurrentViewPop->VPInd [((TVPIndListItem *) Item2)->VPIndIndex];
  a = ComparInt(Iint[SortField][i1],Iint[SortField][i2]);
  if (a==0) return ComparInt( ((TVPIndListItem *) Item1)->SortRank,
                              ((TVPIndListItem *) Item2)->SortRank  );
  else return SortDir*a;
} //CompareInts

int __fastcall CompareFloats(void *Item1, void *Item2)
{ int i1,i2,a;
  i1=CurrentViewPop->VPInd [((TVPIndListItem *) Item1)->VPIndIndex];
  i2=CurrentViewPop->VPInd [((TVPIndListItem *) Item2)->VPIndIndex];
  a = ComparFloat(Ifloat[SortField][i1],Ifloat[SortField][i2]);
  if (a==0) return ComparInt( ((TVPIndListItem *) Item1)->SortRank,
                              ((TVPIndListItem *) Item2)->SortRank  );
  else return SortDir*a;
} //CompareFloats

int __fastcall CompareLabels(void *Item1, void *Item2)
{ int i1,i2,a;
  i1=CurrentViewPop->VPInd [((TVPIndListItem *) Item1)->VPIndIndex];
  i2=CurrentViewPop->VPInd [((TVPIndListItem *) Item2)->VPIndIndex];
  a = ComparInt(Ilabel[SortField][i1],Ilabel[SortField][i2]);
  if (a==0) return ComparInt( ((TVPIndListItem *) Item1)->SortRank,
                              ((TVPIndListItem *) Item2)->SortRank  );
  else return SortDir*a;
} //CompareInts

void TViewPop::SortIData(int Field)
{ SortDir= (DataSortUp[Field] ? -1 : 1);
  switch (IndivData[Field].DataType)
  { case dtC: SortField=IndivData[Field].TypeIndex; IL->Sort(CompareChars); break;
    case dtI: SortField=IndivData[Field].TypeIndex; IL->Sort(CompareInts); break;
    case dtF: SortField=IndivData[Field].TypeIndex; IL->Sort(CompareFloats); break;
    case dtS: SortField=IndivData[Field].TypeIndex; IL->Sort(CompareLabels); break;
  }
  DataSortUp[Field] = !DataSortUp[Field];
}


void TViewPop::SortIL(int ColIndex)
{ int i;
  //first: store current sort order, to keep if new sort ranks equal:
  for (i=0; i<VPIndCount; i++)
    IL->Ind[i]->SortRank = i;
  //select sorting routine:
  switch (ColIndex)
  { case 0: SortINumber(); break;
    case 1: SortIName(); break;
    case 2:
    case 3: SortIParent(ColIndex-2); break;
    default: SortIData(ColIndex-4); break;
  }
}




// ******** Color Routines ************

void DecompColor(TColor Col, int &Red, int &Green, int &Blue)
{ Red=   Col % 256;
  Green= (int(Col) /= 256) % 256;
  Blue=  (int(Col) /= 256) % 256;
} //DecompColor

TColor MakeColor (int R, int G, int B)
{ return TColor( ((B * 256) + G) * 256 + R ); }

TColor ColorRange(TColor LoCol, TColor HiCol, float f)
{ int RL,GL,BL, RH,GH,BH, RR,GR,BR;
  TColor Result;
  if (f<=0) Result=LoCol; else
  if (f>=1) Result=HiCol;
  else
  { DecompColor(LoCol, RL,GL,BL);
    DecompColor(HiCol, RH,GH,BH);
    RR = int (RL + (RH-RL)*f);
    GR = int (GL + (GH-GL)*f);
    BR = int (BL + (BH-BL)*f);
    Result = MakeColor(RR,GR,BR);
  }
  return Result;
} //ColorRange

int GreyScale(TColor Col)
{ int R,G,B;
  DecompColor(Col,R,G,B);
  return int(0.299*R + 0.587*G + 0.114*B);
} //GreyScale

int Lighten(int i) //used in LightVersion
{ int r; double d;
  const double Lim=255.99999999;
  /*
  switch (i)
  { case 0: r=160; break;  //128 te donker op papier
    case 128: r=192; break;
    default: r=255;
  }   */
  //r= 255 - (255-i)*3/8; //0->159, 128 -> 207
  //r= 255 - (255-i)/4; //0->191, 128 -> 223
  d= i/255.0; d=d*d; // visual intensity, 0.0 .. 1.0
  d= (1.0+d)/2.0; // visual intensity halfway between d and 1
  r = int(sqrt(d)*Lim); //absolute intensity, 0..255
  return r;
}

TColor LightVersion(TColor Col)
{ int R,G,B;
  DecompColor(Col,R,G,B);
  return MakeColor(Lighten(R),Lighten(G),Lighten(B));
} //LightVersion

void FillColors(int ColCount, TColor *Col,
              TColor LoCol, TColor HiCol , int trait)
/* No error checking;
   IndivData[trait].DataType must be dtC or dtS
   size of array Col must be ColCount
*/
{ int i, LoVal, Range;
  if (IndivData[trait].DataType==dtC && IndivData[trait].V.CV.LevelCount>0)
  { LoVal=IndivData[trait].V.CV.Levels[0];
    Range=IndivData[trait].V.CV.Levels[IndivData[trait].V.CV.LevelCount-1] - LoVal;
    for (i=0; i<ColCount; i++)
        Col[i] = ColorRange(LoCol, HiCol,
            double(IndivData[trait].V.CV.Levels[i]-LoVal)/Range);
  }
  else if (IndivData[trait].DataType==dtS && IndivData[trait].V.SV.LabelCount>0)
  { for (i=0; i<ColCount; i++)
        Col[i] = ColorRange(LoCol, HiCol, double(i)/(ColCount-1));
  }
} //FillColors

// ******** TViewOptions ************

TColor NewColor(int index)
{ TColor C;
  switch (index)
  { case 0: C=clWhite; break;
    case 1: C=clBlack; break;
    case 2: C=MakeColor(255,0,0); break;   //red
    case 3: C=MakeColor(0,255,0); break;   //green
    case 4: C=MakeColor(0,0,255); break;   //blue
    case 5: C=MakeColor(0,255,255); break; //aqua
    case 6: C=MakeColor(255,0,255); break; //fuchsia
    case 7: C=MakeColor(255,255,0); break; //yellow
    case 8: C=MakeColor(255,128,0); break;
    case 9: C=MakeColor(255,0,128); break;
    case 10: C=MakeColor(128,255,0); break;
    case 11: C=MakeColor(128,0,255); break;
    case 12: C=MakeColor(0,255,128); break;
    case 13: C=MakeColor(0,128,255); break;
    default: C=MakeColor(128,128,128);      //gray
  }
  return C;
} //NewColor

/*
void SetDefAlleleColors(TColor* &ACol)
{ int i;
  delete[] ACol;
  ACol = new TColor [IBDAlleleCount];
  for (i=0; i<IBDAlleleCount; i+=2)
  { ACol[i] = NewColor(i/2 + 2);
    if (i+1 < IBDAlleleCount)
    { ACol[i+1] = LightVersion(ACol[i]); }
  }
} //SetDefAlleleColors
*/

void TViewOptions::SetDefTraitColors() {
  //delete any previous TraitCol array and reset the low, high and missong colors:
  if (TraitCol!=NULL) { delete[]TraitCol; TraitCol=NULL; }
  TraitColCount=0;
  LoCol=clBlue;
  HiCol=clRed;
  MVCol=clWhite;
  //set colors based on data type of ColorTrait
  if (ColorTrait<0 || ColorTrait>IndivDataCount-1 ||
      IndivData[ColorTrait].DataType==dtF ||
      IndivData[ColorTrait].DataType==dtI) {
    //no (valid) ColorTrait, or trait of type Float or Integer:
    //no additional action needed
  }
  else { //discrete trait, fill TraitCol array
    if (IndivData[ColorTrait].DataType==dtC &&
        IndivData[ColorTrait].V.CV.LevelCount<=MaxTraitColors)
    { TraitColCount=IndivData[ColorTrait].V.CV.LevelCount;
      TraitCol = new TColor[TraitColCount];
      FillColors(TraitColCount, TraitCol, LoCol, HiCol, ColorTrait);
    }
    else  if (IndivData[ColorTrait].DataType==dtS &&
              IndivData[ColorTrait].V.SV.LabelCount<=MaxTraitColors)
    { TraitColCount=IndivData[ColorTrait].V.SV.LabelCount;
      TraitCol = new TColor[TraitColCount];
      FillColors(TraitColCount, TraitCol, LoCol, HiCol, ColorTrait);
    }
  }
} //SetDefTraitColors

void TViewOptions::SetDefAlleleColors(void)
//new version, for all even ploidy levels
//if HomFounders, assign 1 color per founder,
//else use ploidy/2 dark, and ploidy/2 light versions of same color
{ int i,j,TotCol,
      Rn, Gn, Bn, //number of Red, Green and Blue levels
      Ri, Gi, Bi; //current levels
  bool EvenPloidy;
  const double Lim=255.99999999;
  delete[] ACol;
  ACol = new TColor [IBDAlleleCount];
  EvenPloidy = (Ploidy/2)*2==Ploidy;
  if (HomFounders) TotCol=IBDAlleleCount / Ploidy;
  else if (EvenPloidy) TotCol=(IBDAlleleCount+1)/2; //for every color also a light version is generated
  else TotCol=IBDAlleleCount; //all colors independent
  Rn=2; while (Rn*Rn*Rn < TotCol) Rn++;
  Gn=Rn; Bn=Rn;
  do
  { i=0; //stop sign
    if (Rn*Bn*(Gn-1) > TotCol) {Gn--; i=1;}
    if (Rn*(Bn-1)*Gn > TotCol) {Bn--; i=1;}
  } while (i==1);

  Ri=Gi=Bi=0;
  i=0;
  while (i<IBDAlleleCount)
  //for (i=0; i<IBDAlleleCount; i+=Ploidy)
  { Gi++; if (Gi==Gn)
          {Gi=0; Bi++; if (Bi==Bn)
                       {Bi=0; Ri++;}
    }
    ACol[i]=MakeColor( int(sqrt(1.0*Ri/Rn)*Lim),
                       int(sqrt(1.0*Gi/Gn)*Lim),
                       int(sqrt(1.0*Bi/Bn)*Lim));
    if (HomFounders)
    { for (j=1; j<Ploidy; j++) if (i+j<IBDAlleleCount) ACol[i+j] = ACol[i];
      i+=Ploidy;
    }
    else if (EvenPloidy)
    { ACol[i+1] = LightVersion(ACol[i]);
      i+=2;
    }
    else i++;
  }

/*
{ int i,
      Rn, Gn, Bn, //number of Red, Green and Blue levels
      Ri, Gi, Bi; //current levels
  const double Lim=255.99999999;
  delete[] ACol;
  ACol = new TColor [IBDAlleleCount];
  Rn=2; while (Rn*Rn*Rn < ((IBDAlleleCount+1)/2)+2) Rn++;
  Gn=Rn; if (Rn*Rn*(Gn-1) >= ((IBDAlleleCount+1)/2)+2) Gn--;
  Bn=Rn; if (Rn*Gn*(Bn-1) >= ((IBDAlleleCount+1)/2)+2) Bn--;
  Ri=Gi=Bi=0;
  for (i=0; i<IBDAlleleCount; i+=2)
  { Gi++; if (Gi==Gn)
          {Gi=0; Bi++; if (Bi==Bn)
                       {Bi=0; Ri++;}
    }
    ACol[i]=MakeColor( int(sqrt(1.0*Ri/Rn)*Lim),
                       int(sqrt(1.0*Gi/Gn)*Lim),
                       int(sqrt(1.0*Bi/Bn)*Lim));
    if (i+1 < IBDAlleleCount)
    { if (HomFounders) ACol[i+1] = ACol[i];
      else ACol[i+1] = LightVersion(ACol[i]);
    }
  }
*/
}; //TViewOptions::SetDefAlleleColors


int VOptCount=1700;

TViewOptions::TViewOptions(void)
{ TStringList *SL;
  ID=VOptCount++;
  //set arrays to NULL:
  ACol=NULL;
  TraitCol=NULL;
  MarkCodeCol=NULL;
  TraitSelected=NULL;
  MarkSelected=NULL;
  SelIBDmrk=NULL;
  InfoTrait=NULL;
  MakeBuiltinOpt();
  if (FileExists(ProgDir+"Options.ini"))
  { SL = new TStringList;
    __try
    { SL->LoadFromFile(ProgDir+"Options.ini");
      ReadFromStringlist(SL,-1,true);
    }
    __finally { delete SL; }
  }
} //TViewOptions::TViewOptions

void TViewOptions::ClearArrays(void)
{ if (ACol!=NULL)          {delete[] ACol;          ACol=NULL;}          //NOT: IBDAlleleCount=0;
  if (TraitCol!=NULL)      {delete[] TraitCol;      TraitCol=NULL;}      //NOT: TraitCodeCount=0;
  if (MarkCodeCol!=NULL)   {delete[] MarkCodeCol;   MarkCodeCol=NULL;}   //NOT: MarkCodeCount=0;
  if (TraitSelected!=NULL) {delete[] TraitSelected; TraitSelected=NULL; TraitSelCount=0;}
  if (MarkSelected!=NULL)  {delete[] MarkSelected;  MarkSelected=NULL;  MarkSelCount=0;}
  if (SelIBDmrk!=NULL)     {delete[] SelIBDmrk;     SelIBDmrk=NULL;}
  if (InfoTrait!=NULL)     {delete[] InfoTrait;     InfoTrait=NULL;}
} //TViewOptions::ClearArrays

TViewOptions::~TViewOptions(void)
{ ClearArrays(); } //TViewOptions::~TViewOptions

TViewOptions::TViewOptions (TViewOptions &Source) //copy-constructor
{ ACol=NULL; TraitCol=NULL; TraitSelected=NULL;
  MarkCodeCol=NULL; MarkSelected=NULL; SelIBDmrk=NULL; InfoTrait=NULL;
  Copy(Source);
} //TViewOptions::TViewOptions (TViewOptions &Source) //copy-constructor

TViewOptions TViewOptions::operator= (TViewOptions &Source)
{ Copy(Source);
  return *this; // only for repeated assignments: A=B=C; otherwise this is unneeded construction and destruction of TViewOptions object!
} //TViewOptions::operator=  */

void TViewOptions::Copy (const TViewOptions &Source)
{ int i;
  if (&Source != this)  //because of statement x.Copy(x);
  {
    //Page options:
      //PgWidthMM  = Source.PgWidthMM;
      //PgHeightMM = Source.PgHeightMM;
      //for (i=0;i<4; i++) MargMM[i]=Source.MargMM[i];
      //PgTransp   = Source.PgTransp;
      //ShowFrame  = Source.ShowFrame;
      PageCol    = Source.PageCol;
      //FrameCol   = Source.FrameCol;

    //Layout options:
      ShowCrossings = Source.ShowCrossings;
      CrossSizeMM   = Source.CrossSizeMM;
      LeftRight     = Source.LeftRight;
      GenerDist     = Source.GenerDist;
      BetwFamSpace  = Source.BetwFamSpace;
      BetwSibSpace  = Source.BetwSibSpace;

    //Individuals options:
      IBackCol   = Source.IBackCol;
      LoCol      = Source.LoCol;
      HiCol      = Source.HiCol;
      MVCol      = Source.MVCol;
      //INameCol   = Source.INameCol;
      TraitColCount = Source.TraitColCount;
      delete[] TraitCol;
      if (Source.TraitCol==NULL) TraitCol=NULL;
      else
      { TraitCol = new TColor [TraitColCount];
        for (i=0; i<TraitColCount; i++) TraitCol[i]=Source.TraitCol[i];
      }
      ColorTrait = Source.ColorTrait;
      IndFront   = Source.IndFront;
      IFixedColor = Source.IFixedColor;
      IndFrames  = Source.IndFrames;
      NameCentered = Source.NameCentered;
      IndivContents = Source.IndivContents;
      INameFont = Source.INameFont;

    //Traits options:
      TraitSelCount = Source.TraitSelCount;
      delete[] TraitSelected;
      if (Source.TraitSelected==NULL) TraitSelected=NULL;
      else
      { TraitSelected = new int [TraitSelCount];
        for (i=0; i<TraitSelCount; i++) TraitSelected[i]=Source.TraitSelected[i];
      }

    //Marker allele options:
      MarkByName = Source.MarkByName;
      delete[] MarkCodeCol;
      if (Source.MarkCodeCol==NULL) MarkCodeCol=NULL;
      else
      { MarkCodeCol = new TColor [MarkCodeCount];
        for (i=0; i<MarkCodeCount; i++) MarkCodeCol[i]=Source.MarkCodeCol[i];
      }
      MarkSelCount=Source.MarkSelCount;
      delete[] MarkSelected;
      if (Source.MarkSelected==NULL) MarkSelected=NULL;
      else
      { MarkSelected = new int [MarkSelCount];
        for (i=0; i<MarkSelCount; i++) MarkSelected[i]=Source.MarkSelected[i];
      }

    //IBD options:
      ProbWidthMM  = Source.ProbWidthMM;
      ProbHeightMM = Source.ProbHeightMM;
      LG           = Source.LG;
      HomFounders  = Source.HomFounders;
      ProbBig      = Source.ProbBig;
      ProbSmall    = Source.ProbSmall;
      AllIBDmrk    = Source.AllIBDmrk;
      delete[] SelIBDmrk; SelIBDmrk=NULL;
      if ( !AllIBDmrk && LG>=0)
      { SelIBDmrk= new bool [LinkGrp[LG]->LocCount];
        for (i=0; i<LinkGrp[LG]->LocCount; i++)
          SelIBDmrk[i] = Source.SelIBDmrk[i];
      }
      delete[] ACol;
      if (IBDAlleleCount==0) ACol=NULL;
      else
      { ACol = new TColor [IBDAlleleCount];
        for (i=0; i<IBDAlleleCount; i++) ACol[i]=Source.ACol[i];
      }
      //MinAllProb = Source.MinAllProb;
      AlleleFont   = Source.AlleleFont;

    //Other options:
      CrossCol      = Source.CrossCol;
      Par1Col       = Source.Par1Col;
      Par2Col       = Source.Par2Col;
      ParSelfedCol  = Source.ParSelfedCol;
      InfoSelected  = Source.InfoSelected;
      InfoParents   = Source.InfoParents;
      InfoTrait = new bool[IndivDataCount];
      for (i=0; i<IndivDataCount; i++) InfoTrait[i]=Source.InfoTrait[i];
  }
} //TViewOptions::Copy

void TViewOptions::MakeBuiltinOpt(void)
{ int i;
  ClearArrays();
  //Page options:
    PageCol=clWhite;

  //Layout options:
    ShowCrossings=true;
    CrossSizeMM=400;  //4 mm
    LeftRight=false;  //pedigree top-down
    GenerDist=5;      //distance between generations, times cross symbol size
    BetwFamSpace=100; //% of Individual size (0-10000%)
    BetwSibSpace=50;  //% of Between family space (0-100%)

  //Individuals options:
    //MinSpaceMM=275; //min. 2.75 mm between indivs
    IBackCol=MakeColor(255,255,150); //pale yellow
    ColorTrait=-1;
    SetDefTraitColors();
    /*LoCol=clBlue;
    HiCol=clRed;
    MVCol=clWhite;
    TraitColCount=0; */
    IndFront=false; //individuals appear behind connections
    IFixedColor=true;
    IndFrames=true;
    NameCentered=true;
    IndivContents=icNone; //only name
    INameFont.Name="Arial"; INameFont.Size=9; INameFont.Style=0; INameFont.Color=clBlack;

  //Traits options:
    TraitSelCount=0;

  //Marker allele options:
    MarkByName=false;
    if (MarkCodeCount==0) MarkCodeCol=NULL;
    else
    { MarkCodeCol = new TColor[MarkCodeCount]; //one color for each marker allele code
      /*
      MarkCodeCol[0]=clBlack;
      if (MarkCodeCount>1) MarkCodeCol[1]=clRed;
      if (MarkCodeCount>2) MarkCodeCol[2]=clGreen;
      if (MarkCodeCount>3) MarkCodeCol[3]=clBlue;
      for (i=4; i<MarkCodeCount; i++) MarkCodeCol[i]=clGray;
      */

      i=0;
      while (i<10 && i<MarkCodeCount)
      { MarkCodeCol[i]=DefCodeCol[i];
        i++;
      }
      for (i=10; i<MarkCodeCount; i++) MarkCodeCol[i]=clGray;

    }
    MarkSelCount=0;

  //IBD options:
    ProbWidthMM=1000;
    ProbHeightMM=2500;
    LG=-1; //LG=0;
    ProbBig=9000; //allele names bold if P>=0.9
    ProbSmall=5000; //allele names italic if P<0.5
    AllIBDmrk=true;
    HomFounders=false;    //before Set allele colors
    SetDefAlleleColors(); //sets ACol
    //MinAllProb=1001; //none shown
    AlleleFont.Name="Arial"; AlleleFont.Size=7; AlleleFont.Style=0; AlleleFont.Color=clBlack;

  //Other options:
    CrossCol=clBlack;
    if (FemaleParent==1) {
      Par1Col=clBlue;
      Par2Col=clRed;
    } else {
      Par1Col=clRed;
      Par2Col=clBlue;
    }
    ParSelfedCol=clPurple;
    InfoSelected=true;
    InfoParents=true;
    InfoTrait=new bool[IndivDataCount];
    for (i=0; i<IndivDataCount; i++) InfoTrait[i]=true; //all shown by default
} //TViewOptions::MakeBuiltinOpt

void TViewOptions::MakeOverviewOpt(void) //changes only the relevant options
{ //PgTransp = true; //ShowFrame=false;
  //MinSpaceMM=110; //min. 1.1 mm between indivs
    PageCol=clWhite;
  //Layout options:
    ShowCrossings=true;
    CrossSizeMM=400; //4 mm
    LeftRight=true;  //pedigree left-right
    //InFamSpace=500; //50% of between-family space
    BetwFamSpace=30;
    BetwSibSpace=100;

  //Individual options:
    IBackCol=clWhite;
    IndFront=false;
    IFixedColor=true;
    IndFrames=false;
    NameCentered=false;
    INameFont.Name="Arial"; INameFont.Size=9; INameFont.Style=0; //INameFont.Color=clBlack;
    IndivContents=icNone;

  //Other options:
    CrossCol=clBlack;
    if (FemaleParent==1) {
      Par1Col=clBlue;
      Par2Col=clRed;
    } else {
      Par1Col=clRed;
      Par2Col=clBlue;
    }
    ParSelfedCol=clPurple;
} //TViewOptions::MakeOverviewOpt


bool TViewOptions::LocSelected(int LocNr) //true if LocNr in MarkSelected array
{ int i;
  if (MarkSelected==NULL || MarkSelCount<=0) return false;
  i=0;
  while (i<MarkSelCount && MarkSelected[i]!=LocNr) i++;
  return i<MarkSelCount;
} //TViewOptions::LocSelected

void GetInt(AnsiString Name, int &i, TStringList *SL)
{ try { i=StrToInt(SL->Values[Name]); }
  catch (...) { ; }
} //GetInt

void GetBool(AnsiString Name, bool &b, TStringList *SL)
{ try { b= StrToInt(SL->Values[Name])==1; }
  catch (...) { ; }
} //GetBool

void GetString(AnsiString Name, AnsiString &s, TStringList *SL)
{ AnsiString t; int i,j;
  try
  { t = SL->Values[Name];
    //strip double quotes:
    i=1; while (i<t.Length() && t[i]=='"') i++;
    j=t.Length(); while (j>i && t[j]=='"') j--;
    t=t.SubString(i,j-i+1);
    if (t>"") s=t;
  }
  catch (...) { ; }
} //GetString

TColor* GetColors(AnsiString Name, int &ColCount, TStringList *SL)
/* looks for first line with Name;
   if value equal to ColCount, read color values from next line
   if anything wrong, ColCount=0 and Col=NULL */
{ int i,r,p; AnsiString woord; TColor *Col;

  Col=NULL;
  i=-1; GetInt(Name,i,SL);
  if (i!=ColCount) ColCount=0;
  else //expected number of colors can be read
  { //look for the line with MarkCodeCol values:
    r=0; i=0;
    while (r<SL->Count && SL->Names[r]!=Name) r++;
    if (r<SL->Count-1)
    { //read values from next line)
      Col = new TColor[ColCount];
      r++; p=1;
      try
      { do {
          ReadWoordSL(r,p,woord,SL);
          Col[i] = TColor(StrToInt(woord));
          i++;
        } while (i<ColCount);
      }
      catch (...)
      { //invalid or insufficient color values read
        delete[] Col; Col=NULL; ColCount=0;
      }
    }
  }
  return Col;
} //GetColors

void TViewOptions::ReadFromStringlist(TStringList *SL,
             int OldIBDAlleleCount, //in project file being read, maybe different from current data
             bool Default)
//if Default=true, the following options are not read, and left unchanged:
// -  ColorTrait and Trait colors
// -  MarkSelCount and selected marker names
{ int i,j,r,p,ColCount;
  AnsiString woord; TLocus *Loc;

  this->MakeBuiltinOpt();
  //Page options:
    //GetInt("PgWidthMM",PgWidthMM,SL);
    //GetInt("PgHeightMM",PgHeightMM,SL);
    //GetInt("MargMM0",MargMM[0],SL);
    //GetInt("MargMM1",MargMM[1],SL);
    //GetInt("MargMM2",MargMM[2],SL);
    //GetInt("MargMM3",MargMM[3],SL);
    //GetBool("PgTransp",PgTransp,SL);
    //GetBool("ShowFrame",ShowFrame,SL);
    GetInt("PageCol",int(PageCol),SL);
    //GetInt("FrameCol",int(FrameCol),SL);

  //Layout options:
    GetBool("ShowCrossings",ShowCrossings,SL);
    GetInt("CrossSizeMM",CrossSizeMM,SL);
    GetBool("LeftRight",LeftRight,SL);
    //GetInt("InFamSpace",InFamSpace,SL);
    GetInt("GenerDist",GenerDist,SL);
    GetInt("BetwFamSpace",BetwFamSpace,SL);
    GetInt("BetwSibSpace",BetwSibSpace,SL);
    //GetInt("MinSpaceMM",MinSpaceMM,SL);

  //Individuals options:
    GetInt("IBackCol",int(IBackCol),SL);
    GetInt("LoCol",int(LoCol),SL);
    GetInt("HiCol",int(HiCol),SL);
    GetInt("MVCol",int(MVCol),SL);
    GetBool("IFixedColor",IFixedColor,SL);
    delete[] TraitCol;
    if (!Default && !IFixedColor)
    { woord=""; GetString("ColorTrait",woord,SL);
      i=IndivDataCount-1;
      while (i>=0 && IndivData[i].Caption!=woord) i--;
      if (i>=0)
      { ColorTrait=i;
        if (IndivData[ColorTrait].DataType==dtC)
        { TraitColCount=IndivData[ColorTrait].V.CV.LevelCount;
          TraitCol=GetColors("TraitColCount",TraitColCount,SL);
          if (TraitColCount==0)
          { //happens also if the stored number of colors != the actual number of levels
            IFixedColor=true; ColorTrait=-1; }
        }
        else if (IndivData[ColorTrait].DataType==dtS)
        { TraitColCount=IndivData[ColorTrait].V.SV.LabelCount;
          TraitCol=GetColors("TraitColCount",TraitColCount,SL);
          if (TraitColCount==0)
          { IFixedColor=true; ColorTrait=-1; }
        }
      }
    }
    GetBool("IndFront",IndFront,SL);
    GetBool("IndFrames",IndFrames,SL);
    GetBool("NameCentered",NameCentered,SL);
    woord=""; GetString("IndivContents",woord,SL);
      IndivContents=icMarkers;
      while (IndivContents>icNone && IndivContentsTypes[IndivContents]!=woord)
            IndivContents--;
    GetString("INameFontName",INameFont.Name,SL);
    GetInt("INameFontSize",INameFont.Size,SL);
    GetInt("INameFontStyle",INameFont.Style,SL);
    GetInt("INameFontColor",int(INameFont.Color),SL);

  //Traits options:
    //GetInt("TraitSelCount",TraitSelCount,SL);
    TraitSelCount=0;  //not implemented yet ; will be excluded from Default options

  //Marker allele options:
    //ShowMessage(SL->Values["MarkByName"]);
    GetBool("MarkByName",MarkByName,SL);
    ColCount=0; GetInt("MarkCodeColors",ColCount,SL);
    delete[] MarkCodeCol; MarkCodeCol=NULL;
    if (MarkCodeCount>0)
    { MarkCodeCol = new TColor[MarkCodeCount]; //one color for each marker allele code
      //note: the actual MarkCodeCount may be different from the number
      //read from the file; read up to the minimum of the current and stored number,
      //and if current>stored, add default colors.
      //look for the line with MarkCodeCol values:
      r=0; i=0;
      while (r<SL->Count && SL->Names[r]!="MarkCodeColors") r++;
      if (r<SL->Count-1)
      { //read color values from next line)
        r++; p=1;
        try
        { while (i<MarkCodeCount && i<ColCount)
          { ReadWoordSL(r,p,woord,SL);
            MarkCodeCol[i] = TColor(StrToInt(woord));
            i++;
          }
        }
        catch (...) {; }
      }
      //assign Builtin colors to MarkCodeCol [i] .. [MarkCodeCount-1]:
      if (MarkCodeCount<=DefCodeColCount)
        for (j=i; j<MarkCodeCount; j++) MarkCodeCol[j] = DefCodeCol[j];
      else {
        for (j=i; j<DefCodeColCount; j++) MarkCodeCol[j] = DefCodeCol[j];
        for (j=DefCodeColCount; j<MarkCodeCount; j++) MarkCodeCol[j] = clGray;
      }
      /*previous replaces following lines d.d. 6-1-2010:
      if (i==0) MarkCodeCol[0]=clBlack;
      if (i<=1 && MarkCodeCount>1) MarkCodeCol[1]=clRed;
      if (i<=2 && MarkCodeCount>2) MarkCodeCol[2]=clGreen;
      if (i<=3 && MarkCodeCount>3) MarkCodeCol[3]=clBlue;
      if (i<=4) p=4; else p=i;
      for (i=p; i<MarkCodeCount; i++) MarkCodeCol[i]=clGray;
      */
    }

    if (!Default)
    { GetInt("MarkSelCount",MarkSelCount,SL);
      if (MarkSelCount>0)
      { delete[] MarkSelected; MarkSelected = new int [MarkSelCount];
        //look for the line with Marker names values:
        r=0; i=0;
        while (r<SL->Count && SL->Names[r]!="MarkSelCount") r++;
        if (r>=SL->Count-1) throw Exception("ReadViewOptions: Marker names not found");
        //read values from next line)
        r++; p=1;
        try
        { for (i=0; i<MarkSelCount; i++)
          { ReadWoordSL(r,p,woord,SL); Loc=GetLocusByName(woord);
            if (Loc==NULL) throw Exception("ReadViewOptions: marker "+woord+" not present");
            MarkSelected[i]=GetLocNr(Loc);
          }
        }
        catch (...)
        { delete[] MarkSelected; MarkSelected=NULL; MarkSelCount=0;
          if (IndivContents==icMarkers) IndivContents=icNone;
        }
      }
    }

  //IBD options:
    GetInt("ProbWidthMM",ProbWidthMM,SL);
    GetInt("ProbHeighMM",ProbHeightMM,SL);
    GetString("LG",woord,SL);
    LG=LinkGrpCount-1;
    while (LG>=0 && LinkGrp[LG]->LGName!=woord) LG--;
        //if linkage group doesn't occur, -1 is result which is default
    GetInt("ProbBig",ProbBig,SL);
    GetInt("ProbSmall",ProbSmall,SL);
    if ( (IBDAlleleCount<=0 || LG==-1) &&
         (IndivContents==icIBDall || IndivContents==icIBDprob) )
    { IndivContents=icNone; LG=-1; }
    if (OldIBDAlleleCount==IBDAlleleCount && IBDAlleleCount>0)
    { //try reading ACol from SL
      r=0; p=1; woord="";
      while (r<SL->Count && woord!="IBDColors")
      { p=1; ReadWoordSL(r++,p,woord,SL); }
      if (woord=="IBDColors")
      { //read values from rest of line)
        r--;
        try
        { i=0;
          do {
            ReadWoordSL(r,p,woord,SL);
            ACol[i++] = TColor(StrToInt(woord));
          } while (i<IBDAlleleCount);
        }
        catch (...) { SetDefAlleleColors(); }
      }
    }
    // else SetDefAlleleColors(): not needed, ACol already has Builtin values for new IBDAlleleCount
    GetBool("HomFounders",HomFounders,SL);
    GetString("AlleleFontName",AlleleFont.Name,SL);
    GetInt("AlleleFontSize",AlleleFont.Size,SL);
    GetInt("AlleleFontStyle",AlleleFont.Style,SL);
    GetInt("AlleleFontColor",int(AlleleFont.Color),SL);
    if (SelIBDmrk!=NULL) {delete[] SelIBDmrk; SelIBDmrk=NULL;}
    if (Default || LG<0) AllIBDmrk=true;
    else
    { GetBool("AllIBDmrk",AllIBDmrk,SL);
      if (!AllIBDmrk)
      { //look for the line with Marker names :
        r=0; i=0;
        while (r<SL->Count && SL->Names[r]!="AllIBDmrk") r++;
        if (r>=SL->Count-1) throw Exception("ReadViewOptions: IBD Marker names not found");
        //read names from next line)
        r++; p=1;
        SelIBDmrk = new bool[LinkGrp[LG]->LocCount];
        for (i=0; i<LinkGrp[LG]->LocCount; i++) SelIBDmrk[i]=false;
        try
        { do
          { ReadWoordSL(r,p,woord,SL);
            if (woord=="") break;
            i=0; while (i<LinkGrp[LG]->LocCount && LinkGrp[LG]->Locus[i]->LocName!=woord) i++;
            if (i>=LinkGrp[LG]->LocCount)
               throw Exception("ReadViewOptions: IBD marker "+woord+" not present");
            SelIBDmrk[i]=true;
          }
          while (true);
          /*test for 0 IBD markers: removed 14-02-2010
          p=0; for (i=0; i<LinkGrp[LG]->LocCount; i++) if (SelIBDmrk[i]) p++;
          if (p==0) throw Exception("ReadViewOptions: IBD Marker names not found");
          */
        }
        catch (...)
        { delete[] SelIBDmrk; SelIBDmrk=NULL; AllIBDmrk=true;
          if (IndivContents==icIBDprob || IndivContents==icIBDall) IndivContents=icNone;
        }
      } // if !AllIBDmrk
    } //if Default else

  //Other options:
    GetInt("CrossCol",int(CrossCol),SL);
    GetInt("Par1Col",int(Par1Col),SL);
    GetInt("Par2Col",int(Par2Col),SL);
    GetInt("ParSelfedCol",int(ParSelfedCol),SL);
    GetBool("InfoSelected",InfoSelected,SL);
    GetBool("InfoParents",InfoParents,SL);
    //look for the line with InfoTraits:
    r=0; woord="";
    while (r<SL->Count && woord!="InfoTraits")
    { p=1; ReadWoordSL(r,p,woord,SL); r++; }
    if (woord=="InfoTraits")
    { r--;
      for (i=0;i<IndivDataCount; i++) InfoTrait[i]=false;
      do
      { ReadWoordSL(r,p,woord,SL);
        try
        { try {i=StrToInt(woord);}
          catch (...) {throw Exception("InfoTrait not a number");}
          if (i<0 || i>=IndivDataCount)
            throw Exception("InfoTrait not valid");
          else InfoTrait[i]=true;
        }
        catch (...) { break; }
      } while (true);
    }
    else //InfoTraits not found, default: all true
    { for (i=0;i<IndivDataCount; i++) InfoTrait[i]=true; }
} //TViewOptions::ReadFromStringlist

void TViewOptions::WriteData(TStringList *SL, bool Default)
//if Default=true, the following options are not written:
// -  ColorTrait
// -  MarkSelCount and selected marker names
{ const AnsiString b1="1", b0="0";
  int i; AnsiString s;
  //Page options:
    SL->Add("PageCol="+IntToStr(PageCol));

  //Layout options:
    SL->Add("ShowCrossings="+(ShowCrossings?b1:b0));
    SL->Add("CrossSizeMM="+IntToStr(CrossSizeMM));
    SL->Add("LeftRight="+(LeftRight?b1:b0));
    SL->Add("GenerDist="+IntToStr(GenerDist));
    SL->Add("BetwFamSpace="+IntToStr(BetwFamSpace));
    SL->Add("BetwSibSpace="+IntToStr(BetwSibSpace));

  //Individuals options:
    SL->Add("IBackCol="+IntToStr(IBackCol));
    SL->Add("LoCol="+IntToStr(LoCol));
    SL->Add("HiCol="+IntToStr(HiCol));
    SL->Add("MVCol="+IntToStr(MVCol));
    SL->Add("IFixedColor="+(IFixedColor?b1:b0));
    if ( !Default && !IFixedColor &&
         ColorTrait>=0 && ColorTrait<IndivDataCount )
    { if ( ( IndivData[ColorTrait].DataType==dtC &&
             TraitColCount==IndivData[ColorTrait].V.CV.LevelCount )
           ||
           ( IndivData[ColorTrait].DataType==dtS &&
             TraitColCount==IndivData[ColorTrait].V.SV.LabelCount )
         )
      { SL->Add("ColorTrait="+QuotedString(IndivData[ColorTrait].Caption));
        SL->Add("TraitColCount="+IntToStr(TraitColCount));
        s="";
        for (i=0; i<TraitColCount; i++) s=s+IntToStr(TraitCol[i])+" ";
        SL->Add(s);
      }
      else //DataType=dtI or dtS, no TraitCol list to save
        SL->Add("ColorTrait="+QuotedString(IndivData[ColorTrait].Caption));
    }
    SL->Add("IndFront="+(IndFront?b1:b0));
    SL->Add("IndFrames="+(IndFrames?b1:b0));
    SL->Add("NameCentered="+(NameCentered?b1:b0));
    SL->Add("IndivContents="+IndivContentsTypes[IndivContents]);
    SL->Add("INameFontName="+INameFont.Name);
    SL->Add("INameFontSize="+IntToStr(INameFont.Size));
    SL->Add("INameFontStyle="+IntToStr(INameFont.Style));
    SL->Add("INameFontColor="+IntToStr(INameFont.Color));

  //Traits options:
    //SL->Add("TraitSelCount="+IntToStr(TraitSelCount));
    SL->Add("TraitSelCount=0"); //not implemented yet ; will be excluded for Default options

  //Marker allele options:
    SL->Add("MarkByName="+(MarkByName?b1:b0));
    SL->Add("MarkCodeColors="+IntToStr(MarkCodeCount)); //used to find marker code colors
    if (MarkCodeCount>0)
    { s="";
      for (i=0; i<MarkCodeCount; i++) s=s+IntToStr(MarkCodeCol[i])+" ";
      SL->Add(s);
    }
    if (!Default)
    { SL->Add("MarkSelCount="+IntToStr(MarkSelCount));
      if (MarkSelCount>0)
      { s="";
        for (i=0; i<MarkSelCount; i++)
          s=s+QuotedString(GetLocusByNr(MarkSelected[i])->LocName)+" ";
        SL->Add(s);
      }
    }

  //IBD options:
    SL->Add("ProbWidthMM="+IntToStr(ProbWidthMM));
    SL->Add("ProbHeighMM="+IntToStr(ProbHeightMM));
    //SL->Add("MinAllProb="+IntToStr(MinAllProb));
    //oud: SL->Add("LG="+IntToStr(LG));
    if (LG>=0) SL->Add("LG="+QuotedString(LinkGrp[LG]->LGName));
    else SL->Add("LG=");
    SL->Add("ProbBig="+IntToStr(ProbBig));
    SL->Add("ProbSmall="+IntToStr(ProbSmall));
    s="IBDColors";
    for (i=0; i<IBDAlleleCount; i++) s=s+" "+IntToStr(ACol[i]);
    SL->Add(s);
    SL->Add("HomFounders="+(HomFounders?b1:b0));
    SL->Add("AlleleFontName="+AlleleFont.Name);
    SL->Add("AlleleFontSize="+IntToStr(AlleleFont.Size));
    SL->Add("AlleleFontStyle="+IntToStr(AlleleFont.Style));
    SL->Add("AlleleFontColor="+IntToStr(AlleleFont.Color));
    if (Default) SL->Add("AllIBDmrk=1");
    else
    { SL->Add("AllIBDmrk="+(AllIBDmrk?b1:b0));
      if (!AllIBDmrk)
      { s="";
        for (i=0; i<LinkGrp[LG]->LocCount; i++)
          if (SelIBDmrk[i]) s=s+QuotedString(LinkGrp[LG]->Locus[i]->LocName)+" ";
        SL->Add(s);
      }
    }
  //Other options:
    SL->Add("CrossCol="+IntToStr(CrossCol));
    SL->Add("Par1Col="+IntToStr(Par1Col));
    SL->Add("Par2Col="+IntToStr(Par2Col));
    SL->Add("ParSelfedCol="+IntToStr(ParSelfedCol));
    SL->Add("InfoSelected="+(InfoSelected?b1:b0));
    SL->Add("InfoParents="+(InfoParents?b1:b0));
    s="InfoTraits";
    for (i=0; i<IndivDataCount; i++)
    { if (InfoTrait[i])
        s=s+" "+IntToStr(i);
    }
    SL->Add(s);

} //TViewOptions::WriteData



// ******** TView ************

void RFI(int i, TCanvas *CV)
{
  ShowMessage(IntToStr(i)+" "+
              "Size="+IntToStr(CV->Font->Size) +" "+
              "Height="+IntToStr(CV->Font->Height)+" "+
              "PPI="+IntToStr(CV->Font->PixelsPerInch));
}


TView::TView(TViewPop *ViewPop, AnsiString ViewName, bool Overview)
{ int i;
  if (ViewPop==NULL) throw Exception ("ViewPop=NULL in TView::TView");
  VP=ViewPop;
  this->ViewName=ViewName;
  ManualAdjust=false;
  //initialize Gener and GIx: for each VPInd, the Generation and index within generation
  Gener = new int [ViewPop->VPIndCount];
  GIx = new int [ViewPop->VPIndCount];
  ZoomValues = new TStringList();
  Zoom=100;
  ZoomedLeftVis=ZoomedTopVis=0; //and stay unchanged ifndef VISAREA
  for (i=0; i<VP->VPIndCount; i++)
  { Gener[i]=-1; GIx[i]=-1; }
  //initialize Generation structures:
  GCount=0; GS = NULL;
  //for (g=0; g<GCount; g++) {GS[g]=new TGenStrip;} //doet niks want GCount==0 ??
  CS=NULL;
  //initialize Graphics structures:
  //OldGCount=0;
  //OldICount=NULL;
  //IndMF=NULL;
  //IPos=NULL;
  //GPos=NULL;
  //CPos=NULL; CCPos=NULL;
  //OldGCount=0;
  //initialize pixel sizes and other graphic members:
  INameWidth=INameHeight=LGNameWidth=LGNameHeight=0;
  ChartCalculated=false; //PrIndWidth=-1; PrIndHeight=-1; //signals: not calculated yet
  //IndWidth=IndHeight=0;
  LocPx=NULL; IBDPx=NULL;
  MaxIndPerGS=0;
  //MinXPix=MinYPix=MaxXPix=MaxYPix=0;
  CrossMF=NULL;
  SelfMF=NULL;
  DHMF=NULL;
  MutMF=NULL;
  VegPropMF=NULL;
  ErrorMF=NULL;
  ChartMF=NULL;
  this->Overview=Overview;
  //Opt are automatically initialized, but:
  if (Overview) Opt.MakeOverviewOpt();
  SelBorderWidth=4;
  ProbLeft = new int[Ploidy];
  IFrameCol=NULL;
} //TView::TView

TView::~TView(void)
{ DeleteGraphicsStruct();
  DeleteGenerationStruct();
  DeleteCrossMetafiles();
  delete[] Gener;
  delete[] GIx;
  delete ZoomValues;
  //delete OrigOpt;
  delete[] ProbLeft;
  delete[] IFrameCol;
} //TView::~TView

TView::TView(TView &Source)
{ ShowMessage("Warning: TView copy-constructor of TView not implemented"); }

int TView::AddGS(int AddNum)
{ TGenStrip* *Temp;
  int g;
  Temp = new TGenStrip* [GCount+AddNum];
  for (g=0; g<GCount; g++) Temp[g]=GS[g];
  for (g=GCount; g<GCount+AddNum; g++) Temp[g]=new TGenStrip(this);
  delete[] GS;
  GS=Temp; Temp=NULL;
  GCount=GCount+AddNum;
  return GCount;
} //TView::AddGS

void TView::AddVPInd(int VPI, int G, int GP)
//add VPInd[VPI] to GS[G] at position GIx[GP]
{ int j;
  if (VPI<0 || VPI>=VP->VPIndCount) {throw Exception("TView::AddVPInd : VPI not valid");}
  if (Gener[VPI]!=-1) {throw Exception("TView::AddVPInd : VPI already placed");}
  if (G<0 || G>=GCount) {throw Exception("TView::AddVPInd : G not valid");}
  if (GP == -1) GP=GS[G]->ICount;
  if (GP<0 ||GP>GS[G]->ICount) {throw Exception("TViewPop::AddIndiv : GP not valid");}
  GS[G]->AddVPInd(VPI,GP);
  Gener[VPI]=G; GIx[VPI]=GP;
  //All Ind after GP have moved 1 pos to right; adjust:
  for (j=GP+1; j<GS[G]->ICount; j++)
  { GIx[GS[G]->Ind[j]]=j; }
} //TView::AddVPInd

void TView::RemoveIndIndex(int G, int GP)
//remove VPInd present at GS[G}, position GIx[GP]; does nothing if GP==-1
{ int j;
  if (G<0 || G>=GCount) {throw Exception("TViewPop::RemoveIndIndex : G not valid");}
  if (GP != -1)
  { if (GP<0 ||GP>GS[G]->ICount) {throw Exception("TViewPop::RemoveIndIndex : GP not valid");}
    Gener[GS[G]->Ind[GP]]=-1; GIx[GS[G]->Ind[GP]]=-1;
    GS[G]->RemoveIndIndex(GP);
    //All Ind after GP have moved 1 pos to left; adjust:
    for (j=GP; j<GS[G]->ICount; j++)
    { GIx[GS[G]->Ind[j]]=j; }
  }
} //TView::RemoveIndIndex

void TView::RemoveVPInd(int VPI)
//remove VPInd[VPI] if present from its position in the generations
{ int j;
  if (VPI<0 || VPI>=VP->VPIndCount) {throw Exception("TViewPop::RemoveIndNum : VPI not valid");}
  if (Gener[VPI] == -1) {throw Exception("TViewPop::RemoveIndNum : VPI not placed");}
  j=GS[Gener[VPI]]->GetIndIndex(VPI); //why not use GIx[VPI] ???
  if (j==-1) {throw Exception("TViewPop::RemoveIndNum : VPI not in GS[G]");}
  RemoveIndIndex(Gener[VPI],j);
} //TView::RemoveIndNum

/*
void TView::RemoveWProg(int I)
//NB gevaarlijk: schuift individuen in generatie van I + nakomelingen
//terug, als er op dat moment naar verwezen wordt via indexen kloppen die niet meer
//removes VPInd[I] and all of its progeny from ViewPop
{
  int m,p;
  if (I<0 || I>=VP->VPIndCount) {throw Exception("TView::RemoveWProg : I not valid");}
  if (Gener[I] != -1)
  { for (m=0; m<Indiv[VP->VPInd[I]].MCount; m++)
    { for (p=0; p<Indiv[VP->VPInd[I]].Mating[m]->OCount; p++)
      { RemoveWProg(VP->GetVPIndex(Indiv[VP->VPInd[I]].Mating[m]->Offsp[p])); }
    }
    RemoveIndNum(I);
  }
} //TView::RemoveWProg
*/

/*****************************************************
The initial generation tree structure is calculated in TView,
by a call to TView::CreateGenerationStructure.
Each TView stores its own generation structure, composed of
TGenStrip's (generation strips containing Individuals) and
TCrossStrip's (generation strips containing Crossings).
At the moment these are identical (except for the position
information) in all TView's of a TViewPop, so they could be
stored more efficiently in the TViewPop. But in the future
individuals may be moved manuallyto other positions or
generations.

In TView the following methods are involved:
  TView::CreateGenerationStructure
  TView::FillGenerations
  TView::MoveSubtreesDown
  TView::SeparateAncestries
The trees (TGenStrip's and TCrossStrip's) are stored
in TView::GS and TView::CS.
The arrays TView::Gener and TView::GIx store the generation
number and the index within the GenStrip of each VPInd.

TViewPop has some tools used in creating the generation
structure:
  TViewPop::IsVPFounder
  TViewPop::MarkFounders
  TViewPop::MarkSubtree
  TViewPop::MinGenerDist
The bool array TViewPop::Sbt records which VPInd are in the current Subtree
*********************************************************/

void TView::CreateGenerationStructure(void)
//starting with VP, create initial generation structure,
//and fill it with Individuals and Crossings
//Note: the Indivs and Crossings are placed in the correct generations and the
//correct order within generations, but the positions are not calculated here
//(they are calculated in CalcInitialPositions)
{ int g,i;
  TCrossing *Cro1, *Cro2;
  //First, place the Individuals:
  FillGenerations();
  if (IndivCount<=500) MoveSubtreesDown(); //for 1433 indiv it takes too long
  SeparateAncestries();
  //Next, place the Crossings:
  //(assumption: each full-sib family in one consecutive block in one generation)
  delete[] CS; CS= new TCrossStrip* [GCount];
  CS[0]=NULL;
  for (g=1; g<GCount; g++)    //not for g==0 !
  { CS[g] =  new TCrossStrip();
    i=0; Cro1=NULL;
    while (i<GS[g]->ICount)
    { //skip same and NULL crosses:
      while ( ( i<GS[g]->ICount ) &&
              ( (Cro2=Indiv[VP->VPInd[GS[g]->Ind[i]]].ParentCross) == NULL ||
                Cro2 == Cro1 ) )
      i++;
      if (i<GS[g]->ICount)
      { Cro1=Cro2;
        //only add this Cross if at least one parent belongs to VP:
        if ( Cro1 != NULL &&
             ( VP->GetVPIndex(Cro1->Parent[0]) != -1 ||
               VP->GetVPIndex(Cro1->Parent[1]) != -1 ) )
        { CS[g]->AddCross(Cro1); }
      } //if i<ICount
    } //while i
  } //for g
} //TView::CreateGenerationStructure


void TView::FillGenerations(void)
// Create the initial pedigree structure of the VP->VPInd population
// Note: this is done for each TView, because in the future it may be possible
// to manually move individuals to other positions (and other generations)
{ int i, m, p, g0, g1, Mate, MateVPI, N;
  TCrossing *M;
  TVPCrossListItem* CLI;
  bool Ready, FillCrossList;

  FillCrossList = VP->CL->Count==0; //fill crosslist only once
  //First, place all founders in GS[0]:
  AddGS(1); //add first GS = GS[0]
  for (i=0; i<VP->VPIndCount; i++)
  { if (VP->IsVPFounder(i)) AddVPInd(i,0); }

  //Next, generation by generation place all progeny below parents:

  do //while (!Ready)
  { Ready=true; g0=GCount-1; g1=g0+1; //last and current generation
    for (i=0; i<GS[g0]->ICount; i++)
    { for (m=0; m<Indiv[VP->VPInd[GS[g0]->Ind[i]]].MCount; m++)
      { M=Indiv[VP->VPInd[GS[g0]->Ind[i]]].Mating[m];
        //first: check if Mating[m] did not occur earlier in same generation,
        //or if the other mate does not occur at a later generation:
        if ( M->Parent[0] == VP->VPInd[GS[g0]->Ind[i]] )  Mate= M->Parent[1];
        else  Mate= M->Parent[0];
        //now Mate is index in Indiv array ...
        MateVPI=VP->GetVPIndex(Mate); // .. and MateVPI is index in VP->VPInd array ...
        if (MateVPI<0 || (Gener[MateVPI]!=-1 && Gener[MateVPI]<=g0) )
        { //there is no mate, or the mate does not occur in a later generation
          Mate=GS[g0]->GetIndIndex(MateVPI); //... and now Mate is index in GS[g0]->Ind array
          if (Mate==-1 || Mate>=i)
          { //there is no mate, or the mate did not occur earlier in same generation
            for (p=0; p<M->OCount; p++)
            { N = VP->GetVPIndex(M->Offsp[p]) ; //N is index in VP->VPInd array
              if (N>0)
              { if (Gener[N] != -1)
                { throw Exception("Error: individual "+Indiv[VP->VPInd[N]].IName+" already placed"); }
                if (g1==GCount) AddGS(1);
                AddVPInd(N,g1);
                Ready=false; //because at least one Indiv added in current generation
                if ( FillCrossList &&
                     ( VP->CL->Count==0 ||
                       VP->CL->Cro[VP->CL->Count-1]->VPCro != M ) )
                { //if mating not yet added to VP->CL, add it now:
                  CLI = new TVPCrossListItem;
                  VP->CL->Add(CLI);
                  CLI->VPCro = M;
                  CLI->SortRank = VP->CL->Count-1;
                }
              } //N>0: offspring in VP->VPInd
            } //for p: loop over all offspring of M=Mating[m]
          } //Mate not earlier in same generation
        } //Mate not in later generation
      } //for m
    } //for i
  } while (!Ready);
  //still to do:
  // - if several unrelated pedigrees in population: separate side by side
  // - see if some partial pedigrees incl. founders can be moved down one or more generations
  MaxIndPerGS=0;
  for (g0=0; g0<GCount; g0++)
  { if (MaxIndPerGS<GS[g0]->ICount) MaxIndPerGS=GS[g0]->ICount; }
} //TView::FillGenerations

void TView::DeleteGenerationStruct(void)
{ int i;
  DeleteGraphicsStruct();
  for (i=0; i<GCount; i++)
  { delete GS[i];
    delete CS[i];
  }
  delete[] GS; delete[] CS;
  GCount=0;
  for (i=0; i<VP->VPIndCount; i++)
  { Gener[i]=-1; GIx[i]=-1; }
} //TView::DeleteGenerationStruct


void TView::MoveSubtreesDown(void)
/* Aim:
   Define a subtree as a set of founders and all members of the population that
   are only descended from these founders (and possibly from unknown parents or
   parents outside VP->VPInd).
   Initally (after FillGenerations) the founders of each subtree are at generation 0.
   A subtree relates to the rest of the population only because some of its members
   are parents of individuals outside the subtree in the population
   (the other parent of this "outside offspring" therefore does not belong to the
    subtree)
   If in each case the "outside offspring" occurs at more than one generation
   below the "inside parent", the whole subtree can be moved down one or more
   generations.

   Note: this doesn't take care of all situations where a move down is possible;
   see the pedigree in FailedMoveDown.txt

   Method:
   Check all individuals, starting at generation 0.
   When an individual is found that
   - has no offspring in this ViewPop, OR
   - has all its offspring at more than 1 generation lower
   then check if the whole subtree of this individual can be moved down:
   That is done if
   - the subtree has outside offspring in this ViewPop, AND
   - all the outside offspring is at more than 1 generation below its inside parent

   Purpose of bool array Checked[]:
   Checked[vi] is made true for an individual vi when a subtree containing vi
   is checked. If Checked[vi] is true when it is this individual's turn to be checked
   then this individual is skipped. This is done to avoid a lot of double work.
   There are two possible cases:
   - the subtree of vi is identical to the subtree of which it was part when it was Checked:
     in that case it would clearly be useless to check the same subtree again.
   - the subtree of vi is part of the subtree that was moved down. In that case it is possible
     that vi's subtree could stil be moved down within the larger subtree. However
     that is unlikely, as that would usually (always?) have been noticed earlier.
   Therefore Checked saves a lot of work, possibly at the cost of missing a few cases.

*/
{ int g,i,vi,m,Dist,p,vp,I,h,j;
  bool *Checked;    //for each VPInd: subtree already checked?
  TCrossing *Cro;
  //TStringList *LogSL; int di,dj; AnsiString s; //for debugging

  /*LogSL=new TStringList; //for debugging
  s="VPInd=";
  for (di=0; di<VP->VPIndCount; di++)
    s=s+" "+Indiv[VP->VPInd[di]].IName;
  LogSL->Add(s); */
  Checked = new bool [VP->VPIndCount];
  for (vi=0; vi<VP->VPIndCount; vi++) Checked[vi]=false;

  for (g=0; g<GCount; g++)
  { /*LogSL->Add("");
    LogSL->Add("At start of g="+IntToStr(g));
    for (di=0; di<GCount; di++)
    { s="G="+IntToStr(di);
      for (dj=0; dj<GS[di]->ICount; dj++)
        s=s+" "+Indiv[VP->VPInd[GS[di]->Ind[dj]]].IName;
      LogSL->Add(s);
    }
    s="Gener=";
    for (di=0; di<VP->VPIndCount; di++)
      s=s+" "+IntToStr(Gener[di]);
    LogSL->Add(s);
    s="GIx=  ";
    for (di=0; di<VP->VPIndCount; di++)
      s=s+" "+IntToStr(GIx[di]);
    LogSL->Add(s);
    LogSL->SaveToFile("MoveSubtrees.log"); */

    for (i=0; i<GS[g]->ICount; i++)
    //determine Dist: max. number of generations this individual could be moved
    //down (is above its own children); 0 if no children in VP
    { vi=GS[g]->Ind[i]; I=VP->VPInd[vi]; //index of Ind[i] in VPInd and Indiv
      if (!Checked[vi])
      { if (Indiv[I].MCount==0) Dist=0;  //no matings, no children at all
        else //check generation distance:
        { Dist=MAXINT; m=0; //Dist=minimum nr of generations down to children
          while (Dist>1 && m<Indiv[I].MCount)
          { Cro=Indiv[I].Mating[m++];
            p=0;
            while (Dist>1 && p<Cro->OCount)
            { vp=VP->GetVPIndex(Cro->Offsp[p++]);
              if (vp>=0 && (Gener[vp]-g)<Dist) Dist=Gener[vp]-g;
            }
          } //while Dist && m
          if (Dist==MAXINT) Dist=0; //children outside but not inside VP
        } //else
        //hier bleef Dist soms MAXINT, en dan ging het fout. Opgelost?
        if (Dist>1 || //all children of vi at more than 1 generation lower; or ...
            Dist==0)  //... vi has no offspring, maybe still move tree down
        { VP->MarkSubtree(GS[g]->Ind[i]);
          for (vi=0; vi<VP->VPIndCount; vi++)
              Checked[vi]= Checked[vi] || VP->Sbt[vi];
          //Check if whole subtree can be moved down
          // and find out Dist=number of generations to move down:
          Dist = VP->MinGenerDist(Gener);
          /*LogSL->Add("Subtree of "+Indiv[I].IName);
          s="Sbt    =";
          for (di=0; di<VP->VPIndCount; di++)
              s=s+" "+IntToStr(VP->Sbt[di]);
          LogSL->Add(s);
          s="Checked=";
          for (di=0; di<VP->VPIndCount; di++)
              s=s+" "+IntToStr(Checked[di]);
          LogSL->Add(s); */
          if (Dist>1 && Dist!=MAXINT) //!=MAXINT added 15-1-04, necessary;
             //only move subtree if all external progeny > 1 generation below,
             //not if no outside progeny (MAXINT)
          { //Move down all individuals in Subtree by Dist-1 generations;
            //when removing individuals from GS[g] before or including the current i,
            // i must be decreased
            /*LogSL->Add("Move subtree of "+Indiv[I].IName+" "+IntToStr(Dist-1)+" generations down");
            LogSL->SaveToFile("MoveSubtrees.log"); */
            for (h=GCount-1; h>=0; h--)
            { j=0;
              while (j<GS[h]->ICount)
              { vi=GS[h]->Ind[j];
                if ( VP->Sbt[vi] )
                { /*LogSL->Add("Move "+Indiv[VP->VPInd[vi]].IName+" from "+IntToStr(h)+" to "+IntToStr(h+Dist-1));
                  LogSL->SaveToFile("MoveSubtrees.log"); */
                  //Move down Ind[j] = VPInd[vi]:
                  if (h==g && j<=i) i--;
                  RemoveIndIndex(h,j);
                  if (h+Dist-1 >= GCount) AddGS( h+Dist-GCount );
                  AddVPInd(vi,h+Dist-1);
                }
                else j++;
              } //while j
            } //for h
          } //Dist>1 and !=MAXINT for whole Subtree
        } //Dist>1 || Dist==0 for Gs[g]->Ind[i]
      } //Gs[g]->Ind[i] not Checked
    } //for i
  } //for g
 delete[] Checked;
 /*LogSL->Add("");
    LogSL->Add("At end of MoveSubtreesDown:");
    for (di=0; di<GCount; di++)
    { s="G="+IntToStr(di);
      for (dj=0; dj<GS[di]->ICount; dj++)
        s=s+" "+Indiv[VP->VPInd[GS[di]->Ind[dj]]].IName;
      LogSL->Add(s);
    }
    s="Gener=";
    for (di=0; di<VP->VPIndCount; di++)
      s=s+" "+IntToStr(Gener[di]);
    LogSL->Add(s);
    s="GIx=  ";
    for (di=0; di<VP->VPIndCount; di++)
      s=s+" "+IntToStr(GIx[di]);
    LogSL->Add(s);
    LogSL->SaveToFile("MoveSubtrees.log");
 LogSL->Free(); */
} //TView::MoveSubtreesDown

/* old version:
 void TView::SeparateAncestries(void)
/* Aim:
   This routine tries to separate the ancestries of individuals if possible.
   A side effect is that completely unrelated sub-populations will also be
   separated. (TView::SeparateFamilies not needed any more)
   Strategy:
   From the last generation upwards, for all individuals in the generation
   (from left to right):
   - Move its parents in the preceding generation (if present there), together with
     their whole full-sib families, to the left next to any earlier moved families.

   Previously a more complicated strategy was as follows: NOT ANY MORE NOW!!
   - if an individual has only one parent in the preceding generation: do nothing
   - if an individual has both parents in the preceding generation, and both
     parents belong to the offspring of the same crossing: do nothing
   - if both parents in the preceding generation belong to different families:
     move those two families (groups of full-sibs) next to each other,
     provided that none of them has been moved before (as recorded in the
     SAMoved field of the parental crossing)

   Note:
   The separation of families can fail at points where parent-offspring distances
   are more than 1 generation. See FailedSeparateFamilies.txt for an example.

{ int g,i,x,x1,x2,y,p,
      *FUM,  //array: First UnMoved individual in each generation
      *Temp, //array holding the sib-family to be moved
      P,     //indices of parents in Indiv array
      PVP,   //index of parent in VP->VPInd array
      GSP;           //index of parent in GS[g-1]->Ind array
  TCrossing *Cro, *CroP;

  FUM = new int[GCount];
  for (g=0; g<GCount; g++) FUM[g]=0;

  for (g=GCount-1; g>0; g--)
  //from last generation backwards ... up to g=1 (not 0!)
  { i=0;
    while (i<GS[g]->ICount)   //for all individuals in GS[g] ...
    { Cro=Indiv[VP->VPInd[GS[g]->Ind[i]]].ParentCross;
      if (Cro==NULL) {i++; continue;}
      for (p=0; p<2; p++)
      //for both parents of the current individual ...
      { P=Cro->Parent[p];       //P is index of parent[p] in array Indiv
        PVP=VP->GetVPIndex(P);  //PVP is index of parent[p] in VP->VPInd
        GSP=GS[g-1]->GetIndIndex(PVP); //GSP is index of parent[p] in GS[g-1]->Ind
        if (GSP<FUM[g-1]) continue; //parent not present in GS[g-1] or already moved
        //parent found in GS[g-1], and must be moved (together with sibs)
        //find first and last sib of parent (x1 and x2):
        x1=x2=GSP;
        if ( ! VP->IsVPFounder(PVP) ) //else parent is founder -> no sibs, x1=x2=GSP
        {
          CroP=Indiv[P].ParentCross;
          do x1--;
          while (x1>=0 && Indiv[VP->VPInd[GS[g-1]->Ind[x1]]].ParentCross==CroP);
          x1++;
          do x2++;
          while (x2<GS[g-1]->ICount && Indiv[VP->VPInd[GS[g-1]->Ind[x2]]].ParentCross==CroP);
          x2--;
        }
        if (x1>=FUM[g-1]) //else already in correct position
        { Temp = new int [x2-x1+1];
          for (x=0; x<=x2-x1; x++) Temp[x]=GS[g-1]->Ind[x1+x];
          for (x=x1-1; x>=FUM[g-1]; x--)
          { y=x+x2-x1+1;
            GS[g-1]->Ind[y]=GS[g-1]->Ind[x];
            GIx[GS[g-1]->Ind[y]]=y;
          }
          for (x=0; x<=x2-x1; x++)
          { y=FUM[g-1]+x;
            GS[g-1]->Ind[y] = Temp[x];
            GIx[GS[g-1]->Ind[y]]=y;
          }
          delete[] Temp;
        }
        FUM[g-1] += x2-x1+1;
      } //for p
      //skip all sibs: individuals with same parental cross:
      do i++;
      while (i<GS[g]->ICount && Indiv[VP->VPInd[GS[g]->Ind[i]]].ParentCross==Cro);
    } //while i
  } //for g
  delete[] FUM;
} //TView::SeparateAncestries
end old version */

/* new version (4-7-04): recursive
   Advantage: works also across generation gaps (if parent >1 generation above offspring)
  - make & clear array FUM (first Unmoved) per generation
  - for G=GCount-1 downto 0 do
     for I=FUM[G] to GS[G]->ICount-1 (all remaining unmoved individuals in G) do
       call MoveLeftWithSibsNAncestry( G, I);
  - delete array FUM
  end;

  in MoveLeftWithSibsNAncestry( G, I)
     if VPFounder: move left to FUM[G]; inc(FUM[G]);
     else
       - move complete FS family in G left to FUM[G]; increase FUM[G]
       - for both parents (p=0..1)
          - find J=Parent[p] and its generation Gp
          - MoveLeftWithSibsNAncestry( Gp, J)
     end;
*/

void TView::MoveLeftWithSibsNAncestry(int G, int I)
{ //I is index in GS[G]->Ind array
  int j,x1,x2,y,
      VPI, //index of same indiv in VP->VPInd array
      *Temp, //array holding the sib-family to be moved
      P,     //indices of parents in Indiv array
      PVP,   //index of parent in VP->VPInd array
      GSP;           //index of parent in GS[g-1]->Ind array
  TCrossing *Cro;

  if (I>=FUM[G]) //else already moved, do nothing
  { VPI=GS[G]->Ind[I];
    if (VP->IsVPFounder(VPI))
    { //no ancestors, no sibs; just move I to FUM[G} and increase FUM[G} by 1
      x1=x2=I;
    }
    /*
      if (I>FUM[G])
      { j=GS[G]->Ind[I];
        for (i=I; i>FUM[G]; i--) GS[G]->Ind[i]=GS[G]->Ind[i-1];
        GS[G]->Ind[FUM[G]]=j;
      }
      ++FUM[G];
    }
    */
    else
    { Cro=Indiv[VP->VPInd[VPI]].ParentCross;
      //move both parents through recursive call:
      for (j=0; j<2; j++)
      //for both parents of the current individual ...
      { P=Cro->Parent[j];       //P is index of parent[p] in array Indiv
        PVP=VP->GetVPIndex(P);  //PVP is index of parent[p] in VP->VPInd
        if (PVP>=0)
        { GSP=GS[Gener[PVP]]->GetIndIndex(PVP); //GSP is index of parent[p] in GS[Gener[PVP]]->Ind
          MoveLeftWithSibsNAncestry(Gener[PVP],GSP);
        }
      }
      //move complete FullSib family to FUM[G]:
      //find first and last sib I:
      x1=x2=I;
      do x1--;
      while (x1>=0 && Indiv[VP->VPInd[GS[G]->Ind[x1]]].ParentCross==Cro);
      x1++;
      do x2++;
      while (x2<GS[G]->ICount && Indiv[VP->VPInd[GS[G]->Ind[x2]]].ParentCross==Cro);
      x2--;
    } //not IsFounder

    //now move whole FS family (x1..x2) to FUM[G] and adjust FUM[G}
    if (x1>=FUM[G]) //else already in correct position
    { Temp = new int [x2-x1+1];
      for (j=0; j<=x2-x1; j++) Temp[j]=GS[G]->Ind[x1+j];
      for (j=x1-1; j>=FUM[G]; j--)
      { y=j+x2-x1+1;
        GS[G]->Ind[y]=GS[G]->Ind[j];
        GIx[GS[G]->Ind[y]]=y;
      }
      for (j=0; j<=x2-x1; j++)
      { y=FUM[G]+j;
        GS[G]->Ind[y] = Temp[j];
        GIx[GS[G]->Ind[y]]=y;
      }
      delete[] Temp;
    }
    FUM[G] += x2-x1+1;
  } // I>=FUM[G]
} //MoveLeftWithSibsNAncestry

void TView::SeparateAncestries(void)
/* Aim:
   This routine tries to separate the ancestries of individuals if possible.
   A side effect is that completely unrelated sub-populations will also be
   separated. (TView::SeparateFamilies not needed any more)
   Strategy:
   From the last generation upwards, for all individuals in the generation
   (from left to right):
   - Move its parents in the preceding generation (if present there), together with
     their whole full-sib families, to the left next to any earlier moved families.

   Previously a more complicated strategy was as follows: NOT ANY MORE NOW!!
   - if an individual has only one parent in the preceding generation: do nothing
   - if an individual has both parents in the preceding generation, and both
     parents belong to the offspring of the same crossing: do nothing
   - if both parents in the preceding generation belong to different families:
     move those two families (groups of full-sibs) next to each other,
     provided that none of them has been moved before (as recorded in the
     SAMoved field of the parental crossing)

   Note:
   The separation of families can fail at points where parent-offspring distances
   are more than 1 generation. See FailedSeparateFamilies.txt for an example.
*/
{ int g,i;
      //*FUM,  //array: First UnMoved individual in each generation

  FUM = new int[GCount];
  for (g=0; g<GCount; g++) FUM[g]=0;

  for (g=GCount-1; g>0; g--)
  //from last generation backwards ... up to g=1 (not 0!)
  { i=FUM[g];
    while (i<GS[g]->ICount)   //for all individuals in GS[g] ...
    { MoveLeftWithSibsNAncestry(g,i);
      i=FUM[g];
    } //while i
  } //for g
  delete[] FUM;
} //TView::SeparateAncestries

void TView::CreateGraphicsStruct(void)
{ int g,i;
  if (GCount==0) throw Exception("TView::CreateGraphicsStruct: GCount=0");
  //if (OldGCount!=0) DeleteGraphicsStruct();
  //OldGCount=GCount;
  //OldICount = new int [GCount];
  //GPos = new float [GCount];
  //IPos = new float* [GCount];
  //IndMF = new TMetafile** [GCount];
  for (g=0; g<GCount; g++)
  { //OldICount[g] = GS[g]->ICount;
    //GPos[g]=-1.0;
    //IPos[g] = new float [GS[g]->ICount];
    //IndMF[g] = new TMetafile* [GS[g]->ICount];
    for (i=0; i<GS[g]->ICount; i++)
    { //IPos[g][i] = -1.0;
      //IndMF[g][i]=NULL;
    }
  }
} //TView::CreateGraphicsStruct

/*
void TView::DeleteIndMetafiles(void)
//keeps the graphics structures intact, just empties the IndMF array
// to implement again in future, if generation/GIx structure if allowed to change!
{ int g,i;
  for (g=0; g<OldGCount; g++)
  { for (i=0; i<OldICount[g]; i++)
    { delete IndMF[g][i]; IndMF[g][i]=NULL; }
  }
  PrIndWidth=-1; PrIndHeight=-1;
} //TView::DeleteIndMetafiles
*/

void TView::DeleteGraphicsStruct(void) {}
//deletes the complete arrays
// to implement again in future, if generation/GIx structure if allowed to change!
/*
{ int g,i;
  MaxIndPerGS=0;
  for (g=0; g<OldGCount; g++)
  { { for (i=0; i<OldICount[g]; i++) delete IndMF[g][i]; }
    delete[] IndMF[g];
    //delete[] IPos[g];
  }
  //delete[] OldICount; OldICount=NULL;
  delete[] IndMF;   IndMF=NULL;
  //delete[] IPos;    IPos=NULL;
  //delete[] GPos;    GPos=NULL;
  //OldGCount=0;
} //TView::DeleteGraphicsStruct */


int TView::MaxINameWidth(TCanvas *CV)
//Font (incl PixelsPerInch) should already been set
{ int g,i,W,tw;
  //FntAttToFont(Opt.INameFont,CV->Font);
  W=0;
  for (g=0; g<GCount; g++)
  { for (i=0; i<GS[g]->ICount; i++)
    { tw = CV->TextWidth(Indiv[VP->VPInd[GS[g]->Ind[i]]].IName);
      if (tw>W) W=tw;
    }
  }
  return W;
} //TView::MaxINameWidth

short int **AlPx; //for each allele for each locus the end pixel;
                  //16-bit as may be larger than 255 (although unlikely)
float *AlPrb;     //for the current locus and parent the allele probs
TPoint *Points;   //the vertices of the polygon for the current allele

TMetafile* TView::DrawIndMF(int VPI, bool DrawFrame)
//DrawFrame true for exporting or printing but false for onscreen display
{ TMetafile *MF;
  TMetafileCanvas *MFC;
  int I, a, j, k, n, loc, par, maxpr, maxa; //par: parental copy of LG: 0 or 1
  TSize ts;
  AnsiString s;
  TLocus *Loc;
  TIBDposition *Ip;
  TColor IBackCol; //background (fill) color

  MF = new TMetafile;
  MF->Width = PrIndWidth+LinePx+1;
  MF->Height = PrIndHeight+LinePx+1;
  MFC = new TMetafileCanvas(MF, 0);
  //RFI(10,MFC);
  MFC->Font->PixelsPerInch=PrPPI; //default 96
  MFC->Pen->Width = LinePx; //(63+PrPPI) / 127; approx. 0.2 mm

  //Draw frame and background:
  I=VP->VPInd[VPI];

  if (Opt.IFixedColor || Opt.ColorTrait<0 || Opt.ColorTrait>=IndivDataCount)
     IBackCol = Opt.IBackCol;
  else
  { if (IndivData[Opt.ColorTrait].DataType==dtI)
    { int LoVal,Range,ix;
      ix=IndivData[Opt.ColorTrait].TypeIndex;
      if (Iint[ix][I]==-MAXINT) IBackCol=Opt.MVCol; //missing value
      else
      { LoVal=Iint[ix][IndivData[Opt.ColorTrait].V.OV.SmallInd];
        Range=Iint[ix][IndivData[Opt.ColorTrait].V.OV.LargInd]-LoVal;
        if (Range<=0) IBackCol = Opt.IBackCol; //to avoid division by 0
        else IBackCol= ColorRange(Opt.LoCol,Opt.HiCol,double(Iint[ix][I]-LoVal)/Range);
      }
    }
    else if (IndivData[Opt.ColorTrait].DataType==dtF)
    { double LoVal,Range; int ix;
      ix=IndivData[Opt.ColorTrait].TypeIndex;
      if (Ifloat[ix][I]==-MAXDOUBLE) IBackCol=Opt.MVCol; //missing value
      else
      { LoVal=Ifloat[ix][IndivData[Opt.ColorTrait].V.OV.SmallInd];
        Range=Ifloat[ix][IndivData[Opt.ColorTrait].V.OV.LargInd]-LoVal;
        if (Range<=0.0) IBackCol = Opt.IBackCol; //to avoid division by 0
        else IBackCol= ColorRange(Opt.LoCol,Opt.HiCol,(Ifloat[ix][I]-LoVal)/Range);
      }
    }
    else if (IndivData[Opt.ColorTrait].DataType==dtC)
    { char LoVal,Range; int ix;
      ix=IndivData[Opt.ColorTrait].TypeIndex;
      if (Ichar[ix][I]==NULLCHAR) IBackCol=Opt.MVCol; //missing value
      else if (IndivData[Opt.ColorTrait].V.CV.LevelCount>MaxTraitColors)
      { //too many levels: treat as integer values
        LoVal=IndivData[Opt.ColorTrait].V.CV.Levels[0];
        Range=IndivData[Opt.ColorTrait].V.CV.Levels[IndivData[Opt.ColorTrait].V.CV.LevelCount-1]-LoVal;
        if (Range<=0.0) IBackCol = Opt.IBackCol; //to avoid division by 0
        else IBackCol= ColorRange(Opt.LoCol,Opt.HiCol,double(Ichar[ix][I]-LoVal)/Range);
      }
      else //few levels: use individual colors:
      { int lev;
        lev=0;
        while (lev<IndivData[Opt.ColorTrait].V.CV.LevelCount &&
               IndivData[Opt.ColorTrait].V.CV.Levels[lev]!=Ichar[ix][I])
          lev++;
        IBackCol = Opt.TraitCol[lev];
      }
    }
    else //dtS
    { int ix;
      ix=IndivData[Opt.ColorTrait].TypeIndex;
      if (Ilabel[ix][I]==-MAXINT) IBackCol=Opt.MVCol; //missing value
      else if (IndivData[Opt.ColorTrait].V.SV.LabelCount>MaxTraitColors)
      { //too many levels: treat as integer values
        IBackCol= ColorRange(Opt.LoCol,Opt.HiCol,
           double(Ilabel[ix][I])/IndivData[Opt.ColorTrait].V.SV.LabelCount);
      }
      else //few levels: use individual colors:
      { IBackCol = Opt.TraitCol[Ilabel[ix][I]];
      }
    }
  }
  if (Opt.IndFrames)
  { MFC->Pen->Color = clMedGray; //GreyScale(IBackCol)>128 ? clBlack : clWhite;
    if (!Opt.IFixedColor) IFrameCol[VPI] = MFC->Pen->Color;
  }
  else MFC->Pen->Color = IBackCol;
  if (DrawFrame) MFC->Pen->Width=LinePx;
  else
  { MFC->Pen->Width=1; MFC->Pen->Color=IBackCol; } //no frame visible
  MFC->Brush->Color = IBackCol;
  MFC->Brush->Style = bsSolid;
  MFC->Rectangle(0,0,PrIndWidth+1,PrIndHeight+1);
  MFC->Pen->Width=LinePx;

  //draw Ind Name:
  FntAttToFont(Opt.INameFont,MFC->Font);
  MFC->Font->Color = //Opt.INameCol;
      (GreyScale(IBackCol)>128 ? clBlack : clWhite);
  ts = MFC->TextExtent(Indiv[I].IName);
  if (Opt.IndFrames) a=ts.cy/4; //if frame, space between top and name
  else a=0; //if no frame, text starts at top
  if (Opt.NameCentered) MFC->TextOutA((PrIndWidth-ts.cx)/2, a, Indiv[I].IName);
  else MFC->TextOutA(0, a, " "+Indiv[I].IName); //left aligned with leading space

  if (Opt.IndivContents==icTraits && Opt.TraitSelCount>0)
  {
  }

  else if (Opt.IndivContents==icMarkers && Opt.MarkSelCount>0)
  { FntAttToFont(Opt.AlleleFont,MFC->Font);
    j=MFC->TextHeight("$");
    for (a=0; a<Opt.MarkSelCount; a++)
    { Loc=GetLocusByNr(Opt.MarkSelected[a]);
      for (par=0; par<Ploidy; par++)
      { //if (Opt.MarkCodeCol!=NULL) cannot happen anymore: if ALLELES read, at least 1 color created
        MFC->Font->Color = Opt.MarkCodeCol[Loc->AllCode[I][par]];
        if (Loc->AllIx[I][par]>=0) s=Loc->AllName[Loc->AllIx[I][par]];
        else if (Loc->AllIx[I][par]==-1) s=strUnknown[0];
        else if (Loc->AllIx[I][par]==-2) s=strNullHomoz;
        else s=strConfirmedNull;
        if (par==0 && Ploidy==2)
        { k=MFC->TextWidth(s);
          MFC->TextOut(ProbLeft[0]+ProbWidth-k, ProbTop+a*j, s);
        }
        else MFC->TextOut(ProbLeft[par], ProbTop+a*j, s);
      }
    }
  }

  else if (Opt.IndivContents==icIBDall && Opt.LG>-1)
  { FntAttToFont(Opt.AlleleFont,MFC->Font);
    j=MFC->TextHeight("$");
    n=0; //counts the number of actually drawn loci:
    for (loc=0; loc<LinkGrp[Opt.LG]->LocCount; loc++)
    { if (Opt.AllIBDmrk || Opt.SelIBDmrk[loc])
      { Loc=LinkGrp[Opt.LG]->Locus[loc];
        Ip=LinkGrp[Opt.LG]->IBDpos[Loc->IBDix];
        for (par=0; par<Ploidy; par++)
        { //determine most probable allele: its color, name and font attrib
          maxpr=0; maxa=0;
          for (a=0; a<IBDAlleleCount; a++)
                  if (Ip->AllProb[I][par][a] > maxpr)
                  { maxa=a; maxpr=Ip->AllProb[I][par][a]; }
          MFC->Font->Color = Opt.ACol[maxa];
          MFC->Font->Style = TFontStyles();
          if (10000*maxpr >= 240 * Opt.ProbBig)
             MFC->Font->Style = TFontStyles() << fsBold;
          else if (10000*maxpr < 240*Opt.ProbSmall)
                  MFC->Font->Style = TFontStyles() << fsItalic;

          //draw the allele:
          if (Loc->FounderAllele[maxa]==-1) s=strUnknown[0];
          //-2=strNullHomoz should not appear as FounderAllele
          else if (Loc->FounderAllele[maxa]==-3) s=strConfirmedNull;
          else s=Loc->AllName[Loc->FounderAllele[maxa]];
          if (par==0 && Ploidy==2)
          { k=MFC->TextWidth(s);
            MFC->TextOut(ProbLeft[0]+ProbWidth-k, ProbTop+n*j, s);
          }
          else MFC->TextOut(ProbLeft[par], ProbTop+n*j, s);
        } //for par
        n++;
      } //if (loc is drawn)
    } //for loc
  }

  else if (Opt.IndivContents==icIBDprob && Opt.LG>-1)
  { //draw the maternal and paternal linkage groups:
    for (par=0; par<Ploidy; par++)
    { //draw the LG[par] probability chart:
      //fill the AlPx array:
      for (loc=0; loc<LinkGrp[Opt.LG]->IBDCount; loc++) //loc points to IBD positions here!
      { Ip=LinkGrp[Opt.LG]->IBDpos[loc];
        for (a=0; a<IBDAlleleCount; a++)
        { AlPrb[a]=float(Ip->AllProb[I][par][a]) / 240; }
        ConvertToBase(AlPrb,AlPx[loc],IBDAlleleCount,ProbWidth-1);
        j=0; k=AlPx[loc][0];
        for (a=0; a<IBDAlleleCount; a++)
        { AlPx[loc][a]=j; j=j+k; k=AlPx[loc][a+1]; }
        AlPx[loc][IBDAlleleCount]=ProbWidth-1;
      }
      //draw all polygons:
      MFC->Pen->Width = 1;
      MFC->Brush->Style = bsSolid;
      for (a=0; a<IBDAlleleCount; a++)
      { if (LinkGrp[Opt.LG]->IBDCount==1)
        { //whole Prob rectangle shows IBD's at the one position
          //left top:
          Points[0].x = ProbLeft[par]+AlPx[0][a]+1;
          Points[0].y = ProbTop;
          //right bottom:
          Points[1].x = ProbLeft[par]+AlPx[0][a+1]+1;
          Points[1].y = ProbTop+ProbHeight;
          MFC->Pen->Color=Opt.ACol[a];
          MFC->Brush->Color=Opt.ACol[a];
          MFC->Rectangle(Rect(Points[0],Points[1]));
        }
        else //IBDCount>1
        { //left side, from top to bottom: (note: loc points to IBDpos elements)
          for (loc=0; loc<LinkGrp[Opt.LG]->IBDCount; loc++)
          { Points[loc].x = ProbLeft[par]+AlPx[loc][a]+1;
            Points[loc].y = ProbTop+IBDPx[loc];
          }
          //right side, from bottom to top:
          for (loc=LinkGrp[Opt.LG]->IBDCount-1; loc>=0; loc--)
          { Points[(2*LinkGrp[Opt.LG]->IBDCount)-1-loc].x = ProbLeft[par]+AlPx[loc][a+1]+1;
            Points[(2*LinkGrp[Opt.LG]->IBDCount)-1-loc].y = ProbTop+IBDPx[loc];
          }
          MFC->Pen->Color=Opt.ACol[a];
          MFC->Brush->Color=Opt.ACol[a];
          MFC->Polygon(Points, 2*LinkGrp[Opt.LG]->IBDCount-1);
        }
      }
      //Draw the locus lines:
      MFC->Pen->Width = (63+PrPPI) / 127; //integer division; approx. 0.2 mm
      MFC->Pen->Color = //Opt.IFrameCol;
          (GreyScale(IBackCol)>128 ? clBlack : clWhite);
      for (loc=0; loc<LinkGrp[Opt.LG]->LocCount; loc++)
      { if (Opt.SelIBDmrk==NULL || Opt.SelIBDmrk[loc])
        { //if locus selected, draw the locus line
          if (par==0 || Ploidy>2)
          { MFC->MoveTo(ProbLeft[par]-ProbWidth/20, ProbTop+LocPx[loc]);
            MFC->LineTo(ProbLeft[par]+ProbWidth, ProbTop+LocPx[loc]); }
          else
          { MFC->MoveTo(ProbLeft[1], ProbTop+LocPx[loc]);
            MFC->LineTo(ProbLeft[1]+ProbWidth+ProbWidth/20, ProbTop+LocPx[loc]); }
        }
        else
        { //if not selected, only draw a short indicator outside rectangle
          if (par==0 || Ploidy>2)
          { MFC->MoveTo(ProbLeft[par]-ProbWidth/20, ProbTop+LocPx[loc]);
            MFC->LineTo(ProbLeft[par], ProbTop+LocPx[loc]); }
          else
          { MFC->MoveTo(ProbLeft[1]+ProbWidth, ProbTop+LocPx[loc]);
            MFC->LineTo(ProbLeft[1]+ProbWidth+ProbWidth/20, ProbTop+LocPx[loc]); }
        }
      } //for loc
      //form rectangle from line segments: with StretchDraw, a rectangle
      //doesn't exactly resize like the polygons inside
      MFC->Brush->Style=bsClear;
      MFC->MoveTo(ProbLeft[par],ProbTop);
      MFC->LineTo(ProbLeft[par],ProbTop+ProbHeight);
      MFC->LineTo(ProbLeft[par]+ProbWidth,ProbTop+ProbHeight);
      MFC->LineTo(ProbLeft[par]+ProbWidth,ProbTop);
      MFC->LineTo(ProbLeft[par],ProbTop);
    } //for par
  } //LG>=0

  // else: IndivContents==icNone or invalid, or no Traits, Markers or LG selected:
  // nothing to draw

  delete MFC;
  return MF;
} //TView::DrawIndMF

/* attempt to draw the focused / selected rectangles with
   double line width.
   Doesn't work because the StretchDraw isn't pixel-exact; even of nice
   in the 200% zoom it isn't at the 100% zoom
void TView::DrawIndFrame(int VPI, TMetafileCanvas *MFC)
{ TColor C, SelectCol=clBlue, FocusCol=clRed, FocNSelCol=clFuchsia;
  TRect R,R2;
  bool selfoc;
  R=CalcIRect(VPI); //the normal outline of the indiv
  R2.left=R.left-LinePx%2-2;
  R2.top=R.top-LinePx%2-2;
  R2.right=R.right+LinePx%2+2;
  R2.bottom=R.bottom+LinePx%2+2;
  if (selfoc = (VP->Selected[VPI] || VPI==VP->FocusInd)) { //not ==
    if (VP->Selected[VPI]) {
      if (VPI==VP->FocusInd) C=FocNSelCol;
      else C=SelectCol;
    }
    else C=FocusCol; //not selected, so must be focused
  }
  else C=Opt.PageCol; //not selected, not focused
  //draw a double-width rectangle
  MFC->Pen->Color=C;
  MFC->Pen->Width=3*LinePx;;
  MFC->Brush->Style=bsClear;
  MFC->Rectangle(R2);
  if (!selfoc) {
    //draw the fingle-width frame
    if (Opt.IFixedColor) {
      if (Opt.IndFrames) C= clMedGray; //GreyScale(Opt.IBackCol)>128 ? clBlack : clWhite;
      else C=Opt.IBackCol;
    }
    else C=IFrameCol[VPI];
    MFC->Pen->Color=C;
    MFC->Pen->Width=LinePx;
    MFC->Rectangle(R);
  }
} //TView::DrawIndFrame
*/

void TView::DrawIndFrame(int VPI, TMetafileCanvas *MFC)
{ TColor C, SelectCol=clBlue, FocusCol=clRed, FocNSelCol=clFuchsia;
  TRect R;
  R=CalcIRect(VPI);
  if (VP->Selected[VPI])
  { if (VPI==VP->FocusInd) C=FocNSelCol;
    else C=SelectCol;
  }
  else
  { if (VPI==VP->FocusInd) C=FocusCol;
    else
    { if (Opt.IFixedColor)
      { if (Opt.IndFrames) C= clMedGray; //GreyScale(Opt.IBackCol)>128 ? clBlack : clWhite;
        else C=Opt.IBackCol;
      }
      else C=IFrameCol[VPI];
    }
  }
  MFC->Pen->Color=C;
  MFC->Pen->Width=LinePx*2;
  MFC->Brush->Style=bsClear;
  MFC->Rectangle(R);
} //TView::DrawIndFrame

TMetafile* TView::DrawCrossMF(int PixSize)
//PixSize is size in pixels of each side of square metafile
//A Crossing symbol is an X
{ TMetafile *MF;
  TMetafileCanvas *MFC;
  int Lw, //Line width, later half linewidth
      C, //Centre is (C,C)
      D; //radius * cos(PI/4)

  MF = new TMetafile;
  MF->Width = PixSize;
  MF->Height = PixSize;
  MFC = new TMetafileCanvas(MF, 0);
  Lw = PixSize / 10;
  if (Lw==0) Lw=1;
  MFC->Pen->Width = Lw;
  MFC->Pen->Color = Opt.CrossCol;
  MFC->Brush->Color = Opt.PageCol;
  Lw=Lw/2; //half linewidth, possibly 0
  //Visually it is more pleasing (compared with the other cross types)
  //to draw the cross at the same size of the cross inside the circle
  //as in DrawSelfMF:

  //MFC->MoveTo(Lw,Lw); MFC->LineTo(PixSize-Lw,PixSize-Lw);
  //MFC->MoveTo(Lw,PixSize-Lw); MFC->LineTo(PixSize-Lw,Lw);
  C = PixSize/2;
  D = (C-Lw) / M_SQRT2; // = r/sqrt(2) = r * cos(pi/4)
  //MFC->Ellipse(Lw,Lw,PixSize-Lw,PixSize-Lw);
  MFC->MoveTo(C-D,C-D); MFC->LineTo(C+D,C+D);
  MFC->MoveTo(C-D,C+D); MFC->LineTo(C+D,C-D);

  delete MFC;
  return MF;
} //TView::DrawCrossMF

TMetafile* TView::DrawSelfMF(int PixSize)
//PixSize is size in pixels of each side of square metafile
//A Selfing symbol is an X in a circle: (X)
{ TMetafile *MF;
  TMetafileCanvas *MFC;
  int Lw, //Line width, later half linewidth
      C, //Centre of circle is (C,C)
      D; //radius * cos(PI/4)
  MF = new TMetafile;
  MF->Width = PixSize;
  MF->Height = PixSize;
  MFC = new TMetafileCanvas(MF, 0);
  Lw = PixSize / 10;
  if (Lw==0) Lw=1;
  MFC->Pen->Width = Lw;
  MFC->Pen->Color = Opt.CrossCol;
  MFC->Brush->Color = Opt.PageCol;
  Lw = Lw/2; //half linewidth, possibly 0
  C = PixSize/2;
  D = (C-Lw) / M_SQRT2; // = r/sqrt(2) = r * cos(pi/4)
  MFC->Ellipse(Lw,Lw,PixSize-Lw,PixSize-Lw);
  MFC->MoveTo(C-D,C-D); MFC->LineTo(C+D,C+D);
  MFC->MoveTo(C-D,C+D); MFC->LineTo(C+D,C-D);

  delete MFC;
  return MF;
} //TView::DrawSelfing

TMetafile* TView::DrawMutMF(int PixSize)
//PixSize is size in pixels of each side of square metafile
//A Mutation symbol is a lightning symbol: a zig-zag arrow
{ const double Sq3=sqrt(3);
  const double AHead=0.25; //size of arrowhead as fraction of PixSize
  TMetafile *MF;
  TMetafileCanvas *MFC;
  int Lw, //Line width, later half linewidth
      X, Y; // X and Y of first zig-zag point
  MF = new TMetafile;
  MF->Width = PixSize;
  MF->Height = PixSize;
  MFC = new TMetafileCanvas(MF, 0);
  Lw = PixSize / 10;
  if (Lw==0) Lw=1;
  MFC->Pen->Width = Lw;
  MFC->Pen->Color = Opt.CrossCol;
  MFC->Brush->Color = Opt.PageCol;
  Lw = Lw/2; //half linewidth, possibly 0
  //Zig-zag consists of 3 lines inclined at 60 degrees;
  //Outer two through corners, middle one through center:
  X = 0.25 * PixSize * (1.0 + 1/Sq3);
  Y = 0.25 * PixSize * (3.0 - Sq3);
  MFC->MoveTo(Lw,PixSize-Lw); MFC->LineTo(X,Y);
  MFC->LineTo(PixSize-X,PixSize-Y);
  MFC->LineTo(PixSize-Lw,Lw);
  //plus 60-degree arrowhead:
  MFC->MoveTo(PixSize-0.5*Sq3*AHead*PixSize, 0.5*AHead*PixSize);
  MFC->LineTo(PixSize-Lw,Lw);
  MFC->LineTo(PixSize-Lw,AHead*PixSize);

  delete MFC;
  return MF;
} //TView::DrawMutMf

TMetafile* TView::DrawTextCrossMF(int PixSize, int CrType)
//PixSize is size in pixels of each side of square metafile
//A text in Arial Bold (fitting in the PixSize*PixSize square) is drawn as symbol
{ AnsiString Text;
  TSize TextSize;
  TMetafile *MF;
  TMetafileCanvas *MFC;
  MF = new TMetafile;
  MF->Width = PixSize;
  MF->Height = PixSize;
  MFC = new TMetafileCanvas(MF, 0);

  switch (CrType)
  { case ctDH : Text="DH"; break;
    case ctVegProp : Text="VP"; break;
    default : Text="?";
  }

  MFC->Font->Name="Arial";
  MFC->Font->Style = TFontStyles()<< fsBold;
  MFC->Font->Color=Opt.CrossCol;
  MFC->Brush->Color = Opt.PageCol;
  MFC->Font->Height=-PixSize;
  while (TextSize=MFC->TextExtent(Text), TextSize.cx>PixSize)
    MFC->Font->Height++;
  MFC->TextOut((PixSize-TextSize.cx)/2, (PixSize-TextSize.cy)/2, Text);

  delete MFC;
  return MF;
} //TView::DrawTextCrossMF

TMetafile* TView::GetCroMF(TCrossing *Cro) //draws if NULL
{ TMetafile *MF;
  switch (Cro->CType)
  { case ctCross   : { if (CrossMF==NULL) CrossMF=DrawCrossMF(PrCrossSize);
                      MF=CrossMF; } break;
    case ctSelfing : { if (SelfMF==NULL) SelfMF=DrawSelfMF(PrCrossSize);
                      MF=SelfMF; } break;
    case ctMutation: { if (MutMF==NULL) MutMF=DrawMutMF(PrCrossSize);
                      MF=MutMF; } break;
    case ctDH      : { if (DHMF==NULL) DHMF=DrawTextCrossMF(PrCrossSize,ctDH);
                      MF=DHMF; } break;
    case ctVegProp : { if (VegPropMF==NULL) VegPropMF=DrawTextCrossMF(PrCrossSize,ctVegProp);
                      MF=VegPropMF; } break;
    default : { if (ErrorMF==NULL) ErrorMF=DrawTextCrossMF(PrCrossSize,Cro->CType);
                MF=ErrorMF; }
  }
  return MF;
} //TView::GetCroMF

/*
void TView::CalcIPos(int G)
/*{ int i,space;
  space = (MaxXPix-MinXPix-GS[G]->ICount*IndWidth) / (GS[G]->ICount+1);
  for (i=0; i<GS[G]->ICount; i++)
  { IPos[G][i] = space + i * (IndWidth+space); }
{ for (int i=0; i<GS[G]->ICount; i++)
    GS[G]->IPos[i] = (i+1)/(GS[G]->ICount+1);
} //TView::CalcIPos

void TView::CalcGPos(void)
/*{ int g,space;
  space = (MaxYPix-MinYPix-GCount*IndHeight) / (GCount +1);
  for (g=0; g<GCount; g++)
  { GPix[g] = space + g * (IndHeight+space); }
{ for (int g=0; g<GCount; g++)
    GS[g]->GPos = (g+1)/(GCount+1);
} //TView::CalcGPos
*/
/*
TRect TView::CalcMargRect(int CurrPPI)
//calculates MinXPix ..MaxYPix
/*{ MinXPix=Opt.MargMM[0] * CurrPPI / 2540;
  MinYPix=Opt.MargMM[1] * CurrPPI / 2540;
  MaxXPix=(Opt.PgWidthMM-Opt.MargMM[2]) * CurrPPI /2540;
  MaxYPix=(Opt.PgHeightMM-Opt.MargMM[3]) * CurrPPI /2540;
{ TRect Rect;
  Rect.Left=Opt.MargMM[0] * CurrPPI / 2540;
  Rect.Top=Opt.MargMM[1] * CurrPPI / 2540;
  Rect.Right=(Opt.PgWidthMM-Opt.MargMM[2]) * CurrPPI /2540;
  Rect.Bottom=(Opt.PgHeightMM-Opt.MargMM[3]) * CurrPPI /2540;
  return Rect;
} //TView::CalcChartBounds

bool TView::CalcSizesNPositions(TCanvas *CV, int CurrPPI)
//returns true if chart will fit on page
//CurrPPI=PPI for drawing on screen, PrPPI for drawing metafile (for printing etc)
{ TRect Rect;
  bool Fits;

  // Calculate pixel size of Ind metafiles at Printer resolution:
  // (Ind metafiles will be stretch-drawn to fit both screen and printer pages)
  CV->Font->PixelsPerInch=PrPPI;
  FntAttToFont(Opt.INameFont,CV->Font);
  INameWidth=MaxINameWidth(CV) + CV->TextWidth("  ");
  INameHeight=2*CV->TextHeight("  ");

  if (Opt.ShowProb && Opt.LG > -1)
  { ProbWidth= Opt.ProbWidthMM * PrPPI / 2540;
    if (INameWidth < 11*ProbWidth/4) PrIndWidth=11*ProbWidth/4;
    else PrIndWidth=INameWidth;
    LGLeft[0]= (PrIndWidth/2-9*ProbWidth/8); //separation between parentals is 1/4 ProbWidth
    LGLeft[1]= (PrIndWidth/2+ProbWidth/8);
    LGTop= INameHeight;
    ProbHeight= Opt.ProbHeightMM * PrPPI / 2540;
    PrIndHeight= 1.5 * INameHeight + ProbHeight;
  }
  else //probability rectangles not shown
  { LGLeft[0]=LGLeft[1]=LGTop=ProbWidth=ProbHeight=0;
    PrIndWidth = INameWidth;
    PrIndHeight = INameHeight;
  }
  // Calculate Ind sizes at screen of printer resolution CurrPPI:
  //IndWidth= PrIndWidth * CurrPPI / PrPPI;
  //IndHeight = PrIndHeight * CurrPPI / PrPPI;
  //size of chart at current resolution:
  CalcChartBounds(CurrPPI);
  Fits= (MaxYPix-MinYPix > GCount * IndHeight) &&
        (MaxXPix-MinXPix > MaxIndPerGS * IndWidth);
  if (Fits)
  { //calculate GPix and IPix:
    CreateGraphicsStruct();
    CalcGPos();
    for (int g=0; g<GCount; g++) CalcIPos(g);
  }
  else ShowMessage("Chart does not fit on page");
  return Fits;
} //TView::CalcSizesNPositions
*/
/*
int GetCy(int Cx, int dx, int dy, int r2, int px, int py)
// Used by DrawArc; returns the y-coordinate Cy for a given Cx coordinate of
// a point on the midline between PP and PO
{ return int(0.5+ -(dx/dy)*(Cx-px) + r2/(2*dy) + py); }

int GetCx(int Cy, int dx, int dy, int r2, int px, int py)
// as GetCy
{ return int(0.5+ -(dy/dx)*(Cy-py) + r2/(2*dx) + px); }
*/

int GetCy(TPoint PO, TPoint PP, int Cx)
{ int dx,dy; //double dxy;
  dx=PO.x-PP.x; dy=PO.y-PP.y; //dxy=double(dx)/dy;
  //return int( 0.5 + dxy*((PO.x+PP.x)/2+PO.x) + (PO.y+PP.y)/2 );
  return int (0.5+ 0.5*(PP.y+PO.y) - (0.5*dx*dx)/dy) ;
}

int GetCx(TPoint PO, TPoint PP, int Cy)
{ int dx,dy; //double dyx;
  dx=PO.x-PP.x; dy=PO.y-PP.y; //dyx=double(dy)/dx;
  //return int( 0.5 + dyx*((PO.y+PP.y)/2+PO.y) + (PO.x+PP.x)/2 );
  return int (0.5 - 0.5*(PP.x+PO.x) - (0.5*dy*dy)/dx) ;
}

void TView::DrawArc(TCanvas *CV, TPoint PP, TPoint PO, TRect Marg)
/* Arcs are drawn instead of straight line segments if a parent occurs more than
   1 generation before the offspring (to avoid complete overlap of lines when
   both parents and the offspring are exactly aligned).

   In the discussion here Opt.LeftRight is assumed false, the values if true are
   indicated (within brackets).

   The arc (a 60-degree circle segment) is from PP: center of Parent bottom (Parent
   right) to PO: center of Offspring top (Offspring left). The bulge of the arc
   points away from the parent.

   Exceptions:
   1. if the arc would go further down (right) than the top (left) side of the
      offspring, it is replaced by a straight line.
      ( Earlier solution: make the arc less than 60 deg with a larger radius,
        by setting the circle center (C.x,C.y) on the midline between PP and PO,
        with C.y (C.x) equal to PO.y (PO.x); with formulas:
        C.x=PO.x; C.y=int (0.5 - 0.5*(PP.y+PO.y) - (0.5*dx*dx)/dy); r=PO.y-C.y;
        (C.y=PO.y; C.x=int (0.5 - 0.5*(PP.x+PO.x) - (0.5*dy*dy)/dx); r=PO.x-C.x; )
        Problems with that were:
        a. arcs could become very flat, with very large radius; this led to
           inaccurate endpoints (rounding errors)
        b. Flat arcs arriving horizontally (vertically) at PO are not easily followed
           back to PP: they appear as a horizontal (vertical) line
      )
   2. if the arc would go outside the margin it is replaced with a flatter arc
      tangential to the margin (using CalcCircles algorithm from Apollonius.bpr).
*/
// Marg is the Margins Rectangle (in pixels)
{ TPoint C; //centre of circle
  int r, sign;
  long int dx,dy;
  const double Sqrt3=sqrt(3);

  try  //catches overflows; in that case, draw straight line
  { dx=PO.x-PP.x; dy=PO.y-PP.y;
    //First: check if a straight line should be drawn (see Exception 1 above):
    if ( (Opt.LeftRight && dy*dy >= 3*dx*dx) ||
         (!Opt.LeftRight && dx*dx >= 3*dy*dy) )
    { CV->MoveTo(PO.x,PO.y); CV->LineTo(PP.x,PP.y); }
    else //no straight line, draw arc
    { //catch overflows (sqrt will always show an error message to the user)
      r=dx*dx+dy*dy;
      if (r<0 || r>1000000000) {
        //throw Exception("Arc overflow");
        //probably overflow error in arc calculation, draw straight line:
        CV->MoveTo(PO.x,PO.y); CV->LineTo(PP.x,PP.y);
      }
      else { //no Arc overflow
        r=int(0.5+sqrt(r)); //for 60 dg arc: radius==distance endpoints
        /* sign determines direction of bulge. There are four cases:
           LeftRight sign
             false    -1  arc bulges down-left
             false     1  arc bulges down-right
             true     -1  arc bulges right-down
             true      1  arc bulges right-up
        */
        if ( ( !Opt.LeftRight && PO.x>PP.x ) ||
             (  Opt.LeftRight && PO.y<PP.y ) )
           sign=1;
        else sign=-1;

        //Calculate C: center of arc-circle:
        C.x= PP.x + int(( sign*Sqrt3*dy + dx +1) / 2);
        C.y= PP.y + int((-sign*Sqrt3*dx + dy +1) / 2);
        //Check if arc outside margins; draw arc
        if (Opt.LeftRight)
        { if ( C.x>PP.x &&  //else the arc goes directly in right direction
               ( (sign==-1 && C.y-Marg.Top<r) || (sign==1 && Marg.Bottom-C.y<r) )
             )
          { //arc goes outside margin; reverse the arc (use the alternative circle centre)
            //sign=-sign;
            //C.x= PP.x + int(( sign*Sqrt3*dy + dx +1) / 2);
            //C.y= PP.y + int((-sign*Sqrt3*dx + dy +1) / 2);
            //New: calculate flatter arc tangential to Margin:
            if (sign==1) CalcCircles(PO,PP,Marg.Bottom,true,C,r);
            else CalcCircles(PO,PP,Marg.Top,true,C,r);
          }
          if (r==0) //no fitting circle, draw straight line
          { CV->MoveTo(PO.x,PO.y); CV->LineTo(PP.x,PP.y); }
          else if (sign==1)
            CV->Arc(C.x-r,C.y-r,C.x+r,C.y+r,PP.x,PP.y,PO.x,PO.y);
          else
            CV->Arc(C.x-r,C.y-r,C.x+r,C.y+r,PO.x,PO.y,PP.x,PP.y);
        }
        else //not Opt.LeftRight
        { if ( C.y>PP.y &&   //else the arc goes directly in right direction
               ( (sign==1 && C.x-Marg.Left<r) || (sign==-1 && Marg.Right-C.x<r) )
             )
          { //arc goes outside margin; reverse the arc (use the alternative circle centre)
            //sign=-sign;
            //C.x= PP.x + int(( sign*Sqrt3*dy + dx +1) / 2);
            //C.y= PP.y + int((-sign*Sqrt3*dx + dy +1) / 2);
            //New: calculate flatter arc tangential to Margin:
            if (sign==1) CalcCircles(PO,PP,Marg.Left,false,C,r);
            else CalcCircles(PO,PP,Marg.Right,false,C,r);
          }
          if (r==0) //no fitting circle, draw straight line
          { CV->MoveTo(PO.x,PO.y); CV->LineTo(PP.x,PP.y); }
          else if (sign==1)
            CV->Arc(C.x-r,C.y-r,C.x+r,C.y+r,PP.x,PP.y,PO.x,PO.y);
          else
            CV->Arc(C.x-r,C.y-r,C.x+r,C.y+r,PO.x,PO.y,PP.x,PP.y);
        }
      } //no Arc overflow  
    } //else draw arc
  }
  catch (...)
  { //probably overflow error in arc calculation, draw straight line:
    CV->MoveTo(PO.x,PO.y); CV->LineTo(PP.x,PP.y);
  }
} //TView::DrawArc

inline int RelPos2Pix(int NetPgSize, float RelPos)
// converts the relative position RelPos to the pixel position from left or top margin,
// given the NetPgSize (between the margins) in pixels
{ return int(0.5+ RelPos*NetPgSize);
} //RelPos2Pix

void TView::DeleteIndMetafiles(void)
//only deletes the metafiles, not the IndMF array of each GS
{ int g,i;
  for (g=0; g<GCount; g++)
  { for (i=0; i<GS[g]->ICount; i++)
    { delete GS[g]->IndMF[i];
      GS[g]->IndMF[i]=NULL;
    }
  }
  //PrIndWidth=-1; PrIndHeight=-1;
} //TView::DeleteIndMetafiles

void TView::DeleteCrossMetafiles(void)
{ delete CrossMF; CrossMF=NULL;
  delete SelfMF;  SelfMF=NULL;
  delete DHMF;    DHMF=NULL;
  delete MutMF;   MutMF=NULL;
  delete VegPropMF; VegPropMF=NULL;
  delete ErrorMF; ErrorMF=NULL;
} //TView::DeleteCrossMetafiles

bool TView::FindCrossing(TCrossing *Cro, int &G, int &I, int MaxGener)
//finds Cro in the CS array.
//if found, returns the generation and the index within the generation in G and I.
//Searches backwards, from generation MaxGener downto 1 (
//(set MaxGener to GCount-1 to search all generations)
//Return value: true if found, false if not.
{ bool Found;
  Found=false;
  G=MaxGener;
  while (!Found && G>0)
  { I=0;
    while (I<CS[G]->CCount && CS[G]->Cross[I]!=Cro) I++;
    Found= I<CS[G]->CCount;
    if (!Found) G--;
  }
  if (!Found) {G=-1; I=-1;}
  return Found;
} //TView::FindCrossing

TRect TView::CalcIRect(int VPI)
{ if (VPI<0 || VPI>=VP->VPIndCount)
  { ShowMessage("VPI not valid in CalcIRect"); return Rect(0,0,0,0); }
  return CalcIRect(Gener[VPI], GIx[VPI]);
};

TRect TView::CalcIRect(int G, int I)
{ TPoint PO;
  // nieuw 17-6-04: voor focus/ selection shape
  if (G<0 || G>=GCount ||
      I<0 || I>=GS[G]->ICount)
  { ShowMessage("G or I not valid in CalcIRect"); return Rect(0,0,0,0); }
  /*calc PO = topleft corner of this individual ("Offspring")
  if (Opt.LeftRight)
  { PO.x=PrMargPx + GS[G]->GPos;
    PO.y=PrMargPx + GS[G]->IPos[I];
  }
  else
  { PO.x=PrMargPx + GS[G]->IPos[I];
    PO.y=PrMargPx + GS[G]->GPos;
  }
  return TRect ( PO.x, PO.y, PO.x+PrIndWidth, PO.y+PrIndHeight);  */
  if (Opt.LeftRight) return TRect(GS[G]->GPos, GS[G]->IPos[I],
                                  GS[G]->GPos + PrIndWidth+1,
                                  GS[G]->IPos[I] + PrIndHeight+1);
  else return TRect(GS[G]->IPos[I], GS[G]->GPos,
                    GS[G]->IPos[I] + PrIndWidth+1,
                    GS[G]->GPos + PrIndHeight+1);
}; //TView::CalcIRect


TMetafile* TView::DrawChartExportMF(TStringList* coord)
{ Graphics::TMetafile *MF;

  DeleteIndMetafiles();
  MF=DrawChartMF(true, coord); //draw individuals with frames
  DeleteIndMetafiles();
  return MF;
} //TView::DrawChartExportMF

TMetafile* TView::DrawChartMF(bool DrawIndFrames, TStringList* coord) {
//draws the chart with 10 mm margins, including background color if not transparent
//size must be set to PrChartWidth, PrChartHeight before calling.
//Metafile is drawn always in PrPPI pixels, no CurrPPI any more,
//scaling of metafile is done when (zoomed) picture is shown.
//SelBorderWidth is not set here any more
//Frame rectangle is discarded
//Note: CalcInitialPositions must have been called before
//new: coord is a TStringlist, each line will contain the name and coordinates
//(left, top, right, bottom) of an individual

  int j, k, //indices of Ind resp Cross in generation/Cross-Strips
      g, cg,//generation nr of indiv/cross and parental cross
      p,    //index of parent (0 or 1)
      VPI,  //index of Indiv in VP->VPIndiv
      VPa,  //index of parental Indiv in VP->VPIndiv
      I;    //index in Indiv array
  TMetafile *MF;
  TMetafileCanvas *CV;
  TPoint PP,PO,CR; //points on Parent and Offspring and Cross where connections start
  bool Uniparent;
  TCrossing *Cro;
  float StartCM, EndCM; //cM position of first and last locus

  coord->Clear();
  MF = new TMetafile;
  MF->Width  = PrChartWidth;
  MF->Height = PrChartHeight;
  CV = new TMetafileCanvas(MF,0); //screen canvas

  if (VP->VPIndCount>IndivDrawLimit)
    CV->Draw(0,0,DrawNoImageMessageMF());
  else {

    /*debug:
    CV->Brush->Style=bsClear; CV->Pen->Width=1;
    CV->Pen->Color=clFuchsia;
    CV->Rectangle(0,0,PrChartWidth,PrChartHeight);
    CV->Pen->Color=clGreen;
    CV->Rectangle(PrMargPx,PrMargPx,PrChartWidth-PrMargPx,PrChartHeight-PrMargPx);
    */

    //if Cross symbols must be drawn, make sure that the current metafiles are erased
    //(so the new ones will be drawn according to current Options)
    if (Opt.ShowCrossings) DeleteCrossMetafiles();

    //Ind sizes and relative Ind positions must be available here !
    //GS[g]->IndMF[j] must be up-to-date or NULL

    //in case some or all IndMF need to be redrawn the following arrays are needed:
    if (Opt.IndivContents==icIBDprob && Opt.LG > -1)
    { LocPx=new int [LinkGrp[Opt.LG]->LocCount];
      IBDPx=new int [LinkGrp[Opt.LG]->IBDCount];
      AlPrb=new float [IBDAlleleCount];
      Points=new TPoint [2 * LinkGrp[Opt.LG]->IBDCount];
      AlPx= new short int* [LinkGrp[Opt.LG]->IBDCount];
      for (j=0; j<LinkGrp[Opt.LG]->IBDCount; j++) AlPx[j] = new short int [IBDAlleleCount+1];
      //calculate LocPx and IBDPx, relative to LGTop (assumed Locus[0] at 0 cM):
      StartCM=LinkGrp[Opt.LG]->IBDpos[0]->cM;
      EndCM=LinkGrp[Opt.LG]->IBDpos[LinkGrp[Opt.LG]->IBDCount-1]->cM;
      for (j=0; j<LinkGrp[Opt.LG]->LocCount; j++)
      { if (EndCM==StartCM) LocPx[j]= ProbHeight/2; //one locus line in middle of rectangle
        else LocPx[j] = (LinkGrp[Opt.LG]->Locus[j]->cM - StartCM) /
                        (EndCM - StartCM)  *
                        ProbHeight +0.5;  //0.5 for rounding   */
      }
      for (j=0; j<LinkGrp[Opt.LG]->IBDCount; j++)
      { if (EndCM==StartCM) IBDPx[j]= ProbHeight/2; //one IBD in middle of rectangle
        else IBDPx[j] = (LinkGrp[Opt.LG]->IBDpos[j]->cM - StartCM) /
                        (EndCM - StartCM)  *
                         ProbHeight +0.5;  //0.5 for rounding   */
      }
    }

    //set the width for all lines:
    CV->Pen->Width = LinePx; // =(63+PrPPI) / 127; approx. 0.2 mm

    //draw the IndMF and CrossMF at their positions, and the lines to their parents
    if (Opt.IFixedColor)
    { delete[] IFrameCol; IFrameCol=NULL; }
    else if (IFrameCol==NULL) IFrameCol = new TColor[VP->VPIndCount];

    for (g=0; g<GCount; g++)
    { if (g>0 && Opt.ShowCrossings)
      { //Draw cross symbols and lines to parents:
        for (k=0; k<CS[g]->CCount; k++)
        { Cro=CS[g]->Cross[k];
          //draw cross symbol (CR=CRoss topleft corner):
          if (Opt.LeftRight)
          { CR.x=CS[g]->CGPos;
            CR.y=CS[g]->CIPos[k];
          }
          else
          { CR.x=CS[g]->CIPos[k];
            CR.y=CS[g]->CGPos;
          }
          CV->Draw( CR.x, CR.y, GetCroMF(Cro) );
          //draw lines from cross symbol to parents
          Uniparent=Cro->Parent[1]==Cro->Parent[0];
          if (Opt.LeftRight) CR.y=CR.y+PrCrossSize/2; //CR: connection at left center
          else CR.x=CR.x+PrCrossSize/2;               //CR: connection at top center
          for (p=0; p<2; p++)  //for both parents of Cro
          { if ( (p==0 || !Uniparent) &&
                 (VPa=VP->GetVPIndex(Cro->Parent[p])) >=0 )
            { //Pa present in VP and not already done; draw line from I to Pa:
              CV->Pen->Color= Uniparent ? Opt.ParSelfedCol :
                              p==0 ? Opt.Par1Col : Opt.Par2Col ;
              //calc connection point on parent (PP):
              if (Opt.LeftRight)   //PP: connection at right center:
              { PP.x= GS[Gener[VPa]]->GPos + PrIndWidth;
                PP.y= GS[Gener[VPa]]->IPos[GIx[VPa]] + PrIndHeight/2;
              }
              else                 //PP: connection at bottom center:
              { PP.x= GS[Gener[VPa]]->IPos[GIx[VPa]] + PrIndWidth/2;
                PP.y= GS[Gener[VPa]]->GPos + PrIndHeight;
              }
              if (g-Gener[VPa]==1)
              { CV->MoveTo(CR.x, CR.y);
                CV->LineTo(PP.x, PP.y);
              }
              else DrawArc(CV, PP, CR, Rect(0,0,PrChartWidth,PrChartHeight) );
            }
          } // for p
        } //for k
      } //if ShowCrossings
      //draw individuals:
      for (j=0; j<GS[g]->ICount; j++)
      { VPI=GS[g]->Ind[j]; I=VP->VPInd[VPI]; //index of individual in VPInd and Indiv
        if (GS[g]->IndMF[j]==NULL) GS[g]->IndMF[j]=DrawIndMF(VPI,DrawIndFrames);
        //calc PO = topleft corner of this individual ("Offspring")
        if (Opt.LeftRight)
        { PO.x=GS[g]->GPos;
          PO.y=GS[g]->IPos[j];
        }
        else
        { PO.x=GS[g]->IPos[j];
          PO.y=GS[g]->GPos;
        }
        CV->Draw( PO.x, PO.y, GS[g]->IndMF[j] );
        coord->Add(Indiv[I].IName+Tb+
                   IntToStr(PO.x)+Tb+
                   IntToStr(PO.y)+Tb+
                   IntToStr(PO.x+GS[g]->IndMF[j]->Width)+Tb+
                   IntToStr(PO.y+GS[g]->IndMF[j]->Height));
        if (!VP->IsVPFounder(VPI))
        { //draw lines from Offspring (PO) to Parents (PP) or parental CRoss (CR):
          if (Opt.LeftRight)
          { PO.y=PO.y+PrIndHeight/2; //left center of current Indiv ("Offspring")
            if (Opt.ShowCrossings)
            { Cro=Indiv[I].ParentCross;
              if (!FindCrossing(Cro,cg,k,g))
              {ShowMessage("Parental Cross not found"); exit;}
              //connect to right center of parental cross:
              CR.x=CS[cg]->CGPos + PrCrossSize;
              CR.y=CS[cg]->CIPos[k] + PrCrossSize/2;
              CV->Pen->Color=Opt.CrossCol;
              if (g-cg==0)
              { CV->MoveTo(PO.x, PO.y);
                CV->LineTo(CR.x, CR.y);
              }
              else DrawArc(CV, CR, PO, Rect(0,0,PrChartWidth,PrChartHeight));
            } //if ShowCrossings
            else //not ShowCrossings, connect to parents (still LeftRight)
            { Uniparent=Indiv[I].ParentCross->Parent[0]==Indiv[I].ParentCross->Parent[1];
              for (p=0; p<2; p++)  //for both parents of I
              { if ( (p==0 || !Uniparent) &&
                     (VPa=VP->GetVPIndex(Indiv[I].ParentCross->Parent[p])) >=0 )
                { //Pa present in VP and not already done; draw line from I to Pa:
                  CV->Pen->Color= Uniparent ? Opt.ParSelfedCol :
                                  p==0 ? Opt.Par1Col : Opt.Par2Col ;
                  //PP: connection at middle right:
                  PP.x= GS[Gener[VPa]]->GPos + PrIndWidth;
                  PP.y= GS[Gener[VPa]]->IPos[GIx[VPa]] + PrIndHeight/2;
                  if (g-Gener[VPa]==1)
                  { CV->MoveTo(PO.x, PO.y);
                    CV->LineTo(PP.x, PP.y);
                  }
                  else DrawArc(CV, PP, PO, Rect(0,0,PrChartWidth,PrChartHeight) );
                }
              } // for p
            } //if ShowCrossings else
          } //if Opt.LeftRight
          else //not Opt.LeftRight
          { PO.x=PO.x+PrIndWidth/2; //top center of current Indiv ("Offspring")
            if (Opt.ShowCrossings)
            { Cro=Indiv[I].ParentCross;
              if (!FindCrossing(Cro,cg,k,g))
              {ShowMessage("Parental Cross not found"); exit;}
              //connect to center-bottom of parental cross:
              CR.x=CS[cg]->CIPos[k] + PrCrossSize/2;
              CR.y=CS[cg]->CGPos + PrCrossSize;
              CV->Pen->Color=Opt.CrossCol;
              if (g-cg==0)
              { CV->MoveTo(PO.x, PO.y);
                CV->LineTo(CR.x, CR.y);
              }
              else DrawArc(CV, CR, PO, Rect(0,0,PrChartWidth,PrChartHeight));
            } //if ShowCrossings
            else //not ShowCrossings, connect to parents (still not LeftRight)
            { Uniparent=Indiv[I].ParentCross->Parent[0]==Indiv[I].ParentCross->Parent[1];
              for (p=0; p<2; p++)  //for both parents of I
              { if ( (p==0 || !Uniparent) &&
                     (VPa=VP->GetVPIndex(Indiv[I].ParentCross->Parent[p])) >=0 )
                { //Pa present in VP and not already done; draw line from I to Pa:
                  CV->Pen->Color= Uniparent ? Opt.ParSelfedCol :
                                  p==0 ? Opt.Par1Col : Opt.Par2Col ;
                  //PP: connection at middle of bottom:
                  PP.x= GS[Gener[VPa]]->IPos[GIx[VPa]] + PrIndWidth/2;
                  PP.y= GS[Gener[VPa]]->GPos + PrIndHeight;
                  if (g-Gener[VPa]==1)
                  { CV->MoveTo(PO.x, PO.y);
                    CV->LineTo(PP.x, PP.y);
                  }
                  else DrawArc(CV, PP, PO, Rect(0,0,PrChartWidth,PrChartHeight));
                }
              } // for p
            } //if ShowCrossings else
          } //if Opt.LeftRight else
        } //!VPFounder
      } //for j
    } //for g

    //free the arrays if they were used:
    if (Opt.IndivContents==icIBDprob && Opt.LG > -1)
    { for (j=0; j<LinkGrp[Opt.LG]->IBDCount; j++) delete[] AlPx[j];
      delete[] AlPx; AlPx=NULL;
      delete[] AlPrb; AlPrb=NULL;
      delete[] Points; Points=NULL;
      delete[] LocPx; LocPx=NULL;
      delete[] IBDPx; IBDPx=NULL;
    }

  } //not too many individuals
  delete CV;
  /* adjust MF to true print size (doesn't work as listed here)
  MF->SaveToFile(TempDir+"Chart.emf");
  j=0.5+2540.0*MF->Width/PrPPI;  //true width in 0.01 mm units
  k=0.5+2540.0*MF->Height/PrPPI;
  EditEMFfile( (TempDir+"Chart.emf").c_str(),j,k,PrPPI);
  delete MF;
  MF->LoadFromFile(TempDir+"Chart.emf");
  DeleteFile(TempDir+"Chart.emf");
  */
  return MF;
} //TView::DrawChartMF

/*
TMetafile* TView::DrawMetafile(void)
{ TMetafile *MF;
  TMetafileCanvas *MFC;
  TRect R ;
  int a1,a2,a3; //dummy parameters for DrawChart

  R=CalcMargRect(PrPPI);
  R=Rect(0,0,R.Right-R.Left,R.Bottom-R.Top); //zero-based: margins not in Metafile
  MF = new TMetafile;
  MF->Width=R.Right+1;
  MF->Height=R.Bottom+1;
  MFC=new TMetafileCanvas(MF,0); //screen canvas
  if (!Opt.PgTransp)
  { MFC->Brush->Style=bsSolid; MFC->Brush->Color=Opt.PageCol;
    MFC->FillRect(Rect(0,0,MF->Width+1,MF->Height+1));
  }
  DrawChart(MFC,R,PrPPI,a1,a2,a3);
  delete MFC;
  //hier MF nog bewerken naar echte printer afmetingen
  MF->SaveToFile("Chart.emf");
  return MF;
} //TMetafile* TView::DrawMetafile
*/

void TView::CalcZoomData(void)
//Based on Zoom and PrPPI
{ ZoomFact = Zoom/100.0 * Screen->PixelsPerInch / double(PrPPI); //in pix
  ZoomedChartWidth  = ZoomFact * ChartMF->Width +1;
  ZoomedChartHeight = ZoomFact * ChartMF->Height+1;
} //TView::CalcZoomData

TPoint TView::CalcNoImageMessageSize(void) {
  Graphics::TBitmap *BM;
  TCanvas *CV;
  AnsiString s;
  TPoint Result;
  int i;
  BM= new Graphics::TBitmap;
  CV= BM->Canvas;
  CV->Font->PixelsPerInch=PrPPI;
  CV->Font->Name="Arial";
  CV->Font->Size=12;
  Result.x=CV->TextWidth("No images are drawn for populations");
  i=CV->TextWidth("with more than "+IntToStr(IndivDrawLimit)+" individuals");
  if (i>Result.x) Result.x=i;
  i=CV->TextWidth("Change draw limit with command line option /DL=<number>");
  if (i>Result.x) Result.x=i;
  Result.y=3*CV->TextHeight("A");
  return(Result);
}

TMetafile* TView::DrawNoImageMessageMF(void) {
  TMetafile *MF;
  TMetafileCanvas *MFC;
  int lineheight;

  MF = new TMetafile;
  MF->Width = PrChartWidth;
  MF->Height = PrChartHeight;
  MFC = new TMetafileCanvas(MF,0);
  MFC->Font->PixelsPerInch=PrPPI;
  MFC->Brush->Style=bsSolid; MFC->Brush->Color=clWhite;
  MFC->FillRect(Rect(0,0,MF->Width+1,MF->Height+1));
  MFC->Font->Name="Arial"; MFC->Font->Size=9; MFC->Font->Color=clBlack;
  lineheight=MFC->TextHeight("A");;
  MFC->TextOut(PrMargPx,PrMargPx+lineheight, "No images are drawn for populations");
  MFC->TextOut(PrMargPx,PrMargPx+2*lineheight, "with more than "+IntToStr(IndivDrawLimit)+" individuals");
  MFC->TextOut(PrMargPx,PrMargPx+3*lineheight, "Change draw limit with command line option /DL=<number>");
  delete MFC;
  return MF;
} //TView::DrawNoImageMessageMF

void TView::CalcIndivNCrossSize(void)
//calculates this.PrIndWidth and -Height (=pixel size of Ind metafiles),
//and PrCrossSize, in printer pixels
// (Ind and Cross metafiles will be stretch-drawn to fit both screen and printer pages)
{ Graphics::TBitmap *BM;
  TCanvas *CV;
  AnsiString s;
  int i,j,p,w;
  TLocus *Loc;
  // use a new canvas for calculations; nothing is drawn
  BM= new Graphics::TBitmap;
  CV= BM->Canvas;
  CV->Font->PixelsPerInch=PrPPI;
  FntAttToFont(Opt.INameFont,CV->Font);
  INameWidth=MaxINameWidth(CV) + CV->TextWidth("  ");
  if (Opt.IndFrames) INameHeight=1.5*CV->TextHeight("  ");
  else INameHeight=CV->TextHeight("  "); //only room for name itself
  if (Opt.IndivContents==icTraits && Opt.TraitSelCount>0)
  { //while not implemented, leave Opt.TraitSelCount=0
  }

  else if (Opt.IndivContents==icIBDprob && Opt.LG > -1)
  { ProbWidth= Opt.ProbWidthMM * PrPPI / 2540;
    //keep a space of 1/4 ProbWidth between (sides and) Prob-rectangles:
    if (INameWidth < (1+5*Ploidy)*ProbWidth/4) PrIndWidth=(1+5*Ploidy)*ProbWidth/4;
    else PrIndWidth=INameWidth;
    for (p=0; p<Ploidy;p++)
      ProbLeft[p]=ProbWidth/4 + (5*p*ProbWidth)/4; //separation between prob rectangles is 1/4 ProbWidth
    //ProbLeft[0] = (PrIndWidth/2-9*ProbWidth/8); //separation between prob rectangles is 1/4 ProbWidth
    //ProbLeft[1] = (PrIndWidth/2+ProbWidth/8);
    ProbTop = INameHeight;
    ProbHeight = Opt.ProbHeightMM * PrPPI / 2540;
    PrIndHeight = 1.5 * INameHeight + ProbHeight;
  }

  else if (Opt.IndivContents==icIBDall && Opt.LG > -1)
  { //calculate max. width of Ploidy columns of allele names with separating space:
    FntAttToFont(Opt.AlleleFont,CV->Font);
    CV->Font->Style=TFontStyles() << fsBold; //measure size with bold

    ProbWidth=CV->TextWidth("W"); //widest single char; covers missing or NULL/Homoz scores
    for (i=0; i<LinkGrp[Opt.LG]->LocCount; i++)
    { Loc=LinkGrp[Opt.LG]->Locus[i];
      for (j=0; j<Loc->AllNameCount; j++)
      { if (CV->TextWidth(Loc->AllName[j]) > ProbWidth)
          ProbWidth=CV->TextWidth(Loc->AllName[j]);
      }
    }
    i=CV->TextWidth(" ");
    j=Ploidy*ProbWidth+(Ploidy+1)*i; //one space at each end and between columns;
    if (INameWidth<j) {PrIndWidth=j; w=i;}
    else
    { PrIndWidth=INameWidth;
      w=(PrIndWidth - Ploidy*ProbWidth - (Ploidy-1)*i) / 2; //left margin
    }
    for (p=0; p<Ploidy; p++)
      ProbLeft[p]=w+p*(ProbWidth+i);
    //ProbLeft[0] = PrIndWidth/2 - CV->TextWidth(" ")/2; //ref. for right aligned column 0
    //ProbLeft[1] = ProbLeft[0] + CV->TextWidth(" ");    //ref. for left aligned column 1
    ProbTop = INameHeight; // =1.5 * font height: 0.25 above & 0.25 below name
    //count j = number of markers to display:
    if (Opt.AllIBDmrk) j=LinkGrp[Opt.LG]->LocCount;
    else
    { j=0;
      for (i=0; i<LinkGrp[Opt.LG]->LocCount; i++)
      { if (Opt.SelIBDmrk[i]) j++; }
    }
    ProbHeight = j * CV->TextHeight("$");
    PrIndHeight = 1.5 * INameHeight + ProbHeight; //at bottom another 0.5 Nameheight = 0.75 font height
  }

  else if (Opt.IndivContents==icMarkers && Opt.MarkSelCount>0)
  { //calculate max. width of two columns of allele names with separating space:
    FntAttToFont(Opt.AlleleFont,CV->Font);
    ProbWidth=CV->TextWidth("W"); //widest single char; covers missing or NULL/Homoz scores
    for (i=0; i<Opt.MarkSelCount; i++)
    { Loc=GetLocusByNr(Opt.MarkSelected[i]);
      for (j=0; j<Loc->AllNameCount; j++)
      { if (CV->TextWidth(Loc->AllName[j]) > ProbWidth)
          ProbWidth=CV->TextWidth(Loc->AllName[j]);
      }
    }
    i=CV->TextWidth(" ");
    j=Ploidy*ProbWidth+(Ploidy+1)*i; //one space at each end and between columns;
    if (INameWidth<j) {PrIndWidth=j; w=i;}
    else
    { PrIndWidth=INameWidth;
      w=(PrIndWidth - Ploidy*ProbWidth - (Ploidy-1)*i) / 2; //left margin
    }
    for (p=0; p<Ploidy; p++)
      ProbLeft[p]=w+p*(ProbWidth+i);
    //ProbLeft[0] = PrIndWidth/2 - CV->TextWidth(" ")/2; //ref. for right aligned column 0
    //ProbLeft[1] = ProbLeft[0] + CV->TextWidth(" ");    //ref. for left aligned column 1
    ProbTop = INameHeight; // =1.5 * font height: 0.25 above & 0.25 below name
    ProbHeight = Opt.MarkSelCount * CV->TextHeight("$");
    PrIndHeight = 1.5 * INameHeight + ProbHeight; //at bottom another 0.5 Nameheight = 0.75 font height
  }

  else //Opt.IndivContents==icNone or invalid, or no Traits, Markers or LG selected)
  { for (p=0; p<Ploidy; p++) ProbLeft[p]=0;
    ProbTop=ProbWidth=ProbHeight=0;
    PrIndWidth = INameWidth;
    PrIndHeight = INameHeight;
  }


  //PrCrossSize= Opt.ShowCrossings ? Opt.CrossSizeMM * PrPPI / 2540 : 0;
  PrCrossSize= Opt.CrossSizeMM * PrPPI / 2540;
  delete BM;
} //TView::CalcIndivNCrossSize(void)

/*
bool TView::ChartFits(void)
//if Overview, the PrMarg is fitted to the necessary chart size
//note: CalcIndivNCrossSize() and CalcMargRect(PrPPI) must have been called earlier
//the parameters return the minimum chartwidth and -height within the margins,
//in printer pixels
{ bool Fits;
  int i,MinSpacePx,PrIWPx,PrIHPx;
  MinSpacePx=0.5 + Opt.MinSpaceMM * (PrPPI/2540.0);
  PrIWPx=PrIndWidth+MinSpacePx; PrIHPx=PrIndHeight+MinSpacePx;
  if (Overview)
  { // Create page large enough for VP
    PrMarg.Left=0; PrMarg.Top=0;
    if (Opt.LeftRight)
    { PrMarg.Bottom = PrMarg.Top + MaxIndPerGS * (PrIHPx+1);
      PrMarg.Right = PrMarg.Left + GCount*PrIWPx + (GCount-1)* 10 * PrCrossSize;
    }
    else
    { PrMarg.Bottom = PrMarg.Top + GCount*PrIHPx + (GCount-1)* 10 * PrCrossSize;
      PrMarg.Right = PrMarg.Left + MaxIndPerGS * (PrIWPx*1.1);
    }
    PrMinWidth=PrMarg.Right; PrMinHeight=PrMarg.Bottom;
    Opt.PgWidthMM= 0.5 + 2540.0 * PrMarg.Right / PrPPI;
    Opt.PgHeightMM = 0.5 + 2540.0 * PrMarg.Bottom / PrPPI;
    for (i=0; i<4; i++) Opt.MargMM[i] = 0;
    Fits = true;
  }
  else //check if specified page is large enough:
  { if (Opt.LeftRight)
    { PrMinWidth  = GCount*PrIWPx + (GCount-1)*PrCrossSize;
      PrMinHeight = MaxIndPerGS * PrIHPx;
    }
    else
    { PrMinWidth  = MaxIndPerGS * PrIWPx;
      PrMinHeight = GCount*PrIHPx + (GCount-1)*PrCrossSize;
    }
    Fits = PrMarg.Right-PrMarg.Left > PrMinWidth &&
           PrMarg.Bottom-PrMarg.Top > PrMinHeight ;
    //note: no check if all Crosses fit in each CrossStrip
  }
  ChartsizeError = !Fits;
  return Fits;
} //TView::ChartFits
*/

/* old version:
void TView::CalcStrip(int g, int Psize, int Isize)
//fills CS[g] with TCrossings, and calculates the relative positions of the
//Indivs in GS[g] and the Crossings in CS[g]
//assumption: each full-sib families in one generation, in one consecutive block
{ int i,f, //indices to Ind and Fam arrays
      FCount; //Family count
  double IFS, //Opt.InFamSpace converted to 0.0 .. 1.0 value
         Space; //size in pix of space between families, and between margins and first or last family
  TCrossing *C;
  struct Family { TCrossing *Cro;
                  int I1,I2;     //index of first and last offspring in GS[g]->Ind
                } *Fam,*Fam1;
  if (GS[g]->ICount==0)
  { ShowMessage("GS[g]->ICount==0 in CalcStrip"); exit; }

  //First: create and fill the Fam array and the CS[g]->Cross array
  Fam=NULL;
  FCount=0; i=0;
  while (i<GS[g]->ICount)
  { //add new family:
    if (VP->IsVPFounder(GS[g]->Ind[i])) C=NULL;
    else C=Indiv[VP->VPInd[GS[g]->Ind[i]]].ParentCross;
    Fam1=new Family[++FCount];
    for (f=0; f<FCount-1; f++) Fam1[f]=Fam[f]; delete[] Fam;
    Fam=Fam1;
    Fam[FCount-1].Cro=C; Fam[FCount-1].I1=i;
    //run through all offspring of current Crossing:
    do i++;
    while ( i<GS[g]->ICount &&
            Indiv[VP->VPInd[GS[g]->Ind[i]]].ParentCross==C &&
            Indiv[VP->VPInd[GS[g]->Ind[i]]].ParentCross!=NULL) ;
    Fam[FCount-1].I2=i-1;
  } //while i
  //calculate Space:
  IFS=0.001*Opt.InFamSpace;
  Space= (Psize-GS[g]->ICount*Isize)/(FCount+1+(GS[g]->ICount-FCount)*IFS);

  delete[] CS[g]->Cross; CS[g]->Cross=NULL;
  delete[] CS[g]->CIPos;  CS[g]->CIPos=NULL;
  CS[g]->CCount=0;
  //go through Families, calc Indiv positions and add Crosses:
  for (f=0; f<FCount; f++)
  { //calculate relative positions of each Indiv in Fam[f]:
    for (i=Fam[f].I1; i<=Fam[f].I2; i++)
      GS[g]->IPos[i]= ((i+0.5)*Isize + (f+1+(i-f)*IFS)*Space) / Psize ;
    //add Crossing to CS[g], and center above offspring:
    if (Fam[f].Cro!=NULL)
    { CS[g]->AddCross(Fam[f].Cro);
      CS[g]->CIPos[CS[g]->CCount-1] =
          (GS[g]->IPos[Fam[f].I1] + GS[g]->IPos[Fam[f].I2]) / 2 ;
    }
  } //for f
  delete[]Fam;
} //TView::CalcStrip
*/

//new version:
void TView::CalcStrip(int g, int MargPx, int ChartPx, int IndPx)
//only called from CalcInitialPositions
//fills CS[g] with TCrossings, also if ShowCrossings=false, also with
//crossings that may also appear at higher generation.
//calculates the pixel positions of the Indivs in GS[g] and the Crossings in CS[g]
//places the crossings centered on the first and last offspring,
//even if these are not in one block.
{ int i,f,  //indices to Ind and Fam arrays
      FCount, //Family count
      TotalSpacePx, //chart size minus space for all Ind
      BetwFamPx,    //size of gaps between FS families, in pixels
      BetwSibPx,    //size of gaps between sibs within families
      OuterPx;      //size of gaps between outer Inds and chart edges
  TCrossing *Cro, *LastCro;
  struct Family { TCrossing *Cro;
                  int I1,I2;     //index of first and last offspring in GS[g]->Ind
                } *Fam,*Fam1;
  if (GS[g]->ICount==0)
  { ShowMessage("GS[g]->ICount==0 in CalcStrip");}
  else
  {
    //Determine gap sizes (pixels):
    //see if spacing with two outer BetwFamSpaces fits:
    TotalSpacePx = ChartPx - 2*MargPx - GS[g]->ICount*IndPx;
    BetwFamPx = 0.5 + TotalSpacePx /
                 ((GS[g]->FamGapCount+2+0.01*Opt.BetwSibSpace*GS[g]->SibGapCount));
    if (100*BetwFamPx < IndPx*Opt.BetwFamSpace+1)
    { //doesn't fit:
      BetwFamPx = 0.5 + 0.01*Opt.BetwFamSpace*IndPx;
    } //else: fits, leave BetwFamPx unchanged
    BetwSibPx=0.01*Opt.BetwSibSpace*BetwFamPx;
    OuterPx=(TotalSpacePx //- GS[g]->ICount*IndPx
                          - GS[g]->FamGapCount*BetwFamPx
                          - GS[g]->SibGapCount*BetwSibPx) / 2;

    if (g==0)
    { //no CrossStrip; all Ind are unrelated
      GS[0]->IPos[0]=MargPx+OuterPx;
      for (i=1; i<GS[0]->ICount; i++)
          GS[0]->IPos[i] = GS[0]->IPos[i-1]+IndPx+BetwFamPx;
    }

    else //g>0
    { //First: create and fill the Fam array and the CS[g]->Cross array
      Fam=NULL; FCount=0; LastCro=NULL;
      for (i=0; i<GS[g]->ICount; i++)
      { if (VP->IsVPFounder(GS[g]->Ind[i])) Cro=NULL;
        else Cro=Indiv[VP->VPInd[GS[g]->Ind[i]]].ParentCross;
        if (Cro==NULL)
        { if (i==0) GS[g]->IPos[0]=MargPx+OuterPx;
          else GS[g]->IPos[i]=GS[g]->IPos[i-1]+IndPx+BetwFamPx;
        }
        else
        { f=0; while (f<FCount && Fam[f].Cro!=Cro) f++;
          if (Cro==LastCro)
          { GS[g]->IPos[i]=GS[g]->IPos[i-1]+IndPx+BetwSibPx;
            Fam[f].I2=i;
          }
          else
          { if (f>=FCount)
            { //add new Family:
              Fam1=new Family[++FCount];
              for (f=0; f<FCount-1; f++) Fam1[f]=Fam[f]; delete[] Fam;
              Fam=Fam1; Fam1=NULL;
              Fam[FCount-1].Cro=Cro; Fam[FCount-1].I1=i; Fam[FCount-1].I2=i;
            }
            else Fam[f].I2=i;
            if (i==0) GS[g]->IPos[0]=MargPx+OuterPx;
            else GS[g]->IPos[i]=GS[g]->IPos[i-1]+IndPx+BetwFamPx;
          } //Cro==LastCro else
        } //Cro==NULL else
        LastCro=Cro;
      } //for i

      delete[] CS[g]->Cross; CS[g]->Cross=NULL;
      delete[] CS[g]->CIPos;  CS[g]->CIPos=NULL;
      CS[g]->CCount=0;
      //go through Families, calc Indiv positions and add Crosses:
      for (f=0; f<FCount; f++)
      { //add Crossing to CS[g], and center above offspring:
        CS[g]->AddCross(Fam[f].Cro);
        CS[g]->CIPos[CS[g]->CCount-1] =
              (GS[g]->IPos[Fam[f].I1] + GS[g]->IPos[Fam[f].I2] + IndPx - PrCrossSize) / 2;
      } //for f
      delete[]Fam;
      //finally: set the CGPos:
      CS[g]->CGPos = GS[g]->GPos - Opt.GenerDist*PrCrossSize/2;
    } //g>0
  } //ICount>0
} //TView::CalcStrip

/* old version:
bool TView::CalcInitialPositions(void)
//calculate initial Indiv positions, based on generation structure and Opt;
//and the initial Crossing positions
//Note: if Opt.ShowCrossings=false, still the relative positions of the Crossings
//are calculated, but to determine the Fit, CrossSize should be 0
//returns false if doesn't fit on page; in that case use the minimum
//printer pixel sizes of the chart (within margins) in the error message
{ int g,i,
      MinSpacePx,
      Pi, Pd, Ii, Id; //pix sizes of Page and Indiv in- and down-the-generations
  double GSpace; //distance between Indiv and Cross-symbol

  MinSpacePx=0.5 + Opt.MinSpaceMM * (PrPPI/2540.0);
  CalcIndivNCrossSize(); //in PrPPI
  PrMarg=CalcMargRect(PrPPI); //also in PrPPI
  if ( ChartFits() )
  { //CreateGraphicsStruct();

    //calculation of initial positions:
    // 1. for generations and crossings:
    // The distance between two generations = distance between margin and
    // first or last generation. Cross symbols are centered in this space.
    // GSpace is the distance between individuals and cross symbols, so the
    // generation distance = PrCrossSize + 2*GSpace.
    // 2. for individuals within a GenStrip:
    // The distance between families in a Genstrip = distance between margin and
    // first or last individual. The distance between individuals within
    // families = Opt.InFamSpace * distance between families.
    // 3. for Crosses in CrossStrip:
    // the Crosses are centered above their progeny

    if (Opt.LeftRight)
    { Pd=PrMarg.Right-PrMarg.Left; Id=PrIndWidth+MinSpacePx;
      Pi=PrMarg.Bottom-PrMarg.Top; Ii=PrIndHeight+MinSpacePx; }
    else
    { Pd=PrMarg.Bottom-PrMarg.Top; Id=PrIndHeight+MinSpacePx;
      Pi=PrMarg.Right-PrMarg.Left; Ii=PrIndWidth+MinSpacePx; }

    GSpace = (Pd-GCount*Id-(GCount+1)*PrCrossSize) / (2*GCount+2);

    for (g=0; g<GCount; g++)
    { //GS[g]->GPos=( (g+1.0)/(GCount+1) * (Pd+Id+PrCrossSize) - 0.5*Id - PrCrossSize ) / Pd;
      GS[g]->GPos= ( (g+0.5)*Id + (g+1)*(PrCrossSize + 2*GSpace) ) / Pd ;
      if (g==0) //no CrossStrip, only (VP-)unrelated VPFounder Indivs
      { for (i=0; i<GS[g]->ICount; i++)
          GS[g]->IPos[i]=( (i+1.0)/(GS[g]->ICount+1) * (Pi+Ii) - 0.5*Ii ) / Pi;
      }
      else //g>0
      { //CS[g]->CGPos=( (g+0.5)/(GCount+1) * (Pd+Id+PrCrossSize) - 0.5*Id - PrCrossSize ) / Pd;
        CS[g]->CGPos= ( g*Id + (g+0.5)*(PrCrossSize + 2*GSpace) ) / Pd ;
        //Fill CrossStrip, and position Crossings and Indivs in their strip
        CalcStrip(g,Pi,Ii);
        //if (Opt.InFamSpace==1.0)
        //  GS[g]->IPos[i]=( (i+1.0)/(GS[g]->ICount+1) * (Pi+Ii) - 0.5*Ii ) / Pi;
        //else CalcIndPos(g,Pi,Ii);
        //FillCrossStrip(g);
      } //g>0
    } //for g
    return true;
  }
  else return false;
} //TView::CalcInitialPositions
*/

void TView::CalcInitialPositions(void)     //new version 3-6-04
//called from CalcInitialChart and from ToolsOptionsExecute
//calculate initial Indiv positions, based on generation structure and Opt;
//and the initial Crossing positions
{ int g,         //generation index
      MaxGS;     //index to biggest GS
  double MaxGSsize, //size of biggest GS = MaxGSsize * Ind.size
         d;
  TPoint MessageSize;


  //calc 10 mm margin in pixels:
  PrMargPx=100.0/254.0 * PrPPI;

  if (VP->VPIndCount>IndivDrawLimit) {
      MessageSize=CalcNoImageMessageSize();
      PrChartWidth= 2* PrMargPx + MessageSize.x;
      PrChartHeight= 2* PrMargPx + MessageSize.y;
      return;
  }

  //first, calculate individual and cross sizes:
  CalcIndivNCrossSize(); //in PrPPI

  //1. find biggest GenStrip: determines chart size across (not down) the generations
  MaxGSsize=-1.0; //relative size of GenStrip, with IndivSize as 1
  for (g=0; g<GCount; g++)
  { GS[g]->CountGaps();
    d = GS[g]->ICount + 0.01*Opt.BetwFamSpace*GS[g]->FamGapCount +
        0.0001*Opt.BetwFamSpace*Opt.BetwSibSpace*GS[g]->SibGapCount;
    if (d>MaxGSsize) { MaxGSsize=d; MaxGS=g; }
  }

  //2. calculate chart size in Pr-pixels
  if (Opt.LeftRight)
  { PrChartWidth= 2* PrMargPx + GCount*PrIndWidth +
                  (GCount-1) * PrCrossSize * Opt.GenerDist;
    PrChartHeight = 2 * PrMargPx + PrIndHeight *
        ( GS[MaxGS]->ICount +
          0.01*Opt.BetwFamSpace*GS[MaxGS]->FamGapCount +
          0.0001*Opt.BetwFamSpace*Opt.BetwSibSpace*GS[MaxGS]->SibGapCount );
  }
  else
  { PrChartHeight= 2 * PrMargPx + GCount*PrIndHeight +
                  (GCount-1) * PrCrossSize * Opt.GenerDist;
    PrChartWidth = 2 * PrMargPx + PrIndWidth *
        ( GS[MaxGS]->ICount +
          0.01*Opt.BetwFamSpace*GS[MaxGS]->FamGapCount +
          0.0001*Opt.BetwFamSpace*Opt.BetwSibSpace*GS[MaxGS]->SibGapCount );
  }

  //3. for each GS calculate position of GS and of Ind and Crosses in GS
  for (g=0; g<GCount; g++)
  { if (Opt.LeftRight)
    { GS[g]->GPos= PrMargPx + g*(PrIndWidth + PrCrossSize*Opt.GenerDist);
      CalcStrip(g, PrMargPx, PrChartHeight, PrIndHeight);
    }
    else
    { GS[g]->GPos= PrMargPx + g*(PrIndHeight + PrCrossSize*Opt.GenerDist);
      CalcStrip(g, PrMargPx, PrChartWidth, PrIndWidth);
    }
  } //for g
  //return true;
} //TView::CalcInitialPositions

/* New CalcInitialPositions: for unspecified page size.
Aims: Chart should exactly fill page within margins, i.e. no empty borders
-> for size of Gentrips:
Let TViewOptions have new fields:
 BetwFamSpace = MINIMUM space between families within GenStrip,
   expressed as % of Indiv.size (>=0, may be smaller and larger than 100%)
 BetwSibSpace = space within families within GenStrip, as % of the current
   betweenFam space (which is usually larger than the minimum in BetwFamSpace)
   must be <= MinBetwFamFact
In GenStrips with more room, the ratio of BetwFam and BetwSib spaces stays the same.
The widest GS determines the page size, with the outer Indivs touching the borders.
For all other GS, the Indivs are positioned as follows.
First, calculate the empty space ES = page size - (#Indiv * IndWidth)
Next, divide the ES by the number of BetwFam and WithFam spaces, including two BetwFam
spaces outside the outer Indivs. If the spaces become smaller than the minimum values
in TViewOptions (above), use the same (minimum) spaces as in the widest GS
and set the two outer spaces to match the page size (of the widest GS)
Finally, with these values calculate positions.
-> for distances between GensStrips:
Let TViewOptions have one new field:
  GenerDist = distance between generations, expressed as integer times
    the size of the CrossSymbol (GenerDist >= 1)
The CrossSymbols, if specified, are drawn in the center of the between-generatioin
space. The distance is the same with or without cross symbols.

The page as viewed onscreen consists of the chart (with size as calculated above)
surrounded with the Margins (in mm).
When printed, the margins are subtracted from the printer page size. The chart
is resized (with constant aspect ratio) so it fits within the remaining space.
Note that the relative size of chart and margins changes between printer
and viewed chart.
(Maybe further options to print as multiple pages will be developed)

The chart as exported (saved or copied) does not include margins, and exactly
fits the outermost individuals.

*/


void TView::CalcInitialChart(void)
//Starting with VP and Opt, calculate all data for new chart:
//generation structure,
//sizes of chart, indivs and cross symbols (in printer pix),
//and positions of indivs and cross symbols (in printer pix, top-left corners).
//Called from several MainForm methods, including
// StartProject, ReadViewData, ReadViewpoData, ToolsCreateView, ToolsCreateViewpop
{ if (VP->VPIndCount<=IndivDrawLimit)
        CreateGenerationStructure();
  CalcInitialPositions();
  ChartCalculated=true;
} //TView::CalcInitialChart

void TView::FillZoomvalues(int WidthPx, int HeightPx)
//fills, depending on minimum (zoom to show full page) and maximum
//(zoom to show printer resolution)
//WidthPx and HeightPx are the size of the window for display
//#define PRINTRES  //if defined, include zoom to full printer resolution,
                  //else 200% is maximum
{ int i,r; //mini,maxi;
  AnsiString t;
  const i100=6, valcount=8;
  const int fixedvalues[valcount] = {1,2,5,10,20,50,100,200}; //zoom in %
  double PrToScr, //convert printer pixels to screen pixels
         minfact, //factor for full-page zoom
         d,
         fixedfactors[valcount], //corresponding to fixedvalues
         realfact[valcount+1]; //factors to be shown: excluding smaller than minfact

  PrToScr = Screen->PixelsPerInch / double(PrPPI);
  for (i=0; i<valcount; i++) fixedfactors[i] = 0.01 * fixedvalues[i] * PrToScr;

  minfact = double(WidthPx-2*Bor) / PrChartWidth;
  d = double(HeightPx-2*Bor) / PrChartHeight;
  if (d<minfact) minfact=d;

  //the minimum listed zoom level should be not higher than 100%,
  //even if picture still very small;
  //if picture very small, the minimum listed zoom level should be 100%;
  //otherwise, the minimum should be minval to show full page (further zoomin not needed),
  //but not less than 1%
  //if PRINTRES defined, the full-res value should be inserted at the end

  if (minfact<fixedfactors[i100]) //100%
  { if (minfact<fixedfactors[0]) //1%
    { //minval<1%: start with 1%
      r=0; i=0;
    }
    else
    { //minval>1% but <100%: start with minval and continue with next-larger fixedvalue
      realfact[0]=minfact; r=1;
      i=0; while (fixedfactors[i]<minfact) i++;
    }
  }
  else
  { //minval>100%: start with 100%
    realfact[0]=fixedfactors[i100]; r=1; i=i100+1;
  }

  //now, include all fixed values up to 200%:
  while (i<valcount)
  { realfact[r++]=fixedfactors[i++]; }

  #ifdef PRINTRES
  //include maximum zoom to full printer resolution at the end:
  realfact[r++]=1.0;
  #endif

  if (ZoomValues!=NULL)
  { ZoomValues->Clear(); //ZoomValues=NULL;
  }
  i=r;
  for (r=0; r<i; r++)
  { t=IntToStr(int(100.0*realfact[r]/PrToScr));
    if (r==0 || t!=ZoomValues->Strings[ZoomValues->Count-1])
       ZoomValues->Add(t);
  }
#undef PRINTRES
} //TView::FillZoomvalues  Compiler warning about valcount and i100 never used is incorrect!

int TView::GetZoomValIndex(void)
//finds index of current Zoom in list of ZoomValues; -1 if not found
{ AnsiString s;
  int Result;
  s=IntToStr(Zoom);
  Result=ZoomValues->Count-1;
  while (Result>=0 && ZoomValues->Strings[Result]!=s)
   Result--;
  return Result;
} //TView::GetZoomValIndex

int TView::GetBestZoomValIndex(void)
//finds index of current Zoom in list of ZoomValues;
//if not present, return first smaller index or 0
//always a valid index, not -1 as in GetZoomValIndex
{ AnsiString s;
  int Result;
  Result=ZoomValues->Count-1;
  while ( Result>=1 && StrToInt(ZoomValues->Strings[Result])>Zoom )
   Result--;
  return Result;
} //TView::GetBestZoomValIndex

void TView::GetUnzoomedPoint(int ZoomX, int ZoomY, int &X, int &Y)
//ZoomX,Y: point on VisArea; X,Y: point on ChartMF
{ X=0.5+(ZoomedLeftVis+ZoomX)/ZoomFact;
  Y=0.5+(ZoomedTopVis+ZoomY)/ZoomFact; }

void TView::GetZoomedPoint(int X, int Y, int &ZoomX, int &ZoomY)
//X,Y: point on ChartMF; ZoomX,Y: point on VisArea
{ ZoomX=0.5+(X*ZoomFact)-ZoomedLeftVis;
  ZoomY=0.5+(Y*ZoomFact)-ZoomedTopVis; }

TRect TView::GetZoomedRect(TRect R)
//only called in MainForm::SetShapeRect
//important: must be at least as large as ind,
//therefore round down topleft, roundup bottomright:
{ int P,Q,V,W;
  P = (R.Left*ZoomFact)-ZoomedLeftVis;
  Q = (R.Top*ZoomFact)-ZoomedTopVis;
  V = 0.9999+(R.Right*ZoomFact)-ZoomedLeftVis;
  W = 0.9999+(R.Bottom*ZoomFact)-ZoomedTopVis;
  return Rect(P,Q,V,W);
}


void TView::GetObjAtPoint(int X, int Y, int &g, int &VPI, TCrossing* &Cro)
/* if on an individual: g=generation, Cro=NULL, VPI=index in VP->VPInd
   if on a cross symbol: g= generation, VPI=-1, Cro=pointer to TCrossing
   else if in a GenStrip or CrossStrip: g=generation, Cro=NULL, VPI=-1
   otherwise g=-1, Cro=NULL, VPI=-1 */
{ int i; //GPx, IPx, i;
  /*moet anders 6-6-04
  VPI=-1; Cro=NULL;
  //Marg=CalcMargRect(PrPPI);
  //IndWpx=PrIndWidth*PPI/PrPPI;
  //IndHpx=PrIndHeight*PPI/PrPPI;

  if (Opt.LeftRight)
  { g=GCount-1;
    while ( ( g>=0 ) &&
            ( ( GPx=Marg.Left + RelPos2Pix(Marg.Right-Marg.Left,GS[g]->GPos) - IndWpx/2)>X ) )
    g--;
    if ( (g>=0) && ((GPx+IndWpx)>=X) )
    { //X in GenStrip g
      i=GS[g]->ICount-1;
      while ( ( i>=0 ) &&
              ( ( IPx=Marg.Top + RelPos2Pix(Marg.Bottom-Marg.Top,GS[g]->IPos[i]) - IndHpx/2)>Y ) )
      i--;
      if ( (i>=0) && ((IPx+IndHpx)>=Y) )
      //Y in Ind[i]
      VPI=GS[g]->Ind[i];
    }
    else
    { if ( Opt.ShowCrossings && ++g>0 && g<GCount &&
           ( GPx=Marg.Left + RelPos2Pix(Marg.Right-Marg.Left,CS[g]->CGPos) - CrossPx/2)<=X  &&
           GPx+CrossPx>=X )
      { //X in CrossStrip g
        i=CS[g]->CCount-1;
        while ( ( i>=0 ) &&
                ( ( IPx=Marg.Top + RelPos2Pix(Marg.Bottom-Marg.Top,CS[g]->CIPos[i]) - CrossPx/2)>Y ) )
        i--;
        if ( (i>=0) && ((IPx+CrossPx)>=Y) )
        //Y in Cross[i]
        Cro=CS[g]->Cross[i];
      }
      else g=-1; //not in GenStrip
    }
  }
  else //not LeftRight
  { g=GCount-1;
    while ( g>=0 &&
            ( GPx=Marg.Top + RelPos2Pix(Marg.Bottom-Marg.Top,GS[g]->GPos) - IndHpx/2)>Y  )
    g--;
    if ( (g>=0) && ((GPx+IndHpx)>=Y) )
    { //Y in GenStrip g
      i=GS[g]->ICount-1;
      while ( ( i>=0 ) &&
              ( ( IPx=Marg.Left + RelPos2Pix(Marg.Right-Marg.Left,GS[g]->IPos[i]) - IndWpx/2)>X ) )
      i--;
      if ( (i>=0) && ((IPx+IndWpx)>=X) )
      //X in Ind[i]
      VPI=GS[g]->Ind[i];
    }
    else
    { if ( Opt.ShowCrossings && ++g>0 && g<GCount &&
           ( GPx=Marg.Top + RelPos2Pix(Marg.Bottom-Marg.Top,CS[g]->CGPos) - CrossPx/2)<=Y  &&
           GPx+CrossPx>=Y )
      { //Y in CrossStrip g
        i=CS[g]->CCount-1;
        while ( ( i>=0 ) &&
                ( ( IPx=Marg.Left + RelPos2Pix(Marg.Right-Marg.Left,CS[g]->CIPos[i]) - CrossPx/2)>X ) )
        i--;
        if ( (i>=0) && ((IPx+CrossPx)>=X) )
        //X in Cross[i]
        Cro=CS[g]->Cross[i];
      }
      else g=-1; //not in GenStrip
    }
  }
  */
  // nieuwe versie 16-6-04
  VPI=-1; Cro=NULL;
  if (Opt.LeftRight)
  { g=GCount-1;
    while ( g>=0 && GS[g]->GPos+PrIndWidth > X )
    g--;
    g++;
    if ( g>=0 && g<GCount && GS[g]->GPos <= X )
    { //X in GenStrip g
      i=GS[g]->ICount-1;
      while ( i>=0 && GS[g]->IPos[i] > Y )
      i--;
      if ( i>=0 && GS[g]->IPos[i]+PrIndHeight >= Y )
      //Y in Ind[i]
      VPI=GS[g]->Ind[i];
    }
    else
    { if ( Opt.ShowCrossings && g>0 && g<GCount &&
           CS[g]->CGPos<=X && CS[g]->CGPos+PrCrossSize>=X)
      { //X in CrossStrip g
        i=CS[g]->CCount-1;
        while ( i>=0 && CS[g]->CIPos[i] > Y )
        i--;
        if ( i>=0 && CS[g]->CIPos[i]+PrCrossSize >= Y )
        //Y in Cross[i]
        Cro=CS[g]->Cross[i];
      }
      else g=-1; //not in GenStrip
    }
  }
  else //nt Opt.LeftRight
  { g=GCount-1;
    while ( g>=0 && GS[g]->GPos+PrIndHeight > Y )
    g--;
    g++;
    if ( g>=0 && g<GCount && GS[g]->GPos <= Y )
    { //Y in GenStrip g
      i=GS[g]->ICount-1;
      while ( i>=0 && GS[g]->IPos[i] > X )
      i--;
      if ( i>=0 && GS[g]->IPos[i]+PrIndWidth >= X )
      //X in Ind[i]
      VPI=GS[g]->Ind[i];
    }
    else
    { if ( Opt.ShowCrossings && g>0 && g<GCount &&
           CS[g]->CGPos<=Y && CS[g]->CGPos+PrCrossSize>=Y)
      { //Y in CrossStrip g
        i=CS[g]->CCount-1;
        while ( i>=0 && CS[g]->CIPos[i] > X )
        i--;
        if ( i>=0 && CS[g]->CIPos[i]+PrCrossSize >= X )
        //X in Cross[i]
        Cro=CS[g]->Cross[i];
      }
      else g=-1; //not in GenStrip
    }
  }
} //TView::GetObjAtPoint

TMetafile* TView::DrawChartsizeError(void)
{ //const char *newl="\n";
  //int w,h,wmax;
  TMetafile *ErrMF;
  TMetafileCanvas *CV;
  AnsiString s;

  ErrMF = new TMetafile;
  ErrMF->Width = 450;
  ErrMF->Height = 65;
  CV = new TMetafileCanvas(ErrMF,0);
  CV->Brush->Style=bsSolid; CV->Brush->Color=clWhite;
  CV->FillRect(Rect(0,0,ErrMF->Width+1,ErrMF->Height+1));
  CV->Font->Name="Arial"; CV->Font->Size=9; CV->Font->Color=clBlack;
  s=AnsiString("Chart doesn't fit on page:");
  CV->TextOut(10,10,s);
  //s=AnsiString("the minimum required size (within margins) is " +
  //  IntToStr(int(PrMinWidth*25.4/300+0.5)) + "mm wide x " +
  //  IntToStr(int(PrMinHeight*25.4/300+0.5)) + "mm high,");
  //CV->TextOut(10,25,s);
  //s=AnsiString("but the available space (within margins) is only " +
  //  IntToStr( (Opt.PgWidthMM - Opt.MargMM[0] - Opt.MargMM[2])/100 ) + "mm wide x " +
  //  IntToStr( (Opt.PgHeightMM - Opt.MargMM[1] - Opt.MargMM[3])/100 ) + "mm high");
  //CV->TextOut(10,40,s);
  delete CV;
  return ErrMF;
} //TView::DrawChartsizeError


// ******** TLocus ************

TLocus::TLocus(short int Num, short int LG, AnsiString &Name, float Position)
{ int i,p;
  LocNum=Num;
  LGnr=LG;
  LocName=Name;
  cM=Position;
  IBDix=-1; //not assigned
  AllNameCount=NULL;
  AllName=NULL;
  FounderAllele=NULL;
  //create allele and code arrays and fill with defaults:
  AllIxArr = new signed char [IndivCount*Ploidy];
  AllCodeArr = new byte [IndivCount*Ploidy];
  for (p=0; p<Ploidy; p++)
    for (i=0; i<IndivCount; i++)
    {  AllIx[i][p]=-1;  //Unknown allele
       AllCode[i][p]=0; //default color
    }
} //TLocus::TLocus

TLocus::~TLocus(void)
{ delete[] AllIxArr;
  delete[] AllCodeArr;
  delete[] AllName;
  delete[] FounderAllele;
} //TLocus::~TLocus

TLocus::TLocus(TLocus &Source)
{ ShowMessage("Warning: TLocus copy-constructor not implemented"); }

/* January 2007: now in constructor
void TLocus::CreateObsArrays(void)
{ int p,i;
  AllIxArr = new short int [IndivCount*Ploidy];
  for (p=0; p<Ploidy; p++)
  { for (i=0; i<IndivCount; i++)
      AllIx[i][p]=-1; //Unknown; see ParseUnit - ReadLocus
  }
  AllCodeArr = new byte [IndivCount*Ploidy];
  for (p=0; p<Ploidy; p++)
  { for (i=0; i<IndivCount; i++) AllCode[i][p]=0; } //default color
} //TLocus::CreateObsArrays
*/
TLocus* GetLocusByNr(int LocNr) //LocNr counts over all loci in all LinkGrp
{ int LG;

  if (LocNr<0 || LocNr>=TotLocCount) return NULL;
  LG=1; //not 0!
  while (LG<LinkGrpCount && LinkGrp[LG]->StartLocNr<=LocNr) LG++;
  //LG now points one LinkGrp too far
  return LinkGrp[LG-1]->Locus[LocNr-LinkGrp[LG-1]->StartLocNr];
} //GetLocusByNr

TLocus* GetLocusByName(AnsiString LocName)
{ TLocus *Result; int LG,i;
  Result=NULL;
  LG=0;
  while (Result==NULL && LG<LinkGrpCount)
  { i=0;
    while (Result==NULL && i<LinkGrp[LG]->LocCount &&
           LinkGrp[LG]->Locus[i]->LocName!=LocName)
    { i++; }
    if (i<LinkGrp[LG]->LocCount) Result=LinkGrp[LG]->Locus[i];
    LG++;
  }
  return Result;
} //GetLocusByName


int GetLocNr(TLocus *Loc)   //LocNr counts over all loci in all LinkGrp
{ return LinkGrp[Loc->LGnr]->StartLocNr + Loc->LocNum; }

// ******** TIBDposition ************

TIBDposition::TIBDposition(short int LG, float Position)
{ int i,p,a;
  LGnr=LG;
  cM=Position;
  AllProbArr = new byte[IndivCount*Ploidy*IBDAlleleCount];
  for (a=0; a<IBDAlleleCount; a++)
  { for (i=0; i<IndivCount; i++)
    { for (p=0; p<Ploidy; p++)
        AllProb[i][p][a]=0;
    }
  }
} //TIBDposition::TIBDposition

TIBDposition::~TIBDposition(void)
{ delete[] AllProbArr;
} //TIBDposition::~TIBDposition

TIBDposition::TIBDposition(TIBDposition &Source)
{ ShowMessage("Warning: TIBDPosition copy-constructor not implemented"); }



// ******** TLinkGrp ************

TLinkGrp::TLinkGrp(int Num, AnsiString &Name, int StartLoc, int StartIBD)
{ LGNum=Num;
  LGName=Name;
  LocCount=0;
  IBDCount=0;
  StartLocNr=StartLoc;
  StartIBDNr=StartIBD;
  Locus=NULL;
  IBDpos=NULL;
}

TLinkGrp::~TLinkGrp (void)
{ int i;
  for (i=0; i<LocCount; i++) delete Locus[i];
  for (i=0; i<IBDCount; i++) delete IBDpos[i];
  delete[] Locus; delete[]IBDpos;
}

TLinkGrp::TLinkGrp(TLinkGrp &Source)
{ ShowMessage("Warning: TLinkgrp copy-constructor not implemented"); }


/*
int TLinkGrp::AddLocus (AnsiString &Name)
{ TLocus **Temp;
  int i;
  Temp = new TLocus*[LocCount++];
  for (i=0; i<LocCount-1; i++) Temp[i]=Locus[i];
  Temp[LocCount-1] = new TLocus(LocCount-1, Name);
  delete[] Locus;
  Locus = Temp;
  return LocCount-1;
} //TLinkGrp::AddLocus
*/

// ******** TSelScriptLine ************

// TSelScriptLine are lines of TSelectionScript

TSelScriptLine::TSelScriptLine(void)
{ IndCount=0; IndList=NULL;
   BaseInd=-1; AncestorGen=0; ProgenyGen=0; Parentline=2;
   Select=true; Sibs=false; ColdParents=false;
};

TSelScriptLine::~TSelScriptLine(void)
{ delete[] IndList; }

void TSelScriptLine::Clear(void)
{ delete[] IndList; IndList=NULL; IndCount=0;
  BaseInd=-1; AncestorGen=0; ProgenyGen=0; Parentline=2;
  Select=true; Sibs=false; ColdParents=false;
};

AnsiString TSelScriptLine::WriteLine(bool Verbose)
{ AnsiString s, ParentlineName[2]; int i;
  if (Verbose)
  { if (IndCount==-1) s="select all";
    else if (IndCount==0)
    { //write Auto line:
      switch (FemaleParent) {
        case 0: {ParentlineName[0]="female line"; ParentlineName[1]="male line"; break;};
        case 1: {ParentlineName[0]="male line"; ParentlineName[1]="female line"; break;};
        default: {ParentlineName[0]="line="+ParentCaption(0);
                  ParentlineName[1]="line="+ParentCaption(1);}
      }
      if (Select) s="auto select ";
      else s="auto deselect ";
      s = s + "Base individual="+Indiv[BaseInd].IName+" "+
            "Ancestors="+IntToStr(AncestorGen)+" "+
            "Progeny="+IntToStr(ProgenyGen);
      if (Parentline==0 || Parentline==1) s=s+" ("+ParentlineName[Parentline]+")";
      //else: both line, not mentioned
      if (Sibs) { s=s+" include sibs"; if (ColdParents) s=s+","; }
      if (ColdParents) s=s+" include progeny parents";
    }
    else //not all, not auto
    { if (Select) s="select"; else s="deselect";
      for (i=0; i<IndCount; i++)
      { if (i>0) s=s+",";
        s=s+" "+Indiv[IndList[i]].IName;
      }
    }
  }
  else //not Verbose
  { s = IntToStr(IndCount) + " " + (Select ? "1" : "0") ;
    if (IndCount==0)
    { //write Auto line:
      s= s + " " + QuotedString(Indiv[BaseInd].IName) + " " +
         IntToStr(AncestorGen) + " " +
         IntToStr(ProgenyGen) + " " +
         (Sibs ? "1" : "0") + " " +
         (ColdParents ? "1" : "0") + " " +
         IntToStr(Parentline);
    }
    else //not auto
    { for (i=0; i<IndCount; i++)  //will be skipped for "select all": IndCount=-1
         s=s + " " + QuotedString(Indiv[IndList[i]].IName) ;
    }
  }
  return s;
} //TSelScriptLine::WriteLine

bool TSelScriptLine::ReadLine(int r, TStringList *SL, TViewPop *VP)
{ int p,i; AnsiString woord; bool Result;
  Result=false;
  try
  { p=1;
    ReadWoordSL(r,p,woord, SL); IndCount=StrToInt(woord);
    ReadWoordSL(r,p,woord, SL); Select= woord=="1";
    if (IndCount==0)
    { //read auto line
      ReadWoordSL(r,p,woord, SL); BaseInd=FindIndName(woord);
      if ( BaseInd==-1 ||
           ( VP!=NULL && VP->GetVPIndex(BaseInd)==-1) )
        throw Exception("BaseInd not found");
      ReadWoordSL(r,p,woord, SL); AncestorGen=StrToInt(woord);
      ReadWoordSL(r,p,woord, SL); ProgenyGen=StrToInt(woord);
      ReadWoordSL(r,p,woord, SL); Sibs= woord=="1";
      ReadWoordSL(r,p,woord, SL); ColdParents= woord=="1";
      //read Parentline; not present in files of versions pre- 2-2-2010:
      Parentline=2; //both = default
      ReadWoordSL(r,p,woord, SL);
      if (woord!="") Parentline=StrToInt(woord);
    }
    else
    { //read manual line
      if (IndCount>0) IndList = new int [IndCount];
      for (i=0; i<IndCount; i++) //will be skipped for "select all": IndCount=-1
      { ReadWoordSL(r,p,woord, SL); IndList[i]=FindIndName(woord);
        if ( IndList[i]==-1 ||
             ( VP!=NULL && VP->GetVPIndex(IndList[i])==-1) )
          throw Exception(woord+" not found");
      }
    }
    Result=true;
  }
  catch (...) {Result=false;}
  return Result;
} //TSelScriptLine::ReadLine

bool TSelScriptLine::RunLine(TViewPop *VP)
//already checked that all individuals are in VP->VPInd: no checking done here
{ int i, VPI;
  try
  { if (IndCount<0)
    { //select all
      for (i=0; i<VP->VPIndCount; i++) VP->Selected[i]=true;
    }
    else if (IndCount==0)
    { //auto select
      VPI = VP->GetVPIndex(BaseInd);
      VP->Selected[VPI]=Select;
      VP->SelectVPAncestors(VPI, AncestorGen, Parentline, Select);
      VP->SelectVPProgeny(VPI, ProgenyGen, Parentline, Select, ColdParents);
      if (Sibs) VP->SelectVPSibs(VPI, Select);
    }
    else
    { //manual select
      for (i=0; i<IndCount; i++)
        VP->Selected[VP->GetVPIndex(IndList[i])] = Select;
    }
    return true;
  }
  catch (...) { return false; }
} //TSelScriptLine::RunLine

void TSelScriptLine::Copy (const TSelScriptLine &Source)
{ int i;
  if (&Source != this)  //because of statement x.Copy(x);
  { delete[] IndList;
    IndCount=Source.IndCount;
    if (IndCount==0) IndList=NULL;
    else
    { IndList=new int[IndCount];
      for (i=0; i<IndCount; i++) IndList[i]=Source.IndList[i];
    }
    Select=Source.Select;
    AncestorGen=Source.AncestorGen;
    ProgenyGen=Source.ProgenyGen;
    Parentline=Source.Parentline;
    BaseInd=Source.BaseInd;
    Sibs=Source.Sibs;
    ColdParents=Source.ColdParents;
  }
} //TSelScriptLine::Copy


TSelectionScript::TSelectionScript(void)
{ LineCount=0; SSLine=NULL; }

TSelectionScript::~TSelectionScript(void)
{ Clear(); }

void TSelectionScript::Clear(void)
{ int i;
  for (i=0; i<LineCount; i++) SSLine[i].Clear();
  delete[] SSLine; SSLine=NULL; LineCount=0;
}

void TSelectionScript::AddSelScriptLine(void)
{ TSelScriptLine *Temp;
  int i;
  Temp = new TSelScriptLine[++LineCount];
  for (i=0; i<LineCount-1; i++) Temp[i].Copy(SSLine[i]);
  delete[] SSLine;
  SSLine=Temp; Temp=NULL;
}

void TSelectionScript::WriteLines(bool Verbose, TStrings *SL)
{ int i;
  if (!Verbose) SL->Add("SelLines "+IntToStr(LineCount));
  for (i=0; i<LineCount; i++) SL->Add(SSLine[i].WriteLine(Verbose));
} //TSelectionScript::WriteLines

bool TSelectionScript::ReadLines(int &r, TStringList *SL, TViewPop *VP)
//reads SelectionScript data from line r in SL (written in non-verbose format)
//result=false if any individual not present in the ViewPop
{ int p,n,i;
  AnsiString woord;
  bool Result;
  p=1;
  ReadWoordSL(r,p,woord,SL);
  if (UpperCase(woord) != "SELLINES") return false;
  Result=true;
  ReadWoordSL(r,p,woord,SL); n=StrToInt(woord);
  for (i=0; i<n; i++)
  { r++;
    AddSelScriptLine();
    Result=Result && SSLine[i].ReadLine(r, SL, VP);
  }
  return Result;
} //TSelectionScript::ReadLines

bool TSelectionScript::Run(TViewPop *VP)
// VP is the ViewPop where the selection is created
{ int i; bool Result=true;
  VP->ClearSelection();
  for (i=0; i<LineCount; i++)
    Result &= SSLine[i].RunLine(VP);
  return Result;
} //TSelectionScript::Run

void TSelectionScript::Copy (const TSelectionScript &Source)
{ int i;
  if (&Source != this)  //because of statement x.Copy(x);
  { delete[] SSLine;
    LineCount=Source.LineCount;
    if (LineCount==0) SSLine=NULL;
    else
    { SSLine = new TSelScriptLine [LineCount];
      for (i=0; i<LineCount; i++) SSLine[i].Copy(Source.SSLine[i]);
    }
  }
} //TSelectionScript::Copy

// ******** Non-class function ************

void ClearData(void)
//also clears the PopTreeView and closes LegendForm
{ int i;
  if (LegendForm!=NULL) LegendForm->Close();
  ProjectSaved=true;
  MainForm->MainListview->Visible=false;
  MainForm->ViewScrollbox->Visible=false;
  MainForm->MainTabctrl->Tabs->Clear();
  if (MainForm->PopTreeView->Items->Count > 0)
     delete MainForm->PopTreeView->Items->Item[0]; //root, clears all nodes and automatically ViewPops
  //NwMainForm->PopTreeView->Clear(); automatically
  TViewPop::VPCount=0;
  CurrentViewPop=NULL; SourceViewPop=NULL;
  delete FirstCrossing; FirstCrossing=NULL;
  TCrossing::CCount=0; TCrossing::Last=NULL;
  delete[] Indiv; Indiv=NULL; IndivCount=0;
  for (i=0; i<LinkGrpCount; i++) delete LinkGrp[i];
  delete[] LinkGrp; LinkGrp=NULL;
  LinkGrpCount=0;
  for (i=0; i<DataTypeCount[dtC]; i++) delete[] Ichar[i];
  delete[] Ichar; Ichar=NULL; DataTypeCount[dtC]=0;
  for (i=0; i<DataTypeCount[dtI]; i++) delete[] Iint[i];
  delete[] Iint; Iint=NULL; DataTypeCount[dtI]=0;
  for (i=0; i<DataTypeCount[dtF]; i++) delete[] Ifloat[i];
  delete[] Ifloat; Ifloat=NULL; DataTypeCount[dtF]=0;
  for (i=0; i<DataTypeCount[dtS]; i++) delete[] Ilabel[i];
  delete[] Ilabel; Ilabel=NULL; DataTypeCount[dtS]=0;
  for (i=0; i<IndivDataCount; i++)
  { if (IndivData[i].DataType==dtC && IndivData[i].V.CV.Levels!="")
       delete[] IndivData[i].V.CV.Levels;
    if (IndivData[i].DataType==dtS && IndivData[i].V.SV.Labels!=NULL)
       delete[] IndivData[i].V.SV.Labels;
  }
  delete[] IndivData; IndivData=NULL; IndivDataCount=0;
  delete[] IBDalleleFounder; IBDalleleFounder=NULL;
  delete[] IBDalleleFounderHom; IBDalleleFounderHom=NULL;
  delete[] IBDalleleIndex; IBDalleleIndex=NULL;
  TotLocCount=0; TotIBDCount=0;
  MarkCodeCount=0;
  IBDAlleleCount=0;
  //AllNameCountIsIBDAllCount=false;
  DataFileName="";
  DataFileExt="";
  ProjectFileName="";
  MainForm->Caption="Pedimap";
  delete[] strUnknown; strUnknown=NULL; UnknownCount=0;
  delete CopiedViewOptions; CopiedViewOptions=NULL;
  delete LocusList; LocusList=NULL;

  MainForm->MainTabctrl->Tabs->Clear();
} //ClearData

int AddLinkGrp(AnsiString &Name)
{ TLinkGrp **Temp;
  int i;
  Temp = new TLinkGrp* [ ++LinkGrpCount ];
  for (i=0; i<LinkGrpCount-1; i++) Temp[i]=LinkGrp[i];
  Temp[LinkGrpCount-1] = new TLinkGrp(LinkGrpCount-1, Name, TotLocCount, TotIBDCount);
  delete[] LinkGrp;
  LinkGrp=Temp;
  return LinkGrpCount-1;
} //AddLinkGrp

int FindIndName(const AnsiString &Name)
{ int i;
  if (UpperCase(Name)==strUnknown[0]) {i=-1;}
  else
  { i=0;
    while (i<IndivCount && Indiv[i].IName!=Name) i++;
    if (i==IndivCount) i=-1;
  }
  return i;
} //FindIndName

TCrossing* FindCrossParents(int P0, int P1, int CrType)
{ TCrossing* Cro;
  //can be made more efficient by searching only matings of P0
  Cro=FirstCrossing;
  while ( Cro!=NULL &&
          (Cro->CType!=CrType || Cro->Parent[0]!=P0 || Cro->Parent[1]!=P1) )
  { Cro=Cro->Next;}
  return Cro;
} //FindCrossParents

TViewPop* MakeOverallViewPop(TTreeNode* Node)
//creates a ViewPop with all individuals in population directly,
//avoiding AddVPInd for each individual
{ TViewPop* VP;
  int i;
  TVPIndListItem* ILI;

  VP = new TViewPop(Node,-1);
  VP->VPInd = new int [IndivCount];
  VP->Selected = new bool [IndivCount];
  VP->IL->Capacity = IndivCount;
  for (i=0; i<IndivCount; i++)
  { VP->VPInd[i]=i;
    VP->Selected[i]=false;
    ILI = new TVPIndListItem;
    ILI->VPIndIndex=i;
    ILI->SortRank=i;
    //ILI->Selected=false;
    VP->IL->Add(ILI);
  }
  VP->VPIndCount=IndivCount;
  VP->AddVW("Overview", true); //first view:Overview
  //VP->VW[0]->FillGenerations();
  return VP;
} //MakeOverallViewPop

int GetPPI(void)
{ return Screen->PixelsPerInch; }

int GetPrPPI(void)
//returns 0 if no printer defined
{ if (Prntr == NULL) return 0;
  else return GetDeviceCaps(Prntr->Handle, LOGPIXELSX);
}

void FillerUp(short int *Ar, int Count, int Base)
/* Ar contains an array of Count numbers, that should add up to Base.
   If this isn't exactly correct (due to rounding errors),
   the largest number(s) are changed first */
{ int *Ix; //index from largest downto smallest
  int i,j,k,m,ArSum,Eq;
  ArSum=0; for (i=0; i<Count; i++) ArSum += Ar[i];
  if (ArSum != Base)
  { //first: create and fill Ix : Index to Ar
    Ix = new int [Count];
    for (i=0; i<Count; i++) Ix[i]=i;
    //sort Ix so that Ar[Ix[0]] is the largest, Ar[Ix[1]] second largest
    // etc until Ar[Ix[Count-1]] is the smallest in Ar:
    //bubble-sort:
    for (i=0; i<Count-1; i++) {
      k=Count-1;
      for (j=Count-2; j>=i; j--) {
        if (Ar[Ix[j]]>Ar[Ix[k]])
          k=j;
      }
      //now Ar[Ix[k]] is the largest remaining item;
      //let Ix[i] point to this, i.e. exchange Ix[i] and Ix[k]
      j=Ix[i]; Ix[i]=Ix[k]; Ix[k]=j;
    }

    /* pre- 30-1-2010
    Ix = new int [Count];
    for (i=0; i<Count-1; i++)
    { j=0;
      while (j<i && Ar[Ix[j]]>Ar[i]) j++;
      //insert i at position j, move all smaller ones down:
      for (k=i; k>j; k--) Ix[k]=Ix[k-1];
      Ix[j]=i;
    }
    end pre */

    //How many numbers are (almost) equal to the largest?
    i=Ar[Ix[0]]-1; //the largest, minus1
    Eq=0;
    while (Eq<Count && Ar[Ix[Eq]]>=i)
      Eq++;
    //correct the values:
    if (Base>ArSum)
    { //increase the largest numbers:
      j = (Base-ArSum) / Eq; //amount to add to all Eq largest numbers
      k = (Base-ArSum) % Eq; //the first k numbers should be corrected one more
      for (i=0; i<Eq; i++) Ar[Ix[i]] += j;
      for (i=0; i<k; i++) Ar[Ix[i]] ++;
    }
    else
    { //decrease the largest numbers:
      j = (ArSum-Base) / Eq; //amount to subtract from all Eq largest numbers
      k = (ArSum-Base) % Eq; //the first k numbers should be corrected one more
      for (i=0; i<Eq; i++) Ar[Ix[i]] -= j;
      for (i=0; i<k; i++) Ar[Ix[i]] --;
    }
    //finally destroy Ix:
    delete[] Ix;
  } //ArSum != Base
} //FillerUp

void ConvertToBase(float *Src, short int *Dst, int Count, int Base)
/* Converts an array Src of fractions to new short int array Dst,
   where each number in Dst is a fraction of Base.
   In general simple rounding is used, but values close to zero
   (between 0.25/Base and 0.5/Base ) are rounded to 1 instead of zero
   Dst must be short int, not (un-)signed char as sometimes Base is > 255;
   this means that each element of Dst must be converted to (unsigned) char
   whre a char array is needed */

{ int i;
  short int d;
  float SrcSum;
  SrcSum=0.0; for (i=0; i<Count; i++) SrcSum += Src[i];
  for (i=0; i<Count; i++)
  { d = Src[i]/SrcSum * Base + 0.5;   //0.5 for rounding
    if (d==0 && Src[i]/SrcSum > 0.25/Base) Dst[i]=1;
    else Dst[i]=d;
  }
  FillerUp(Dst,Count,Base);
} //ConvertToBase

void FntAttToFont(TFntAtt FA, TFont* F)
{ F->Name=FA.Name;
  F->Size=FA.Size;
  F->Style=TFontStyles();
  if (FA.Style & fntBold) F->Style = F->Style << fsBold;
  if (FA.Style & fntItal) F->Style = F->Style << fsItalic;
  if (FA.Style & fntUndl) F->Style = F->Style << fsUnderline;
  F->Color=FA.Color;
} //FntAttToFont

void FontToFntAtt(TFont* F, TFntAtt &FA)
{ FA.Name=F->Name;
  FA.Size=F->Size;
  FA.Style=0;
  if (F->Style.Contains(fsBold)) FA.Style+=fntBold;
  if (F->Style.Contains(fsItalic)) FA.Style+=fntItal;
  if (F->Style.Contains(fsUnderline)) FA.Style+=fntUndl;
  FA.Color=F->Color;
} //FontToFntAtt

long double StrToFloatGen(AnsiString s)
//StringToFloatGeneral: doesn't care if decimal separator is '.' or ','
{ int p; long double f;

   try { f=StrToFloat(s); }
   catch (const Exception& e)
   { p=s.Pos(".");
     if (p>0)
     {   //assume that '.' should be ','}
         s=s.SubString(1,p-1)+","+s.SubString(p+1,s.Length());
         f=StrToFloat(s); //causes exception if still incorrect}
     }
     else
     {   p=s.Pos(",");
         if (p>0)
         {  //assume that ',' should be '.'}
            s=s.SubString(1,p-1)+"."+s.SubString(p+1,s.Length());
            f=StrToFloat(s); //causes exception if still incorrect}
         }
         else throw; //error was not due to decimal separator, rethrow
     }
   }
   return f;
} //StrToFloatGen}

AnsiString DataToStr(int d, int i, AnsiString missing)
//d=datafield, i=Indiv index
{ AnsiString R;
  switch (IndivData[d].DataType)
  {
    case dtC : if (Ichar[IndivData[d].TypeIndex][i]==NULLCHAR)
                  R=missing;
               else R=AnsiString(Ichar[IndivData[d].TypeIndex][i]);
               break;
    case dtI : if (Iint[IndivData[d].TypeIndex][i]==-MAXINT)
                  R=missing;
               else R=IntToStr(Iint[IndivData[d].TypeIndex][i]);
               break;
    case dtF : if (Ifloat[IndivData[d].TypeIndex][i]==-MAXDOUBLE)
                  R=missing;
               else R=FloatToStrF( Ifloat[IndivData[d].TypeIndex][i],
                                        ffFixed,
                                        IndivData[d].Fieldwidth,
                                        IndivData[d].V.OV.Decimals );
               break;
    case dtS : if (Ilabel[IndivData[d].TypeIndex][i]==-MAXINT)
                  R=missing;
               else R=IndivData[d].V.SV.Labels[Ilabel[IndivData[d].TypeIndex][i]];
               break;
    default :  R="error";
  }
  return R;
} //DataToStr

void DeleteFiles(AnsiString FileNameWithWildCards)
{
  TSearchRec SearchRec;
  AnsiString Dir;

  Dir = ExtractFilePath(FileNameWithWildCards);
  if (FindFirst(FileNameWithWildCards,0,SearchRec)==0)
  { do DeleteFile(Dir+SearchRec.Name);
    while  (FindNext(SearchRec)!=0);
  }
} //DeleteFiles

void CompareIntSets(int* First,   int FirstCount,
                    int* Second,  int SecondCount,
                    int* &Gained, int &GainedCount,
                    int* &Lost,   int &LostCount)
//Compares two ordered sets of integers (each entry may occur only once in each)
//and produces two new sets:
//Gained contains the items in Second not present in First (Second-First);
//Lost contains the items in First not present in Second (First-Second)
//Used fot selection scripts
{ int f,s,*Temp;
  if (FirstCount==0 && SecondCount==0)
  { GainedCount=0; Gained=NULL; LostCount=0; Lost=NULL; }
  else if (FirstCount==0)
  { LostCount=0; Lost=NULL;
    GainedCount=SecondCount; Gained = new int [GainedCount];
    for (s=0; s<GainedCount; s++) Gained[s]=Second[s];
  }
  else if (SecondCount==0)
  { LostCount=FirstCount; Lost = new int [LostCount];
    for (f=0; f<FirstCount; f++) Lost[f]=First[f];
    GainedCount=0; Gained=NULL;
  }
  else //both sets not empty
  { //Lost:
    LostCount=0; Lost = new int [FirstCount]; //temporarily max. possible size
    s=0;
    for (f=0; f<FirstCount; f++)
    { while (s<SecondCount && Second[s]<First[f]) s++;
      if (s>=SecondCount || Second[s]!=First[f])
      Lost[LostCount++] = First[f];
    }
    if (LostCount<FirstCount)
    { if (LostCount==0) Temp=NULL;
      else
      { Temp = new int [LostCount];
        for (f=0; f<LostCount; f++) Temp[f]=Lost[f];
      }
      delete[] Lost; Lost=Temp; Temp=NULL;
    }
    //Gained:
    GainedCount=0; Gained = new int [SecondCount]; //temporarily max. possible size
    f=0;
    for (s=0; s<SecondCount; s++)
    { while (f<FirstCount && First[f]<Second[s]) f++;
      if (f>=FirstCount || First[f]!=Second[s])
      Gained[GainedCount++] = Second[s];
    }
    if (GainedCount<SecondCount)
    { if (GainedCount==0) Temp=NULL;
      else
      { Temp = new int [GainedCount];
        for (s=0; s<GainedCount; s++) Temp[s]=Gained[s];
      }
      delete[] Gained; Gained=Temp; Temp=NULL;
    }
  }
} //CompareIntSets

AnsiString QuotedString(AnsiString S)
{ // returns S enclosed in double quotes if it contains blanks, else returns S
  int p;
  p=S.Length();
  while (p>=1 && S[p]>' ') p--;
  if (p==0) return S;
  else return DQ+S+DQ;
} //QuotedString

//Indexing

//example of TIndexCompare type function:
//bool CompareB(int Item1, int Item2)
//{ return B[Item1] < B[Item2]; }

void MakeIndex(int n, int indx[], TIndexCompare Compare)
//Original version Numerical Recipes in C; QuickSort algorithm
//(with l changed to p for legibility, and index 1..n changed to 0..n-1)
//Indexes an array arr[0..n-1], i.e. outputs the array indx[0..n-1] such that
//arr[indx[j]] is in sorted order for j=0,1,2...n-1.
//the comparisons of the array elements are done by function Compare,
//such that it returns true if Item1 must occur before Item2, else false
//n is not changed.
{ const M=7, NSTACK=50; //in spite of compiler warning, M and NSTACK are used!
  int i, indxt, ir=n-1, itemp, j, k, p=0, z;
  int jstack=0, *istack;

  istack = new int[NSTACK];
  for (j=0; j<n; j++) indx[j]=j;
  for (;;)
  { //sort current subarray: p..ir
    if (ir-p < M)
    { //insertion sort when subarray is small enough
      for (j=p+1; j<=ir; j++)
      { indxt = indx[j];
        //a = arr[indxt];
        for (i=j-1; i>=p; i--)
        { // if (arr[indx[i]] <= a) break;
          if (Compare(indxt,indx[i])) break;
          indx[i+1] = indx[i];
        }
        indx[i+1] = indxt;
      }
      if (jstack == 0) break; //this was last subarray to be sorted: finished
      //Pop stack: get next subarray to sort (new p and ir)
      ir = istack[jstack--];
      p = istack[jstack--];
    }
    else //current subarray still too big for insertion sort:
         //partition into further subarrays
    { //get median of elements p, (p+ir)/2 and ir as new partitioning element a
      //and rearrange them: arr[indx[p]]<=arr[indx[p+1]]<=arr[indx[ir]]
      k = (p+ir) >> 1; // div 2
      z=indx[k]; indx[k]=indx[p+1]; indx[p+1]=z;//SWAP(indx[k], indx[p+1]);
      //if (arr[indx[p]] > arr[indx[ir]])
      if (Compare(indx[p],indx[ir]))
        SWAP(indx[p], indx[ir]);
      //if (arr[indx[p+1]] > arr[indx[ir]])
      if (Compare(indx[p+1],indx[ir]))
       SWAP(indx[p+1], indx[ir]);
      //if (arr[indx[p]] > arr[indx[p+1]])
      if (Compare(indx[p],indx[p+1]))
        SWAP(indx[p], indx[p+1]);
      //now arr[indx[p]]<=arr[indx[p+1]]<=arr[indx[ir]]
      i = p+1;
      j = ir;
      indxt = indx[p+1];
      //a = arr[indxt]; //partitioning element
      for (;;)
      { do i++; while (Compare(indxt,indx[i]));//(arr[indx[i]] < a);
        do j--; while (Compare(indx[j],indxt));//(arr[indx[j]] > a);
        if (j < i) break; //pointers crossed, partitioning complete
        SWAP(indx[i], indx[j]);
      }
      //now: i=j+1, arr[indx[j]] is <=a
      //by swapping indx[p+1] and indx[j], all arr[indx[p..j]] are <= a
      //and all arr[indx[i..ir]] are >= a  :
      indx[p+1] = indx[j];
      indx[j] = indxt;
      //indx[j] is now in correct (final) place, sort the two subarrays
      //on either side: p..j-1 and i(=j+1)..ir
      //push larger subarray on stack:
      jstack += 2;
      if (jstack > NSTACK) throw Exception("NSTACK too small in indexx.");
      if (ir-i+1 >= j-p)
      { //push upper subarray on stack: i..ir
        istack[jstack] = ir;
        istack[jstack-1] = i;
        ir = j-1; //new subarray is p..j-1
      }
      else
      { //push lower subarray on stack: p..j-1
        istack[jstack] = j-1;
        istack[jstack-1] = p;
        p=i; //new subarray is i..ir
      }
    }
  }
  delete[] istack;
} //MakeIndex  Compiler warning about M and NSTACK never used incorrect!

AnsiString ParentCaption(int parent) { //parent 0 or 1
  if (FemaleParent==2) {
    return ( parent==0 ? "Parent1" : "Parent2");
  } else {
    return ( parent==FemaleParent ? "Female" : "Male");
  }
}

AnsiString ParentWithString(int parent, AnsiString s) { //parent 0 or 1
//if s="parent" returns e.g. Parent1 or Female parent
  if (FemaleParent==2) return (ParentCaption(parent));
  else return (ParentCaption(parent)+" "+s);
}

