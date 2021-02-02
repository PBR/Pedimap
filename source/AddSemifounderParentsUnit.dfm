object AddSemifounderParentsForm: TAddSemifounderParentsForm
  Left = 0
  Top = 104
  Width = 434
  Height = 265
  Caption = 'Add Semi-founder Parents'
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
    Left = 32
    Top = 18
    Width = 288
    Height = 13
    Caption = 'Representation of non-transmitted allele in  unknown parents:'
  end
  object Label2: TLabel
    Left = 32
    Top = 40
    Width = 311
    Height = 13
    Caption = 
      'NOTE: Should be different from all of the known or possible alle' +
      'les'
  end
  object Label3: TLabel
    Left = 68
    Top = 59
    Width = 189
    Height = 13
    Caption = 'at any locus in any of the semi-founders.'
  end
  object Label4: TLabel
    Left = 32
    Top = 98
    Width = 102
    Height = 13
    Caption = 'Save new datafile as:'
  end
  object AlleleEd: TEdit
    Left = 328
    Top = 14
    Width = 65
    Height = 21
    TabOrder = 0
    Text = '999'
  end
  object OKbtn: TButton
    Left = 60
    Top = 193
    Width = 75
    Height = 25
    Caption = 'O&K'
    ModalResult = 1
    TabOrder = 1
    OnClick = OKbtnClick
  end
  object Cancelbtn: TButton
    Left = 292
    Top = 193
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object FilenameEd: TEdit
    Left = 32
    Top = 122
    Width = 361
    Height = 21
    TabOrder = 3
  end
  object Browsebtn: TButton
    Left = 318
    Top = 90
    Width = 75
    Height = 25
    Caption = '&Browse'
    TabOrder = 4
    OnClick = BrowsebtnClick
  end
  object Importcbx: TCheckBox
    Left = 32
    Top = 152
    Width = 361
    Height = 17
    Caption = '&Import new datafile into project'
    TabOrder = 5
  end
  object SaveDialog1: TSaveDialog
    Left = 200
    Top = 192
  end
end
