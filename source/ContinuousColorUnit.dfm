object ContinuousColorForm: TContinuousColorForm
  Left = 874
  Top = 93
  BorderIcons = []
  BorderStyle = bsDialog
  ClientHeight = 343
  ClientWidth = 119
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object PaintBox1: TPaintBox
    Left = 39
    Top = 56
    Width = 41
    Height = 153
    OnPaint = PaintBox1Paint
  end
  object MVColbtn: TButton
    Left = 19
    Top = 264
    Width = 81
    Height = 25
    Caption = 'm.v. color ...'
    TabOrder = 2
    OnClick = MVColbtnClick
  end
  object HiColbtn: TButton
    Left = 19
    Top = 16
    Width = 81
    Height = 25
    Caption = 'High color ...'
    TabOrder = 0
    OnClick = HiColbtnClick
  end
  object LowColbtn: TButton
    Left = 19
    Top = 224
    Width = 81
    Height = 25
    Caption = 'Low color ...'
    TabOrder = 1
    OnClick = LowColbtnClick
  end
  object OKbtn: TButton
    Left = 15
    Top = 304
    Width = 41
    Height = 25
    Caption = 'O&K'
    Default = True
    ModalResult = 1
    TabOrder = 3
  end
  object Cancelbtn: TButton
    Left = 63
    Top = 304
    Width = 41
    Height = 25
    Cancel = True
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 4
  end
  object ColorDialog1: TColorDialog
    Ctl3D = True
    Left = 88
    Top = 64
  end
end
