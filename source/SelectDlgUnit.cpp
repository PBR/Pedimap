//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SelectDlgUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TSelectDlgForm *SelectDlgForm;
//---------------------------------------------------------------------------
__fastcall TSelectDlgForm::TSelectDlgForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void TSelectDlgForm::SetParentlineRgrpItems(int FemaleParent) {
        switch (FemaleParent) {
          case 0: { ParentlineRgrp->Items->Strings[0]="Female line";
                    ParentlineRgrp->Items->Strings[1]="Male line";
                    ParentlineRgrp->Items->Strings[2]="Both";
                    break; }
          case 1: { ParentlineRgrp->Items->Strings[0]="Male line";
                    ParentlineRgrp->Items->Strings[1]="Female line";
                    ParentlineRgrp->Items->Strings[2]="Both";
                    break; }
          default:{ ParentlineRgrp->Items->Strings[0]="First parent";
                    ParentlineRgrp->Items->Strings[1]="Second parent";
                    ParentlineRgrp->Items->Strings[2]="Both";
                    break; }
        } //switch
} //SetParentlineRgrpItems

