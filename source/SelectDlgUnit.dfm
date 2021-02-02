object SelectDlgForm: TSelectDlgForm
  Left = 251
  Top = 220
  ActiveControl = AncestorEd
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Select'
  ClientHeight = 341
  ClientWidth = 223
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
    Left = 24
    Top = 63
    Width = 100
    Height = 13
    Caption = '&Ancestor generations'
    FocusControl = AncestorEd
  end
  object Label2: TLabel
    Left = 24
    Top = 95
    Width = 97
    Height = 13
    Caption = '&Progeny generations'
    FocusControl = ProgenyEd
  end
  object ProgParentsCbx: TCheckBox
    Left = 24
    Top = 223
    Width = 169
    Height = 17
    Caption = '&Include progeny parents'
    TabOrder = 2
  end
  object SibsCbx: TCheckBox
    Left = 24
    Top = 255
    Width = 169
    Height = 17
    Caption = 'I&nclude sibs'
    TabOrder = 3
  end
  object AncestorEd: TEdit
    Left = 152
    Top = 58
    Width = 33
    Height = 21
    TabOrder = 0
    Text = '1'
  end
  object ProgenyEd: TEdit
    Left = 152
    Top = 90
    Width = 33
    Height = 21
    TabOrder = 1
    Text = '0'
  end
  object OKbtn: TButton
    Left = 47
    Top = 295
    Width = 49
    Height = 25
    Caption = 'O&K'
    Default = True
    ModalResult = 1
    TabOrder = 4
  end
  object Cancelbtn: TButton
    Left = 127
    Top = 295
    Width = 49
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 5
  end
  object AncesterUpdown: TUpDown
    Left = 185
    Top = 58
    Width = 15
    Height = 21
    Associate = AncestorEd
    Min = 0
    Max = 1000
    Position = 1
    TabOrder = 6
    Wrap = False
  end
  object ProgUpdown: TUpDown
    Left = 185
    Top = 90
    Width = 15
    Height = 21
    Associate = ProgenyEd
    Min = 0
    Max = 1000
    Position = 0
    TabOrder = 7
    Wrap = False
  end
  object SelectRbtn: TRadioButton
    Left = 24
    Top = 24
    Width = 81
    Height = 17
    Caption = '&Select'
    Checked = True
    TabOrder = 8
    TabStop = True
  end
  object DeselectRbtn: TRadioButton
    Left = 136
    Top = 24
    Width = 70
    Height = 17
    Caption = '&Deselect'
    TabOrder = 9
  end
  object ParentlineRgrp: TRadioGroup
    Left = 24
    Top = 121
    Width = 177
    Height = 89
    Caption = ' &Trace pedigree through '
    ItemIndex = 2
    Items.Strings = (
      'Parent 1'
      'Parent 2'
      'Both')
    TabOrder = 10
  end
end
