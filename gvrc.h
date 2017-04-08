#define VersionGV          1

#define MenuOpen        1
#define  MenuNew        101
#define  MenuMulti      102
#define  MenuImageFirst 103
#define  MenuImagePrev  104
#define  MenuImageNext  105
#define  MenuSave       106
#define  MenuSaveAs     107
#define  MenuSaveMulti  108
#define  MenuAppend     109
#define  MenuBatch      110
#define  MenuBatchApp   111
#define  MenuAcquire    112
#define  MenuASelect    113
#define  MenuPrint      114
#define  MenuPSetup     115
#define  MenuExit       116
#define  MenuLastView   117     /* 4 items go here */
#define MenuUndo        2
#define  MenuCopyFull   201
#define  MenuCopyView   202
#define  MenuPaste      203
#define  MenuRefresh    204
#define MenuSlide       3        /* Must be integer starting position of this menu */
#define  MenuSlSave     301
#define  MenuSlSaveAs   302
#define  MenuSlClose    303
#define  MenuSlOptions  304
#define  MenuPreview    305
#define  MenuSlInc      306
#define  MenuSlDec      307
#define  MenuSlMan      308
#define  MenuSlAuto     309
#define  MenuSlClear    310
#define  MenuSlNon      311
#define  MenuSlNext     312
#define  MenuSlNextGroup 313
#define  MenuCat        320
#define  MenuCat0       MenuCat+30
#define  MenuCatS0      MenuCat0+10
#define MenuWhole       4
#define  MenuZEnlarge   401
#define  MenuZReduce    402
#define  MenuOrig       403
#define  MenuZ025       404
#define  MenuZ050       405
#define  MenuZ100       406
#define  MenuZ200       407
#define  MenuZ400       408
#define  MenuZPlus      409
#define  MenuZMinus     410
#define  MenuZAspect    411
#define  MenuZ78        412
#define MenuLetFit      5
#define  MenuLetBlack   501
#define   MenuLetGrey   5011
#define   MenuLetWhite  5012
#define   MenuLetCon    5013
#define  MenuGrey       502
#define   MenuColor     5021
#define   MenuDither    5022
#define   MenuInter     5023
#define   MenuInter2    5024
#define   MenuSpiff     5025
#define   MenuSpiffOff  5026
#define  MenuRotateCW   503
#define  MenuRotateCCW  504
#define  MenuZLock      505
#define  MenuZPrev      506
#define  MenuZL100      507
#define  MenuError      508
#define  MenuWarn       509
#define  MenuSole       510
#define  MenuTop        511
#define  MenuExt        512
#define MenuInfo        6
#define  MenuHelp       601
#define  MenuHelpS      602
#define  MenuAbout      605
#define MoveShift       7
#define  MenuPrevSave   701
#define  MenuSelectAll  702
#define  MenuPrOptions  703
#define  MenuViewed     704
#define  MenuPurge      705
#define MenuShowImage   8
#define  MenuCopyImage  801
#define  MenuDelete     802
#define  MenuMove       803
#define  MenuRename     804
#define  MenuSort       805
#define  MenuFind       806
#define  MenuFindNext   807
#define MenuKeyDown     9
#define  MenuKeyEnd     901
#define  MenuKeyHome    902
#define  MenuKeyInsert  903
#define  MenuKeyLeft    904
#define  MenuKeyPgDown  905
#define  MenuKeyPgUp    906
#define  MenuKeyRight   907
#define  MenuKeyUp      908
#define  MenuZoomF10    909

#define ICON1          1001
#define ICON2          1002
#define ICON3          1003
#define ICON4          1004
#define ICON5          1005
#define ICON6          1006
#define ICON7          1007
#define ICON8          1008

#define CursorNull     1009

#define HelpAspect        1
#define HelpFind          2
#define HelpGreyscale     3
#define HelpOpen         10 /*  \                       */
#define HelpOpenNew      11 /*   |__ Must be sequential */
#define HelpOpenBatch    12 /*   |                      */
#define HelpOpenBApp     13 /*  /                       */
#define HelpOpenSlide    20 /*  \                       */
#define HelpOpenPreview  21 /*   |                      */
#define HelpChangeRoot   22 /*   |-- Must be sequential */
#define HelpMove         23 /*   |                      */
#define HelpCopy         24 /*  /                       */
#define HelpPreview      30
#define HelpPreviewSave  31
#define HelpPrint        40 /*  \___ Must be sequential */
#define HelpPrintPrev    41 /*  /                       */
#define HelpRename       50
#define HelpSave         70 /*  \                       */
#define HelpSaveAs       71 /*   |                      */
#define HelpAppend       72 /*   |                      */
#define HelpSaveBatch    73 /*   |-- Must be sequential */
#define HelpSaveBApp     74 /*   |                      */
#define HelpSaveSlide    75 /*   |                      */
#define HelpSavePreview  76 /*  /                       */
#define HelpSaveBMP      80 /*  \                       */
#define HelpSaveCUR      81 /*   |                      */
#define HelpSaveGIF      82 /*   |                      */
#define HelpSaveGRO      83 /*   |                      */
#define HelpSaveICO      84 /*   |__ Must be sequential */
#define HelpSaveJPG      85 /*   |                      */
#define HelpSavePPM      86 /*   |                      */
#define HelpSavePCX      87 /*   |                      */
#define HelpSaveTGA      88 /*   |                      */
#define HelpSaveTIF      89 /*  /                       */
#define HelpSaveCustom  100
#define HelpSlide       101
#define HelpSlideCat    102
#define HelpSort        103

