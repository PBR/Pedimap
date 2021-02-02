object OptionsForm: TOptionsForm
  Left = 351
  Top = 215
  Width = 353
  Height = 480
  BorderIcons = []
  Caption = 'View Options'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCloseQuery = FormCloseQuery
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object PageControl1: TPageControl
    Left = 0
    Top = 0
    Width = 345
    Height = 411
    ActivePage = IndivSheet
    Align = alClient
    MultiLine = True
    TabIndex = 1
    TabOrder = 0
    object PageSheet: TTabSheet
      Caption = 'PgSize'
      ImageIndex = 2
      TabVisible = False
      object PaperSizeGrpbx: TGroupBox
        Left = 8
        Top = 6
        Width = 300
        Height = 156
        Caption = ' Paper Size '
        TabOrder = 0
        object Label4: TLabel
          Left = 23
          Top = 54
          Width = 81
          Height = 13
          Caption = 'Paper &width (mm)'
          FocusControl = PapWidthEd
        end
        object Label5: TLabel
          Left = 23
          Top = 86
          Width = 85
          Height = 13
          Caption = 'Paper h&eight (mm)'
          FocusControl = PapHeightEd
        end
        object PaperSizeCbox: TComboBox
          Left = 10
          Top = 19
          Width = 279
          Height = 21
          Style = csDropDownList
          DropDownCount = 4
          ItemHeight = 13
          TabOrder = 0
          Visible = False
          Items.Strings = (
            'A4 210 x 297 mm'
            'Letter 8.5 x 11 in'
            'Legal 8.5 x 14 in'
            'Custom size')
        end
        object PapWidthEd: TEdit
          Left = 235
          Top = 50
          Width = 50
          Height = 21
          TabOrder = 1
        end
        object PapHeightEd: TEdit
          Left = 235
          Top = 82
          Width = 50
          Height = 21
          TabOrder = 2
        end
        object OrientRGroup: TRadioGroup
          Left = 10
          Top = 109
          Width = 279
          Height = 40
          Caption = ' &Orientation '
          Columns = 2
          ItemIndex = 1
          Items.Strings = (
            'Portrait'
            'Landscape')
          TabOrder = 3
          Visible = False
        end
      end
      object MarginsGrpbx: TGroupBox
        Left = 8
        Top = 184
        Width = 300
        Height = 85
        Caption = ' Margins '
        TabOrder = 1
        object LeftLbl: TLabel
          Left = 14
          Top = 22
          Width = 43
          Height = 13
          Caption = 'Le&ft (mm)'
          FocusControl = LeftMargEd
        end
        object RightLbl: TLabel
          Left = 14
          Top = 54
          Width = 50
          Height = 13
          Caption = '&Right (mm)'
          FocusControl = RightMargEd
        end
        object TopLbl: TLabel
          Left = 160
          Top = 24
          Width = 44
          Height = 13
          Caption = '&Top (mm)'
          FocusControl = TopMargEd
        end
        object Bottom: TLabel
          Left = 160
          Top = 56
          Width = 58
          Height = 13
          Caption = 'Botto&m (mm)'
          FocusControl = BottomMargEd
        end
        object LeftMargEd: TEdit
          Left = 81
          Top = 18
          Width = 35
          Height = 21
          TabOrder = 0
          Text = '25'
        end
        object RightMargEd: TEdit
          Left = 81
          Top = 50
          Width = 35
          Height = 21
          TabOrder = 1
          Text = '25'
        end
        object TopMargEd: TEdit
          Left = 250
          Top = 20
          Width = 35
          Height = 21
          TabOrder = 2
          Text = '25'
        end
        object BottomMargEd: TEdit
          Left = 250
          Top = 52
          Width = 35
          Height = 21
          TabOrder = 3
          Text = '25'
        end
      end
    end
    object LayoutSheet: TTabSheet
      Caption = '&Layout'
      ImageIndex = 6
      object BetwGenerGbox: TGroupBox
        Left = 16
        Top = 154
        Width = 289
        Height = 60
        Caption = ' Spacing between generations '
        TabOrder = 0
        object Label9: TLabel
          Left = 78
          Top = 25
          Width = 108
          Height = 13
          Caption = 'times cross symbol size'
        end
        object BetwGenerEd: TEdit
          Left = 14
          Top = 21
          Width = 41
          Height = 21
          TabOrder = 0
          Text = '5'
        end
        object BetwGenerUpDown: TUpDown
          Left = 55
          Top = 21
          Width = 15
          Height = 21
          Associate = BetwGenerEd
          Min = 1
          Position = 5
          TabOrder = 1
          Wrap = False
        end
      end
      object PedOrientRGrp: TRadioGroup
        Left = 16
        Top = 4
        Width = 289
        Height = 41
        Caption = ' Pedigree orientation '
        Columns = 2
        ItemIndex = 0
        Items.Strings = (
          'Top to Bottom'
          'Left to Right')
        TabOrder = 1
      end
      object InGenerGrbox: TGroupBox
        Left = 16
        Top = 234
        Width = 289
        Height = 129
        Caption = ' Spacing within generations '
        TabOrder = 2
        object Label8: TLabel
          Left = 16
          Top = 21
          Width = 115
          Height = 13
          Caption = 'Space between families:'
        end
        object Label10: TLabel
          Left = 87
          Top = 41
          Width = 89
          Height = 13
          Caption = '% of Individual size'
        end
        object Label11: TLabel
          Left = 16
          Top = 71
          Width = 99
          Height = 13
          Caption = 'Space between sibs:'
        end
        object Label12: TLabel
          Left = 89
          Top = 91
          Width = 125
          Height = 13
          Caption = '% of between-family space'
        end
        object BetwFamEd: TEdit
          Left = 15
          Top = 38
          Width = 41
          Height = 21
          TabOrder = 0
          Text = '100'
        end
        object BetwFamUpDown: TUpDown
          Left = 56
          Top = 38
          Width = 15
          Height = 21
          Associate = BetwFamEd
          Min = 0
          Max = 10000
          Position = 100
          TabOrder = 1
          Wrap = False
        end
        object BetwSibEd: TEdit
          Left = 16
          Top = 88
          Width = 41
          Height = 21
          TabOrder = 2
          Text = '50'
        end
        object BetwSibUpDown: TUpDown
          Left = 57
          Top = 88
          Width = 15
          Height = 21
          Associate = BetwSibEd
          Min = 0
          Position = 50
          TabOrder = 3
          Wrap = False
        end
      end
      object CossSymbGrbox: TGroupBox
        Left = 16
        Top = 62
        Width = 289
        Height = 78
        Caption = ' Cross symbols '
        TabOrder = 3
        object CrossSizeLbl: TLabel
          Left = 11
          Top = 49
          Width = 107
          Height = 13
          Caption = 'Cross symbol size (mm)'
        end
        object ShowCrossCbx: TCheckBox
          Left = 11
          Top = 20
          Width = 177
          Height = 17
          Alignment = taLeftJustify
          Caption = 'Show cross symbols'
          Checked = True
          State = cbChecked
          TabOrder = 0
        end
        object CrossSizeEd: TEdit
          Left = 175
          Top = 44
          Width = 41
          Height = 21
          TabOrder = 1
          Text = '4'
        end
      end
    end
    object IndivSheet: TTabSheet
      Caption = 'Individuals'
      ImageIndex = 3
      object INameFontbtn: TButton
        Left = 11
        Top = 16
        Width = 125
        Height = 25
        Caption = 'Name font ...'
        TabOrder = 0
        OnClick = INameFontbtnClick
      end
      object GroupBox1: TGroupBox
        Left = 8
        Top = 160
        Width = 321
        Height = 193
        Caption = ' Show in individuals '
        TabOrder = 1
        object NameRbtn: TRadioButton
          Left = 16
          Top = 32
          Width = 250
          Height = 17
          Caption = 'Name only'
          Checked = True
          TabOrder = 0
          TabStop = True
        end
        object IBDprobRbtn: TRadioButton
          Left = 16
          Top = 74
          Width = 250
          Height = 17
          Caption = 'Name and IBD probabilities (IBD page)'
          TabOrder = 1
          OnClick = IBDprobRbtnClick
        end
        object MarkRbtn: TRadioButton
          Left = 16
          Top = 160
          Width = 250
          Height = 17
          Caption = 'Name and marker scores (Markers page)'
          TabOrder = 3
          OnClick = MarkRbtnClick
        end
        object IBDallRbtn: TRadioButton
          Left = 16
          Top = 117
          Width = 250
          Height = 17
          Caption = 'Name and most probable alleles (IBD page)'
          TabOrder = 2
          OnClick = IBDallRbtnClick
        end
      end
      object FillColorGrpBox: TGroupBox
        Left = 8
        Top = 48
        Width = 321
        Height = 105
        Caption = ' Fill color '
        TabOrder = 2
        object FixedColbtn: TRadioButton
          Left = 16
          Top = 24
          Width = 113
          Height = 17
          Caption = 'Fixed color :'
          Checked = True
          TabOrder = 1
          TabStop = True
          OnClick = IBackColbtnClick
        end
        object TraitColbtn: TRadioButton
          Left = 16
          Top = 64
          Width = 113
          Height = 17
          Caption = 'Based on trait ...'
          TabOrder = 2
          OnClick = IBackColbtnClick
        end
        object TraitCombx: TComboBox
          Left = 128
          Top = 62
          Width = 145
          Height = 21
          ItemHeight = 13
          TabOrder = 4
        end
        object SetFixedColBtn: TButton
          Left = 278
          Top = 21
          Width = 33
          Height = 25
          Caption = 'Set'
          TabOrder = 3
          OnClick = ColorbtnClick
        end
        object FixedColorPanel: TPanel
          Left = 128
          Top = 24
          Width = 25
          Height = 17
          BevelOuter = bvLowered
          Color = clYellow
          Enabled = False
          TabOrder = 0
        end
        object SetTraitColbtn: TButton
          Left = 279
          Top = 59
          Width = 33
          Height = 25
          Caption = 'Set'
          TabOrder = 5
          OnClick = SetTraitColbtnClick
        end
      end
    end
    object TraitSheet: TTabSheet
      Caption = 'Traits'
      ImageIndex = 5
      TabVisible = False
    end
    object IBDSheet: TTabSheet
      Caption = 'IBD'
      ImageIndex = 3
      object LinkGrpLbl: TLabel
        Left = 16
        Top = 11
        Width = 94
        Height = 13
        Caption = 'Show linkage group'
      end
      object LinkGrpCombx: TComboBox
        Left = 128
        Top = 11
        Width = 113
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        TabOrder = 0
        OnChange = LinkGrpCombxChange
      end
      object ProbRectGroupBox: TGroupBox
        Left = 8
        Top = 44
        Width = 321
        Height = 58
        Caption = ' Probability rectangles '
        TabOrder = 1
        object Label1: TLabel
          Left = 160
          Top = 25
          Width = 56
          Height = 13
          Caption = 'Height (mm)'
          FocusControl = ProbHeightEd
        end
        object Label2: TLabel
          Left = 8
          Top = 25
          Width = 53
          Height = 13
          Caption = 'Width (mm)'
          FocusControl = ProbWidthEd
        end
        object ProbWidthEd: TEdit
          Left = 72
          Top = 21
          Width = 57
          Height = 21
          TabOrder = 0
          Text = '15'
        end
        object ProbHeightEd: TEdit
          Left = 224
          Top = 21
          Width = 57
          Height = 21
          TabOrder = 1
          Text = '30'
        end
      end
      object AlleleGroupBox: TGroupBox
        Left = 8
        Top = 240
        Width = 321
        Height = 134
        Caption = ' Haplotype colors '
        TabOrder = 3
        object AlleleColLbl: TLabel
          Left = 8
          Top = 73
          Width = 152
          Height = 13
          Caption = 'Set color for selected haplotype:'
        end
        object Label3: TLabel
          Left = 8
          Top = 16
          Width = 124
          Height = 13
          Caption = 'Reset all haplotype colors:'
        end
        object AlleleColbtn: TButton
          Left = 187
          Top = 70
          Width = 49
          Height = 20
          Caption = 'Set ...'
          TabOrder = 4
          OnClick = AlleleColbtnClick
        end
        object AlleleCombx: TComboBox
          Left = 8
          Top = 99
          Width = 307
          Height = 19
          Style = csOwnerDrawFixed
          ItemHeight = 13
          TabOrder = 3
          OnDrawItem = AlleleCombxDrawItem
        end
        object HomFounderBtn: TButton
          Left = 8
          Top = 35
          Width = 95
          Height = 25
          Caption = 'Homoz. founders'
          TabOrder = 0
          OnClick = HomFounderBtnClick
        end
        object HeterzFounderBtn: TButton
          Left = 113
          Top = 35
          Width = 95
          Height = 25
          Caption = 'Heteroz. founders'
          TabOrder = 1
          OnClick = HeterzFounderBtnClick
        end
        object AllSamebtn: TButton
          Left = 219
          Top = 35
          Width = 95
          Height = 25
          Caption = 'All same color ...'
          TabOrder = 2
          OnClick = AllSamebtnClick
        end
      end
      object ProbAlleleGroupBox: TGroupBox
        Left = 8
        Top = 115
        Width = 321
        Height = 111
        Caption = ' Most probable alleles '
        TabOrder = 2
        object AlleleThresholdLbl: TLabel
          Left = 9
          Top = 51
          Width = 200
          Height = 13
          Caption = 'Probability thresholds for showing alleles ...'
        end
        object BoldLabel: TLabel
          Left = 9
          Top = 77
          Width = 44
          Height = 13
          Caption = 'Bold >='
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object ItalicLabel: TLabel
          Left = 239
          Top = 77
          Width = 40
          Height = 13
          Caption = ' > Italic'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -11
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsItalic]
          ParentFont = False
        end
        object NormalLabel: TLabel
          Left = 120
          Top = 77
          Width = 57
          Height = 13
          Caption = '> Normal >='
        end
        object AlleleFontbtn: TButton
          Left = 9
          Top = 18
          Width = 120
          Height = 25
          Caption = 'Allele Font ...'
          TabOrder = 0
          OnClick = AlleleFontbtnClick
        end
        object BoldEd: TEdit
          Left = 66
          Top = 73
          Width = 33
          Height = 21
          TabOrder = 1
          Text = '0.90'
        end
        object ItalEd: TEdit
          Left = 194
          Top = 73
          Width = 33
          Height = 21
          TabOrder = 2
          Text = '0.50'
        end
        object SelectMrkbtn: TButton
          Left = 193
          Top = 18
          Width = 120
          Height = 25
          Caption = 'Select markers ...'
          TabOrder = 3
          OnClick = SelectMrkbtnClick
        end
      end
    end
    object MarkerSheet: TTabSheet
      Caption = 'Markers'
      ImageIndex = 4
      object Label7: TLabel
        Left = 168
        Top = 262
        Width = 107
        Height = 13
        Caption = 'Set marker code color:'
      end
      object MarkerLview: TListView
        Left = 8
        Top = 40
        Width = 321
        Height = 177
        Checkboxes = True
        Columns = <
          item
            AutoSize = True
            Caption = 'Marker'
            MinWidth = 10
          end
          item
            AutoSize = True
            Caption = 'Linkage Group'
            MinWidth = 10
          end
          item
            Alignment = taRightJustify
            Caption = 'cM'
            MinWidth = 10
          end>
        HideSelection = False
        ReadOnly = True
        TabOrder = 0
        ViewStyle = vsReport
      end
      object MarkFontbtn: TButton
        Left = 19
        Top = 272
        Width = 125
        Height = 24
        Caption = 'Marker font ...'
        TabOrder = 4
        OnClick = AlleleFontbtnClick
      end
      object MarkColorCombx: TComboBox
        Left = 168
        Top = 280
        Width = 129
        Height = 19
        Style = csOwnerDrawFixed
        ItemHeight = 13
        TabOrder = 5
        OnDrawItem = MarkColorCombxDrawItem
      end
      object CodeColbtn: TButton
        Left = 168
        Top = 305
        Width = 129
        Height = 24
        Caption = 'Change ...'
        TabOrder = 6
        OnClick = CodeColbtnClick
      end
      object CheckAllMarkersbtn: TButton
        Left = 16
        Top = 224
        Width = 75
        Height = 17
        Caption = 'Check All'
        TabOrder = 2
        OnClick = CheckAllMarkersbtnClick
      end
      object ClearAllMarkersbtn: TButton
        Left = 96
        Top = 224
        Width = 75
        Height = 17
        Caption = 'Clear All'
        TabOrder = 3
        OnClick = ClearAllMarkersbtnClick
      end
      object MrkOrderPanel: TPanel
        Left = 8
        Top = 16
        Width = 321
        Height = 25
        BevelOuter = bvNone
        TabOrder = 1
        object Label6: TLabel
          Left = 6
          Top = 4
          Width = 69
          Height = 13
          Caption = 'Order markers:'
        end
        object MrkPosRbtn: TRadioButton
          Left = 184
          Top = 2
          Width = 89
          Height = 17
          Caption = 'by Position'
          Checked = True
          TabOrder = 1
          TabStop = True
          OnClick = MrkPosRbtnClick
        end
        object MrkNameRbtn: TRadioButton
          Left = 96
          Top = 2
          Width = 73
          Height = 17
          Caption = 'by Name'
          TabOrder = 0
          OnClick = MrkPosRbtnClick
        end
      end
    end
    object ColorsSheet: TTabSheet
      Caption = 'Colors'
      ImageIndex = 2
      object CrossColBtn: TButton
        Left = 32
        Top = 101
        Width = 150
        Height = 25
        Caption = 'Cross symbol color ...'
        TabOrder = 1
        OnClick = ColorbtnClick
      end
      object Par1ColBtn: TButton
        Left = 32
        Top = 163
        Width = 150
        Height = 25
        Caption = 'Line to parent 1 ...'
        TabOrder = 2
        OnClick = ColorbtnClick
      end
      object Par2ColBtn: TButton
        Left = 32
        Top = 203
        Width = 150
        Height = 25
        Caption = 'Line to parent 2 ...'
        TabOrder = 3
        OnClick = ColorbtnClick
      end
      object ParSelfedColBtn: TButton
        Left = 32
        Top = 243
        Width = 150
        Height = 25
        Caption = 'Line to single parent ...'
        TabOrder = 4
        OnClick = ColorbtnClick
      end
      object PageColBtn: TButton
        Left = 32
        Top = 39
        Width = 150
        Height = 25
        Caption = 'Page color ...'
        TabOrder = 0
        OnClick = ColorbtnClick
      end
      object PageColPanel: TPanel
        Left = 240
        Top = 43
        Width = 25
        Height = 17
        BevelOuter = bvLowered
        Color = clWhite
        Enabled = False
        TabOrder = 5
      end
      object CrossColPanel: TPanel
        Left = 240
        Top = 105
        Width = 25
        Height = 17
        BevelOuter = bvLowered
        Color = clBlack
        Enabled = False
        TabOrder = 6
      end
      object Par1ColPanel: TPanel
        Left = 240
        Top = 167
        Width = 25
        Height = 17
        BevelOuter = bvLowered
        Color = clRed
        Enabled = False
        TabOrder = 7
      end
      object Par2ColPanel: TPanel
        Left = 240
        Top = 207
        Width = 25
        Height = 17
        BevelOuter = bvLowered
        Color = clBlue
        Enabled = False
        TabOrder = 8
      end
      object ParSelfedColPanel: TPanel
        Left = 240
        Top = 247
        Width = 25
        Height = 17
        BevelOuter = bvLowered
        Color = clPurple
        Enabled = False
        TabOrder = 9
      end
    end
    object InfoSheet: TTabSheet
      Caption = 'Info'
      ImageIndex = 7
      object InfoTraitsLbl: TLabel
        Left = 16
        Top = 40
        Width = 121
        Height = 13
        Caption = 'Traits in Information panel'
      end
      object InfoTraitChlist: TCheckListBox
        Left = 16
        Top = 56
        Width = 297
        Height = 279
        ItemHeight = 13
        TabOrder = 0
      end
      object InfAllbtn: TButton
        Left = 16
        Top = 345
        Width = 75
        Height = 25
        Caption = 'Check All'
        TabOrder = 1
        OnClick = InfAllbtnClick
      end
      object InfNonebtn: TButton
        Left = 240
        Top = 345
        Width = 75
        Height = 25
        Caption = 'Clear All'
        TabOrder = 2
        OnClick = InfNonebtnClick
      end
      object InfoSelChbx: TCheckBox
        Left = 16
        Top = 10
        Width = 97
        Height = 17
        Caption = 'Show Selected'
        Checked = True
        State = cbChecked
        TabOrder = 3
      end
      object InfoParentsChbx: TCheckBox
        Left = 216
        Top = 10
        Width = 97
        Height = 17
        Caption = 'Show Parents'
        Checked = True
        State = cbChecked
        TabOrder = 4
      end
    end
  end
  object ButtonPanel: TPanel
    Left = 0
    Top = 411
    Width = 345
    Height = 41
    Align = alBottom
    TabOrder = 1
    object OKbtn: TButton
      Left = 24
      Top = 8
      Width = 75
      Height = 25
      Caption = 'O&K'
      Default = True
      ModalResult = 1
      TabOrder = 0
    end
    object Cancelbtn: TButton
      Left = 128
      Top = 8
      Width = 75
      Height = 25
      Cancel = True
      Caption = 'Cancel'
      ModalResult = 2
      TabOrder = 1
    end
    object Defaultbtn: TButton
      Left = 232
      Top = 8
      Width = 75
      Height = 25
      Cancel = True
      Caption = 'Default ...'
      TabOrder = 2
      OnClick = DefaultbtnClick
    end
  end
  object ColorDialog1: TColorDialog
    Ctl3D = True
    Left = 312
  end
end
