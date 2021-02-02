#include "PCH.h"
//#include <vcl.h>
#pragma hdrstop

#ifndef DataUnitH
#define DataUnitH
//---------------------------------------------------------------------------

#include <vcl.h>
#include <Printers.hpp>
#include <Clipbrd.hpp>
//#include <dstring.h>

//#define VISAREA
 #undef VISAREA
/*if defined, DrawVisArea is used: only the visible area of the chart is drawn
  on the TBitmap VisArea to avoid excessive memory use with large charts.
  VisArea is updated with every scroll and resize operation.
  In order to show the focus and selection rectangles, the VisArea is Tranparent
  which requires a TranspColor.
  if not defined, the ViewImage contains the entire ZoomBM which is the zoomed
  ChartMF, and scrolling or resizing don't require special handling
*/


extern int IndivCount;     //size of array Indiv
extern int IBDAlleleCount; //number of different founder alleles; if >0, IBD probabilities are available
extern int Ploidy;         //must be even

extern int IndivDrawLimit; // no views are generated for Viewpops with more individuals
                           // to avoid out-of-resources problems
  
class TPedObject {};

class TCrossing ; //forward declaration

class TIndiv : public TPedObject
        { public:
            int MCount;
            AnsiString IName; //always valid, also if INameNumbers=true
            int INameNum; //if all names are numbers, the number value (for sorting)
                          //used only if INameNumbers is true
            TCrossing* ParentCross;
            TCrossing* *Mating; //array of pointers to TCrossing
            TIndiv(TCrossing *CParent=NULL);
            TIndiv(const AnsiString Name, TCrossing *CParent=NULL);
            TIndiv(TIndiv &Source); //copy-constructor
            ~TIndiv(void);
            void AddMating(TCrossing* Cro);
        }; //TIndiv

enum TDataType {dtC, dtI, dtF, dtS}; //Char, Integer, Double (float), String

struct CharVariant
       { char* Levels;
         int LevelCount;
       };

struct StringVariant
       { AnsiString* Labels;
         int LabelCount;
       };

struct OtherVariant
       { int SmallInd, LargInd, //indices of individuals with smallest and largest value,
                                //or for string: (only LargInd): longest string
             Decimals;          //only for float field: max # of decimals in data
       };

union TypeVariant {CharVariant CV; StringVariant SV; OtherVariant OV;};

struct TIndivDatum
       { AnsiString Caption;
         TDataType DataType;
         int TypeIndex,    //n-th char/integer/floating/string field
             Fieldwidth,   //in characters: max length
             ListPx;       //width of data column in MainListView, in pixels
         TypeVariant V;    //use e.g. x.V.LevelCount, x.V.Decimals
         /*union
         {  {Char* Lv; int LVC;}
            {int SmI,LgI,Dc;}
         } VD;*/
       };

struct TLicenseInfo
       { AnsiString UserName,
         UserAffiliation,
         Software;
         int Version;
         TDateTime ExpiryDate;
       };

extern TLicenseInfo LicInfo;

extern TIndivDatum* IndivData;
extern int IndivDataCount;
extern int DataTypeCount[dtS-dtC+1];
// arrays of array[0...IndCount-1] of each datatype:
// missing values are NULLCHAR, -MAXINT, -MAXDOUBLE and "" respectively,
// so always sorted as smaller than smallest value
extern char** Ichar;  //example: IChar[3,150] is fourth character datafield of 151-st Individual
extern int**  Iint;
extern double** Ifloat;
extern int** Ilabel;
extern AnsiString DataToStr(int d, int i, AnsiString missing);


const int ctCross=0, ctSelfing=1, ctDH=2, ctMutation=3, ctVegProp=4,
          CrossTypeCount=5;

class TCrossing : public TPedObject
        { public:
            static int CCount;
            static TCrossing* Last;
            TCrossing *Prev, *Next;
            int CNum,    //ID number of TCrossing (used for debugging)
                CType,   //cross, selfing, mutation
                OCount;  //number of Offspring
            int Parent[2]; // Indices of parents in array Indiv
            int *Offsp;    // open array of indices to offspring in array Indiv
            //bool SAMoved;  //used by TView::SeparateAncestries
            bool Marked;  //used only during MarkSubtrees
            TCrossing(int P0, int P1, int CrType);
            ~TCrossing(void);
            TCrossing(TCrossing &Source); //copy-constructor
            void AddOffspring(int Ind);
            //TCrossing* GetCrossing(int P0, int P1);
        }; //TCrossing

