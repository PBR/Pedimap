#include "PCH.h"
//---------------------------------------------------------------------------

#ifndef RegistryUnitH
#define RegistryUnitH
//---------------------------------------------------------------------------
#endif


const MaxMRU=5;

extern AnsiString MRUfile[MaxMRU];

void CheckPedimapKey();
//Creates and fills key if not present
void RegWriteMRUfiles();
void RegReadMRUfiles();

