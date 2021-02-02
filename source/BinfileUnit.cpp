//---------------------------------------------------------------------------


#pragma hdrstop

#include "BinfileUnit.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TBinaryfile::TBinaryfile(const AnsiString FN, const int OpenMode, bool &Error)
{ Stream.open(FN.c_str(), OpenMode);
  FileName=FN;
  Error=!Stream; //use overloaded meaning of ! for fstream
}

TBinaryfile::~TBinaryfile()
{ Stream.close();
  Stream.clear();
}

bool TBinaryfile::Eof()
{ if (!Stream) //use overloaded meaning of ! for fstream
  return true;
  else return Stream.eof();
}

void TBinaryfile::Clear()
{ Stream.clear(); }

void TBinaryfile::seekg(const long int p)
{ Stream.seekg(p); }

void TBinaryfile::seekp(const long int p)
{ Stream.seekp(p); }

void TBinaryfile::read (byte *B, const long int i)
{ Stream.read((char*) B, i); }

void TBinaryfile::write (const byte *B, const long int i)
{ Stream.write((char*) B, i); }

void TBinaryfile::read (signed char *B, const long int i)
{ Stream.read((char*) B, i); }

void TBinaryfile::write (const signed char *B, const long int i)
{ Stream.write((char*) B, i); }

void TBinaryfile::read (Word *W, const long int i)
{ Stream.read((char*) W, i*sizeof(Word)); }

void TBinaryfile::write (const Word *W, const long int i)
{ Stream.write((char*) W, i*sizeof(Word)); }

void TBinaryfile::read (int *I, const long int i)
{ Stream.read((char*) I, i*sizeof(int)); }

void TBinaryfile::write (const int *I, const long int i)
{ Stream.write((char*) I, i*sizeof(int)); }





























AnsiString DoBF()
{  std::fstream BF;
   byte b,e;
   int i;
   Word w;
   char *c, *d;
   AnsiString a;

   a = "hallo";
   c = "doei";
   b = 63;
   w = 65535;
   i = 0;
   BF.open("bintext.bin", /*std::ios::in | */ std::ios::out | std::ios::binary);
   if (!BF)
     a="BF not open";
   else
   { BF.write((char*) &b, 1 );
     BF.write((char*) &i, 4 );
     BF.write((char*) &w, 2 );
     BF.write(a.c_str(), a.Length() );
     BF.write(c, 4);
     BF.seekg(7); //zero-based
     d = new char[8];
     d = "xxxxxxxx";
     BF.read(d,6);
     BF.seekg(6);
     BF.read((char*) &e, 1);
     BF.close();
     i=e;
     a = AnsiString(d)+"---"+d[0]+d[6]+d[7]+d[8]+(char*)&e;
   }
   return a;
}