/* On Relative Positions (used in TGenStrip and TCrossStrip)
   The positions of TIndivs and TCrossings are stored as the relative position
   of their centers on the net page size (between the margins).
   * This representation is independent of the pixel resolution used for drawing
   * Once Indivs or Crossings have been moved by the user from their initial
     positions, their relative positions can be maintained even after changing
     the page sizes or after changing the pedigree orientation from Left-Right
     to Top-Down or v.v. (although this may cause them to overlap the margins or
     each other). As long as all are still in their initial positions, new
     positions will be recalculated on such changes.

   This is not to be confused with the base on which the initial positions are
   calculated: That is done (in the early-late generation direction) on base
   of (-0.5*Indiv size -Crossing size) to
      (Net page size + 0.5*Indiv size + Crossing size),
   because this leads to pleasant appearance and guarantees everything stays
   within the margins. The result of these calculations are then converted to
   "normal" relative positions.
*/

class TView;

class TGenStrip                   //generation strip in tree view
                                  //stores the generation structure and the
                                  //graphics information of the TView
        { public:
            TView *VW;            //View to which this GenStrip belongs
            int ICount,           //number of TIndiv's in GenStrip
                FamGapCount,      //number of gaps between full-sib families
                SibGapCount;      //number of spaces between sibs within families
            int *Ind;             //open array of Indiv indices
            int GPos,   //position of top/left of this GenStrip
                  *IPos;  //array: position of top/left of each Ind[i] in this GenStrip
            TMetafile **IndMF; //array: one metafile per Ind[i] in this GenStrip
            TGenStrip(TView *View);
            TGenStrip(TGenStrip &Source); //copy-constructor
            ~TGenStrip(void);
            int AddVPInd(int VPI, int index=-1);
            int RemoveIndIndex(int IndIndex);
            int RemoveVPInd(int VPI);
            void CountGaps(void); //count FamGapCount and SibGapCount
            inline int GetIndIndex(int VPI);
        };  //TGenStrip

class TCrossStrip             //Strip containing Cross etc symbols
        { public:
            int CCount;
            TCrossing **Cross;
            int CGPos,     //position of top/left of this CrossStrip
                  *CIPos;    //array: position of top/left of each Cross[i] in this GenStrip
            TCrossStrip(void);
            TCrossStrip(TCrossStrip &Source); //copy-constructor
            ~TCrossStrip(void);
            int AddCross(TCrossing *Cro, int index=-1);
            int RemoveCrossIndex(int CrIndex);
            int RemoveCross(TCrossing *Cro);
            inline int GetCrossIndex(TCrossing *Cro);
       }; //TCrossStrip


const int fntBold=1, fntItal=2, fntUndl=4;

struct TFntAtt                //Font Attributes
        { AnsiString Name;
          int Size;
          int Style; //0=none; add fntBold,fntItal,fntUndl
          TColor Color;
        };

class TView; //forward

struct TVPIndListItem
       { int VPIndIndex, //points to item in array VPInd
             SortRank;   //Rank after last Sort
         bool Selected();
       };

class TVPIndList: public TList
      { private:
          TVPIndListItem* GetVPIndListItem(int Index);
          //void SetVPIndListItem(int Index, TVPIndListItem* It);
        public:
          __property TVPIndListItem* Ind[int Index] =
              { read=GetVPIndListItem //,
                //write=SetVPIndListItem  not needed
              };
          int GetIndex(int VPI);
      };

struct TVPCrossListItem
       { TCrossing* VPCro; //pointer to global TCrossing instance
         int SortRank;     //Rank after last Sort
       };

class TVPCrossList: public TList
      { private:
          TVPCrossListItem* GetVPCrossListItem(int Index);
        public:
          __property TVPCrossListItem* Cro[int Index] =
              { read=GetVPCrossListItem //,
                //write=SetVPIndListItem  not needed
              };
      };

class TViewPop; //forward declaration

//const sstManualSelect=0, sstManualDeselect=1, sstAutoSelect=2, sstAutoDeselect=3;

class TSelScriptLine
      { public:
          int IndCount,      //Manual: >0, nr of manually (de-selected individuals;
                             //Auto: 0
                             //Select All: -1
              *IndList,      //Manual: selected individuals (indices in Indiv array), ordered
                             //Auto, All: NULL
              BaseInd,       //Auto select: base individual (index in Indiv array)
                             //Manual, All: -1
              AncestorGen,   //Auto: ancestor generations
              ProgenyGen,    //Auto: progeny generations
              Parentline;    //Auto: search pedigree through first parent(0), second parent(1) or both(2)
          bool Select,       //true: select, false: deselect as specified
               Sibs,         //Auto: include full sibs
               ColdParents;  //Auto: include progeny parents
          TSelScriptLine(void);
          ~TSelScriptLine(void);
          AnsiString WriteLine(bool Verbose);
          bool ReadLine(int r, TStringList *SL, TViewPop *VP);
          bool RunLine(TViewPop *VP);
          void Copy (const TSelScriptLine &Source);
          void Clear(void);
      };


