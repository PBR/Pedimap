#include "PCH.h"
//#include <vcl.h>
#pragma hdrstop

#include "ContinuousColorUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
#include "DataUnit.h"

TContinuousColorForm *ContinuousColorForm;
//---------------------------------------------------------------------------
__fastcall TContinuousColorForm::TContinuousColorForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TContinuousColorForm::PaintBox1Paint(TObject *Sender)
{ int x,y;
  TColor Col;
  //draw outline:
  PaintBox1->Canvas->Pen->Color = clBlack;
  PaintBox1->Canvas->Brush->Style = bsClear;
  PaintBox1->Canvas->Rectangle(0,0,PaintBox1->Width,PaintBox1->Height);
  //fill:
  for (y=1; y<PaintBox1->Height; y++)
  {  Col = ColorRange(LoCol,HiCol,float(PaintBox1->Height-1-y )/(PaintBox1->Height-2));
     for (x=1; x<PaintBox1->Width; x++) PaintBox1->Canvas->Pixels[x][y]=Col;
  }
}
//---------------------------------------------------------------------------
void __fastcall TContinuousColorForm::FormShow(TObject *Sender)
{ PaintBox1Paint(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TContinuousColorForm::HiColbtnClick(TObject *Sender)
{ ColorDialog1->Color=HiCol;
  if (ColorDialog1->Execute())
  { HiCol=ColorDialog1->Color;
    PaintBox1Paint(NULL);
  }
}
//---------------------------------------------------------------------------
void __fastcall TContinuousColorForm::LowColbtnClick(TObject *Sender)
{ ColorDialog1->Color=LoCol;
  if (ColorDialog1->Execute())
  { LoCol=ColorDialog1->Color;
    PaintBox1Paint(NULL);
  }
}
//---------------------------------------------------------------------------
void __fastcall TContinuousColorForm::MVColbtnClick(TObject *Sender)
{ ColorDialog1->Color=MVCol;
  if (ColorDialog1->Execute())
  { MVCol=ColorDialog1->Color;
  }
}
//---------------------------------------------------------------------------
