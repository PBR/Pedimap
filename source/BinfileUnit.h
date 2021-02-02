//---------------------------------------------------------------------------

#ifndef BinfileUnitH
#define BinfileUnitH
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
//vanaf hier eigen code
#include<iostream>
#include<fstream>
#include<system.hpp> //for AnsiString

class TBinaryfile
{ private:
    std::fstream Stream;
  public:
    AnsiString FileName; //as used when opening
    TBinaryfile(const AnsiString FN, const int OpenMode, bool &Error);
    ~TBinaryfile();
    void seekg(const long int p);
    void seekp(const long int p);
    bool Eof();
    void Clear();
    void read (byte *B, const long int i);
    void read (signed char *B, const long int i);
    void read (Word *W, const long int i);
    void read (int *I, const long int i);
    /*void read (char *C, const long int i);
    void read (AnsiString &S, const long int i); */
    void write (const byte *B, const long int i);
    void write (const signed char *B, const long int i);
    void write (const Word *W, const long int i);
    void write (const int *I, const long int i);
};
