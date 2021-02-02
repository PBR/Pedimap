object LegendForm: TLegendForm
  Left = 627
  Top = 201
  Width = 192
  Height = 374
  HorzScrollBar.Smooth = True
  HorzScrollBar.Tracking = True
  VertScrollBar.Smooth = True
  VertScrollBar.Tracking = True
  BorderIcons = [biSystemMenu]
  Caption = 'Legend'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object PageControl1: TPageControl
    Left = 0
    Top = 0
    Width = 184
    Height = 340
    ActivePage = IBDColSheet
    Align = alClient
    MultiLine = True
    TabIndex = 2
    TabOrder = 0
    object ContinuousTraitColSheet: TTabSheet
      Caption = 'Trait colors'
      object ContColScrollBox: TScrollBox
        Left = 0
        Top = 0
        Width = 176
        Height = 294
        HorzScrollBar.Smooth = True
        HorzScrollBar.Tracking = True
        VertScrollBar.Smooth = True
        VertScrollBar.Tracking = True
        Align = alClient
        BorderStyle = bsNone
        Color = clWhite
        ParentColor = False
        TabOrder = 0
        object ContTraitLbl: TLabel
          Left = 24
          Top = 14
          Width = 27
          Height = 13
          Caption = 'Trait: '
        end
        object MaxVallbl: TLabel
          Left = 72
          Top = 38
          Width = 55
          Height = 13
          Caption = '(Max.value)'
        end
        object ContColPaintBox: TPaintBox
          Left = 23
          Top = 44
          Width = 41
          Height = 153
          OnPaint = ContColPaintBoxPaint
        end
        object MinVallbl: TLabel
          Left = 72
          Top = 190
          Width = 52
          Height = 13
          Caption = '(Min.value)'
        end
        object MVshape: TShape
          Left = 24
          Top = 222
          Width = 41
          Height = 17
        end
        object LabelMV: TLabel
          Left = 72
          Top = 224
          Width = 69
          Height = 13
          Caption = 'Missing values'
        end
      end
    end
    object LGMapSheet: TTabSheet
      Caption = 'LG map'
      ImageIndex = 1
      object LGmapPanel: TPanel
        Left = 0
        Top = 0
        Width = 176
        Height = 25
        Align = alTop
        Alignment = taLeftJustify
        BevelOuter = bvNone
        Caption = '  Linkage group:'
        Color = clWhite
        TabOrder = 0
      end
      object LGmapLbox: TListBox
        Left = 0
        Top = 25
        Width = 176
        Height = 269
        TabStop = False
        Style = lbVirtualOwnerDraw
        AutoComplete = False
        Align = alClient
        BevelInner = bvNone
        BorderStyle = bsNone
        ItemHeight = 16
        TabOrder = 1
        OnDrawItem = LGmapLboxDrawItem
      end
    end
    object IBDColSheet: TTabSheet
      Caption = 'IBD colors'
      ImageIndex = 2
      object IBDColorListbox: TListBox
        Left = 0
        Top = 0
        Width = 176
        Height = 294
        Style = lbVirtualOwnerDraw
        Align = alClient
        BorderStyle = bsNone
        ItemHeight = 16
        TabOrder = 0
        OnDrawItem = IBDColorListboxDrawItem
      end
    end
    object MarkerSheet: TTabSheet
      Caption = 'Markers'
      ImageIndex = 3
      object MarkerLbox: TListBox
        Left = 0
        Top = 0
        Width = 176
        Height = 294
        TabStop = False
        Style = lbVirtualOwnerDraw
        AutoComplete = False
        Align = alClient
        BevelInner = bvNone
        BorderStyle = bsNone
        ExtendedSelect = False
        ItemHeight = 16
        TabOrder = 0
        OnDrawItem = MarkerLboxDrawItem
      end
    end
    object DiscreteTraitColSheet: TTabSheet
      Caption = 'Trait colors'
      ImageIndex = 4
      object DiscrColListbox: TListBox
        Left = 0
        Top = 25
        Width = 176
        Height = 269
        Style = lbVirtualOwnerDraw
        Align = alClient
        BorderStyle = bsNone
        Color = clWhite
        ItemHeight = 16
        TabOrder = 0
        OnDrawItem = DiscrColListboxDrawItem
      end
      object DiscrTraitPanel: TPanel
        Left = 0
        Top = 0
        Width = 176
        Height = 25
        Align = alTop
        Alignment = taLeftJustify
        BevelOuter = bvNone
        Caption = '  Trait:'
        Color = clWhite
        TabOrder = 1
      end
    end
  end
end