class TSelectionScript
      { public:
          int LineCount;
          TSelScriptLine *SSLine;
          TSelectionScript(void);
          ~TSelectionScript(void);
          void AddSelScriptLine(void);
          void Clear(void);
          void WriteLines(bool Verbose, TStrings *SL);
          void Copy (const TSelectionScript &Source);
          bool ReadLines(int &r, TStringList *SL, TViewPop *VP);
          bool Run(TViewPop *VP);
      };

const ordFoundersFirst=0, ordSubtreesDown=1, ordManual=2;

class TViewPop: public TObject
                 //describes which indiv's appear at which Generation,
                 //and at which position number in Generation
      { public:
          /*TViewPop *ParentViewPop,   //these are maintained through the PopTreeView
                   **SubViewPop;
          int SubVPCount;  //number of SubViewPop      */
          static int VPCount; //for assigning unique VPnum to each new ViewPop
          int VPnum,  //identification number, used a.o. for sorting in Original order
              LastSubpopNumber, //for assigning name to next subpop
              Flag;   //0..4, 1..4 - exclamation mark, 0=none
          TTreeNode* VPNode; //Node in PopTreeView
          int *VPInd;      //open array of Indivs (indices to global array TIndiv* Indiv)
                           //present in this ViewPop;
                           //ordered!
          TSelectionScript OrigScript; //script how the ViewPop originated from its parent ViewPop
          int VPIndCount,  //number of Indivs in VPInd
              FocusInd,    //points to focussed ind in IL, -1 if none  NU NAAR ITEM in VPIND
              VWCount,     //number of Views in array VW
              CurrVW;      //index of current View; 0=Overview, -1 IndivList
          bool *Selected,  //for each VPInd: is it Selected?
               *Sbt;       //for each VPInd: does it belong to current subtree?
          int LastSelCount,   //number of individuals in last selection
              *LastSelection, //indices (in Indiv array) of last selection
              Ordering;   //how are the individuals ordered between/within generations?
                          //0=ordFoundersFirst : all founders at first generation
                          //1=ordSubtreesDown  : result of MoveSubtreesDown
                          //2=ordManual        : manually rearranged (not implemented yet)
          TSelectionScript SelScript; //script how the current selection was obtained
          TVPIndList *IL;  //list of all indivs in ViewPop, ordered according to last Sort:
                           //each element of list is composed of VPI + previous sort rank
          bool NumberSortUp, //numbers of Indiv last sorted up (=index in Indiv array)
               NameSortUp,   //INames or NameNums last sorted Up
               ParentSortUp[2], //Parent Names or NameNums last sorted Up
               *DataSortUp;  //Data last sorted Up
          TVPCrossList *CL;  //list of all Crosses in ViewPop, ordered according to last Sort:
                             //each element of list is composed of pointer to global
                             //TCrossing instance + previous sort rank
          TView **VW;      //array of pointers to Views
          TStringList *Notes;
          TViewPop(TTreeNode *Node, int VPnumber);
          TViewPop(); //only call to make a temporary ViewPop without Node!
          __fastcall ~TViewPop(void);
          TViewPop (TViewPop &Source); //copy-constructor
          //void AddSubViewPop(TViewPop* SubVP);
          //bool RemoveSubViewPop(TViewPop* SubVP);
          int AddVW(AnsiString ViewName, bool Overview=false);
          int DeleteVW(AnsiString ViewName);
          int AddVPInd(int I);
          int GetVPIndex(int I); //index of Indiv[I] in VPInd; -1 if not present
          int GetVPIndex(AnsiString Name);
          bool Present(int I); //is Indiv[I] present in VPInd?
          bool IsVPFounder(int VPI); //is VPInd[VPI] a founder in ViewPop?
          bool IsVPSemifounder(int VPI); //is VPInd[VPI] a semi-founder in ViewPop?
          void MarkFounders(int VPI); //mark founders in VPInd for VPInd[VPI]
          bool MarkSubtree(int VPI); //mark whole subtree for VPInd[VPI]
          int MinGenerDist(int *Gener); //minimum generation distance between
                                        //individuals in current Subtree and their
                                        //offspring outside current subtree
          void SelectVPAncestors(int VPI, int Gen, int Parentline, bool Select);
          void SelectVPProgeny(int VPI, int Gen, int Parentline, bool Select, bool WithColdParent);
          void SelectVPSibs(int VPI, bool Select);
          void SortIL(int ColIndex); //sort IL
          void SortINumber(void);
          void SortIName(void);
          void SortIParent(int Par);
          void SortIData(int Field);
          void SelectedToSet(int* &Set, int &Count);
          void UpdateSelection(int IgnoreVPInd);
          void ClearSelection(void);
          void WriteData(TStringList *SL);
          void ChangeOrdering(int NewOrdering);
          int CopyIndivs(TViewPop* VP, bool CompleteSemifounders);
          TStringList* MakeIndListSL(bool Header);
      }; //TViewPop

