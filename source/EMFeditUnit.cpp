//---------------------------------------------------------------------------


#pragma hdrstop

#include "EMFeditUnit.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

// rdRand.cpp -- 030325 (sjd)
//    Reading data directly from a random-access file

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

using namespace std;


int EditEMFfile(char* fname, int MFwidthMM, int MFheightMM, int PPI)
{  long int w;
   ifstream inEMF( fname, ios::in | ios::binary );
   ofstream outEMF( fname, ios::out | ios::in | ios::binary );
       // open file -- note ios::in needed to prevent file truncation
   if ( !inEMF || !outEMF )
   {  ShowMessage("File could not be opened." );
      exit( 1 );
   }

   //adjust metafile size (1/100 mm):

   //w:=PaperDimension(Map.PgSettings,Horz)-Map.PgSettings.PgMargMM[sLeft]-Map.PgSettings.PgMargMM[sRight];
   //seek(LW,8); write(LW,w); //new metafile width in 1/100 mm
   w=MFwidthMM;
   outEMF.seekp( 8*4 ); //position of item * SizeOf(w) : new metafile width in 1/100 mm
   outEMF.write( reinterpret_cast<const char *>( &w ), 4 ); //4 = SizeOf(w)

   //w:=PaperDimension(Map.PgSettings,Vert)-Map.PgSettings.PgMargMM[sTop]-Map.PgSettings.PgMargMM[sBottom];
   //seek(LW,9); write(LW,w); //new metafile height in 1/100 mm
   w=MFheightMM;
   outEMF.seekp( 9*4 ); //position of item * SizeOf(w) : new metafile height in 1/100 mm
   outEMF.write( reinterpret_cast<const char *>( &w ), 4 ); //4 = SizeOf(w)

   //adjust device size (pixels):

   //seek(LW,20); read(LW,w); //device width in mm
   inEMF.seekg( 20*4 ); //device width in mm
   inEMF.read( reinterpret_cast<char *>( &w ), 4 ); //4 = SizeOf(w)
   //w:=round(100 * w * PixPermm100[Vert]);
   w=0.5+(w * PPI/25.4); //new device width in pixels
   //seek(LW,18); write(LW,w); //new device width in pixels
   outEMF.seekp( 18*4 );
   outEMF.write( reinterpret_cast<const char *>( &w ), 4 ); //4 = SizeOf(w)

   //seek(LW,21); read(LW,w); //device height in mm
   inEMF.seekg( 21*4 ); //device height in mm
   inEMF.read( reinterpret_cast<char *>( &w ), 4 ); //4 = SizeOf(w)
   //w:=round(100 * w * PixPermm100[Vert]);
   w=0.5+(w * PPI/25.4); //new device height in pixels
   //seek(LW,19); write(LW,w); //new device height in pixels
   outEMF.seekp( 19*4 );
   outEMF.write( reinterpret_cast<const char *>( &w ), 4 ); //4 = SizeOf(w)

   outEMF.close();
   inEMF.close();
   return 0;
}  //EditEMFfile


