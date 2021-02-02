object ImputeSFparentAllelesForm: TImputeSFparentAllelesForm
  Left = 5
  Top = 196
  Width = 434
  Height = 304
  Caption = 'Impute missing founder alleles'
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
    Width = 244
    Height = 13
    Caption = 'Representation of non-transmitted allele in founders:'
  end
  object Label2: TLabel
    Left = 75
    Top = 40
    Width = 275
    Height = 13
    Caption = 'NOTE: Should be different from all existing alleles at all loci'
  end
  object Label4: TLabel
    Left = 32
    Top = 125
    Width = 102
    Height = 13
    Caption = 'Save new datafile as:'
  end
  object Label5: TLabel
    Left = 34
    Top = 75
    Width = 141
    Height = 13
    Caption = 'Color code for imputed alleles:'
  end
  object AlleleEd: TEdit
    Left = 328
    Top = 14
    Width = 65
    Height = 21
    TabOrder = 2
    Text = '999'
  end
  object OKbtn: TButton
    Left = 60
    Top = 220
    Width = 75
    Height = 25
    Caption = 'O&K'
    ModalResult = 1
    TabOrder = 0
    OnClick = OKbtnClick
  end
  object Cancelbtn: TButton
    Left = 292
    Top = 220
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 1
  end
  object FilenameEd: TEdit
    Left = 32
    Top = 149
    Width = 361
    Height = 21
    TabOrder = 4
  end
  object Browsebtn: TButton
    Left = 318
    Top = 117
    Width = 75
    Height = 25
    Caption = '&Browse'
    TabOrder = 5
    OnClick = BrowsebtnClick
  end
  object Importcbx: TCheckBox
    Left = 32
    Top = 179
    Width = 361
    Height = 17
    Caption = '&Import new datafile into project'
    TabOrder = 6
  end
  object ColorCodeCombx: TComboBox
    Left = 248
    Top = 71
    Width = 145
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 1
    TabOrder = 3
    Text = '1'
    Items.Strings = (
      '0'
      '1'
      '2'
      '3'
      '4'
      '5'
      '6'
      '7'
      '8'
      '9')
  end
  object SaveDialog1: TSaveDialog
    Left = 200
    Top = 219
  end
end
