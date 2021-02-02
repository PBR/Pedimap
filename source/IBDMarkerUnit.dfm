object IBDMarkerForm: TIBDMarkerForm
  Left = 643
  Top = 101
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'IBD markers'
  ClientHeight = 335
  ClientWidth = 214
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 10
    Top = 8
    Width = 71
    Height = 13
    Caption = 'Linkage group:'
  end
  object Label2: TLabel
    Left = -16
    Top = 248
    Width = 3
    Height = 13
  end
  object LGnameEd: TEdit
    Left = 10
    Top = 32
    Width = 193
    Height = 21
    TabStop = False
    BorderStyle = bsNone
    Color = clMenu
    ReadOnly = True
    TabOrder = 5
  end
  object IBDmrkLview: TListView
    Left = 10
    Top = 64
    Width = 193
    Height = 185
    Checkboxes = True
    Columns = <
      item
        Caption = 'Marker'
      end
      item
        Alignment = taRightJustify
        Caption = 'Position'
      end>
    TabOrder = 0
    ViewStyle = vsReport
  end
  object OKbtn: TButton
    Left = 10
    Top = 294
    Width = 75
    Height = 25
    Caption = 'O&K'
    Default = True
    ModalResult = 1
    TabOrder = 3
  end
  object Cancelbtn: TButton
    Left = 128
    Top = 294
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 4
  end
  object CheckAllMarkersbtn: TButton
    Left = 20
    Top = 254
    Width = 75
    Height = 17
    Caption = 'Check All'
    TabOrder = 1
    OnClick = CheckAllMarkersbtnClick
  end
  object ClearAllMarkersbtn: TButton
    Left = 118
    Top = 254
    Width = 75
    Height = 17
    Caption = 'Clear All'
    TabOrder = 2
    OnClick = ClearAllMarkersbtnClick
  end
end
