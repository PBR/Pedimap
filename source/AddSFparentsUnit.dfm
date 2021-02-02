object AddSFparentsForm: TAddSFparentsForm
  Left = 559
  Top = 221
  Width = 425
  Height = 195
  Caption = 'Add semifounder parents'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label4: TLabel
    Left = 32
    Top = 27
    Width = 102
    Height = 13
    Caption = 'Save new datafile as:'
  end
  object OKbtn: TButton
    Left = 60
    Top = 122
    Width = 75
    Height = 25
    Caption = 'O&K'
    ModalResult = 1
    TabOrder = 0
  end
  object Cancelbtn: TButton
    Left = 292
    Top = 122
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object FilenameEd: TEdit
    Left = 32
    Top = 51
    Width = 361
    Height = 21
    TabOrder = 2
  end
  object Browsebtn: TButton
    Left = 318
    Top = 19
    Width = 75
    Height = 25
    Caption = '&Browse'
    TabOrder = 3
  end
  object Importcbx: TCheckBox
    Left = 32
    Top = 81
    Width = 361
    Height = 17
    Caption = '&Import new datafile into project'
    TabOrder = 4
  end
end
