#include "PCH.h"
#pragma hdrstop

#include <Registry.hpp>
#include "RegistryUnit.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

bool RegistryAvailable, AskAssociate, StartMaximized;


AnsiString MRUfile[MaxMRU];

bool RegReadBooleanFromPedimapKey(AnsiString ValueName, bool def)
{ TRegistry *Reg;
  AnsiString s;
  bool oke, Result;

  Result=def; //default
  Reg = new TRegistry();
  try
  { if ( Reg->OpenKey("\Software",true) &&
         Reg->OpenKey("Pedimap",true) )
     { oke = Reg->ValueExists(ValueName);
       if (oke)
       { oke =  Reg->GetDataType(ValueName)==rdString;
         if (oke)
         { s=Reg->ReadString(ValueName);
           oke = ( s=="0" || s=="1" );
           if (oke) Result = s=="1";
         }
         if (!oke)Reg->DeleteValue(ValueName);
       }
       if (!oke) Reg->WriteString(ValueName,IntToStr(def));
            //key not found or deleted
     }
  }
  __finally
  { Reg->Free(); }
  return Result;
} //RegReadBooleanFromMPedimapKey

void RegWriteBooleanToPedimapKey(AnsiString ValueName, const bool value)
{ TRegistry *Reg;

  Reg = new TRegistry();
  try
  { if ( Reg->OpenKey("\Software",true) &&
         Reg->OpenKey("Pedimap",true) )
    { Reg->WriteString(ValueName,IntToStr(value)); }
  }
  __finally
  { Reg->Free(); }
} //RegWriteBooleanToPedimapKey

void RegisterFiletype()
{ char Quote='"';
  TRegistry *Reg;
  AnsiString s,
             LF=AnsiString('\n'),
             FExt=".pmp",          //must start with '.'
             FType="Pedimap.file"; //must be without spaces ?

  Reg = new TRegistry();
  try
  { Reg->RootKey = HKEY_CLASSES_ROOT;

    //first make sure that filetype Pedimap.file exists and points
    //to current Pedimap executable:
    if ( Reg->OpenKey(FType,true) )
    { //filetype PedimapFile existed or created
      Reg->WriteString("","Pedimap file"); //filetype description
      if ( Reg->OpenKey("shell", TRUE) )
      { //Pedimap.file\shell key existed or created
        Reg->WriteString("","open"); //set open as default action ?
        if ( Reg->OpenKey("open",true) )
        { //Pedimap.file\shell\open key existed or created
          Reg->WriteString("","&Open"); //for use in context menu ?
          if ( Reg->OpenKey("command",true) )
          { //Pedimap.file\shell\open\command key existed or created
            s="%1";
            s=AnsiQuotedStr(s,Quote);
            Reg->WriteString("",ParamStr(0)+" "+s); //set current Pedimap executable as command
            Reg->CloseKey(); //Pedimap.file\shell\open\command key
          }
          Reg->CloseKey(); //Pedimap.file\shell\open key
        }
        Reg->CloseKey(); //Pedimap.file\shell key
      }
      Reg->CloseKey(); //Pedimap.file key

      //now associate extension with filetype,
      //ask for confirmation if already associated with other filetype:
      if ( Reg->OpenKey(FExt,false) )
      { //extension already registered
        s=Reg->ReadString("");
        if ( s!=FType && AskAssociate )
        { //not registered to type Pedimap.file
          //confirm to replace the link between extension and filetype:
          switch ( MessageDlg(
            "Extension "+FExt+" is currently associated with files of type "+s+";"+LF+
            "do you want to associate it with Pedimap ?"+LF+
            "(this will cause Pedimap to open a file when you double-click it)"+LF+LF+

            "Choose 'No to All' to suppress this message in the future",
            mtConfirmation, TMsgDlgButtons() << mbYes << mbNo << mbNoToAll, 0) )
          { case mrYes : Reg->WriteString("",FType); break;
            case mrNo  : break;
            default    : AskAssociate=false; //must be written to Registry
          }
        }
        Reg->CloseKey(); //key FExt
      }
      else
      { //extension not registered, register without confirmation:
        if ( Reg->OpenKey(FExt,true) )
        { Reg->WriteString("",FType);
          Reg->CloseKey(); //key FExt
        }
      }
    } //key Pedimap.file existed or created
  }
  __finally
  { Reg->Free(); }
} //RegisterFiletype