//const FamSpaceFact=2.0; //spacing between families 2.0 * within families

//IndivContents values in ViewOptions:
const icNone=0,      //only name
      icTraits=1,    //phenotypic traits
      icIBDprob=2,   //probability rectangles
      icIBDall=3,    //most probable allele
      icMarkers=4;   //scored allele
extern AnsiString IndivContentsTypes[5];

class TViewOptions
      { public:
          int ID;
          // Page options:
          //int PgWidthMM, //all in 1/100 mm units!
          //    PgHeightMM,
          //    MargMM[4];  //left,top,right,bottom
          //bool PgTransp;  //page (background) transparent; always!
          TColor PageCol;   //only for background onscreen (and print?)

          //Layout options:
          bool ShowCrossings;
          int  CrossSizeMM;  //size of cross symbols, in 1/100 mm units
          bool LeftRight;    //pedigree from left to right instead of top-down
          //int InFamSpace;  //0..1000 = 0% .. 100%: the spacing of individuals within a
                             //progeny, relative to the spacing between progenies
                             //(dist from margin = dest between families)
          int  GenerDist,    //distance between generations,
                             //times cross symbol size (>=1)
               BetwFamSpace, //% of Individual size (0-10000%)
               BetwSibSpace; //% of Between family space (0-100%)

          // Individuals options:
          TColor IBackCol,   //fixed color
                 //IFrameCol,  //depends on PageCol, not stored, read or set separately
                 LoCol,      //color of lowest trait value
                 HiCol,      //color of highest trait value
                 MVCol,      //color of missing trait value
                 *TraitCol;  //color for each char or string trait level
                             //[0..TraitColCount-1]
                 //INameCol;
          int TraitColCount, //number of colors for discrete trait
              ColorTrait;    //index in IndivDataCount: trait used for fill color
                             //-1 if fill color not based on trait values

          bool IndFront,     //individuals appear in front of connections
                             // (not implemented yet, indiv always behind connections)
               IFixedColor,  //if false, fill (Background) based on trait
               IndFrames,    //false: used for Overview: rectangle fits exactly
                             //longest Indiv name + 2 spaces;
                             //true: 1.5* height of name + space for extra info
               NameCentered; //false: name aligned to left side (for Overview)
                             //normal views always true

          byte IndivContents; //one of ic-constants: what to show in Indiv

          TFntAtt INameFont;
               //Note: using TFonts in TViewOptions led to many problems with
               //copying the ViewOptions or the Fonts themselves;
               //therefore now TFntAtt used instead (as simple struct's)

          //Traits options:     (not yet implemented)
          int TraitSelCount;
          int *TraitSelected; //array (set) of size TraitSelCount,
                              //with indices in IndivData of selected traits

          //Marker allele options:
          TColor *MarkCodeCol; //one color for each marker allele colorcode
          bool MarkByName;     //true: sort by name, false: sort by position
          int MarkSelCount;
          int *MarkSelected;   //array (set) of size MarkSelCount,
                               //contains LocNrs of all selected markers in display order

          //IBD options:
          int ProbWidthMM,   //sizes of probability rectangles, in 1/100 mm units
              ProbHeightMM,
              //MinSpaceMM,  //minimal distance between Indiv's and Generations,
                             //must be more than twice width of Selection border
              //MinAllProb,  //threshold probability (0..1, in 0.001 units for showing allele name
                             //negative or >1 to hide allele names
              LG,            //number of linkage group to show; none if -1
              ProbBig,       //IBD threshold: <= for bold alleles * 10000
              ProbSmall;     //IBD threshold: >= for italic alleles * 10000
          bool AllIBDmrk,    //true if all IBD markers of current LG are selected, else:
               *SelIBDmrk;   /*indicates which markers IBDs are selected
                               AllIBDmrk is an efficient way to store the default
                               situation that all markers (of the selected LG) are
                               selected. Avoids having to store all these markers
                               for all views where they are not relevant)
                             */
          TColor *ACol;      //one color per IBD allele [0..IBDAlleleCount-1]
          bool HomFounders;  // if founders homozygous, both haplotypes get same color
          TFntAtt AlleleFont;

          //Other options:
          // Crossing symbol options:
          TColor CrossCol,      //color of cross symbol and lines to progeny
                 Par1Col,       //color of line to parent 1
                 Par2Col,       //color of line to parent 2
                 ParSelfedCol;  //color of line to selfed parent
          bool InfoSelected,    //show if individual is selected in InfoMemo
               InfoParents,     //show Parents of individual in InfoMemo
               *InfoTrait;      //array of traits to be shown in InfoMemo

          // Methods:
          TViewOptions(void);
          ~TViewOptions(void);
          TViewOptions (TViewOptions &Source); //copy-constructor
          TViewOptions operator= (TViewOptions &Source); //assignment operator
          void Copy (const TViewOptions &Source);
               //makes a deep copy of Source into already existing *this
          void ClearArrays(void);
          void MakeOverviewOpt(void); //changes only the relevant options
          void MakeBuiltinOpt(void); //changes all options to built-in defaults
          void SetDefTraitColors();
          void SetDefAlleleColors(void);
          bool LocSelected(int LocNr); //true if LocNr in MarkSelected array
          void ReadFromStringlist(TStringList *SL,
                                  int OldIBDAlleleCount, bool Default);
          void WriteData(TStringList *SL, bool Default);
      }; //TViewOptions




