object RenameForm: TRenameForm
  Left = 163
  Top = 178
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Rename'
  ClientHeight = 76
  ClientWidth = 273
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object RenameEd: TEdit
    Left = 8
    Top = 8
    Width = 257
    Height = 21
    TabOrder = 0
  end
  object OKbtn: TButton
    Left = 40
    Top = 40
    Width = 75
    Height = 25
    Caption = 'O&K'
    Default = True
    ModalResult = 1
    TabOrder = 1
  end
  object Cancelbtn: TButton
    Left = 160
    Top = 40
    Width = 75
    Height = 25
    Cancel = True
    Caption = '&Cancel'
    ModalResult = 2
    TabOrder = 2
  end
end
