/*
New developments listed in report for second Hidras year (feb 2004 - feb 2005):
- The possibility to create sub-populations, by selecting individuals manually and/or 
  based on pedigree relationships, and the automatic documentation of these selections;
- The possibility to save a Pedimap project, including all sub-populations and views;
- The possibility to add new individuals, markers and phenotypic data to a project, 
  by submitting a new data file;
- A second visualization of IBD probabilities: the display of the name of the most likely 
  allele  and an indication of the probability;
- The possibility to colour individuals based on the values of phenotypic traits
- Removed limitations on the size of the charts (except limitations of the hardware); 
  charts are automatically sized to contain all selected individuals
- Automatic sorting of pedigrees; pedigrees may be entered in any order
- Automatic ordering of marker scores; may be entered in any order
- Export of data as txt files, charts as bmp and emf files

Changes since Hidras meeting feb 2005:
2005-02-14 - Added progress form for opening datafiles
           - Added space in front of Indiv name in Overview, so left edge not obscured
2005-02-15 - Multiple strUnknown possible for input, only first used for display/output
2005-02-17 - ProgressForm: MainForm disabled during show, corrected handling of
             Stop button (used exception instead of exit in sub-routines of ReadDataFile)
           - Added pop-up menu for View tabs
           - Added Delete View
           - Added Rename View
2005-02-19 - Improved reading speed 5-fold by replacing ReadwoordSL by Readwoord
             in ReadIBDProbabilities
2005-02-23 - Added: FounderAlleles, different from Allelenames
           - Added: FounderAlleles and observed alleles can also be read if AlleleNames
             not present; then Allelename list is built up while reading these.
           - Changed: checks that IBD probabilities are given for all or no linkage groups,
             and FounderAlleles and Allele scores for all or no loci.
2005-02-27 - Added: Copy View options, Paste View options (in context menu View tabs)
2005-03-03 - Improved some error messages
           - Corrected: when Individual list shown and Add View selected, the new View
             was not shown
2005-03-05 - In reading and saving view options in project files, linkage groups are now
             identified by name instead of number
2005-03-06 - Improved: Population treeview starts fully expanded on opening project
           - Corrected: ClearData also hides MainListView and ViewScrollbox, and clearss tabs;
             preventing MouseOver errors while opening new project or importing datafile
           - Corrected: calculation of individual name widths referred to wrong individuals
           - Corrected: width of individuals in Overview now width of name + 2 spaces because
             of change mentioned on 14-02-2005
2005-03-11 - Corrected: bug assigning colorcodes>4
2005-03-12 - Color codes > 3 are by default clGray instead of clBlack
           - if length LG==0, the IBDprob rectangle is filled from top to bottom with the
             probs at the one position, and the locus line is drawn in the middle
           - Corrected: with Ploidy>2, locus lines were not drawn for par>1
2005-04-11 - RenameView: check for duplicate view names (not allowed)
           - Added: Adding all Views (except Overview) from one (sub-)population
             to another
2005-04-12 - in TViewOptions::MakeBuiltinOpt, now colors are defined for code 0-9
2005-04-15 - corrected: error in Export sorted data (if AllIx < 0)
           - changed: OpenDialog->InitialDir now remains where the dialog was last time
           - OptionsForm: no border icons, but style still resizable (because of possible
             problems with large screen fonts). Other dialog screens also either border
             style bsDialog or Resizable, and border icons disabled
           - OptionsForm: in Individuals when clicking on the contents radiobuttons
             the appropriate page is selected automatically
2005-04-18 - version A with ExpiryDate 2005-12-31 (for Hidras partners),
             version B only reads datafiles on 2005-04-18 and -19 (for course)
2005-04-21 - ExpiryData 2005-09-01 (a.o. for Enza)
2005-04-23 - Added check for ProjectSaved (all relevant actions, ClearData, SaveProject)
2005-04-25 - New Export data: FlexQTL compatible, for full or sub-populations
2005-04-26 - bug in ExportData (pedigree) corrected
2005-05-18 - Added: relate IBD allele colors to founder name and homolog
             (done in ParseUnit-ReadDataFile, used in OptionsUnit-AlleleCombxDrawItem)
2005-06-01 - Added: for each (sub-)population the current view is stored in the
             project file; files without this info remain legible
2005-06-10 - Added: selection of markers to show for Most Probable IBD allele
             and to show as lines in IBD probability rectangles
2005-06-16 - Added: LegendForm
2005-06-20 - Corrected: memory leak SelIBDmrk (selection of IBD markers to show)
2005-06-21 - LegendForm AlwaysOnTop
           - in MainForm->MainTabctrlChange all LegendForm tabsheets made invisible
             to prevent errors while tabsheets don't match current View
           - Added: OptionsForm always shows IndivSheet the first time, and when
             re-opened will revert to IndivSheet if the last shown page is not
             appropriate to current IndivContents
2005-06-23 - Corrected: attempt to focus invisible control in OptionsForm->FormShow
           - Corrected: access error occurring if NALLELES>0 but no IBD probabilities
2005-06-29 - Corrected: File Exit now has the "x" as hotkey
2005-07-02 - Added: On entering marker selection listbox in IBDmarkerForm,
             the Show Selected markers radiobutton is automatically checked
2005-07-04 - Added check for presence of IBD probabilities in each linkage group
           - Added MakeIndex in DataUnit, but not yet used in ParseUnit
2005-08-04 - IBDalleleIndex now filled correctly and used in IBD color combobox
             and Legend: IBD allele colors now shown ordered by founder name
             instead of in the data file order; in options dialog and legend
2005-08-09 - Added: markers can now be ordered by name or by position, in options
             dialog, charts and legend, using LocusList
2005-08-17 - ExpiryDate changed to 31-12-2005 (was 01-09-2005)
2005-11-09 - Added: alphabetic sorting of subnodes of selected node
2005-11-15 - Added: sorting subnodes into original order
           - Corrected: PopUp menu of PopTreeView worked on previously selected
             node, not on the node where the right-click was.
             Now, first the node is selected and then the popupmenu called.
2005-11-16 - Improved: PopTreeView OnClick replaced by OnChange; this is now
             implicitly called when the selected node is changed, so all
             explicit calls are deleted.
           - Added: show filename in MainForm caption
2005-11-23 - Corrected: det LG to -1 instead of 0 in TViewOptions::MakeBuiltinOpt
             (caused crash in saving data without LG's)
2005-11-30 - Added: Flags (PopTreeView, InfoForm, MainForm->FlagImageList)
2006-01-30 - dec and jan: various fixes in reading Viewpopdata in case of
             non-existing individuals
2006-03-15 - ExpiryDate changed from 1-4-06 to 1-1-07
           - ExportData adapted to avoid exporting semi-founders
           - ExportData adapted to export an additional dummy locus for linkage
             groups with only one true locus
2006-03-19 - Manual updated "version 20 March 2006"
           - DefaultExt in SaveDialog set and reset in different uses.
2006-05-26 - Added RegistryUnit; filetype *.pmp registered
           - Read filename from commandline (needed when double-clicking on *.pmp)
2006-05-28 - Icon added
2006-06-16 - Added recent files in File menu; stored in Registry
2006-06-18 - simpler icon Pm
2006-11-30 - ExpiryDate set to 1-Oct-2007
           - TViewPop::CopyIndivs: corrected error at end
           - Added and changed some comments
2007-01-10 - Read datafile line by line instead of as StringList->ReadFromFile
             (new InTextfileUnit)
2007-01-13 - added CleanFilename to avoid problems when exporting charts
             in population or view name contains period or other special characters
           - in OptionsForm a wrong page was made active in FormToOptions; solved
2007-01-17 - IBD probabilities saved in and read from binary file
           - error corrected if datafile in other directory than project file
2007-01-26 - Reading from new format datafile
           - ALLELES and IBDPOSITION sections saved in /read from binary
             file (*.pmb, file version 10001)
2007-01-27 - Show progress as labels when reading from binary file
           - Show hourglass cursor during parsing
2007-02-07 - Corrected messages on OpenProgressForm
2007-06-17 - InfoMemo shows top instead of bottom lines
           - InfoMemo shows more information for cross under cursor
           - InfoMemo shows Selected, Parents and Traits for Indiv under cursor,
             customizable through Options-Info dialog. InfoSelected, InfoParents
             and InfoTrait added to TViewOptions
2007-06-19 - corrected: Selected and Focused individuals now stay the same
             after sorting the list
2007-06-21 - After sorting, focused Individual is scrolled into view (MakeVisible)
2007-07-05 - In Export subpopulation: listed names of exported semi-founders
2007-10-04 - ExpiryDate set to 31-Dec-2008
2008-04-06 - Set PopTreeView->Multiselect to true to allow combination of
             populations
           - Adapted PopDeleteExecute to delete all selected populations
           - Added PopCombinePops and item in Population popup menu
           - ExpiryDate set to 2009-12-31
2009-01-14 - ExpiryDate set to 2010-12-31
2009-12-16 - When sorting without a focused individual an out of bounds error
             was caused by MakeVisible (see 21-06-2007); solved
           - ExpiryDate set to 2011-12-31
           - if the first keyword ALLELES in the datafile was not uppercase
             it was not recognized; solved
2010-01-06 - Improved code for assigning marker code colors in
             TViewOptions::MakeBuiltinOptions and ReadFromStringlist solving
             a small bug evident when replacing a datafile with no/few colorcodes
             with one with more colorcodes.
2010-01-23 - Fixed bug: when incorrect data file was read, the binary (pmb) file
             was created but not destroyed; so next time same (wrong) data file
             was read the binary file was used. Solved by FileReadError in
             ReadDataFile.
2010-01-24 - Apart from Parent1/Parent2 als0 Male/Female of Father/Mother
             (or v.v.) are allowed (preparation for tracing pedigree along
             female/male/both lines)
2010-01-30 - Corrected an error in FillerUp
2010-02-03 - columns for Parent1 and Parent2 can now also have captions
             Female/Male or Mother/Father (or v.v.); stored in FemaleParent
           - Selection of relatives now offers possibility to trace pedigree
             only along female or male line (parent1/parent2 line) or (default,
             as earlier) along both lines
           - Corrected: symbols for selfing, DH and VP were drawn with white
             background instead of page color
2010-02-08 - Corrected: error in CombineSubpopulations (in TViewpop::AddVPInd)
2010-02-10 - Corrected: set Selected to false for all individuals
             in TViewpop::AddVPInd
           - Corrected: line color to female/male parent by default red/blue
           - Captions of MainListview now Female/Male if specified in
             data file  
2010-02-11 - ParentCaptions in TMainForm::ExportData are adapted to the
             Female/Male situation
2010-02-14 - Allow to have no markers selected for showing IBD
           - Unselected markers in IBD probability view are now indicated as
             small line part ouside probability rectangle (used to be completely
             suppressed)
           - On TIBDMarkerForm, changed LG edit to gray, deleted the show all /
             show selected radiobuttons and added Check all / clear all buttons
           - Function of TViewOptions::AllIBDmrk is somewhat changed: it now
             is a shortcut for the situation where all markers are selected,
             rather than a toggle between showing the selected or all markers
           - Project name set in caption in TMainForm::DoFileOpen
2010-03-16 - Parsing of license file added
2010-03-17 - AboutBox added
2010-03-18 - Read and write only 2 subpopulations if unlicensed
             TODO: name of types (eg LicenseInfo) visible in exe file
2010-03-19 - Solved: files open in Excel could not be read, because
             FileOpen(FName,0x0) failed for such files.
             Now in ParseUnit - ReadDatafile we only check if FileExists
           - ExpiryDate discarded (now replaced by license handling);
             no checking if datafile is more recent than ExpiryDate and
             no program termination if past ExpiryDate any more
2010-03-20 - Suppress image generation and frame drawing for (sub)populations
             with more than IndivDrawLimit individuals
           - Added command line option /DL=<number> to set IndivDrawLimit
2010-03-21 - Removed EMFeditUnit from project (called in TView::DrawChartMF):
             did not seem to work as intended, and Pedimap doesn't print
2010-03-24 - Corrected: since 20-03-2010 selection rectangles were not redrawn
           - DrawIndFrames with double line width, to make selection more
             pronounced
2010-04-12 - Converted website label in AboutBox to active link
           - Hiding names of types (TLicenseInfo) and fields (AboutBox.Website1Lbl)
             in exe: unchecking Project Options - Linker - Create debug info has
             no effect -> checked again; same with Project Options - Compiler -
             Debug information
2010-04-16 - Deleted all strings "Parent1" etc and replaced by function
             ParentCaption(parent)
2010-04-19 - Caption of InfoForm changed to Info
2010-04-20 - In ViewOptions constructor call MakeBuiltinOptions before trying
             to read Options.ini
           - Caption of DefaultsForm changed to Default Options
           - Help - Manual added
           - Pedimap 1.0 released on website
2010-04-26 - The symbols for missing data in the export file are now mv_parent (0),
             mv_trait(-) and mv_marker(-) for compatibility with FlexQTL.
             The default strUnknown is now mv_trait (-) instead of *
2010-04-28 - Crash due to missing data solved. Solutions based on break statement
             worked erratically, workaround with boolean.
           - Error message if too many data on line in pedigree list  
2010-05-01 - Corrected occasional errors due to non-null pointers after clearing
             up a failed import, by setting levels and labels of new IndivData
             to null and Datatype to dtI
2010-06-04 - Changed export datafile to tab-delimited (easier to load in Excel)
           - Changed missing value symbol for parents in export file (mv_parent)
             to - instead of 0
           - Allow (again) comments after last data field in pedigree list
             (broken on 2010-04-28)
           - Added reference to semi-founder list to the warnings about exporting
             semi-founders
           - Manual updated
2010-06-22 - Fixed: rewrite pmb file if data file is more recent
             (was broken on 2010-03-19 when allowing to read files blocked by
             Excel)
           - Improved speed of display IndivList by setting
             MainListview->Visible=false at start of ShowIndivList, and to true
             at end
2010-06-23 - Fixed new bug: automatic selection of relatives did not work if done
             with IndivList; caused by setting MainListview->Visible to false
             at start of ShowIndivList.
             Solved using global variable MainListviewVisible
2010-08-02 - Added: in case of circular pedigree references the offending
             individuals are exported to a file.
           - Changed current version to 1.1 and development period to 2004-2010
2011-01-17 - start adding functionality to fill in missing semi-founder info:
2011-01-18 - added strConfirmedNull (default $$, keyword NULL)
2011-01-28 - added rudimentary commandline mode for demo to Rosbreed
2011-02-03 - keyword=CONFIRMEDNULL, default strConfirmedNull=null
           - Unit renamed to ImputeSFparentAlleles
2011-02-04 - SaveFile: added .pmp extension to filename in SaveDialog if
             no prior projectname available.
           - Separate action AddSFparents: only adds the individuals, no marker
             alleles imputed
2011-02-06 - Version set to 1.2, copyright 2004-2011
           - Added checks for impossible inheritance when imputing semifounder
             parent alleles
           - Export a "Pedimap imputation.log" file
2011-02-07 - In imputing founder alleles we allow that one allele is known and
             the other unknown. Also we recognize $ $ as null null and treat
             $ - as an error
2011-02-11 - Corrected some bugs in imputing founder alleles
           - changed the prefix for new semifounder parents to sfM_, sfF_ or sfP_
2011-02-13 - Added /TRAIT= option for commandline mode
2011-02-14 - Special license file for Software = Pedimap-Commandline (also valid
             for GUI, but normal license not valid for commandline version)
           - Added generation of image map coordinates in commandline mode
             (parameter coord in TView::DrawChartMF), exported in log file
2011-02-15 - Added TViewOptions::SetDefTraitColors and used this for setting
             colors for the selected trait in Commandline processing
2011-02-24 - Added: set color code for imputed semifounder parent alleles
2011-02-25 - Improved Impute founder alleles dialog
           - Manual updated

To correct:
- projectinfo sheet met o.a. datafile (naam, datum, tijd) en opmerkingen
- info per subpop blijvend open (zoals legend), aanpassend aan geselecteerd subpop;
  met knop in toolbar, niet meer als contextmenu item
  (of: als extra tabsheet?)
- IndivList op zelfde positie laten staan na maken selectie
- nieuwe optie: breedte individu bepalen op naar keuze langste naam in totale populatie
  of in current ViewPop
- Export: dialoog wat: Traits (welke), IBD (welke LG), merkerscores (welke merkers), ...
- ivm vorige: apart Select Markers sheet, te gebruiken voor ViewOptions en Export dialoog



Wish list:
- QTLs (probability of + allele, only bi-allelic qtls)
  (might be done by adding up the probabilities of the corresponding founder alleles
   at the qtl position, so only a list of the corresponding alleles is needed as input)
- IBD probabilities for specific loci as alternative for whole linkage groups
  (and selection of these loci as described above): horizontal rectangle per locus
- handling of group "unmapped": IBD probs not shown for whole group.
suggesties Jos Heldens, Enza, 12-4-2005:
- continu variable trait toch in klasses kleuren
- per generatie de individuen sorteren op trait value
- (ouder)lijnen naar keuze volledig weergeven of vanaf bep. generatie als 1 genotype
  (startgeneratie en naam ergens specificeren)
- verschillende waargenomen allelen als kleurblokjes ipv naam/nummer
Verder:
- Manual: onderscheid discrete / continue traits beschrijven, zie Jos mail
*/