class TView   //describes how to draw a specific view of a ViewPop, including
              // - Options
              // - positions of individuals in "generation strips"
              // - pixel sizes based on MM sizes from options and actual PPI
      { public:
          AnsiString ViewName; //appears on the tab

          bool Overview,     //if true, use Overview options and fit page
               ChartsizeError, //if true, ChartMF contains only errormessage
               ManualAdjust, //true if positions manually adjusted;
                             //on options change then no recalculation of
                             //relative positions
               ChartCalculated; //generation struct, sizes & positions, not drawn
          //Indiv Metafile sizes and data (Pixel sizes based on PrPPI):
          int Zoom, //percentage as displayed
              ZoomedLeftVis,ZoomedTopVis, //Zoomed-chart pixel corresponding to left and top of VisArea
              ZoomedChartWidth, ZoomedChartHeight, //in pix
              ScrollPosX, ScrollPosY, //last Position of Horz en Vert ScrollBar
              PrIndWidth,  //total Indiv size, in printer pixels
              PrIndHeight,
              SelBorderWidth, //width of Selection border
              INameWidth,   //width of longest IName+'  ', in printer pixels
              INameHeight,  //2* font height, in printer pixels
              LGNameWidth,  //same for linkage group names
              LGNameHeight,
              *ProbLeft,    //array of positions of Probabilities rectangle or alleles in IndMF
              ProbTop,
              ProbWidth,    //width of Probabilities rectangle of widest allele name in IndMF
              ProbHeight,
              *LocPx,     //array of vertical locus position relative to LGTop
              *IBDPx,     //array of vertical IBDpos position relative to LGTop
              PrCrossSize; //width and height of (square) Cross symbols, in printer pixels
          TColor *IFrameCol; //for each VPInd the frame color, if not Opt.IFixedCol
          TRect PrMarg; //margins, in printer pixels

          double ZoomFact; //Zoom expressed in pixels factor

          //Data on the picture currently on screen, in screen pixels:
          //TRect Marg;
          int //IndWpx, IndHpx, CrossPx, //Width and Height of Indiv
              MaxIndPerGS, //max. GS->ICount
              //PrMinWidth,  //min. Chart size (within margins) in printer pixels
              //PrMinHeight, //id.
              PrChartWidth, PrChartHeight,
              PrMargPx, //pixels for standard margin of 10 mm included in chart
              ScrollXpos,
              ScrollYpos;
          //Graphics data structures related to Pedigree structure:
          /*
          int *GPix,      //array: Pixel position of each GS[g] in screen pixels
              **IPix;    //array: Pixel position of each Ind[i] in each GS[g] in screen pixels
          */
          //float *GPos,   //array: relative position of center of each GS[g]
          //      **IPos,  //array: relative position of center of each Ind[i] in each GS[g]
                         //both relative to the range: first margin-1/2 Ind.size ... last margin+1/2 Ind. size
          //float *CPos,   //as GPos, for CrossStrips
          //      **CCPos; //as IPos, for Cross symbols in CrossStrips
          //TMetafile ***IndMF, //array: one metafile per Ind[i] in each GS[g]
          TMetafile *CrossMF, //cross symbol
                    *SelfMF,  //selfing symbol
                    *DHMF,    //DH symbol
                    *MutMF,   //mutation symbol
                    *VegPropMF, //vegetative propagation symbol
                    *ErrorMF, //unknown cross type, shown as "?"
                    *ChartMF; //metafile of the complete chart (at screen resolution)
          //Data describing Pedigree structure
          int GCount;      //the number of GenStrips in GS
          TGenStrip **GS;  //array holding the pointers to the GenStrips
          TCrossStrip **CS; //same for CrossStrips
          int *Gener,      //array: for each VPInd the Generation (-1 if not placed)
              *GIx,        //array: for each VPInd the Index number in Gener
                           //this means that GS[Gener[VPI]]->Ind[GIx[VPI]] == VPI
              *FUM;        //first UnMoved individual in each generation
                           //(only used in SeparateAncestries, NULL otherwise)
          TViewPop *VP;    //the ViewPop to which this View belongs
          TViewOptions Opt;
          TStringList *ZoomValues; //percentages displayed
          //methods:
          TView (TViewPop *ViewPop, AnsiString ViewName, bool Overview=false);
          ~TView (void);
          TView (TView &Source); //copy-constructor
          //methods for constructing generation structure:
          int AddGS(int AddNum=1);
          void AddVPInd(int VPI, int G, int GP=-1);
          void RemoveIndIndex(int G, int GP);
          void RemoveVPInd(int VPI);
          //void RemoveWProg(int I);
          void FillGenerations(void);
          void DeleteGenerationStruct(void);
          void AssignWProg(int I, int &FamNr, int* Fam);
          //void SeparateFamilies(void); //niet meer nodig
          void MoveSubtreesDown(void);
          void MoveLeftWithSibsNAncestry(int G, int I);
          void SeparateAncestries(void);
          //methods for constructing charts
          int MaxINameWidth(TCanvas *CV);
          TMetafile* DrawIndMF(int VPI, bool DrawFrame);
          void DrawIndFrame(int VPI, TMetafileCanvas *MFC);
          TMetafile* DrawCrossMF(int PixSize);
          TMetafile* DrawSelfMF(int PixSize);
          TMetafile* DrawMutMF(int PixSize);
          TMetafile* DrawTextCrossMF(int PixSize, int CrType);
          TMetafile* GetCroMF(TCrossing *Cro); //draws if NULL
          void CalcIPos(int G);
          void CalcGPos(void);
          TRect CalcIRect(int VPI);
          TRect CalcIRect(int G, int I);
          //TRect CalcMargRect(int CurrPPI);
          void DrawArc(TCanvas *CV, TPoint PP, TPoint PO, TRect Marg);
          //void DrawChart(TCanvas *CV, TRect Marg, int CurrPPI,
          //               int &IndWpx, int &IndHpx, int &CrossPx);
          //void DrawChart(TCanvas *CV);
          TMetafile* DrawChartMF(bool DrawIndFrames, TStringList* coord);
          TMetafile* DrawChartExportMF(TStringList* coord);
          void CalcZoomData(void);
          void CreateGraphicsStruct(void);
          void DeleteIndMetafiles(void);
          void DeleteCrossMetafiles(void);
          void DeleteGraphicsStruct(void);
          //TMetafile* DrawMetafile(void);
          void CalcInitialChart(void); //starting with VP and Opt, create new chart
          void CreateGenerationStructure(void);
          //void FillCrossStrip(int g);
          bool ChartFits(void); //true if chart fits within margins, using current options
          void CalcStrip(int g, int MargPx, int ChartPx, int IndPx);
          void CalcInitialPositions(void); //based on generation structure and Opt
          void CalcIndivNCrossSize(void); //calculates PrIndWidth and -Height
          TPoint CalcNoImageMessageSize(void);
          TMetafile* DrawNoImageMessageMF(void);
          bool FindCrossing(TCrossing *Cro, int &G, int &I, int MaxGener);
          void GetUnzoomedPoint(int ZoomX, int ZoomY, int &X, int &Y);
          void GetZoomedPoint(int X, int Y, int &ZoomX, int &ZoomY);
          TRect GetZoomedRect(TRect R);
          void GetObjAtPoint(int X, int Y, int &g, int &VPI, TCrossing* &Cro);
          TMetafile* DrawChartsizeError(void);
          void FillZoomvalues(int WidthPx, int HeightPx);
          int GetZoomValIndex(void);
          int GetBestZoomValIndex(void);

      }; //TView