void CheckPedimapKey()
//Creates and fills key if not present
{ int i,j;
  bool oke;
  TRegistry *Reg;
  TStringList *ValueNameList; //list of value names (!) in subkeys of Pedimap}

  RegistryAvailable=false;
  try
  {  Reg = new TRegistry();
      try
      {  if ( Reg->OpenKey("\Software",true) &&
              Reg->OpenKey("Pedimap",true) )
         { //ShowMessage('key '+Reg.CurrentPath+' ok');
           ValueNameList = new TStringList;
           try
           {  //check value names in key Pedimap:
              //AskAssociate and StartMaximized below,
              // maybe others in future}

              //if necessary create, and go to Software\Pedimap\MRUlist:
              if (Reg->OpenKey("MRUlist",true))
              { //ShowMessage('Key '+Reg.CurrentPath+' ok')
                //check all Value Names and Values in MRUlist:
                ValueNameList->Clear();
                Reg->GetValueNames(ValueNameList);
                i=1;
                while ( (i<=MaxMRU) &&
                        (i<=ValueNameList->Count) &&
                        (ValueNameList->Strings[i-1]==IntToStr(i)) &&  //value names = '1', '2' etc}
                        (Reg->GetDataType(ValueNameList->Strings[i-1])==rdString) )
                { i++; }
                //first delete invalid value names if present:
                for (j=ValueNameList->Count; j>=i; j--)
                     Reg->DeleteValue(ValueNameList->Strings[j-1]);
                //then add value names if not all available:
                for (j=i; j<=MaxMRU; j++)
                     Reg->WriteString(IntToStr(j),""); //dd empty filename
                //ShowMessage('Key '+Reg.CurrentPath+' will be closed:');
                Reg->CloseKey(); //key MRUlist
                //ShowMessage('Now '+Reg.CurrentPath+' is current path');
              } //key MRUlist ok
              //if necessary create, and go to Software\Pedimap\Folders:}
              oke=Reg->OpenKey("Folders",true);
              // no need to create or check for contents;
              // if these are not present the function RegReadFolders will return ''
              if (oke) Reg->CloseKey(); //key Folders
           }
           __finally
           { ValueNameList->Free(); }
         } //key Pedimap ok
      }
      __finally
      { Reg->Free(); }
      AskAssociate=RegReadBooleanFromPedimapKey("AskAssociate",true);
      StartMaximized=RegReadBooleanFromPedimapKey("StartMaximized",false);
      //RegReadDataFont;
      RegistryAvailable=true;
  }
  catch (...)
  {
   //ShowMessage("Pedimap: Registry not available!");
  }
  RegisterFiletype();
  RegWriteBooleanToPedimapKey("AskAssociate",AskAssociate);
} //CheckPedimapKey

void RegWriteMRUfiles()
{ int i;
  TRegistry *Reg;
  Reg = new TRegistry();
  try
  {  if ( Reg->OpenKey("\Software",true) &&
          Reg->OpenKey("Pedimap",true) &&
          Reg->OpenKey("MRUlist",true) )
     { for (i=0; i<MaxMRU; i++)
       Reg->WriteString(IntToStr(i+1),MRUfile[i]);
     }
     Reg->CloseKey();
  }
  __finally
  { Reg->Free(); }
} //RegWriteMRUfiles()

void RegReadMRUfiles()
{ int i;
  TRegistry *Reg;

  for (i=0; i<MaxMRU; i++) MRUfile[i]="";
  Reg = new TRegistry();
  try
  {  if ( Reg->OpenKey("\Software",true) &&
          Reg->OpenKey("Pedimap",true) &&
          Reg->OpenKey("MRUlist",true) )
     { for (i=0; i<MaxMRU; i++)
         MRUfile[i]=Reg->ReadString(IntToStr(i+1));
       Reg->CloseKey();
     }
  }
  __finally
  { Reg->Free(); }
} //RegReadMRUfiles
