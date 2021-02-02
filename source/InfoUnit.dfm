object InfoForm: TInfoForm
  Left = 248
  Top = 131
  Width = 474
  Height = 408
  ActiveControl = NotesMemo
  BorderIcons = [biSystemMenu, biMaximize]
  Caption = 'Info'
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
  object Splitter1: TSplitter
    Left = 0
    Top = 114
    Width = 466
    Height = 4
    Cursor = crVSplit
    Align = alTop
  end
  object TopPanel: TPanel
    Left = 0
    Top = 0
    Width = 466
    Height = 25
    Align = alTop
    Alignment = taLeftJustify
    Caption = ' Selection history'
    TabOrder = 0
    object Panel2: TPanel
      Left = 412
      Top = 1
      Width = 53
      Height = 23
      Align = alRight
      BevelOuter = bvNone
      TabOrder = 0
      object Closebtn: TButton
        Left = 0
        Top = 2
        Width = 52
        Height = 19
        Cancel = True
        Caption = '&Close'
        Default = True
        ModalResult = 1
        TabOrder = 0
      end
    end
  end
  object SelMemo: TMemo
    Left = 0
    Top = 25
    Width = 466
    Height = 89
    Align = alTop
    Constraints.MinHeight = 25
    Lines.Strings = (
      'SelMemo')
    ReadOnly = True
    ScrollBars = ssBoth
    TabOrder = 1
  end
  object Panel1: TPanel
    Left = 0
    Top = 118
    Width = 466
    Height = 35
    Align = alTop
    Alignment = taLeftJustify
    Caption = '  Notes'
    Constraints.MinHeight = 25
    TabOrder = 2
    object Label1: TLabel
      Left = 360
      Top = 9
      Width = 20
      Height = 13
      Caption = 'Flag'
    end
    object FlagCombx: TComboBox
      Left = 392
      Top = 4
      Width = 49
      Height = 24
      Style = csOwnerDrawFixed
      ItemHeight = 18
      TabOrder = 0
      OnDrawItem = FlagCombxDrawItem
      Items.Strings = (
        '0'
        '1'
        '2'
        '3'
        '4')
    end
  end
  object NotesMemo: TMemo
    Left = 0
    Top = 153
    Width = 466
    Height = 227
    Align = alClient
    Constraints.MinWidth = 25
    ScrollBars = ssBoth
    TabOrder = 3
  end
end