class TLocus
      { private:
          signed char *AllIxArr; //array[0..IndivCount-1][0..Ploidy-1]
          inline signed char GetAllIx(int I, int P)
            { return *(AllIxArr + I*Ploidy + P) ; }
          inline void SetAllIx(int I, int P, signed char Value)
            { *(AllIxArr + I*Ploidy + P) = Value; }
          byte *AllCodeArr; //array[0..IndivCount-1][0..Ploidy-1]
          inline byte GetAllCode(int I, int P)
            { return *(AllCodeArr + I*Ploidy + P) ; }
          inline void SetAllCode(int I, int P, byte Value)
            { *(AllCodeArr + I*Ploidy + P) = Value; }

        public:
          short int LocNum, //numbered from 0 within LinkGrp
                    LGnr,   //number of LinkGrp to which locus belongs
              AllNameCount, // actual number of allele names; may be different from IBDAlleleCount
                            // (future: if equal to IBDAlleleCount for all loci, some correspondence
                            //  with founder alleles may be implemented/used)
              IBDix;        // index to the TIBDposition corresp. to the locus
                            //If IBDpos[1]==-1, use IBDprobabilities of IBDpos[0];
                            //else interpolate
          AnsiString LocName;
          float cM; //position (zero-based)
          AnsiString *AllName; //array [0..AllNameCount-1] of names of alleles
                               //stored in order of input file, i.e. no sorting, possibly doubles
          signed char *FounderAllele; //array [0...IBDAlleleCount-1] of indices into AllName,
                                    //or NULL if no FounderAlleles are given
          __property signed char AllIx [int I][int P] =
            { read=GetAllIx, write=SetAllIx }; //address AllIx as 2-dimensional array
               //I=Indiv, P=0..Ploidy-1
               //contains index of allele (0..AllNameCount-1) observed
               //   (n=Ploidy alleles, possibly ordered)
               //refers to array AllName, first corresponding name in that array
          __property byte AllCode [int I][int P] =
            { read=GetAllCode, write=SetAllCode }; //address AllCode as 2-dimensional array
               //I=Indiv, P=0..Ploidy-1
               //contains code for allele (any purpose, e.g. wrong, ordered etc; used to highlight/color allele)
          //void CreateObsArrays(void);
          //inline bool AlleleObs(void) { return AllIxArr!=NULL; }
          TLocus(short int Num, short int LG, AnsiString &Name, float Position);
          ~TLocus(void);
          TLocus (TLocus &Source); //copy-constructor
          inline signed char* IndAllIxArray(int I) //for binary reading and writing
            { return AllIxArr + I * Ploidy; }
          inline byte* IndAllCodeArray(int I) //for binary reading and writing
            { return AllCodeArr + I * Ploidy; }
      }; //TLocus

