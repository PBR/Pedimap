//---------------------------------------------------------------------------

#ifndef InTextfileUnitH
#define InTextfileUnitH
//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------
//vanaf hier eigen code

#include<iostream>
#include<fstream>
#include<system.hpp> //for AnsiString

class TInTextfile
{ private:
    std::ifstream Stream;
  public:
    AnsiString Line,     //current line
               FileName; //as used when opening
    long int LineNr,     //0-based number of current line, -1 if unknown
             StartPos,   //start position of current line in file
             NextPos,    //start position of next line in file
             P;          //position in current line
    TInTextfile(AnsiString FN, bool &Error);
    ~TInTextfile();
    AnsiString ReadLn();
    AnsiString ReadWoord(); //within current Line, from P
    AnsiString ReadWoordQ(); //same, woord might be Quoted
    void SkipBlNComm(); //Skip Blanks & Comments (reads multiple lines if needed)
    void SkipPastChar(char *c); //within current Line, from P
    bool Eof();
    void Clear();
};
