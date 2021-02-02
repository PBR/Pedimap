#include "PCH.h"
//#include <vcl.h>
#pragma hdrstop

#include "LegendUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TLegendForm *LegendForm;
//---------------------------------------------------------------------------
__fastcall TLegendForm::TLegendForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void ColorDrawItem(TListBox *ListBx, TColor Col, AnsiString Text,
  TRect Rect, TOwnerDrawState State)
{
   ListBx->Canvas->Pen->Color=clWhite;
   ListBx->Canvas->Pen->Style=psSolid;
   ListBx->Canvas->Brush->Color=clWhite;
   ListBx->Canvas->Brush->Style=bsSolid;
   ListBx->Canvas->Rectangle(Rect.Left,Rect.Top,Rect.Right,Rect.Bottom);
   ListBx->Canvas->Pen->Color=clBlack;
   ListBx->Canvas->Brush->Color=Col;
   ListBx->Canvas->Rectangle(Rect.Left+2,Rect.Top+2,Rect.Left+22,Rect.Bottom-2);
   ListBx->Canvas->Brush->Color=clWhite;
   ListBx->Canvas->Font->Color=clBlack;
   //ListBx->Canvas->Font->Height=Rect.Bottom-Rect.Top; //Height fits in Rect.Height
   ListBx->Canvas->TextOut(Rect.Left+25,Rect.Top,Text); //IntToStr(Index));
} //ColorDrawItem

void __fastcall TLegendForm::IBDColorListboxDrawItem(TWinControl *Control,
      int Index, TRect &Rect, TOwnerDrawState State)
{ TView *VW;

  if (IBDColSheet->TabVisible)
  { VW = CurrentViewPop->VW[CurrentViewPop->CurrVW];
    if (IBDalleleFounder==NULL)
       ColorDrawItem(((TListBox*)(Control)), VW->Opt.ACol[Index],
           IntToStr(Index), Rect,State);
    /*else if (IBDalleleFounder[Index]<0)
       ColorDrawItem(((TListBox*)(Control)),VW->Opt.ACol[Index],
           IntToStr(Index)+" no founder", Rect,State);
    else
       ColorDrawItem(((TListBox*)(Control)),VW->Opt.ACol[Index],
           IntToStr(Index)+" "+Indiv[IBDalleleFounder[Index]].IName+"-"+IntToStr(IBDalleleFounderHom[Index]+1),
                     Rect,State);   */
    else if (IBDalleleFounder[IBDalleleIndex[Index]]<0)
       ColorDrawItem((TListBox*)(Control),VW->Opt.ACol[IBDalleleIndex[Index]],
           "("+IntToStr(IBDalleleIndex[Index])+": no founder)",
           Rect,State);
    else
       ColorDrawItem(
           (TListBox*)(Control),
           VW->Opt.ACol[IBDalleleIndex[Index]],
           //IntToStr(Index)+" "+
             Indiv[IBDalleleFounder[IBDalleleIndex[Index]]].IName+"-"+
             IntToStr(IBDalleleFounderHom[IBDalleleIndex[Index]]+1),
           Rect,State);
  }
} //TLegendForm::IBDColorListboxDrawItem
//---------------------------------------------------------------------------

void __fastcall TLegendForm::DiscrColListboxDrawItem(TWinControl *Control,
      int Index, TRect &Rect, TOwnerDrawState State)
{ TView *VW;

  if (DiscreteTraitColSheet->TabVisible)
  { VW = CurrentViewPop->VW[CurrentViewPop->CurrVW];
    if (Index==0)
      ColorDrawItem((TListBox*)(Control),VW->Opt.MVCol,
        "(missing value)", Rect, State);
    else
    { if (IndivData[VW->Opt.ColorTrait].DataType==dtC)
        ColorDrawItem((TListBox*)(Control),VW->Opt.TraitCol[Index-1],
                    AnsiString(IndivData[VW->Opt.ColorTrait].V.CV.Levels[Index-1]),
                    Rect, State);
      else //DataType==dtS
        ColorDrawItem((TListBox*)(Control),VW->Opt.TraitCol[Index-1],
                    IndivData[VW->Opt.ColorTrait].V.SV.Labels[Index-1],
                    Rect, State);
    }
  }
} //TLegendForm::DiscrColListboxDrawItem
//---------------------------------------------------------------------------

