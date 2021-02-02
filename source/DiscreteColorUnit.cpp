#include "PCH.h"
//#include <vcl.h>
#pragma hdrstop

#include "DiscreteColorUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TDiscreteColorForm *DiscreteColorForm;
//---------------------------------------------------------------------------
__fastcall TDiscreteColorForm::TDiscreteColorForm(TComponent* Owner)
        : TForm(Owner)
{ Col=NULL; ColCount=0; Labels=NULL;
}
//---------------------------------------------------------------------------

void ColListbxDrawItem(TListBox *Combx, TColor Col, AnsiString Text,
  TRect Rect, TOwnerDrawState State)
{
   Combx->Canvas->Pen->Color=clWhite;
   Combx->Canvas->Pen->Style=psSolid;
   Combx->Canvas->Brush->Color=clWhite;
   Combx->Canvas->Brush->Style=bsSolid;
   Combx->Canvas->Rectangle(Rect.Left,Rect.Top,Rect.Right,Rect.Bottom);
   Combx->Canvas->Pen->Color=clBlack;
   Combx->Canvas->Brush->Color=Col; //Opt.ACol[Index];
   Combx->Canvas->Rectangle(Rect.Left+2,Rect.Top+2,Rect.Left+22,Rect.Bottom-2);
   Combx->Canvas->Brush->Color=clWhite;
   Combx->Canvas->Font->Color=clBlack;
   Combx->Canvas->Font->Height=Rect.Bottom-Rect.Top; //Height fits in Rect.Height
   Combx->Canvas->TextOut(Rect.Left+25,Rect.Top,Text); //IntToStr(Index));
} //ColListbxDrawItem


void __fastcall TDiscreteColorForm::ColorListboxDrawItem(
      TWinControl *Control, int Index, TRect &Rect, TOwnerDrawState State)
{ if (Index==0)
    ColListbxDrawItem((TListBox*)(Control),MVCol,
      "(missing value)", Rect,State);
  else
    ColListbxDrawItem((TListBox*)(Control),Col[Index-1],Labels[Index-1],
                     Rect,State);
}
//---------------------------------------------------------------------------

void __fastcall TDiscreteColorForm::FormShow(TObject *Sender)
{ int i;
  DataType=IndivData[InDat].DataType;
  TypeIndex=IndivData[InDat].TypeIndex;
  //set Labels:
  delete[] Labels;
  Labels = new AnsiString[ColCount];
  if (DataType==dtC) //char: single character, eg 0..9
  { for (i=0; i<ColCount; i++)
      Labels[i] = AnsiString(IndivData[InDat].V.CV.Levels[i]);
  }
  else //DataType==dtS : String
  { for (i=0; i<ColCount; i++)
      Labels[i] = IndivData[InDat].V.SV.Labels[i];
  }
  //fill ColorListbox:
  ColorListbox->Items->Clear();
  for (i=0; i<=ColCount; i++) //including 0 = missing value color
    ColorListbox->Items->Add("");
  ColorListbox->ItemIndex=0;
  ColorListbox->Refresh();
  OKbtn->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TDiscreteColorForm::FormDestroy(TObject *Sender)
{  delete[] Col; delete[] Labels; }
//---------------------------------------------------------------------------

void __fastcall TDiscreteColorForm::CurrentbtnClick(TObject *Sender)
{ if (ColorListbox->ItemIndex<0 ||
      ColorListbox->ItemIndex>=ColorListbox->Items->Count)
    return;
  if (ColorListbox->ItemIndex==0)
  { ColorDialog1->Color=MVCol;
    if (ColorDialog1->Execute())
    { MVCol=ColorDialog1->Color;
      ColorListbox->Refresh();
    }
  }
  else
  { ColorDialog1->Color=Col[ColorListbox->ItemIndex-1];
    if (ColorDialog1->Execute())
    { Col[ColorListbox->ItemIndex-1]=ColorDialog1->Color;
      ColorListbox->Refresh();
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TDiscreteColorForm::AllbtnClick(TObject *Sender)
{ TColor LoCol, HiCol;
  LoCol=Col[0];
  HiCol=Col[ColCount-1];
  FillColors(ColCount,Col,LoCol,HiCol,InDat);
}
//---------------------------------------------------------------------------

