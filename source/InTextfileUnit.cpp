//---------------------------------------------------------------------------


#pragma hdrstop

#include "InTextfileUnit.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------
//vanaf hier eigen code

/*
Convert between std::string and AnsiString
As both string types are C-compatible, they have a constructor
taking a pointer to char (a C-style string). Using these yields:
*/

std::string AnsiToString(const AnsiString& ansi)
{
  return std::string(ansi.c_str());
}

AnsiString ToAnsiString(const std::string& myString)
{
  return String(myString.c_str());
}

TInTextfile::TInTextfile(AnsiString FN, bool &Error)
{ Stream.open(FN.c_str());
  FileName=FN;
  Line=""; P=1;
  LineNr=-1;
  StartPos=-1; NextPos=-1;
  Error=!Stream; //use overloaded meaning of ! for fstream
}

TInTextfile::~TInTextfile()
{ Line="";
  Stream.close();
  Stream.clear();
}

AnsiString TInTextfile::ReadLn()
{ std::string st;

  StartPos=Stream.tellg();
  std::getline(Stream,st); //automatically assigns space for st;
                           //reads st up to (not including) \n and skips \n
  Line = ToAnsiString(st);
  st="";
  P=1;
  LineNr++;
  NextPos=Stream.tellg();
  return Line;
}

AnsiString TInTextfile::ReadWoord()
//does NOT read quoted words
{ int q,L; //AnsiString Woord;
  L=Line.Length();
  while (P<=L && Line[P]<=' ') P++;
  q=P;
  while (P<=L && Line[P]>' ') P++;
  //Woord = Line.SubString(q,P-q);
  //return Woord;
  return Line.SubString(q,P-q);
} //ReadWoord

AnsiString TInTextfile::ReadWoordQ()
//as ReadWoord, but also reads quoted words
{ int q,L;
  AnsiString Woord;
  Woord=""; L=Line.Length();
  while (P<=L && Line[P]<=' ') P++;
  if (P<=L && Line[P]=='"')
  { //read quoted word until next '"' or EOLN
    // (may contain any characters including blanks, except '"')
    q=++P;
    while (P<=L && Line[P]!='"') P++;
    Woord=Line.SubString(q,P-q);
    P++; //skip past closing '"'
  }
  else
  { //read unquoted word until next blank or EOLN (may include '"' characters)
    q=P;
    while (P<=L && Line[P]>' ') P++;
    Woord=Line.SubString(q,P-q);
  }
  return Woord;
} //ReadWoordQ

void TInTextfile::SkipBlNComm()
// when called: not in comment
// returns with r=L->Count or r,p at start first non-comment word
{ const char CommentChar = ';' ;
  int L;
  do
  { //first work through current line:
    L=Line.Length();
    while (P <= L /*Line.Length()*/ && Line[P]<=' ')
      P++;
    if (P <= L /*Line.Length()*/ && Line[P]!= CommentChar)
      break;
    else if (Eof()) //if comment but last line of file: go to eoln
    { P=Line.Length()+1; break; }
    else //read next line:
      ReadLn();
  }
  while (true);
} //SkipBlNComm  Compiler warning about CommentChar never used incorrect!

void TInTextfile::SkipPastChar(char *c)
{ while (P<=Line.Length() && Line[P]!=*c) P++;  P++; }

bool TInTextfile::Eof()
{ if (!Stream) //use overloaded meaning of ! for fstream
  return true;
  else return Stream.eof();
}

void TInTextfile::Clear()
{ Stream.clear(); }
