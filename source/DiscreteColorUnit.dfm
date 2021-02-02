object DiscreteColorForm: TDiscreteColorForm
  Left = 497
  Top = 157
  BorderIcons = []
  BorderStyle = bsDialog
  ClientHeight = 310
  ClientWidth = 251
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnDestroy = FormDestroy
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object ColorListbox: TListBox
    Left = 16
    Top = 16
    Width = 217
    Height = 196
    Style = lbOwnerDrawFixed
    IntegralHeight = True
    ItemHeight = 16
    TabOrder = 0
    OnDrawItem = ColorListboxDrawItem
  end
  object Currentbtn: TButton
    Left = 16
    Top = 224
    Width = 105
    Height = 25
    Caption = 'Change current ...'
    TabOrder = 1
    OnClick = CurrentbtnClick
  end
  object Allbtn: TButton
    Left = 128
    Top = 224
    Width = 105
    Height = 25
    Caption = 'Set all'
    TabOrder = 2
    OnClick = AllbtnClick
  end
  object OKbtn: TButton
    Left = 16
    Top = 264
    Width = 49
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 3
  end
  object Cancelbtn: TButton
    Left = 184
    Top = 264
    Width = 49
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 4
  end
  object ColorDialog1: TColorDialog
    Ctl3D = True
    Left = 119
    Top = 265
  end
end
