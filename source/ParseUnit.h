#include "PCH.h"
//#include <vcl.h>
#pragma hdrstop

#ifndef ParseUnitH
#define ParseUnitH
//---------------------------------------------------------------------------

void ReadWoord (int &p, AnsiString &Woord, const AnsiString &Line);
// AnsiString Strip (const AnsiString *S); = AnsiString.Trim;

//void ReadPedFile ( const AnsiString &FName );

bool ReadDataFile (const AnsiString &FName);
bool IsUnknown(AnsiString s);


//---------------------------------------------------------------------------
#endif