//---------------------------------------------------------------------------

#include <vcl.h>
#include "Main.h" //added code to run in commandline mode
#include "DataUnit.h" //added code to run in commandline mode
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("FontUnit.cpp", FontForm);
USEFORM("OptionsUnit.cpp", OptionsForm);
USEFORM("SelectDlgUnit.cpp", SelectDlgForm);
USEFORM("Main.cpp", MainForm);
USEFORM("InfoUnit.cpp", InfoForm);
USEFORM("ContinuousColorUnit.cpp", ContinuousColorForm);
USEFORM("DiscreteColorUnit.cpp", DiscreteColorForm);
USEFORM("DefaultsUnit.cpp", DefaultsForm);
USEFORM("OpenProgressUnit.cpp", OpenProgressForm);
USEFORM("RenameUnit.cpp", RenameForm);
USEFORM("IBDMarkerUnit.cpp", IBDMarkerForm);
USEFORM("LegendUnit.cpp", LegendForm);
USEFORM("AboutboxUnit.cpp", AboutboxForm);
USEFORM("ImputeSFparentAllelesUnit.cpp", ImputeSFparentAllelesForm);
USEFORM("AddSFparentsUnit.cpp", AddSFparentsForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "Pedimap";

   //*********************************************************************
   //added code to run in commandline mode without starting GUI
   //Application->CreateForm(__classid(TMainForm), &MainForm); //needed for the moment even in CommandlineMode
   AnsiString s = GetDataFromCommandline().Trim();
   if (s!="") {
     CommandlineModeInt = 1;
     CommandlineProcessing(s);
   }
   else {
     CommandlineModeInt = 2;

   //end of added code
   //*********************************************************************

   Application->CreateForm(__classid(TMainForm), &MainForm);
   Application->CreateForm(__classid(TFontForm), &FontForm);
   Application->CreateForm(__classid(TOptionsForm), &OptionsForm);
   Application->CreateForm(__classid(TSelectDlgForm), &SelectDlgForm);
   Application->CreateForm(__classid(TInfoForm), &InfoForm);
   Application->CreateForm(__classid(TContinuousColorForm), &ContinuousColorForm);
   Application->CreateForm(__classid(TDiscreteColorForm), &DiscreteColorForm);
   Application->CreateForm(__classid(TDefaultsForm), &DefaultsForm);
   Application->CreateForm(__classid(TOpenProgressForm), &OpenProgressForm);
   Application->CreateForm(__classid(TRenameForm), &RenameForm);
   Application->CreateForm(__classid(TIBDMarkerForm), &IBDMarkerForm);
   Application->CreateForm(__classid(TLegendForm), &LegendForm);
   Application->CreateForm(__classid(TAboutboxForm), &AboutboxForm);
   Application->CreateForm(__classid(TImputeSFparentAllelesForm), &ImputeSFparentAllelesForm);
   Application->CreateForm(__classid(TAddSFparentsForm), &AddSFparentsForm);
   Application->Run();
   }  //added closing bracket
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        catch (...)
        {
                 try
                 {
                         throw Exception("");
                 }
                 catch (Exception &exception)
                 {
                         Application->ShowException(&exception);
                 }
        }
   return 0;
}
//---------------------------------------------------------------------------
