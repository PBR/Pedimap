object OpenProgressForm: TOpenProgressForm
  Left = 193
  Top = 230
  Width = 560
  Height = 199
  Caption = 'Pedimap - Open file progress'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 16
    Width = 74
    Height = 13
    Caption = 'Processing file: '
  end
  object Label2: TLabel
    Left = 8
    Top = 40
    Width = 43
    Height = 13
    Caption = 'Reading:'
  end
  object FileLbl: TLabel
    Left = 96
    Top = 16
    Width = 30
    Height = 13
    Caption = 'FileLbl'
  end
  object ReadLbl1: TLabel
    Left = 96
    Top = 40
    Width = 46
    Height = 13
    Caption = 'ReadLbl1'
  end
  object ReadLbl2: TLabel
    Left = 96
    Top = 64
    Width = 46
    Height = 13
    Caption = 'ReadLbl1'
  end
  object ProgressBar1: TProgressBar
    Left = 16
    Top = 96
    Width = 521
    Height = 9
    Min = 0
    Max = 100
    Smooth = True
    Step = 1
    TabOrder = 0
  end
  object CancelBtn: TButton
    Left = 240
    Top = 120
    Width = 75
    Height = 25
    Cancel = True
    Caption = '&Cancel'
    Default = True
    ModalResult = 2
    TabOrder = 1
    OnClick = CancelBtnClick
  end
end