class TIBDposition
      { private:
          byte *AllProbArr; //array[0..IndivCount-1][0..Ploidy-1][0..IBDAlleleCount-1] of AllProb:
                            //Allele probabilities in part per 240 = 0.417% units
          inline byte GetAllProb(int I, int P, int A)
            { return *(AllProbArr + I*Ploidy*IBDAlleleCount + P*IBDAlleleCount + A) ; }
          inline void SetAllProb(int I, int P, int A, byte Value)
            { *(AllProbArr + I*Ploidy*IBDAlleleCount + P*IBDAlleleCount + A) = Value; }


        public:
          short int LGnr;   //number of LinkGrp to which IBDposition belongs
          float cM; //position (zero-based)
          __property byte AllProb [int I][int P][int A] =
            { read=GetAllProb, write=SetAllProb }; //address AllProb as 3-dimensional array:
                           //I=Indiv, P=0..Ploidy-1, A=Allele
          TIBDposition(short int LG, float Position);
          ~TIBDposition(void);
          TIBDposition (TIBDposition &Source); //copy-constructor
          inline byte* IndArray(int I) //for binary reading and writing
            { return AllProbArr + I * Ploidy * IBDAlleleCount; }

      }; //TIBDposition

class TLinkGrp   //linkage group
      { public:
          int LGNum;
          AnsiString LGName;
          int LocCount,
              StartLocNr, //number of first locus, counting over all LinkGrp
              IBDCount,   //number of IBDpositions
              StartIBDNr; //number of first IBDposition, counting over all LinkGrp
          TLocus **Locus; //array of pointers to TLocus
          TIBDposition **IBDpos; //array of pointers to TIBDposition
          TLinkGrp(int Num, AnsiString &Name, int StartLoc, int StartIBD);
          ~TLinkGrp (void);
          TLinkGrp (TLinkGrp &Source); //copy-constructor
      }; //TLinkGrp


// global variables and constants:
const char CR=13;
const char LF=10;
const char //cDQ=34, //double quote: "
           cTb=9;  //Tab

extern AnsiString TempDir; //system temporary directory
extern AnsiString ProgDir; //path of Pedimap.exe
extern AnsiString DataDir; //path of datafile
extern AnsiString DataFileName; //filename of datafile without extension or path
extern AnsiString DataFileExt; //extension of datafile
extern AnsiString ProjectDir; //path of project file
extern AnsiString ProjectFileName; //filename of project file without extension or path

extern AnsiString CRLF; //=CR+LF;
extern AnsiString DQ,Tb; //=cDQ,cTb
const AnsiString ProjectExt=".pmp";
extern TIndiv *Indiv;    //array with all Indiv
extern TCrossing *FirstCrossing, *LastCrossing;
extern bool INameNumbers, //Indiv names are numbers
            FounderAllelesPresent; // are present or absent at all loci
extern AnsiString *strUnknown; //user specified, for reading unknown parents and missing data
extern int UnknownCount; //the number of strings representing Unknown/missing values
//const AnsiString strNoParent="-"; //for display/output of second parent in uniparental progenies
const AnsiString strDefNullHomoz="$"; //default value for strNullHomoz
extern AnsiString strNullHomoz; //for reading Null-or-homozygote alleles
const AnsiString strDefConfirmedNull="null"; //default value for confirmed null allele
extern AnsiString strConfirmedNull; //for reading confirmed null allele
extern AnsiString CrossTypeString[CrossTypeCount];