void __fastcall TLegendForm::LGmapLboxDrawItem(TWinControl *Control,
      int Index, TRect &Rect, TOwnerDrawState State)
{ AnsiString s; TView *VW; int loc;

  if (LGMapSheet->TabVisible)
  { LGmapLbox->Canvas->Brush->Color= clWhite;
    LGmapLbox->Canvas->FillRect(Rect); //to clear any Selected blue color
      //the "Selection rectangle" seems to be drawn after DrawItem,
      //so it cannot be removed using a larger FillRect

    VW=CurrentViewPop->VW[CurrentViewPop->CurrVW];
    if (LGstart)
    { loc=Index-1;
      if (Index==0)
      { LGmapLbox->Canvas->Font->Color=clGray;
        LGmapLbox->Canvas->TextOut(Rect.Left,Rect.Top,"(start)");
        s=FloatToStrF(LinkGrp[VW->Opt.LG]->IBDpos[0]->cM,ffFixed,8,1);
        LGmapLbox->Canvas->TextOut(Rect.Left+LocnameWidth+PositionWidth-LGmapLbox->Canvas->TextWidth(s),
           Rect.Top,s);
      }
    }
    else loc=Index;
    if (loc>-1)
    { if (loc>=LinkGrp[VW->Opt.LG]->LocCount)
      { LGmapLbox->Canvas->Font->Color=clGray;
        LGmapLbox->Canvas->TextOut(Rect.Left,Rect.Top,"(end)");
        s=FloatToStrF(LinkGrp[VW->Opt.LG]->IBDpos[LinkGrp[VW->Opt.LG]->IBDCount-1]->cM,ffFixed,8,1);
        LGmapLbox->Canvas->TextOut(Rect.Left+LocnameWidth+PositionWidth-LGmapLbox->Canvas->TextWidth(s),
           Rect.Top,s);
      }
      else
      { if ( VW->Opt.SelIBDmrk==NULL || VW->Opt.SelIBDmrk[Index] )
        LGmapLbox->Canvas->Font->Color=clBlack;
        else LGmapLbox->Canvas->Font->Color=clGray;
        LGmapLbox->Canvas->TextOut(Rect.Left,Rect.Top,LinkGrp[VW->Opt.LG]->Locus[Index]->LocName);
        s=FloatToStrF(LinkGrp[VW->Opt.LG]->Locus[Index]->cM,ffFixed,8,1);
        LGmapLbox->Canvas->TextOut(Rect.Left+LocnameWidth+PositionWidth+5-LGmapLbox->Canvas->TextWidth(s),
           Rect.Top,s);
      }
    }
  }
} //TLegendForm::LGmapLboxDrawItem
//---------------------------------------------------------------------------

void __fastcall TLegendForm::MarkerLboxDrawItem(TWinControl *Control,
      int Index, TRect &Rect, TOwnerDrawState State)
{ TLocus *Loc; AnsiString s;

  if (MarkerSheet->TabVisible)
  { Loc = GetLocusByNr(CurrentViewPop->VW[CurrentViewPop->CurrVW]->Opt.MarkSelected[Index]);
    MarkerLbox->Canvas->Brush->Color= clWhite;
    MarkerLbox->Canvas->FillRect(Rect); //to clear any Selected blue color
      //the "Selection rectangle" seems to be drawn after DrawItem,
      //so it cannot be removed using a larger FillRect
    MarkerLbox->Canvas->Font->Color =  clBlack; // is white if Selected
    MarkerLbox->Canvas->TextOut(Rect.Left,Rect.Top,Loc->LocName);
    MarkerLbox->Canvas->TextOut(Rect.Left+LocnameWidth+5,Rect.Top,LinkGrp[Loc->LGnr]->LGName);
    s=FloatToStrF(Loc->cM,ffFixed,8,1);
    MarkerLbox->Canvas->TextOut(Rect.Left+LocnameWidth+LGnameWidth+PositionWidth+10 -
      MarkerLbox->Canvas->TextWidth(s),Rect.Top,s);
  }
} //TLegendForm::MarkerLboxDrawItem
//---------------------------------------------------------------------------

void __fastcall TLegendForm::ContColPaintBoxPaint(TObject *Sender)
{ int x,y;
  TColor Col;

  if (ContinuousTraitColSheet->TabVisible)
  { //draw outline:
    ContColPaintBox->Canvas->Pen->Color = clBlack;
    ContColPaintBox->Canvas->Brush->Style = bsClear;
    ContColPaintBox->Canvas->Rectangle(0,0,ContColPaintBox->Width,ContColPaintBox->Height);
    //fill:
    for (y=1; y<ContColPaintBox->Height-1; y++)
    {  Col = ColorRange(LoCol,HiCol,float(ContColPaintBox->Height-1-y )/(ContColPaintBox->Height-2));
       for (x=1; x<ContColPaintBox->Width-1; x++) ContColPaintBox->Canvas->Pixels[x][y]=Col;
    }
  }
} //TLegendForm::ContColPaintBoxPaint
//---------------------------------------------------------------------------


