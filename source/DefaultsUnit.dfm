object DefaultsForm: TDefaultsForm
  Left = 56
  Top = 199
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Default Options'
  ClientHeight = 186
  ClientWidth = 367
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Bevel1: TBevel
    Left = 25
    Top = 60
    Width = 320
    Height = 2
  end
  object DefaultsRgroup: TRadioGroup
    Left = 16
    Top = 16
    Width = 337
    Height = 113
    Items.Strings = (
      'Save current options as defaults'
      'Replace current options with pre-set defaults'
      'Replace current options with saved defaults')
    TabOrder = 0
  end
  object OKbtn: TButton
    Left = 48
    Top = 144
    Width = 75
    Height = 25
    Caption = 'OK'
    Default = True
    ModalResult = 1
    TabOrder = 1
  end
  object Cancelbtn: TButton
    Left = 240
    Top = 144
    Width = 75
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