extern AnsiString PopName; //Population name
extern int LinkGrpCount; //number of linkage groups
extern int TotLocCount; //total number of loci over all LinkGrp
extern int TotIBDCount; //total number of IBD positions over all LinkGrp
extern TLinkGrp **LinkGrp; //array of pointers to linkage groups
extern int FemaleParent; //0:Female/Male, 1: Male/Female; 2:Parent1/Parent2

extern TPrinter *Prntr;
extern int PPI, PrPPI; //screen and printer PixPerInch

extern int IndNameListWidth; //width of longest Individual name in Listview pixels
extern byte MarkCodeCount; //Marker allelecodes from 0 to MarkCodeCount=maximum code read +1 (max 255)
                          //(= max. number of colors needed to represent allele codes)
//extern bool AllNameCountIsIBDAllCount; //true if for all loci the number of allele
                         //names is equal to the number of founder alleles
                         //(so they can be colored according to founder color)
extern int LinePx; //width of all lines
extern int* IBDalleleFounder;     //Founder indiv corresponding to each IBDallele
extern int* IBDalleleFounderHom;  //Founder homolog corresponding to each IBDallele
extern int *IBDalleleIndex; //sorted alphabetically by founder, within founder by homolog;
                     //gives references into IBDalleleFounder and IBDalleleFounderHom

extern TViewPop *CurrentViewPop, *SourceViewPop;
//extern TViewPop **VwPop; //array of pointers to ViewPop's = SubPop's
extern TViewOptions DefaultOptions;
#define max(a, b)  (((a) > (b)) ? (a) : (b))
enum TRecType {Sel,Foc}; //Selection or Focus rectangle in MainForm::AddShape
//extern TPrinter *Prntr;
#define NULLCHAR 0x00
extern bool ProjectSaved;
extern TList *LocusList; //alphabetically sorted list to all TLocus of all linkage groups

//indexing:
#define SWAP(a,b) { itemp=(a); (a)=(b); (b)=itemp; }  // brackets {} are necessary!
typedef bool (*TIndexCompare)(int Item1, int Item2);
// true if Item1 and Item 2 must be exchanged (i.e. if Item1 must come after Item2
void MakeIndex(int n, int indx[], TIndexCompare Compare);


// functions:
void ReadWoord (int &p, AnsiString &Woord, const AnsiString &Line);
void ReadWoordQ (int &p, AnsiString &Woord, const AnsiString &Line);
void ReadWoordSL(int r, int &p, AnsiString &Woord, TStringList *SL);
void EditEnhancedMetaFile(AnsiString fname);
void ClearData(void);
int FindIndName(const AnsiString &Name);
int AddLinkGrp(AnsiString &Name);
TCrossing* FindCrossParents(int P0, int P1, int CrType);
TViewPop* MakeOverallViewPop(TTreeNode* Node);
int GetPPI(void);
int GetPrPPI(void);
void ConvertToBase(float *Src, short int *Dst, int Count, int Base);
void FntAttToFont(TFntAtt FA, TFont* F);
void FontToFntAtt(TFont* F, TFntAtt &FA);
long double StrToFloatGen(AnsiString s);
void DeleteFiles(AnsiString FileNameWithWildCards);
TLocus* GetLocusByNr(int LocNr); //LocNr counts over all loci in all LinkGrp
TLocus* GetLocusByName(AnsiString LocName);
int GetLocNr(TLocus *Loc);   //LocNr counts over all loci in all LinkGrp
void CompareIntSets(int* First,   int FirstCount,
                    int* Second,  int SecondCount,
                    int* &Gained, int &GainedCount,
                    int* &Lost,   int &LostCount);
void DecompColor(TColor Col, int &Red, int &Green, int &Blue);
TColor MakeColor (int R, int G,int B);
TColor ColorRange(TColor LoCol, TColor HiCol, float f);
void FillColors(int ColCount, TColor *Col, TColor LoCol, TColor HiCol, int trait);

int GreyScale(TColor Col);
TColor LightVersion(TColor Col);
AnsiString QuotedString(AnsiString S);
AnsiString ParentCaption(int parent); // parent 0 or 1
AnsiString ParentWithString(int parent, AnsiString s); //parent 0 or 1
    //if s="parent" returns e.g. Parent1 or Female parent


const MaxTraitColors=35; //for Char and String data: if more levels, use continuous distribution
const Bor=10; //empty border (10 pixels) around zoomed chart

extern TViewOptions *CopiedViewOptions;

//the missing-value symbols expected by FlexQTL:
const AnsiString mv_parent = "-";
const AnsiString mv_trait  = "-";
const AnsiString mv_marker = "-";

extern int CommandlineModeInt;



#endif