#define IDC_STATIC       -1
#define HelpHelp       4000

#define AbDLG           100
#define AbDate         3000

#define AsDLG           101
#define AspectXXX      3000
#define AspectYYY      3001

#define BaDLG           102
#define BatchStat1     3000
#define BatchStat2     3001
#define BatchStat3     3002

#define CopyDLG         103
#define CopyStat1      3000

#define FindDLG         104
#define FindName       3000
#define FindExact      3001
#define FindBegin      3002
#define FindSub        3003

#define GrDLG           105
#define GreyShade      3000

#define OpDLG           106
#define OpenRec          10
#define OpenRec2         11
#define OpenMul          12
#define OpenMul2         13

#define PrDLG           107
#define PrintPage        10
#define PrMargin1        11
#define PrMargin2        12
#define PrMargin3        13
#define PrMargin4        14
#define PrDPI            15
#define MRadio1          16
#define MRadio2          17
#define CRadio1          18
#define CRadio2          19
#define VRadio1          20
#define VRadio2          21
#define PrDPIText        22

#define PrevDLG         108
#define PrevWidth      3000
#define PrevHeight     3001
#define PrevOpt25      3002 /*  \                       */
#define PrevOpt26      3003 /*   |                      */
#define PrevOpt27      3004 /*   |                      */
#define PrevOpt28      3005 /*   |-- Must be sequential */
#define PrevOpt29      3006 /*   |                      */
#define PrevOpt30      3007 /*   |                      */
#define PrevOpt31a     3008 /*  /                       */
#define PrevOpt31b     3009

#define PrevSDLG        109
#define PrevSWidth     3000
#define PrevSHeight    3001
#define PrevSOpt1      3002
#define PrevSOpt2      3003
#define PrevSNum1      3004
#define PrevSNum2      3005

#define ReDLG           110
#define RenameOrig     3000
#define RenameNew      3001

#define RegDLG          111
#define Reg2DLG         112
#define RegistName     3000
#define RegistCode     3001
#define RegistText     3002
#define RegistMore     3003
#define RegistPrint    3004
#define RegistShow     3005
#define RegistReg      3006
#define RegistText2    3007

#define SaDLG           113
#define SaDLG0          120
#define SaDLG1          121
#define SaDLG2          122
#define SaDLG3          123
#define SaDLG4          124
#define SaDLG5          125
#define SaDLG6          126
#define SaDLG7          127
#define SaDLG8          128
#define SaDLG9          129
#define SaDLG5a         135
#define SaveExt          10
#define SaveOptions      11
#define SaveView1        12
#define SaveView2        13
#define SaveExt2         14
#define SaveDef        3000
#define SaveColors     3001
#define SaveCompress   3002
#define SaveDPI        3003
#define SaveFit        3004
#define SaveFit1       3005
#define SaveFit2       3006
#define SaveFit3       3007
#define SaveRad1       3008
#define SaveRad2       3009
#define SaveRad3       3010
#define SaveRad4       3011
#define SaveRad1b      3012
#define SaveRad2b      3013
#define SaveEdit1      3014
#define SaveEdit2      3015
#define SaveEdit3      3016
#define SaveEdit4      3017
#define SaveCheck      3018
#define SaveScroll     3019
#define SaveText1      3020
#define SaveCus        3021
#define SaveCustom     3022
#define SaveQ          3023 /* 64 values */
#define SaveBest       3087
#define SaveWorst      3088
#define SaveKeep       3089

#define SlDLG           140
#define SlideStat1     3000
#define SlideStat2     3001

#define SlOptDLG        141
#define SlCatDLG        142
#define SlideDir       3000
#define SlideRef       3001
#define SlideRoot      3002
#define SlideOpt5      3003
#define SlideOpt7      3004
#define SlideOpt6      3005
#define SlideOpt8      3006
#define SlideOpt3      3007
#define SlideSec       3008
#define SlideOpt4      3009
#define SlideCatList   3010
#define SlideNew       3011
#define SlideDel       3012
#define SlideEdit      3013
#define SlideQuick     3014
#define SlideShowY     3015
#define SlideShowN     3016
#define SlideName      3017
#define SlideOpt10a    3018
#define SlideOpt10b    3019

#define SortDLG         150
#define Sort2DLG        151
#define SortList       3000
#define SortUp         3001
#define SortDown       3002
#define SortStat       3003

#define StdSaveDLG      152
