#include <windows.h>
#include <commctrl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlobj.h>
#include <time.h>
#include "gvrc.h"
#include "gload.h"
#include "gv.h"
#include "gvlib.h"

char WinName[]="GV", WinName2[]="GV2";
FARPROC pDSM_Entry;
HBITMAP wpic=0;
HANDLE devmode=0;
HINSTANCE hctl3d=0, hinst;
HMENU Hmenu;
HPALETTE prevpal=0, wpal=0;
HWND Hwnd, HwndP=0;
OPENFILENAME openfile={0};
PRINTDLG printdlg;
WINDOWPLACEMENT WinPlace, WinPlace2;
char curfile[256], *Findtext=0, inifile[MAX_PATH+8]="GV.INI",
     inihead[]="Graphic Viewer", lastview[256*5], oldspec[LENPSPEC],
     opendir[256*5], *opfile, opfilter[512], opchar, RegName[256],
     RegPass[9], savedir[256*6], slidename[256], titleimage[256];
float AspectX=0, AspectY=0, AspectXY[4]={1,1}, PreviewGamma,
      PrintMarg[12]={0,0,0,0,160};
long BackUpdate=-1, BitsPixel=8, bmpi, Busy=0, CopyMode=0, curheight,
     curwidth, dither16num=-1, Down=0, DownX, DownY, EditCat, extrax=0,
     extray=0, FindType=2, Focus[2]={-1,-1}, FilterType=1, HalfSizeBatch=0,
     LastNum=-1, Letter=0, Lock=0, Msg=2, MultiOpen=0, MultiSave=0, NewCopy,
     NextNum, ODown[8], oldh, oldi, oldw, OrigH, OrigW, OrigX=0, OrigY=0,
     PalChange=0, PMode=0, PreCoorMax, PrepTrans=-1, PreviewArea[8],
     PreviewInv, PreviewOpt=0, PreviewTrans, PreviewXY[2], PrevSaveX=600,
     PrevSaveY=440, PrintSet=0, Redox=0, Reduce=1,
     Rename, rescale=2, SaveAs, SaveDialog, SaveFilt=9, SaveOpt=1,
     Scale=0, ScrollOff=0, ScrollWait=0, Shade, SlideDelay=7, SlideLast,
     SlideTx=40, SlideTy=10, SortDir=1, SortVal=0, TimerOn=0, Topmost=0,
     Undo[UNDOLEN*2], View[8], WholeScreen=0, WindowW, WindowH;
uchar dither16tbl[2048], gquant[128], *newpic, *NextPic=0, oldquant[128],
      *pic=0, *PreviewPal=(uchar *)(-1), *PreviewPic=0,
      SaveRec[SAVEFORMATS*SAVELEN];
ulong SlideOpt=15;
uchar SaveDefault[SAVEFORMATS*SAVELEN]={0,0,0xA0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0xA0,0,2,0,0x10,0,0,0,0x1F,0,0,0,0x10,0,0x10,0, 0,0,0xA0,0,1,0,0xFF,0,0,0,0,0,0,0,0,0,0,0, 0,0,0xA0,0,0,1,0,0,0x80,0x3F,2,0,0,0,0,0,0,0,
      0,0,0xA0,0,2,0,0x10,0,0,0,0x1F,0,0,0,0,0,0,0, 0,0,0xA0,0,1,0,0x1E,0,0,0,0,0,0,0,3,0,0,0, 0,0,0xA0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0xA0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0xA0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
      0,3,0xA0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0};
char No[]="No", Yes[]="Yes", SZero[]="";
char *key[]={
  "Enlarge",SZero,0,
  "Errors",No,Yes,
  "Sole",Yes,No,
  "LastDir",SZero,0,
  "WindowPos",SZero,0,
  "FileFilter",SZero,0,
  "MatchDepth",No,Yes,
  "Dither",No,Yes,
  "Letterbox",SZero,0,
  "LockSize",SZero,0,
  "Aspect",SZero,0,
  "Topmost",No,Yes,
  "Margins",SZero,0,
  "Warnings",No,Yes,
  "PreviewOptions",SZero,0,
  "SaveDir",SZero,0,
  "SaveOptions",SZero,0,
  "MultiOpen",No,Yes,
  "BatchOpen",SZero,0,
  "BatchSave",SZero,0,
  "AppendOpen",SZero,0,
  "AppendSave",SZero,0,
  "TitleImage",SZero,0,
  "Placement",SZero,0,
  "SlideOpen",SZero,0,
  "SlideSave",SZero,0,
  "SlideOptions",SZero,0,
  "SlideDelay",SZero,0,
  "PreviewPlacement",SZero,0,
  "PreviewWidth",SZero,0,
  "PreviewHeight",SZero,0,
  "SortMetric",SZero,0,
  "CopyDir",SZero,0,
  "LastView1",SZero,0,
  "LastView2",SZero,0,
  "LastView3",SZero,0,
  "LastView4",SZero,0,
  "PreviewSaveDir",SZero,0,
  "PreviewSaveSize",SZero,0,
  "ImagesViewed",SZero,0,
  "RegisterName",SZero,0,
  "RegisterPassword",SZero,0,
  "Interpolate",No,Yes,
  "HalfSizeBatch",No,Yes,
  "DefaultSaveDir",SZero,0,
  "MultiSave",No,Yes,
  0};
char *extlist[]={"GV","BMP","DIB","CUR","GIF","GIR","GRO","GRB","ICO","JPG",
     "JPEG","JIF","JFF","JPE","PPM","PBM","PGM","PNM","PCC","PCX","TGA",
     "TIF","TIFF",0};
char *extlist2[]={"BMP","CUR","GIF","GRO","ICO","JPG","PPM","PCX","TGA","TIF"};
char *BatchTitle[]={"Batch Convert","Batch Append","Moving","Copying"};
char *ErrorMsg[]={"Unsupported file format.","Can't read file.","Carte Primus file -- Use CP, not GV.","Insufficient memory to flip image horizontally.","Invalid version number (not 0x40 or 0x12).","Insufficient memory for graphic.",
     "Insufficient memory for file.","Unrecognized compression type.","Invalid graphic size.","Insufficient memory for decoder.","Unsupported compression type.","Unsupported bits per pixel.","Unsupported number of color channels.",
     "Unsupported spectral selection in scan header.","Only four quantization tables are supported.","Frame and scan headers do not match.","Palette larger than supported.","Unsupported orientation.","Unsupported color space.",
     "Planar pixel format not supported.","Too many strips in file.","Insufficient memory for planar conversion.","Insufficient memory for deinterlacing.","Slide file"};
char *ErrorMsg2[]={"Unsupported file format.","Can't write file.","Insufficient memory.","Can't open append file.","File format doesn't match append format.","First GIF image must be at least as large as appended images.",
     "Specified endian different from file."};
char HelpFile[]="GV.HLP", slidecheck[]="GVSlideFileV1.0";
char OpenFilter[]="All files|*.*|Graphics files|*.BMP;*.CUR;*.DIB;*.GIF;"
     "*.GIR;*.GRB;*.GRO;*.ICO;*.JIF;*.JFF;*.JPE;*.JPG;*.PBM;*.PCC;*.PCX;"
     "*.PGM;*.PNM;*.PPM;*.TGA;*.TIF|Windows Bitmap files|*.BMP;*.DIB|"
     "Cursor files|*.CUR|GIF files|*.GIF|HP GROB files|*.GRB;*.GRO|"
     "Icon files|*.ICO|JPEG files|*.JIF;*.JFF;*.JPE;*.JPG|"
     "Portable Bitmap files|*.PBM;*.PGM;*.PNM;*.PPM|PCX files|*.PCC;*.PCX|"
     "Targa files|*.TGA|TIF files|*.TIF|";
char *OpenTitle[]={"Open File","Open File in New Window","Select Files for Batch Convert","Select Files for Batch Append"};
char *Open2Title[]={"Open Slide File or Select Directory","Open Preview File or Select Directory",0,"Select Directory for Move","Select Directory for Copy"};
char *SaveComp[]={"None","RLE - Run Length","LZW","LZW with prediction","Baseline DCT","Extended DCT","Progressive DCT"};
char SaveFilter[]="BMP - Windows Bitmap file|*.BMP|CUR - Cursor file|*.CUR|"
     "GIF - Compuserve GIF file|*.GIF|GRO - HP GROB file|*.GRO|ICO - Icon file"
     "|*.ICO|JPG - JPEG file|*.JPG|PPM - Portable Bitmap file|*.PPM|"
     "PCX - Paintbrush file|*.PCX|TGA - Targa file|*.TGA|"
     "TIF - Tagged Image Format file|*.TIF|";
char *SaveTitle[]={"Save File","Save As","Append File","Batch Convert Destination","Batch Append File","Save Slide File","Save Preview Screens -- Specify Base Name"};
char SlideFilter[]="GV Slide files|*.GV|%s";
long appendtbl[]={1,2,4,5,9,-1};
long PreviewCoor[]={62,98,64,64, 134,98,64,64, 32,98,138,64};
long LetterColor[6], PreCoor[MAXPREVIEW*2+4];
SlideList *slide=0;

BOOL CALLBACK about(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the About dialog box.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        5/26/96-DWM */
{
  char text[80];

  switch (msg) {
    case WM_COMMAND: EndDialog(hdlg, 1); break;
    case WM_INITDIALOG:
      sprintf(text, "Version %d.%02d - %s", VERSION, SUBVERSION, __DATE__);
      SetDlgItemText(hdlg, AbDate, text);
      SetFocus(hdlg);
      return(1); }
  return(0);
}

void add_extensions(HWND hwnd)
/* Enter: HWND hwnd: handle of window.                         5/27/96-DWM */
{
  char ext[]="Extensions", base[256], out[256], *cmd, ekey[5];
  long i, j;
  HKEY hkey;

  if (MsgBox(hwnd, "This function will associate all appropriate\n"
                   "file extensions with the GV program.  When\n"
                   "a graphic file is run, GV will automatically\n"
                   "be used to load it.", "Link Extensions",
      MB_OKCANCEL)!=IDOK)
    return;
  cursor(1);
  cmd = GetCommandLine();
  if (strlen(cmd)<245)  strcpy(base, cmd);  else  strcpy(base, "GV");
  if (find_space(base))  find_space(base)[0] = 0;
  remove_quotes2(base);
  for (i=0; extlist[i]; i++) {
    sprintf(out, "%s ^.%s", base, extlist[i]);
    WriteProfileString(ext, extlist[i], out); }
  sprintf(out, "\"%s\" \"\"%%1\"\"", base);
  RegCreateKeyEx(HKEY_CLASSES_ROOT, "GV\\shell\\open\\command", 0, "",
                 REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hkey, &j);
  RegSetValueEx(hkey, "", 0, REG_SZ, out, strlen(out)+1);
  RegCloseKey(hkey);
  RegCreateKeyEx(HKEY_CLASSES_ROOT, "GV\\DefaultIcon", 0, "",
                 REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hkey, &j);
  strcpy(out, base);  remove_quotes(out);
  RegSetValueEx(hkey, "", 0, REG_SZ, out, strlen(out)+1);
  RegCloseKey(hkey);
  RegCreateKeyEx(HKEY_CLASSES_ROOT, "GV", 0, "",
                 REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hkey, &j);
  RegSetValueEx(hkey, "", 0, REG_SZ, "Graphics File", 14);
  RegCloseKey(hkey);
  for (i=0; extlist[i]; i++) {
    sprintf(out, ".%s", extlist[i]);
    RegCreateKeyEx(HKEY_CLASSES_ROOT, out, 0, "", REG_OPTION_NON_VOLATILE,
                   KEY_ALL_ACCESS, 0, &hkey, &j);
    RegSetValueEx(hkey, "", 0, REG_SZ, "GV", 3);
    RegSetValueEx(hkey, "PerceivedType", 0, REG_SZ, "GV", 3);
    RegCloseKey(hkey);
    sprintf(out, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.%s", extlist[i]);
    RegCreateKeyEx(HKEY_CURRENT_USER, out, 0, "", REG_OPTION_NON_VOLATILE,
                   KEY_ALL_ACCESS, 0, &hkey, &j);
    RegSetValueEx(hkey, "Application", 0, REG_SZ, " ", 2);
    RegSetValueEx(hkey, "Progid", 0, REG_SZ, " ", 2);
    RegCloseKey(hkey); }
  SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
  cursor(0);
  MsgBox(hwnd, "Extensions Linked", "Link Exentsions", MB_OK);
}

BOOL CALLBACK aspect_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the Lock Aspect Ratio dialog box.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        5/26/96-DWM */
{
  char text[80];

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd,HelpFile,HELP_CONTEXT,HelpAspect); break;
      case IDOK: GetDlgItemText(hdlg, AspectXXX, text, 79);
        sscanf(text, "%f", &AspectXY[2]);
        GetDlgItemText(hdlg, AspectYYY, text, 79);
        sscanf(text, "%f", &AspectXY[3]);
        if (AspectXY[2]<=0 || AspectXY[3]<=0) {
          if (Msg&1)
            MsgBox(hdlg, "The aspect width and height\n"
                             "must be positive numbers.", "Error", MB_OK);
          return(0); }
        AspectX = AspectXY[0] = AspectXY[2];
        AspectY = AspectXY[1] = AspectXY[3];
      case IDCANCEL: EndDialog(hdlg, 1);  recheck(Hwnd, 0);  return(1);
      default: return(0); } break;
    case WM_INITDIALOG: AspectXY[2]=AspectXY[0];  AspectXY[3]=AspectXY[1];
      sprintf(text, "%g", AspectXY[2]);
      SetDlgItemText(hdlg, AspectXXX, text);
      sprintf(text, "%g", AspectXY[3]);
      SetDlgItemText(hdlg, AspectYYY, text);
      SetFocus(hdlg);
      return(1); }
  return(0);
}

void batch(void)
/* Get the save location, then carry out a batch convert or batch append.
 *  The NewCopy variable chooses convert/append if it is 2/3. 10/15/96-DWM */
{
  char name[NAMELEN];

  if (!open_pic(Hwnd))  return;
  strcpy(name, opfile);
  if (!save_as(NewCopy+1))  return;
  if (NewCopy==2) {
    if (strchr(opfile, '\\'))  strrchr(opfile, '\\')[1] = 0;
    else                       opfile[0] = 0; }
  strcpy(opfilter, opfile);
  strcpy(opfile, name);
  if (NewCopy==2)  strpath(savedir+256, opfilter);
  else             strpath(savedir+512, opfilter);
  Busy = 1;
  DialogBox(hinst, MAKEINTRESOURCE(BaDLG), Hwnd, (DLGPROC)batch_dialog);
  Busy = 0;
}

BOOL CALLBACK batch_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the Batch cancel dialog box.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        5/26/96-DWM */
{
  static uchar *new, *iname, *oname;
  static long num, cur, part, pw, ph, pi, multi;
  static char oldfile[256], workfile[256];
  long i, next=0, flag=0;
  char *val[]={"Writ", "Append"}, *file, *name, pnum[10];
  uchar gspec[SAVELEN+2];
  uchar *half;

  switch (msg) {
    case WM_INITDIALOG: SetWindowText(hdlg, BatchTitle[NewCopy-2]);
      if (!(iname=malloc2(NAMELEN*2))) {
        low_memory(hdlg, "Batch Processing", 0);
        EndDialog(hdlg, 1);  return(0); }
      oname = iname+NAMELEN;
      TimerOn = 1;
      SetTimer(hdlg, 0, 0, 0);
      num = 1;  cur = part = 0;
      for (i=0; i<strlen(opfile); i++) {
        if (opfile[i]==' ' && !flag)  num++;
        if (opfile[i]=='"')  flag ^= 1; }
      new = 0;
      SetFocus(hdlg);
      return(1);
    case WM_TIMER: if (!TimerOn) break;
      if (!new && opfile[0]) {
        sprintf(iname, "Reading file %d of %d.", cur+1, num);
        SetDlgItemText(hdlg, BatchStat1, iname);
        strcpy(iname, opfile);
        remove_quotes(iname);
        SetDlgItemText(hdlg, BatchStat2, iname);
        strcpy(oname, opfilter);
        if (NewCopy==2) {
          file = iname;
          if (strchr(iname, '\\'))  file = strrchr(iname, '\\')+1;
          strcat(oname, file);
          file = strchr(oname, '.');
          if (!file)  file = oname+strlen(oname);
          if (part) {
            file[0] = 0;
            name = oname+strlen(opfilter);
            sprintf(pnum, "%d", part);
            if (strlen(pnum)+strlen(name)>8)
              file = name+8-strlen(pnum);
            strcpy(file, pnum);
            file += strlen(file); }
          sprintf(file, ".%s", extlist2[SaveFilt-1]); }
        if (strchr(iname, '\\'))
          strpath(opendir+256*(NewCopy-1), iname);
        SetDlgItemText(hdlg, BatchStat3, oname);
        ppart = part;
        LoadPart = -1;
        cursor(1);
        Minpwidth = Minpheight = 0;
        new = load_graphic(iname);
        cursor(0);
        pw = pwidth;  ph = pheight;  pi = pinter;
        multi = pspec[1];
        strcpy(workfile, iname);
        if (!new && (Msg&1)) {
          if (TimerOn) KillTimer(hdlg, 0); TimerOn = 0;
          MsgBox(Hwnd, ErrorMsg[perr>>8], "Error", MB_OK);
          free2(iname);
          EndDialog(hdlg, 1); break; }
        if (!new)
          next = 1; break; }
      if (new) {
        sprintf(iname, "%sing file %d of %d.", val[NewCopy-2], cur+1, num);
        SetDlgItemText(hdlg, BatchStat1, iname);
        gspec[0] = SaveFilt-1;
        gspec[1] = (NewCopy==3);
        memcpy(gspec+2, SaveRec+SAVELEN*(SaveFilt-1), SAVELEN);
        pwidth = pw;  pheight = ph;  pinter = pi;
        cursor(1);
        if (gspec[0]==5)
          ((long *)(gspec+0xC))[0] = (long)(gquant);
        half = 0;
        if (HalfSizeBatch)
          half = scale_graphic(pwidth, pheight, pi, new, pwidth/2, pheight/2, 0);
        if (MultiSave && multi && (gspec[0]==1 || gspec[0]==2 || gspec[0]==4 ||
            gspec[0]==5 || gspec[0]==9) && !half && strcmp(workfile, oname)) {
          strcpy(oldfile, curfile);
          strcpy(curfile, workfile);
          multi_save(oname, gspec);
          strcpy(curfile, oldfile);
          i = 0; }
        else if (half) {
          pwidth /= 2;  pheight /=2;
          i = save_graphic(oname, half, gspec, 1); }
        else
          i = save_graphic(oname, new, gspec, 0);
        cursor(0);
        if (i && (Msg&1)) {
          if (TimerOn) KillTimer(hdlg, 0); TimerOn = 0;
          MsgBox(Hwnd, ErrorMsg2[i-1], "Error", MB_OK);
          free2(new);  new = 0;  free2(iname);
          EndDialog(hdlg, 1); break; }
        next = 1; }
      if (next) {
        if (new)  free2(new);
        if (new && MultiOpen && (pspec[1]&0x80))
          part ++;
        else {
          part = 0;  cur ++;
          strcpy(iname, opfile);
          if (find_space(iname)) {
            find_space(iname)[0] = 0;
            i = strlen(iname)+1;
            if (strchr(iname, '\\'))  strrchr(iname, '\\')[0] = 0;
            if (iname[0]=='"')  strcat(iname, "\" ");
            else                strcat(iname, " ");
            strcat(iname, opfile+i);
            join_names(iname);
            strcpy(opfile, iname); } }
        new = 0;
        if (cur<num) break; }
    case WM_COMMAND: if (msg==WM_COMMAND && (wp&0xFFFF)!=IDCANCEL) break;
      if (TimerOn) KillTimer(hdlg, 0); TimerOn = 0;
      if (new)  free2(new);
      free2(iname);
      EndDialog(hdlg, 1); }
  return(0);
}

void convert_grey(void)
/* Convert the current picture to a greyscale picture.         6/15/96-DWM */
{
  uchar pal[768], *new;
  long i, t, c, j, bw=oldi, s, unit, size=oldw*oldh;

  if (oldi) {
    for (i=0; i<256; i++) {
      if (pic[i*3]!=pic[i*3+1] || pic[i*3]!=pic[i*3+2]) { bw = 0; break; }
      if (pic[i*3]!=0 && pic[i*3]!=255)  bw = 0; }
    if (i==256 && !bw)  return; }
  if (!bw) {
    if (Msg&4)
      if (MsgBox(Hwnd, "Greyscale conversion can not be undone.\nProceed with conversion?",
                     "Warning", MB_OKCANCEL)==IDCANCEL)
        return;
    cursor(1);
    if (oldi) {
      for (i=0; i<256; i++)
        pic[i*3] = pic[i*3+1] = pic[i*3+2] = pic[i*3]*0.3+pic[i*3+1]*0.59+
                                             pic[i*3+2]*0.11; }
    else {
      for (i=0; i<256; i++) {
        pal[i] = i*0.3;  pal[i+256] = i*0.59;  pal[i+512] = i*0.11; }
      if (oldw*oldh<256) {
        if (!(new=malloc2(oldw*oldh+768))) {
          low_memory(Hwnd, "Greyscale Conversion", 0);  return; } }
      else
        new = pic;
      for (i=s=0; i<oldw*oldh; i++,s+=3)
        new[i] = pal[pic[s]]+pal[256+pic[s+1]]+pal[512+pic[s+2]];
      memmove(new+768, new, oldw*oldh);
      if (new!=pic) {
        free2(pic);  pic = new; }
      for (i=0; i<256; i++)
        pic[i*3] = pic[i*3+1] = pic[i*3+2] = i; }
    oldi = 1;
    rescale |= 2;  InvalidateRect(Hwnd, 0, 0);
    cursor(0);
    EnableMenuItem(Hmenu, MenuUndo, MFS_GRAYED);
    return; }
  Busy = 1;
  DialogBox(hinst, MAKEINTRESOURCE(GrDLG), Hwnd, (DLGPROC)grey_dialog);
  Busy = 0;
  if (Shade<2)  return;
  cursor(1);
  unit = oldh/(Shade-1);  while (unit*(Shade-1)<oldh)  unit++;
  unit *= oldw;
  for (i=0; i<256; i++)
    pal[i] = (pic[i*3]!=0);
  for (i=0; i<Shade && i<512; i++)
    pal[i+256] = i*255/(Shade-1);
  for (i=0; i<unit; i++) {
    for (t=c=0,j=i; j<size; j+=unit, t++)
      c += pal[pic[j+768]];
    if (t==Shade-1 && c<512)  pic[i+768] = pal[c+256];
    else if (t)               pic[i+768] = c*255/t; }
  oldh = unit/oldw;
  View[1] /= Shade-1;  View[3] /= Shade-1;
  View[5] /= Shade-1;  View[7] /= Shade-1;
  for (i=0; i<256; i++)
    pic[i*3] = pic[i*3+1] = pic[i*3+2] = i;
  zoom(1, 0);
  rescale |= 2;  InvalidateRect(Hwnd, 0, 0);
  cursor(0);
  EnableMenuItem(Hmenu, MenuUndo, MFS_GRAYED);
}

void copy(HWND hwnd, long full)
/* Copy the current image to the clipboard.
 * Enter: HWND hwnd: handle of window.
 *        long full: 0 for the current view, 1 for the whole image.
 *                                                             5/27/96-DWM */
{
  HGLOBAL gmem;
  char *dest, *src;
  long old=Scale, oldr=Reduce;

  cursor(1);
  if (OpenClipboard(hwnd)) {
    Scale = full;  Reduce = 0;  CopyMode = 1;
    rescale |= 2;  prep_pic(Hwnd);
    gmem = GlobalAlloc(GHND|GMEM_DDESHARE, GlobalSize(wpic));
    if (!gmem) {
      low_memory(hwnd, "Clipboard Allocation", 0);
      CopyMode = 0;
      Scale = old;  Reduce = oldr;  rescale |= 2;  prep_pic(Hwnd);
      cursor(0);
      return; }
    dest = GlobalLock(gmem);
    src = GlobalLock(wpic);
    memmove(dest, src, GlobalSize(wpic));
    GlobalUnlock(wpic);
    GlobalUnlock(gmem);
    EmptyClipboard();
    SetClipboardData(CF_DIB, gmem);
    CloseClipboard();
    CopyMode = 0;
    Scale = old;  Reduce = oldr;  rescale |= 2;  prep_pic(Hwnd); }
  cursor(0);
}

BOOL CALLBACK copy_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Actually copy or move files.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        3/22/97-DWM */
{
  static uchar *new, *iname, *oname;
  static long num, cur, left, newdir;
  char *s;
  long res, okay, i;

  switch (msg) {
    case WM_INITDIALOG: SetWindowText(hdlg, BatchTitle[PMode-1]);
      if (!(iname=malloc2(NAMELEN*2))) {
        low_memory(Hwnd, "File Processing", 0);
        EndDialog(hdlg, 1);  return(0); }
      oname = iname+NAMELEN;
      SetTimer(hdlg, 0, 0, 0);  TimerOn = 1;
      num = preview_deselect(3);  cur = 0;  left = 1;
      SetFocus(hdlg);
      if (PMode==3)
        for (newdir=0; newdir<slide->numdir; newdir++)
          if (!stricmp(opendir+1024, slide->dirname+
              ((long *)(slide->dir+newdir*6))[0]))
            break;
      return(1);
    case WM_TIMER: if (!TimerOn)  break;
      sprintf(iname, "Processing file %d of %d.", left, num);
      SetDlgItemText(hdlg, BatchStat1, iname);
      while (cur<slide->numfile && !(slide->file[cur*FE+0x2F]&2))
        cur++;
      if (cur<slide->numfile) {
        s = slide->file+cur*FE;
        sprintf(iname, "%s\\%s", slide->dirname+((long *)(slide->dir+
                ((short *)s)[0]*6))[0], slide->filename+((long *)(s+2))[0]);
        SetDlgItemText(hdlg, BatchStat2, iname);
        sprintf(oname, "%s\\%s", opendir+1024, slide->filename+((long *)(s+2))[0]);
        sprintf(oname, "%s\\I%04d-%s", opendir+1024, left, slide->filename+((long *)(s+2))[0]); //DWM::
        SetDlgItemText(hdlg, BatchStat3, oname);
        if (PMode==4) {
          if (!(okay=CopyFile(iname, oname, Msg&4)))
            if (GetLastError()==ERROR_ALREADY_EXISTS ||
                GetLastError()==ERROR_FILE_EXISTS) {
              TimerOn = 0;
              res = MsgBox(Hwnd, "The destination file already exists.  Overwrite it?",
                   "Copy Over Existing File?", MB_YESNOCANCEL);
              TimerOn = 1;
              if (res==IDCANCEL) {
                EndDialog(hdlg, 1);  free2(iname);  KillTimer(hdlg, 0);
                return(0); }
              if (res==IDYES)
                okay = CopyFile(iname, oname, 0);
              else
                okay = 1; }
          if (!okay) {
            KillTimer(hdlg, 0);
            if (Msg&1)
              MsgBox(HwndP, "Can't copy file.", "Error", MB_OK);
            KillTimer(hdlg, 0);  EndDialog(hdlg, 1);  free2(iname);
            return(0); }
          else {
            res = LastNum;  LastNum = cur;
            preview_invalidate(1, oname, 0);
            LastNum = res; } }
        else {
          if (!MoveFile(iname, oname)) {
            KillTimer(hdlg, 0);
            if (Msg&1)
              MsgBox(HwndP, "Can't move file.", "Error", MB_OK);
            EndDialog(hdlg, 1);  free2(iname);
            return(0); }
          if (newdir<slide->numdir)
            ((short *)s)[0] = newdir; }
        cur++;  left++;  if (cur<slide->numfile) break; }
    case WM_COMMAND: if (msg==WM_COMMAND && (wp&0xFFFF)!=IDCANCEL) break;
      KillTimer(hdlg, 0);  TimerOn = 0;
      free2(iname);
      EndDialog(hdlg, 1); }
  return(0);
}

void copy_file(long copy)
/* Copy or move the selected files to another directory.
 * Enter: long copy: 0 to move files, 1 to copy them.          3/22/97-DWM */
{
  long i;

  if (!preview_deselect(1)) {
    if (slide->tsel<0)  return;
    slide->file[slide->tsel*FE+0x2F] |= 6; }
  strcpy(opfile, "----");
  strcpy(opfilter, "All files\0*.*\0");
  openfile.lStructSize = sizeof(OPENFILENAME);
  openfile.hwndOwner = HwndP;  openfile.hInstance = hinst;
  openfile.lpstrFilter = opfilter;
  openfile.nFilterIndex = 1;
  openfile.lpstrFile = opfile;
  openfile.nMaxFile = NAMELEN;
  openfile.lpstrFileTitle = openfile.lpstrCustomFilter = 0;
  openfile.lpstrInitialDir = opendir+1024;
  openfile.lpfnHook = (DLGPROC)open_slide_dialog;
  openfile.lpTemplateName = MAKEINTRESOURCE(OpDLG);
  PMode = copy+3;
  openfile.Flags = OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_HIDEREADONLY |
                   OFN_NONETWORKBUTTON;
  Busy = 1;
  if (!GetOpenFileName(&openfile)) { Busy = 0;  return; }
  Busy = 0;
  strpath(opendir+1024, opfile);
  remove_quotes(opfile);
  if (strrchr(opfile, '\\'))  strrchr(opfile, '\\')[0] = 0;
  DialogBox(hinst, MAKEINTRESOURCE(BaDLG), HwndP, (DLGPROC)copy_dialog);
  preview_deselect(1);
  preview_rectangles(0);
}

void copy_preview(long full)
/* Copy the preview thumbnails to the clipboard as a bmp.
 * Enter: long full: 0 for selection, 1 for full preview.      3/26/97-DWM */
{
  RECT rect;
  long w, h, i, j, d, scan, pal;
  HDC hdc, hdc2;
  HBITMAP membmp, oldbmp;
  HGLOBAL gmem;
  char *dest, *buf;
  char fail[]="Can't create clipboard image.";
  LPBITMAPINFOHEADER head;

  GetClientRect(HwndP, &rect);
  w = rect.right;
  if (w<320)  w = 320;
  h = preview_page(0, w, 0, -2, full);
  hdc2 = GetDC(HwndP);
  hdc = CreateCompatibleDC(hdc2);
  membmp = CreateCompatibleBitmap(hdc2, w, h);
  ReleaseDC(HwndP, hdc2);
  if (!membmp) {
    DeleteDC(hdc);
    if (Msg&1)  MsgBox(HwndP, fail, "Error", MB_OK);
    return; }
  oldbmp = SelectObject(hdc, membmp);
  cursor(1);
  preview_page(hdc, w, h, 0, full);
  SelectObject(hdc, oldbmp);
  gmem = BitmapToBMP(membmp, 0, hdc);
  DeleteDC(hdc);
  DeleteObject(membmp);
  if (OpenClipboard(HwndP)) {
    EmptyClipboard();
    SetClipboardData(CF_DIB, gmem);
    CloseClipboard(); }
  cursor(0);
}

void create_savepic(HWND hdlg, HDC hdc)
/* Create and display the thumbnail pictures for a save dialog.
 * Enter: HWND hdlg: handle to dialog to work with.
 *        HDC hdc: device context handle to draw with.         10/2/96-DWM */
{
  long pal=1, i, w, h, fit=(PreviewOpt&2)>>1, trans, x, y;
  RECT rect[2];
  HBITMAP wpic;
  HPALETTE wpal;

  create_savepic_rect(0, &rect[0], hdlg);
  create_savepic_rect(1, &rect[1], hdlg);
  w = rect[0].right-rect[0].left;  h = rect[0].bottom-rect[0].top;
  memcpy(PreviewArea+4, View+4, 4*sizeof(long));
  if (!(PreviewOpt&1)) {
    PreviewArea[4]=PreviewArea[5]=0;  PreviewArea[6]=oldw;
    PreviewArea[7]=oldh; }
  create_savepic_area(w, h);
  switch (PreviewOpt&0xC) {
    case 0: trans = PreviewTrans; break;
    case 4: trans = PreviewInv; break;
    default: trans = -1; }
  if (trans!=-1 && !oldi)  bgr_to_rgb((uchar *)&trans, 1);
  wpic = prep_pic2(PreviewArea+4*fit, w, h, 1, -1, &pal, &wpal);
  if ((PreviewOpt&0xC)>4)
    create_savepic_cross(hdlg, wpic);
  if (wpal) {
    PalChange = 1;
    SelectPalette(hdc, wpal, 0);
    RealizePalette(hdc); }
  draw_bmp(hdc, rect[0].left, rect[0].top, w, h, pal, wpic);
  GlobalFree(wpic);
  if (wpal) { DeleteObject(wpal);  wpal = 0; }
  if ((PreviewOpt&0xC)>4)  return;
  wpic = prep_pic2(PreviewArea+4*fit, w, h, 1, trans, &pal, &wpal);
  if (wpal) {
    PalChange = 1;
    SelectPalette(hdc, wpal, 0);
    RealizePalette(hdc); }
  draw_bmp(hdc, rect[1].left, rect[1].top, w, h, pal, wpic);
  GlobalFree(wpic);
  if (wpal) { DeleteObject(wpal);  wpal = 0; }
}

void create_savepic_area(long w, long h)
/* Constrain the preview picture to a sensible area of the image.
 * Enter: long w, h: size of preview area.                     10/4/96-DWM */
{
  if (PreviewArea[0]>PreviewArea[6]-w)  PreviewArea[0] = PreviewArea[6]-w;
  if (PreviewArea[1]>PreviewArea[7]-h)  PreviewArea[1] = PreviewArea[7]-h;
  if (PreviewArea[0]<PreviewArea[4])  PreviewArea[0] = PreviewArea[4];
  if (PreviewArea[1]<PreviewArea[5])  PreviewArea[1] = PreviewArea[5];
  PreviewArea[2] = PreviewArea[0] + w;
  PreviewArea[3] = PreviewArea[1] + h;
  if (PreviewArea[2]>PreviewArea[6])  PreviewArea[2] = PreviewArea[6];
  if (PreviewArea[3]>PreviewArea[7])  PreviewArea[3] = PreviewArea[7];
}

void create_savepic_cross(HWND hdlg, HBITMAP wpic)
/* Determine if the cursor is in the preview window, and, if so, adjust the
 *  appropriate item to the selected value.
 * Enter: HWND hdlg: dialog handle.
 *        HBITMAP wpic: image to adjust.                         10/4/96-DWM */
{
  RECT rect;
  long w, h, iw, ih, ix, iy, fit=(PreviewOpt&2)>>1, b=0, c=255, i, s, x, y;
  float scale;
  uchar *pic;

  create_savepic_rect(0, &rect, hdlg);
  ix = PreviewXY[0];  iy = PreviewXY[1];
  w = rect.right-rect.left;  h = rect.bottom-rect.top;
  if (!w) w = 1;  if (!h) h = 1;
  iw = PreviewArea[2+fit*4]-PreviewArea[0+fit*4];
  ih = PreviewArea[3+fit*4]-PreviewArea[1+fit*4];
  if (iw<=w && ih<=h) {
    x = ix-PreviewArea[0+fit*4]+(w-iw)/2;
    y = iy-PreviewArea[1+fit*4]+(h-ih)/2; }
  else {
    scale = (float)iw/w;  if (ih/h>scale)  scale = (float)ih/h;
    if (!scale)  scale = 1;
    x = (ix-PreviewArea[0+fit*4])/scale+(w-iw/scale)/2;
    y = (iy-PreviewArea[1+fit*4])/scale+(h-ih/scale)/2; }
  pic = GlobalLock(wpic);
  y = (h-1)-y;  s = ((long *)pic)[5]/h;
  if (((long *)pic)[8]) {
    for (i=0; i<256; i++) {
      if (!pic[i*4+40] && !pic[i*4+41] && !pic[i*4+42])              b = i;
      if (pic[i*4+40]==255 && pic[i*4+41]==255 && pic[i*4+42]==255)  c = i; }
    for (i=0; i<w; i++) if (i+3<x || i-3>x) {
      if (y-1>=0 && y-1<h)  pic[1064+(y-1)*s+i] = b;
      if (y  >=0 && y  <h)  pic[1064+ y   *s+i] = c;
      if (y+1>=0 && y+1<h)  pic[1064+(y+1)*s+i] = b; }
    for (i=0; i<h; i++)  if (i+3<y || i-3>y) {
      if (x-1>=0 && x-1<h)  pic[1064+i*s+x-1] = b;
      if (x  >=0 && x  <h)  pic[1064+i*s+x  ] = c;
      if (x+1>=0 && x+1<h)  pic[1064+i*s+x+1] = b; } }
  else {
    for (i=0; i<w; i++) if (i+3<x || i-3>x) {
      if (y-1>=0 && y-1<h)  pic[40+(y-1)*s+i*3]=pic[40+(y-1)*s+i*3+1]=
                                                pic[40+(y-1)*s+i*3+2]=0;
      if (y  >=0 && y  <h)  pic[40+ y   *s+i*3]=pic[40+ y   *s+i*3+1]=
                                                pic[40+ y   *s+i*3+2]=255;
      if (y+1>=0 && y+1<h)  pic[40+(y+1)*s+i*3]=pic[40+(y+1)*s+i*3+1]=
                                                pic[40+(y+1)*s+i*3+2]=0; }
    for (i=0; i<h; i++)  if (i+3<y || i-3>y) {
      if (x-1>=0 && x-1<h)  pic[40+i*s+(x-1)*3]=pic[40+i*s+(x-1)*3+1]=
                                                pic[40+i*s+(x-1)*3+2]=0;
      if (x  >=0 && x  <h)  pic[40+i*s+(x  )*3]=pic[40+i*s+(x  )*3+1]=
                                                pic[40+i*s+(x  )*3+2]=255;
      if (x+1>=0 && x+1<h)  pic[40+i*s+(x+1)*3]=pic[40+i*s+(x+1)*3+1]=
                                                pic[40+i*s+(x+1)*3+2]=0; } }
  GlobalUnlock(wpic);
}

void create_savepic_grob(HWND hdlg, HDC hdc)
/* Create and display the preview picture for the grob save dialog.
 * Enter: HWND hdlg: handle to dialog to work with.
 *        HDC hdc: device context handle to draw with.         10/2/96-DWM */
{
  RECT rect;
  HBITMAP wpic;
  HPALETTE wpal;
  long w, h, i, j, pal, lasti;
  uchar *lastpic;

  create_savepic_rect(2, &rect, hdlg);
  w = rect.right-rect.left;  h = rect.bottom-rect.top;
  memcpy(PreviewArea+4, View+4, 4*sizeof(long));
  if (!(PreviewOpt&1)) {
    PreviewArea[4]=PreviewArea[5]=0;  PreviewArea[6]=oldw;
    PreviewArea[7]=oldh; }
  for (i=0; i<256; i++) {
    if (PreviewGamma>0)  j = 255*pow((float)i/255, PreviewGamma);
    else                 j = i;
    if (PreviewInv>=2 && PreviewInv<256)
      j = ((long)(j*PreviewInv/256))*255/(PreviewInv-1);
    PreviewPic[i*3] = PreviewPic[i*3+1] = PreviewPic[i*3+2] = j; }
  lasti = oldi;  lastpic = pic;  pic = PreviewPic;  oldi = 1;
  wpic = prep_pic2(PreviewArea+4, w, h, 1, -1, &pal, &wpal);
  pic = lastpic;  oldi = lasti;
  if (wpal) {
    PalChange = 1;
    SelectPalette(hdc, wpal, 0);
    RealizePalette(hdc); }
  draw_bmp(hdc, rect.left, rect.top, w, h, pal, wpic);
  GlobalFree(wpic);
  if (wpal) { DeleteObject(wpal);  wpal = 0; }
}

void create_savepic_rect(long num, RECT *rect, HWND hdlg)
/* Locate the rectangle used in a save preview.
 * Enter: long num: number of rectangle to create: 0-left preview, 1-right
 *                  preview, 2-grob preview, 3-left and right previews.
 *        RECT *rect: pointer to store result.  If null, the rectangle is
 *                    invalidated (refreshed).
 *        HWND hdlg: dialog the rectangle is located in.       10/7/96-DWM */
{
  long l=num, r=num, inv=0;
  RECT rect2;

  if (num==3) { l = 0;  r = 1; }
  if (!rect) { rect = &rect2;  inv = 1; }
  rect->left = PreviewCoor[l*4];
  rect->top = PreviewCoor[l*4+1];
  rect->right = PreviewCoor[r*4]+PreviewCoor[r*4+2];
  rect->bottom = PreviewCoor[r*4+1]+PreviewCoor[r*4+3];
  MapDialogRect(hdlg, rect);
  if (inv)
    InvalidateRect(hdlg, rect, 0);
}

void cursor(long wait)
/* Set the cursor to either the arrow or the wait cursor.
 * Enter: long wait: 0 for arrow, 1 for wait.                  5/26/96-DWM */
{
  HANDLE cur;

  if (Down==2)  wait = 2;
  switch (wait) {
    case 1: if (WholeScreen) {
        SetCursor(cur=LoadCursor(hinst, MAKEINTRESOURCE(CursorNull))); break; }
      SetCursor(cur=LoadCursor(0, IDC_WAIT)); break;
    case 2:  SetCursor(cur=LoadCursor(0, IDC_SIZE)); break;
    default: if (WholeScreen) {
        SetCursor(cur=LoadCursor(hinst, MAKEINTRESOURCE(CursorNull))); break; }
      SetCursor(cur=LoadCursor(0, IDC_ARROW)); }
  SetClassLong(Hwnd, GCL_HCURSOR, (long)cur);
}

void draw_bmp(HDC hdc, long x, long y, long w, long h, long pal, HANDLE bmp)
/* Draw a BMP format graphic.
 * Enter: HDC hdc: device context.
 *        long x, y: location to draw BMP.
 *        long w, h: size of graphic to draw
 *        long pal: 0 for RGB, 1 for palettized image.
 *        HANDLE bmp: handle to BMP format graphic.             5/8/96-DWM */
{
  LPBITMAPINFOHEADER header;
  LPSTR buf;

  header = GlobalLock(bmp);
  buf = (LPSTR)header + header->biSize + 1024*pal;
  SetDIBitsToDevice(hdc, x, y, w, h, 0, 0, 0, h, buf, (LPBITMAPINFO)header,
                    DIB_RGB_COLORS);
  GlobalUnlock(bmp);
}

void draw_text(HDC dc, char *text, long x, long y, long w, long h)
/* Print some text at the specified location.  The text is centered, but if
 *  it does not fit, it is left justified.
 * Enter: HDC dc: device context to draw in.
 *        char *text: text to print.
 *        long x, y: location to print.
 *        long w, h: area for text.                            3/11/97-DWM */
{
  RECT rect, rect2;
  long flags=DT_NOPREFIX|DT_SINGLELINE|DT_LEFT, nw;

  rect.left = x;  rect.top = y;
  rect.right = x+w;  rect.bottom = y+h;
  memcpy(&rect2, &rect, sizeof(rect));
  DrawText(dc, text, strlen(text), &rect2, flags|DT_CALCRECT);
  if (rect2.right-rect2.left<w) {
    nw = w-(rect2.right-rect2.left);
    rect.left = x+nw/2;  rect.right = x+w-nw/2; }
  DrawText(dc, text, strlen(text), &rect, flags);
}

void drop_file(HANDLE drop)
/* Handle dropped-in files just as if they were opened.        10/1/96-DWM */
{
  long n, i, len=0;
  char name[NAMELEN], name2[NAMELEN];
  RECT rect;

  prep_undo(Hwnd);
  if (TimerOn) {
    DragFinish(drop);  return; }
  n = DragQueryFile(drop, -1, 0, 0);
  if (n==0) {
    DragFinish(drop);  return; }
  DragQueryFile(drop, 0, name2, NAMELEN);
  if (name2[0]!='"')  sprintf(opfile, "\"%s\"", name2);
  else                strcpy(opfile, name2);
  for (i=1; i<n && len<NAMELEN-1; i++) {
    name[len] = 0;
    DragQueryFile(drop, i, name2, NAMELEN-len-2);
    if (name2[0]!='"')  sprintf(name+strlen(name), "\"%s\"", name2);
    else                strcat(name, name2);
    len = strlen(name);
    if (i!=n-1 && len<NAMELEN-1) {
      name[len] = ' ';  len++;  name[len] = 0; } }
  DragFinish(drop);
  open_pic_mid(Hwnd, opfile, 0);
  if (n>1)
    open_pic_new(Hwnd, name);
  GetWindowRect(Hwnd, &rect);
  OrigX = rect.left;  OrigY = rect.top;
}

void end_ctl3d(void)
/* Terminate the faux-3D control functions, if started.        10/4/96-DWM */
{
  FARPROC reg;

  if (!hctl3d)  return;
  reg = GetProcAddress(hctl3d, "Ctl3dUnregister");
  (*reg)(hinst);
}

BOOL CALLBACK find_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the find dialog box.  Is done is selected, actually
 *  find the requested files.
 * Enter: HWND hdlg: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        6/11/96-DWM */
{
  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd, HelpFile, HELP_CONTEXT, HelpFind);  break;
      case IDOK: if (!Findtext)
          if (!(Findtext=malloc2(NAMELEN))) {
            EndDialog(hdlg, 0);
            low_memory(HwndP, "Finding image", 1); return(Busy=0); }
        FindType = SendDlgItemMessage(hdlg, FindBegin, BM_GETCHECK, 0, 0)+
                   2*SendDlgItemMessage(hdlg, FindSub, BM_GETCHECK, 0, 0);
        GetDlgItemText(hdlg, FindName, Findtext, NAMELEN-1);
        EndDialog(hdlg, 0);
        find_next();
        return(Busy=0);
      case IDCANCEL: EndDialog(hdlg, 0); return(Busy=0);
      default: return(0); }
    case WM_INITDIALOG: Busy = 1;
      SendDlgItemMessage(hdlg, FindExact, BM_SETCHECK, FindType==0, 0);
      SendDlgItemMessage(hdlg, FindBegin, BM_SETCHECK, FindType==1, 0);
      SendDlgItemMessage(hdlg, FindSub, BM_SETCHECK, FindType==2, 0);
      if (Findtext)
        SetDlgItemText(hdlg, FindName, Findtext);
      SetFocus(hdlg); break;
    default: return(0); }
  return(0);
}

long find_file(char *name, long type, long recurse, WIN32_FIND_DATA *find,
               char *path)
/* Locate a file, recursing subdirectories if requested.  Note that the
 *  maximum recursion is 20 subdirectories.  This requires 1k of data space.
 *  To repeat a call to find_file to locate the next file of a group, pass a
 *  null to the name, type, and recurse variables.  To cancel a find_file
 *  call, pass a null to all variables.
 * Enter: char *name: name including wildcards and possibly a path.  If the
 *                    first character of this is a null, then the second
 *                    character specifies how many extensions to search for.
 *                    Starting with the third character is the path, then
 *                    the extensions, which may be a maximum of three
 *                    characters and do not include the period.  The path and
 *                    extensions are separated by nulls.  A maximum of 63
 *                    extensions may be specified.
 *        long type: low byte contains required field values, high byte
 *                   contains field masks.  The fields are: bit 0-read only,
 *                   1-hidden, 2-system, 3-volume ID, 4-subdirectory,
 *                   5-archive.  0x1E00 locates only 'normal' files.
 *        long recurse: 0 for do not recurse subdirectories, 1 for recurse
 *                      all subdirectories, 2 for recurse only subdirectories
 *                      which would have been found by the name
 *                      specification.
 *        WIN32_FIND_DATA *find: structure to return information in.  0 for
 *                               no return.
 *        char *path: string to store path and name in.  This is the entire
 *                    file name, including path.  It must be a buffer at
 *                    least 256 bytes long, including the terminating null.
 *                    0 for no return.  There is never a terminating \ on the
 *                    path string.
 * Exit:  long none: 0 for file found, 1 for no more files.      6/7/95-DWM */
{
  #define maxrecursedepth 20
  static WIN32_FIND_DATA f[maxrecursedepth], *last=0;
  static HANDLE h[maxrecursedepth];
  static long depth, new, multi;
  static char root[256], spec[256], xtype, atype, rec;
  char fname[256], *temp;
  long i, found=0;

  if (!name && !path && !find) {
    for (i=0; i<=depth; i++)
      FindClose(h[depth]);
    return(0); }
  if (name) {
    if (!name[0]) {
      multi = name[1];  if (multi<0) multi=0;  if (multi>63) multi=63;
      strcpy(root, name+2);
      strcpy(spec, "*.*");
      name += 2+strlen(name+2)+1;
      for (i=0; i<multi; i++) {
        strcpy(spec+4+i*4, name);
        name += strlen(name)+1; }
      name = 0;
      depth = 0;  new = 1;
      rec = recurse;  atype = type>>8;  xtype = type&0x3F; } }
  if (name) {
    multi = 0;
    getcwd(root, 256-1);
    if (strchr(name, '\\')) {
      strcpy(spec, strrchr(name, '\\')+1);
      strrchr(name, '\\')[0] = 0;
      if (name[1]!=':' && name[0]!='\\') {
        if (strlen(name)+strlen(root)>256-2)
          name[256-1-strlen(root)-1] = 0;
        sprintf(root+strlen(root), "\\%s", name); }
      else
        strcpy(root, name); }
    else
      strcpy(spec, name);
    if (root[strlen(root)-1]=='\\')
      root[strlen(root)-1] = 0;
    depth = 0;  new = 1;
    rec = recurse;  atype = type>>8;  xtype = type&0x3F; }

  while (1) {
    if (new) {                                                /* find file */
      new = 0;
      strcpy(fname, root);
      for (i=0; i<depth && strlen(fname)<256; i++)
        sprintf(fname+strlen(fname), "\\%s", f[i].cFileName);
      if (path)
        strcpy(path, fname);
      sprintf(fname+strlen(fname), "\\%s", spec);
      h[depth] = INVALID_HANDLE_VALUE;
      f[depth].dwFileAttributes = 0x3F;
      if ((h[depth]=FindFirstFile(fname, &f[depth]))!=INVALID_HANDLE_VALUE)
          do {
        if (!((f[depth].dwFileAttributes&atype)^xtype) &&
            strcmp(f[depth].cFileName, ".")) {
          if (multi) {
            temp = strrchr(f[depth].cFileName, '.');
            if (temp)  temp++;
            for (i=0; i<multi && temp; i++)
              if (!stricmp(temp, spec+i*4+4))
                found = 1; }
          else
            found = 1; }
        if (found) {
          if (find)
            memcpy(find, &f[depth], sizeof(WIN32_FIND_DATA));
          if (path)
            sprintf(path+strlen(path), "\\%s", f[depth].cFileName);
          return(0); } }
      while (FindNextFile(h[depth], &f[depth]));
      if (h[depth]!=INVALID_HANDLE_VALUE)
        FindClose(h[depth]); }
    else {
      while (FindNextFile(h[depth], &f[depth])) {
        if (!((f[depth].dwFileAttributes&atype)^xtype) &&
            strcmp(f[depth].cFileName, ".")) {
          if (multi) {
            temp = strrchr(f[depth].cFileName, '.');
            if (temp)  temp++;
            for (i=0; i<multi && temp; i++)
              if (!stricmp(temp, spec+i*4+4))
                found = 1; }
          else
            found = 1; }
        if (found) {
          if (find)
            memcpy(find, &f[depth], sizeof(WIN32_FIND_DATA));
          if (path) {
            sprintf(path, "%s", root);
            for (i=0; i<depth && strlen(fname)<256; i++)
              sprintf(path+strlen(path), "\\%s", f[i].cFileName); }
          if (path)
            sprintf(path+strlen(path), "\\%s", f[depth].cFileName);
          return(0); } }
      if (h[depth]!=INVALID_HANDLE_VALUE)
        FindClose(h[depth]); }
    if (!rec)  return(1);                              /* find directories */
    sprintf(fname, "%s\\", root);
    for (i=0; i<depth && strlen(fname)<256; i++)
      sprintf(fname+strlen(fname), "%s\\", f[i].cFileName);
    if (rec==2)  strcat(fname, spec);
    else         strcat(fname, "*.*");
    if (depth<maxrecursedepth) {
      h[depth] = INVALID_HANDLE_VALUE;
      f[depth].dwFileAttributes = 0x10;
      if ((h[depth]=FindFirstFile(fname, &f[depth]))!=INVALID_HANDLE_VALUE)
          do
        if ((f[depth].dwFileAttributes&0x10) && strcmp(f[depth].cFileName,
            ".") && strcmp(f[depth].cFileName, "..")) {
          depth++;  new = 1;  break; }
      while (FindNextFile(h[depth], &f[depth]));
      if (h[depth]!=INVALID_HANDLE_VALUE && !new)
        FindClose(h[depth]); }
    while (!new) {
      if (!depth) return(1);
      depth--;
      while (FindNextFile(h[depth], &f[depth]))
        if ((f[depth].dwFileAttributes&0x10) && strcmp(f[depth].cFileName,
            ".") && strcmp(f[depth].cFileName, "..")) {
          depth++;  new = 1;  break; }
      if (h[depth]!=INVALID_HANDLE_VALUE && !new)
        FindClose(h[depth]); } }
}

void find_next(void)
/* Find the name already specfied in the find dialog.  If no name has ever
 *  been specified, pull up the find dialog.                   3/22/97-DWM */
{
  long i, j, k, okay, found=-1, start;
  char *name;

  if (!Findtext) {
    DialogBox(hinst, MAKEINTRESOURCE(FindDLG), HwndP, (DLGPROC)find_dialog);
    return; }
  preview_deselect(0);
  start = max(slide->tsel, 0);
  for (i=slide->numfile+start; i>start; i--) {
    j = i%slide->numfile;
    name = slide->filename+((long *)(slide->file+j*FE+2))[0];
    switch (FindType) {
      case 0: okay = !stricmp(Findtext, name); break;
      case 1: okay = !strnicmp(Findtext, name, strlen(Findtext)); break;
      case 2: for (k=0; k<strlen(name); k++)
          if (okay=!strnicmp(Findtext, name+k, strlen(Findtext))) break; }
    if (((long *)(slide->file+j*FE+0x40))[0]<0)
      okay = 0;
    if (okay) {
      slide->file[j*FE+0x2F] |= 2;
      slide->tsel = found = j; } }
  if (found==-1) {
    preview_rectangles(0);
    if (Msg&1)
      MsgBox(HwndP, "No images were found.", "Error", MB_OK);
    return; }
  for (i=0; i<PreCoor[0] && i<PreCoor[2]*PreCoor[3]; i++)
    if (slide->tsel==PreCoor[i*2+4])  break;
  if (i==PreCoor[0] || i==PreCoor[2]*PreCoor[3]) {
    slide->ttop = (((long *)(slide->file+slide->tsel*FE+0x40))[0]/
                   PreCoor[2])*PreCoor[2];
    InvalidateRect(HwndP, 0, 1); }
  else
    preview_rectangles(0);
}

char *find_space(char *text)
/* Locate the first space in a list of files.  Note that this does not count
 *  spaces in long file names.  Any long file name must be surrounded by
 *  quotes.
 * Enter: char *text: text to search.
 * Exit:  char *space: location of first space, or zero for none.
 *                                                             11/9/96-DWM */
{
  long first=1;

  while (text[first-1]=='"')  if (text[first]=='"') first++; else break;
  if (text[0]=='"')
    if (strchr(text+first, '"'))
      text = strchr(text+first, '"');
  return(strchr(text, ' '));
}

BOOL CALLBACK grey_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the greyscale conversion dialog box.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        5/26/96-DWM */
{
  char text[80];

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd, HelpFile, HELP_CONTEXT, HelpGreyscale);
        break;
      case IDOK: GetDlgItemText(hdlg, GreyShade, text, 79);
        sscanf(text, "%d", &Shade);
        if (Shade<2) {
          if (Msg&1)
            MsgBox(hdlg, "The number of shades must\n"
                         "be a positive integer.", "Error", MB_OK);
          return(0); }
        EndDialog(hdlg, 1); return(0);
      case IDCANCEL: Shade = 0;  EndDialog(hdlg, 1); return(1);
      default: return(0); } break;
    case WM_INITDIALOG: Shade = oldh/64+1;  if (oldh%64)  Shade = 3;
      if (Shade<2)  Shade = 2;
      sprintf(text, "%d", Shade);
      SetDlgItemText(hdlg, GreyShade, text);
      SetFocus(hdlg);
      return(1); }
  return(0);
}

void image_switch(long dir)
/* Switch images in a multi-part image file.
 * Enter: long dir: -1 for previous image, 0 for first image, 1 for next
 *                  image.  If this is -1, check if the CTRL key is held
 *                  down, in which case go to the first image.12/20/01-DWM */
{
  long num=0;
  char name[512];

  if (!curfile[0] || !oldspec[1])  return;
  if (dir==-1)  if ((GetKeyState(VK_CONTROL)&0x8000)!=0)  dir = 0;
  if (!(oldspec[1]&0x7F) && dir<=0)  return;
  if (!(oldspec[1]&0x80) && dir==1)  return;
  if (dir==-1)  num = (oldspec[1]&0x7F)-1;
  if (dir==1)   num = (oldspec[1]&0x7F)+1;
  strcpy(name, curfile);
  if (strchr(name, '?'))  strchr(name, '?')[0] = 0;
  sprintf(name+strlen(name), "?%d", num+1);
  open_pic_mid(Hwnd, name, 0);
}

void info(HWND hwnd, long prev)
/* Present a dialog containing information about the current picture.
 * Enter: HWND hwnd: owner window for this dialog.
 *        long prev: 0 for current window, 1 for selected preview file.
 *                                                             10/8/96-DWM */
{
  char text[3000], *filt, *infspec=oldspec, *s;
  char *chroma[]={"1x1:1x1:1x1","2x1:1x1:1x1","2x2:1x1:1x1","4x2:1x1:1x1"};
  char *end[]={"Big endian (Motorola style)","Little endian (Intel style)"};
  char *lace[]={No, Yes};
  char *asc[]={"ASCII", "Binary"};
  char *base[]={"Baseline","Extended","Progressive"};
  long i, j, l;

  text[0] = 0;
  if (prev && !slide)  return;
  if (!prev) {
    if (curfile[0])
      sprintf(text, "File: %s\n\n", curfile); }
  else {
    s = slide->file+slide->tsel*FE;
    sprintf(text, "File: %s\\%s\n\n", slide->dirname+((long *)(slide->dir+
            ((short *)s)[0]*6))[0], slide->filename+((long *)(s+2))[0]);
    infspec = s+0xC; }
  if (infspec[1])
    sprintf(text+strlen(text), "Image %d of multi-image file\n\n",
            (((uchar *)infspec)[1]&0x7F)+1);
  if (!prev) {
    sprintf(text+strlen(text), "Picture size: %d x %d\nSelected area: ",
            oldw, oldh);
    if (!View[4] && !View[5] && View[6]==oldw && View[7]==oldh)
      strcat(text, "all");
    else
      sprintf(text+strlen(text), "%d,%d,%d,%d", View[4], View[5], View[6],
              View[7]);
    strcat(text, "\nViewable area: ");
    if (!View[0] && !View[1] && View[2]==oldw && View[3]==oldh)
      strcat(text, "all selected");
    else
      sprintf(text+strlen(text), "%d,%d,%d,%d", View[0], View[1], View[2],
              View[3]);
    sprintf(text+strlen(text), "\n\nBit depth: %d\n\nFile format: ",
            24-16*oldi); }
  else if (((short *)(s+6))[0]) {
    sprintf(text+strlen(text), "Picture size: %d x %d\nSelected area: ",
            ((short *)(s+6))[0], ((short *)(s+6))[1]);
    if (!((short *)(s+0x24))[0] && !((short *)(s+0x24))[1] &&
        ((short *)(s+0x24))[2]==((short *)(s+6))[0] &&
        ((short *)(s+0x24))[3]==((short *)(s+6))[1])
      strcat(text, "all");
    else
      sprintf(text+strlen(text), "%d,%d,%d,%d", ((short *)(s+0x24))[0],
              ((short *)(s+0x24))[1], ((short *)(s+0x24))[2],
              ((short *)(s+0x24))[3]);
    sprintf(text+strlen(text), "\n\nBit depth: %d\n\nFile format: ",
            24-16*((short *)(s+6))[2]); }
  else
    sprintf(text+strlen(text), "\n\nFile format: ");
  if (infspec[0]<0)
    sprintf(text+strlen(text), "unknown");
  else {
    for (i=0, filt=SaveFilter; i<infspec[0]*2; i++)
      filt = strchr(filt, '|')+1;
    strcat(text, filt);
    strchr(text, '|')[0] = 0;
    if (!infspec[0] || infspec[0]==9)
      sprintf(text+strlen(text), "\nCompression: %s", SaveComp[infspec[3]]);
    if (((short *)(infspec+4))[0])
      sprintf(text+strlen(text), "\nDPI: %d", ((short *)(infspec+4))[0]);
    switch (infspec[0]) {
      case 0: sprintf(text+strlen(text), "\nFormat: %d", 40-28*infspec[6]);
        break;
      case 1: case 4: sprintf(text+strlen(text), "\nTransparent color: %d",
                              ((long *)(infspec+8))[0]);
        sprintf(text+strlen(text), "\nInverse color: %d",
                              ((long *)(infspec+12))[0]);  break;
      case 2: sprintf(text+strlen(text), "\nFormat: GIF%da\nInterlacing: %s\nTransparent color: ",
                      87+2*infspec[6], lace[infspec[7]]);
        if (infspec[2])
          sprintf(text+strlen(text), "%d", ((long *)(infspec+8))[0]);
        else
          sprintf(text+strlen(text), "Not used");
        if (((short *)(infspec+16))[0]>1)
          sprintf(text+strlen(text), "\nRepeat image sequence: %d times",
                  ((short *)(infspec+16))[0]);
        if (((short *)(infspec+12))[0])
          sprintf(text+strlen(text), "\nInter-image delay: %4.2f sec",
                  ((short *)(infspec+12))[0]*0.01); break;
      case 3: sprintf(text+strlen(text), "\nCalculated number of shades: %d",
                              ((long *)(infspec+12))[0]);
      case 6: sprintf(text+strlen(text), "\nFormat: %s", asc[infspec[6]]);
        break;
      case 5: sprintf(text+strlen(text), "\nCompression: %s",
                      base[infspec[3]]);
        sprintf(text+strlen(text), "\nChroma decimation: %s",
                chroma[infspec[6]]);
        sprintf(text+strlen(text), "\nQuality: %d",
                              ((long *)(infspec+8))[0]);  break;
      case 9: sprintf(text+strlen(text), "\nEndian: %s", end[infspec[6]]);
        sprintf(text+strlen(text), "\nRows per strip: %d",
                              ((long *)(infspec+8))[0]); }
    if (infspec[0]==1)
      sprintf(text+strlen(text), "\nHot spot: %d, %d",
              ((short *)(infspec+16))[0], ((short *)(infspec+16))[1]); }
  if (LastNum>=0 || prev) {
    if (!prev)  l = LastNum;
    else        l = slide->tsel;
    for (i=j=0; i<30; i++)
      if (((short *)(slide->cat+i*8+4))[0])
        if ((((ulong *)(slide->file+FE*l+0x20))[0]&(1<<i))!=0) {
          if (!j)  sprintf(text+strlen(text), "\n\nCategories:\n");
          else     sprintf(text+strlen(text), "; ");
          j++;
          sprintf(text+strlen(text), "%s", slide->catname+
                  ((long *)(slide->cat+i*8))[0]); }
    s = slide->file+l*FE;
    sprintf(text+strlen(text), "\n\nFile size: %d bytes\n\nFile date: %d/%d/%d %d:%02d:%02d",
            ((long *)(s+0x38))[0], (((short *)(s+0x3E))[0]>>5)&0xF,
            s[0x3E]&0x1F, (((ushort *)(s+0x3E))[0]>>9)+1980,
            ((ushort *)(s+0x3C))[0]>>11, (((ushort *)(s+0x3C))[0]>>5)&0x3F,
            (s[0x3C]&0x1F)<<1); }
  MsgBox(hwnd, text, "Image Info", MB_OK);
}

void join_names(char *text)
/* Combine a directory and a file name which are spearated by a space.
 *  Either portion may be surrounded by quotes, in which case the final part
 *  will have quotes.
 * Enter: char *text: text to join.  There may be additional text after the
 *                    file name.                               11/9/96-DWM */
{
  char *loc, *loc2;

  if (loc=find_space(text)) {
    if (text[0]=='"' && loc[1]=='"') {
      loc[-1] = '\\';  memmove(loc, loc+1, strlen(loc+1)); }
    else if (text[0]=='"') {
      if (!(loc2=find_space(loc+1)))  loc2 = loc + 1 + strlen(loc+1);
      loc[-1] = '\\';  memmove(loc, loc+1, loc2-loc-1);  loc2[-1] = '"'; }
    else if (loc[1]=='"') {
      memmove(text+1, text, loc-text);
      text[0] = '"';  loc[1] = '\\'; }
    else
      loc[0] = '\\'; }
}

void low_memory(HWND hwnd, char *text, long end)
/* Report on an out-of-memory problem, then bail.
 * Enter: HWND hwnd: handle of window.
 *        char *text: text of what was occuring at this time.  0 for none.
 *        long end: 0 to return, 1 to bail.                    5/26/96-DWM */
{
  char t2[256];
  static char *last;

  cursor(0);
  if ((last==text || !(Msg&1)) && !end)  return;
  last = text;
  strcpy(t2, "Insufficient memory.");
  if (text)  sprintf(t2+strlen(t2), "\n\n%s", text);
  MsgBox(hwnd, t2, "Error", MB_OK);
  if (end)
    quit();
}

LRESULT CALLBACK main_loop(HWND hwnd, ulong msg, WPARAM wp, LPARAM lp)
/* Main processing loop.
 * Enter: HWND hwnd: handle of current window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.         5/6/96-DWM */
{
  HDC hdc;
  HWND prog;
  PAINTSTRUCT paint;
  POINT pt;
  RECT rect;
  long w, h, i;
  char text[80];

  switch (msg) {
    case WM_CLOSE:
      if (HwndP)  SendMessage(HwndP, WM_CLOSE, 0, 0);
      if (slide_free(1)) return(0);
      if (WholeScreen) {
        ShowWindow(hwnd, SW_HIDE);
        SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd,GWL_STYLE)|
                      WS_CAPTION);
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|
                     SWP_FRAMECHANGED);
        SetMenu(hwnd, Hmenu);
        ShowWindow(hwnd, SW_NORMAL); }
      return(DefWindowProc(hwnd, msg, wp, lp));
    case WM_COMMAND: Busy = 0;  switch (wp&0xFFFF) {
      case MenuAbout:
        Busy=1;
        DialogBox(hinst, MAKEINTRESOURCE(AbDLG), hwnd, (DLGPROC)about);
        Busy=0;
        break;
      case MenuAppend: save_as(2); break;
      case MenuBatch:    NewCopy = 2;  batch(); break;
      case MenuBatchApp: NewCopy = 3;  batch(); break;
      case MenuColor: Reduce ^= 1; recheck(hwnd, 0);
        if (!oldi && BitsPixel<=16 && Reduce) {
          cursor(1);
          reduce_res(hwnd, 1);
          cursor(0);
          rescale |= 2;  InvalidateRect(hwnd, 0, 0); } break;
      case MenuCopyFull: copy(hwnd, 1); break;
      case MenuCopyView: copy(hwnd, 0); break;
      case MenuDither: Dither ^= 1;
        recheck(hwnd, !oldi && BitsPixel<=8); break;
      case MenuError: Msg ^= 1; recheck(hwnd, 0); break;
      case MenuExit: PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0); break;
      case MenuExt: add_extensions(hwnd); break;
      case MenuGrey: convert_grey(); break;
      case MenuHelp: WinHelp(hwnd, HelpFile, HELP_CONTENTS, 0); break;
      case MenuHelpS: WinHelp(hwnd, HelpFile, HELP_FINDER, 0); break;
      case MenuImageFirst: image_switch(0); break;
      case MenuImageNext: case MenuKeyRight: case MenuKeyPgDown:
        image_switch(1); break;
      case MenuImagePrev: case MenuKeyLeft: case MenuKeyPgUp:
        image_switch(-1); break;
      case MenuInfo: info(hwnd, 0); break;
      case MenuInter:
        Interpolate = (!(Interpolate&~0x1C00))|(Interpolate&0x1C00);
        recheck(hwnd, !oldi);  break;
      case MenuInter2: if ((Interpolate&!0x1C00)!=0x2003)
          Interpolate = 0x2003|(Interpolate&0x1C00);
        else  Interpolate &= 0x1C00;
        recheck(hwnd, !oldi); break;
      case MenuLastView: case MenuLastView+1: case MenuLastView+2:
      case MenuLastView+3: if (TimerOn) break;
        sprintf(lastview+1024, "\"%s\"", lastview+256*((wp&0xFFFF)-
                MenuLastView));
        open_pic_mid(hwnd, lastview+1024, 0); break;
      case MenuLetBlack: Letter = Letter&0xC; recheck(hwnd, 1); break;
      case MenuLetCon:  Letter = (Letter&0xC)+3; recheck(hwnd, 1); break;
      case MenuLetFit:  Letter ^= 8;  Lock &= 0xFFFA;  recheck(hwnd, 0);
        no_letter(hwnd); break;
      case MenuLetGrey: Letter = (Letter&0xC)+1; recheck(hwnd, 1); break;
      case MenuLetWhite: Letter = (Letter&0xC)+2; recheck(hwnd, 1); break;
      case MenuMulti: MultiOpen ^= 1;  recheck(hwnd, 0); break;
      case MenuNew: if (TimerOn) break;  NewCopy = 1;  open_pic(hwnd); break;
      case MenuOpen: if (TimerOn) break; NewCopy = 0;  open_pic(hwnd); break;
      case MenuOrig: View[0] = View[1] = View[4] = View[5] = 0;
        View[2] = View[6] = oldw;  View[3] = View[7] = oldh;
        if (Lock&9) {
          rescale |= 2;  InvalidateRect(hwnd, 0, 0); break; }
        set_client(hwnd, OrigX, OrigY, OrigW, OrigH, 1);
        rescale |= 2;  InvalidateRect(hwnd, 0, 0); break;
      case MenuPaste: paste(); break;
      case MenuPreview: preview(); return(0);
      case MenuPrint: printer(Hwnd, 0); break;
      case MenuPSetup: printer(Hwnd, 1); break;
      case MenuRefresh: rescale |= 2;  InvalidateRect(hwnd, 0, 0); break;
      case MenuRotateCW: rotate(1); break;
      case MenuRotateCCW: rotate(-1); break;
      case MenuSave: SaveFilt = oldspec[0]+1;
        memcpy(SaveRec+SAVELEN*oldspec[0], oldspec+2, SAVELEN);
        if (oldspec[0]==5)  memcpy(gquant, oldquant, 128);
        strcpy(opfile, curfile);
        if (strchr(curfile, '\\')) {
          strcpy(opfile, strrchr(curfile, '\\')+1);
          strpath(savedir, curfile); }
        save_as(0); break;
      case MenuSaveAs: save_as(1); break;
      case MenuSaveMulti: MultiSave ^= 1;  recheck(hwnd, 0); break;
      case MenuSlAuto: slide_command(2); break;
      case MenuSlClear: slide_clear(0); break;
      case MenuSlClose: slide_free(1); break;
      case MenuSlDec: slide_command(-1); break;
      case MenuSlide: slide_open(Hwnd); break;
      case MenuSlInc: slide_command(1); break;
      case MenuSlMan: slide_command(0); break;
      case MenuSlNext: slide_show(); break;
      case MenuSlNextGroup: slide_show_group(); break;
      case MenuSlNon: slide_next_nonmulti(); break;
      case MenuSlOptions: if (!slide) break;
        Busy=1;
        DialogBox(hinst, MAKEINTRESOURCE(SlOptDLG), hwnd, (DLGPROC)slide_dialog);
        Busy=0;
        break;
      case MenuSlSave: slide_save(Hwnd, 0); break;
      case MenuSlSaveAs: slide_save(Hwnd, 1); break;
      case MenuSole: Msg ^= 2; recheck(hwnd, 0); break;
      case MenuSpiff:
        if ((Interpolate&0x1C00)==0x1C00)  Interpolate &= ~0x1C00;
        else                               Interpolate += 0x0400;
        recheck(hwnd, 1);  break;
      case MenuSpiffOff: Interpolate &= ~0x1C00;
        recheck(hwnd, 1);  break;
      case MenuTop: topmost(); break;
      case MenuUndo: undo(hwnd); break;
      case MenuWarn: Msg ^= 4; recheck(hwnd, 0); break;
      case MenuWhole: if (!View[0] && !View[1] && View[2]==oldw &&
                          View[3]==oldh) break;
        if (View[4] || View[5] || View[6]!=oldw || View[7]!=oldh)
          slide_delete(LastNum, 1);
        View[0] = View[1] = View[4] = View[5] = 0;
        View[2] = View[6] = oldw;  View[3] = View[7] = oldh;
        no_letter(hwnd);
        if (Lock&4)  zoom(1, 0); recheck(hwnd, 1); break;
      case MenuZ025: zoom(0.25, 1); break;
      case MenuZ050: zoom(0.5, 1); break;
      case MenuZ100: zoom(1, 1); break;
      case MenuZ200: zoom(2, 1); break;
      case MenuZ400: zoom(4, 1); break;
      case MenuZ78: if (AspectX==78 && AspectY==110)  AspectX = AspectY = 0;
        else { AspectX = 78;  AspectY = 110; }
        recheck(hwnd, 0); break;
      case MenuZAspect: if (AspectX && AspectY && (AspectX!=78 ||
                            AspectY!=110)) {
          AspectX = AspectY = 0;  recheck(hwnd, 0); break; }
        Busy=1;
        DialogBox(hinst, MAKEINTRESOURCE(AsDLG), hwnd, (DLGPROC)aspect_dialog);
        Busy=0;
        break;
      case MenuZEnlarge: Redox = (Redox&1)^1;  recheck(hwnd, 0); break;
      case MenuZL100: Lock = (Lock&0xFFFD)^4;  Letter &= 0xFFF7;
        recheck(hwnd, 0); zoom(1, 0); break;
      case MenuZLock: Lock ^= 1; Letter &= 0xFFF7; recheck(hwnd, 0); break;
      case MenuZMinus: zoom(0, 2); break;
      case MenuZoomF10:
        if (WholeScreen) {
          ShowWindow(hwnd, SW_HIDE);
          SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd,GWL_STYLE)|WS_CAPTION|WS_THICKFRAME);
          SetWindowPos(hwnd, HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|
                       SWP_FRAMECHANGED);
          SetMenu(hwnd, Hmenu);
          ShowWindow(hwnd, SW_NORMAL);
          WholeScreen = 0;
          cursor(0); }
        else if (IsZoomed(Hwnd)) {
          ShowWindow(hwnd, SW_HIDE);
          SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE)&~WS_CAPTION&~WS_THICKFRAME);
          SetWindowPos(hwnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
          SetMenu(hwnd, 0);
          ShowWindow(hwnd, SW_SHOWMAXIMIZED);
          WholeScreen = 1;
          cursor(0); }
        else
          ShowWindow(hwnd, SW_SHOWMAXIMIZED);
        break;
      case MenuZPlus: zoom(1, 2); break;
      case MenuZPrev: Lock = (Lock&0xFFFB)^2;
        recheck(hwnd, 0); zoom(1, 0); break;
      case MenuZReduce: Redox = (Redox&2)^2;  recheck(hwnd, 0); break;
      case MoveShift: if (Down) move(wp, lp&0xFFFF, lp>>16, 0); break;
      default: if ((wp&0xFFFF)>=MenuCat && (wp&0xFFFF)<MenuCat+50)
          slide_cat_select((wp&0xFFFF)-MenuCat);
        break; }
      if (HwndP)  SetTimer(HwndP, 3, 1000, (TIMERPROC)preview_timer);
      break;
    case WM_DESTROY: quit(); break;
    case WM_DROPFILES: drop_file((HANDLE)wp); break;
    case WM_ENTERMENULOOP: Busy=0; return(DefWindowProc(hwnd,msg,wp,lp));
    case WM_EXITMENULOOP: Busy=0; return(DefWindowProc(hwnd,msg,wp,lp));
    case WM_INITMENU: Busy=1; return(DefWindowProc(hwnd,msg,wp,lp));
    case WM_LBUTTONDOWN: prep_undo(hwnd);  Down |= 1;
      DownX = lp&0xFFFF;  DownY = (lp>>16);
      if (Down==1)  ODown[0] = 0;  else  cursor(0);
      SetCapture(hwnd); break;
    case WM_LBUTTONUP: if (Down) move(wp, lp&0xFFFF, lp>>16, 1); break;
    case WM_MBUTTONDOWN: if (Down) break;  prep_undo(hwnd);
      Down |= 3;  DownX = lp&0xFFFF;  DownY = (lp>>16);  ODown[0] = 0;
      SetCapture(hwnd); break;
    case WM_MBUTTONUP: if (Down) move(wp, lp&0xFFFF, lp>>16, 1); break;
    case WM_MOUSEMOVE: if (Down) move(wp, lp&0xFFFF, lp>>16, 0); break;
    case WM_MOVE: GetWindowRect(hwnd, &rect);
      OrigX = rect.left;  OrigY = rect.top;
      return(1);
    case WM_PAINT: if (IsIconic(hwnd)) break;
      if (rescale>=4) {
        cursor(1);
        set_size();
        rescale = 2;
        reduce_res(hwnd, 0);
        set_client(hwnd, OrigX, OrigY, OrigW, OrigH, 1);
        break; }
      hdc = BeginPaint(hwnd, &paint);
      cursor(1);
      if ((rescale>=2 && Scale) || rescale)
        prep_pic(hwnd);
      if (wpal) {
        SelectPalette(hdc, wpal, 0);
        RealizePalette(hdc); }
      draw_bmp(hdc, 0, 0, curwidth, curheight, bmpi, wpic);
      cursor(0);
      EndPaint(hwnd, &paint); break;
    case WM_PALETTECHANGED: PalChange = 1; break;
    case WM_QUERYNEWPALETTE: if (!wpal)  return(0);
      if (PalChange)  InvalidateRect(hwnd, 0, 0);
      PalChange = 0; break;
    case WM_RBUTTONDOWN: prep_undo(hwnd);
      Down |= 2;  DownX = lp&0xFFFF;  DownY = (lp>>16);
      if (Down==2) { ODown[0] = 0;  cursor(2); } else  cursor(0);
      SetCapture(hwnd); break;
    case WM_RBUTTONUP: if (Down) move(wp, lp&0xFFFF, lp>>16, 1); break;
    case WM_SIZE: w = lp&0xFFFF;  h = (lp>>16);
      i = (IsZoomed(hwnd)!=0)*8;
      if ((i^Lock)&8)  Lock ^= 8;
      prep_undo(hwnd);
      if (w!=WindowW || h!=WindowH) {
        WindowW = w;  WindowH = h;
        no_letter(hwnd);
        if (Lock&4) { i = Lock;  Lock |= 1;  zoom(1, 0);  Lock = i;  break; }
        InvalidateRect(hwnd, 0, 0);
        rescale |= 1; } break;
    case WM_TIMER: if (wp!=1 || !TimerOn) break;
      if (LoadPart<2)
        cursor(0);
      Minpwidth = Minpheight = 0;
      for (i=0; ((Msg&2) || (i<NumPart/100+1 && i<100)) && LoadPart; i++)
        newpic = load_graphic(0);
      if (!LoadPart) {
        KillTimer(hwnd, 0);
        TimerOn = 0;
        open_pic_end(hwnd); }
    default: return(DefWindowProc(hwnd, msg, wp, lp)); }
  return(0);
}

void map_dialog_rect(HWND hdlg, long x1, long y1, long x2, long y2, long *out)
/* Convert from the goofy dialog units to pixels.
 * Enter: HWND hdlg: handle of dialog with coordinates.
 *        long x1, y1, x2, y2: coordinates to transform.
 *        long *out: array of 4 longs for output.              1/12/97-DWM */
{
  RECT rect;

  rect.left = x1;   rect.top = y1;
  rect.right = x2;  rect.bottom = y2;
  MapDialogRect(hdlg, &rect);
  out[0] = rect.left;   out[1] = rect.top;
  out[2] = rect.right;  out[3] = rect.bottom;
}

HANDLE metafile_to_bmp(HANDLE gmem)
/* Search a metafile for a bitmap.  If present, convert the bitmap to a
 *  memory area by itself.  Otherwise, return null.
 * Enter: HANDLE gmem: handle to clipboard metafile object.
 * Exit:  HANDLE bmp: handle to a newly allocated bitmap or null for no
 *                    bitmap present.                          6/17/98-DWM */
{
  uchar *temp, *pic, *pic2;
  HANDLE mf;
  long size, i;

  if (!(temp=lock2(gmem)))  return(0);
  mf = (HANDLE)((long *)(temp+12))[0];
  unlock2(temp);
  size = GetMetaFileBitsEx(mf, 0, 0);
  if (size<40)  return(0);
  if (!(pic=malloc2(size)))  return(0);
  GetMetaFileBitsEx(mf, size, pic);
  for (i=0; i<size-3; i++)
    if (((long *)(pic+i))[0]==40)
      break;
  if (i>size-40) { free2(pic);  return(0); }
  memmove(pic, pic+i, size-i);
  return(unlock2(pic));
}

void move(long flags, long x, long y, long clear)
/* Move the mouse cursor, updating the screen accordingly.
 * Enter: long flags: determine which keys (if any) are down.
 *        short x, y: mouse position.
 *        long clear: 0 for normal operation, 1 for ending operation.
 *                                                              6/5/96-DWM */
{
  HDC hdc;
  RECT r;
  long x1, x2, y1, y2, dx, dy, push;
  float sx, sy, ox, oy, w, h;

  if (x>32768)  x -= 65536;  if (y>32768)  y -= 65536;
  hdc = GetDC(Hwnd);
  if (Down==1 || Down==3) {
    if (ODown[0]) {
      SetRect(&r, ODown[1], ODown[2], ODown[3], ODown[4]);
      DrawFocusRect(hdc, &r); }
    if (flags&MK_SHIFT) {
      if (abs(DownX-x)>abs(DownY-y) && (DownY!=y))
        x = DownX+abs(DownY-y)*((x>DownX)-(x<DownX));
      if (abs(DownX-x)<abs(DownY-y) && (DownX!=x))
        y = DownY+abs(DownX-x)*((y>DownY)-(y<DownY)); }
    else if (AspectX && AspectY) {
      x1 = x2 = x;  y1 = y2 = y;
      if (abs(DownX-x)*AspectY>abs(DownY-y)*AspectX && (DownY!=y))
        x1 = DownX+abs(DownY-y)*((x>DownX)-(x<DownX))*AspectX/AspectY;
      if (abs(DownX-x)*AspectY<abs(DownY-y)*AspectX && (DownX!=x))
        y1 = DownY+abs(DownX-x)*((y>DownY)-(y<DownY))*AspectY/AspectX;
      if (abs(DownX-x)*AspectX>abs(DownY-y)*AspectY && (DownY!=y))
        x2 = DownX+abs(DownY-y)*((x>DownX)-(x<DownX))*AspectY/AspectX;
      if (abs(DownX-x)*AspectX<abs(DownY-y)*AspectY && (DownX!=x))
        y2 = DownY+abs(DownX-x)*((y>DownY)-(y<DownY))*AspectX/AspectY;
      if (abs(x1-x)+abs(y1-y)<abs(x2-x)+abs(y2-y)) { x = x1;  y = y1; }
      else                                         { x = x2;  y = y2; } }
    ODown[1] = min(DownX, x);
    ODown[2] = min(DownY, y);
    ODown[3] = max(DownX, x);
    ODown[4] = max(DownY, y);
    ODown[0] = 1;
    SetRect(&r, ODown[1], ODown[2], ODown[3], ODown[4]);
    if (!clear)
      DrawFocusRect(hdc, &r); }
  ReleaseDC(Hwnd, hdc);
  if (View[2]==View[0])  View[2] = 1;
  if (View[3]==View[1])  View[3] = 1;
  if ((Down==1 || Down==3) && clear) {
    if ((x==DownX && (y>=DownY-2 && y<=DownY+2)) || (y==DownY && (x>=DownX-2
         && x<=DownX+2))) {
      push = Redox;  if (Down==3)  push ^= 3;  if (push&1)  push &= 1;
      if (push) {
        sx = (float)WindowW/(View[2]-View[0]);
        sy = (float)WindowH/(View[3]-View[1]);
        if (sy<sx)  sx = sy;  if (!sx)  sx = 1;
        Focus[0] = (float)x/sx-(float)WindowW/(2*sx)+0.5*(View[2]+View[0]);
        Focus[1] = (float)y/sx-(float)WindowH/(2*sx)+0.5*(View[3]+View[1]); }
      if (push==1 && (View[2]-View[0]>2 || View[3]-View[1]>2))
        zoom(2, 0);
      if (push==2)
        zoom(0.5, 0); }
    else {
      w = WindowW;  h = WindowH;
      if (Down==3) { w = ODown[3]-ODown[1];  h = ODown[4]-ODown[2]; }
      sx = w/(View[2]-View[0]);
      sy = h/(View[3]-View[1]);
      if (sy<sx)  sx = sy;  if (!sx)  sx = 1;
      if (sx>1 && (Lock&6))  sx = 1;
      ox = w/(2*sx)-0.5*(View[2]-View[0]);
      oy = h/(2*sx)-0.5*(View[3]-View[1]);
      if (Down!=3) {
        x1 = ODown[1]/sx-ox+View[0];  y1 = ODown[2]/sx-oy+View[1];
        x2 = ODown[3]/sx-ox+View[0];  y2 = ODown[4]/sx-oy+View[1]; }
      else {
        x1 = View[0]-ox-ODown[1]/sx; y1 = View[1]-oy-ODown[2]/sx;
        x2 = x1+WindowW/sx;   y2 = y1+WindowH/sx; }
      if (x1<0) x1 = 0;  if (x2>oldw) x2 = oldw;
      if (y1<0) y1 = 0;  if (y2>oldh) y2 = oldh;
      if (x1!=x2 && y1!=y2) {
        View[0] = View[4] = x1;  View[1] = View[5] = y1;
        View[2] = View[6] = x2;  View[3] = View[7] = y2;
        slide_delete(LastNum, 1);
        no_letter(Hwnd);
        rescale |= 2;  InvalidateRect(Hwnd, 0, 0); } } }
  if (Down==2) {
    cursor(2*(!clear));
    sx = WindowW/(View[2]-View[0]);  sy = WindowH/(View[3]-View[1]);
    if (sy<sx)  sx = sy;  if (!sx)  sx = 1;
    dx = (float)(x-DownX)/sx;  if (!dx && x!=DownX)  dx = x-DownX;
    dy = (float)(y-DownY)/sx;  if (!dy && y!=DownY)  dy = y-DownY;
    DownX = x;  DownY = y;
    x1 = View[0];  y1 = View[1];  x2 = View[2];  y2 = View[3];
    if (dx>0 && View[0]) {
      View[0] -= dx;  if (View[0]<0)    View[0]=0;     View[2]+=View[0]-x1; }
    if (dx<0 && View[2]!=oldw) {
      View[2] -= dx;  if (View[2]>oldw) View[2]=oldw;  View[0]+=View[2]-x2; }
    if (dy>0 && View[1]) {
      View[1] -= dy;  if (View[1]<0)    View[1]=0;     View[3]+=View[1]-y1; }
    if (dy<0 && View[3]!=oldh) {
      View[3] -= dy;  if (View[3]>oldh) View[3]=oldh;  View[1]+=View[3]-y2; }
    if (View[0]<View[4]) { View[6] -= View[4]-View[0];  View[4] = View[0]; }
    if (View[1]<View[5]) { View[7] -= View[5]-View[1];  View[5] = View[1]; }
    if (View[2]>View[6]) { View[4] += View[2]-View[6];  View[6] = View[2]; }
    if (View[3]>View[7]) { View[5] += View[3]-View[7];  View[7] = View[3]; }
    if (x1!=View[0] || y1!=View[1] || x2!=View[2] || y2!=View[3]) {
      rescale |= 2;  InvalidateRect(Hwnd, 0, 0); } }
  if (clear) {
    ODown[0] = 0;
    Down = 0;  ReleaseCapture(); }
}

long MsgBox(HWND hwnd, char *text, char *title, ulong style)
/* Perform the standard MessageBox procedure, but disable slide advances
 *  while this is happening.
 * Enter: HWND hwnd: owner window.
 *        char *text: text within box.
 *        char *title: title on top of box.
 *        ulong style: see windows help for the meaning of this.
 * Exit:  long selected: value of selected button.            11/13/96-DWM */
{
  long sel;

  Busy++;
  sel = MessageBox(hwnd, text, title, style);
  Busy--;
  return(sel);
}

void multi_save(char *name, uchar *spec)
/* Save the current multi-image file to a new file name.  The image is not
 *  scaled in any manner.
 * Enter: char *name: new name of file to save.
 *        uchar *spec: file format specification to save.     12/20/01-DWM */
{
  long part=0, i, next;
  uchar *pic;

  do {
    ppart = part;
    if (!(pic=load_graphic(curfile)))  return;
    next = pspec[1]&0x80;
    spec[1] = (part!=0);
    i = save_graphic(name, pic, spec, 1);
    if (i)  return;
    part++; }
  while (next);
}

void no_letter(HWND hwnd)
/* Check if letterboxing is allowed.  If not, check if the letterboxing will
 *  occur.  If it will, reduce the window size so that it work.  Note that
 *  for small images, letterboxing always occurs.              5/27/96-DWM */
{
  RECT r, r2;
  long w, h, w2, h2;

  if (!(Letter&8) || IsIconic(hwnd) || IsZoomed(hwnd))  return;
  w2 = View[2]-View[0];  h2 = View[3]-View[1];
  if (!w2)  w2 = 1;  if (!h2)  h2 = 1;
  GetClientRect(hwnd, &r);
  GetWindowRect(hwnd, &r2);
  w = r.right;  h = r.bottom;
  if (w-w2*h/h2>1)
    w = w2*h/h2;
  else if (h-h2*w/w2>1)
    h = h2*w/w2;
  if (w<MINWIDTH)
    w = MINWIDTH;
  if (w==r.right && h==r.bottom)  return;
  Letter ^= 8;
  set_client(hwnd, r2.left, r2.top, w, h, 1);
  Letter ^= 8;
  WindowW = w;  WindowH = h;
}

BOOL CALLBACK null_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle a zero-input dialog.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        5/26/96-DWM */
{
  return(0);
}

BOOL CALLBACK open_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Set the title in the open dialog box.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        6/11/96-DWM */
{
  HWND ctrl;

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd,HelpFile,HELP_CONTEXT,HelpOpen+NewCopy);
        break; } break;
    case WM_INITDIALOG: SetWindowText(hdlg, OpenTitle[NewCopy]);
      ctrl = GetDlgItem(hdlg, OpenRec);   DestroyWindow(ctrl);
      ctrl = GetDlgItem(hdlg, OpenRec2);  DestroyWindow(ctrl);
      ctrl = GetDlgItem(hdlg, OpenMul);   DestroyWindow(ctrl);
      ctrl = GetDlgItem(hdlg, OpenMul2);  DestroyWindow(ctrl);
      if (NewCopy>=2)
        SetDlgItemText(hdlg, IDOK, "Next >>");
      SetFocus(hdlg); }
  return(0);
}

long open_pic(HWND hwnd)
/* Set up the necessary trash for an open file dialog.  NewCopy is 0-open,
 *  1-open new window, 2-batch convert, 3-batch append.
 * Enter: HWND hwnd: handle of window to open this for.
 * Exit:  long okay: 0 for no file specified, 1 for file specified.
 *                                                              5/6/96-DWM */
{
  long len, i;
  char name[NAMELEN];

  /*
  char lpstrFile[MAX_PATH * 2] = "";

  OPENFILENAME ofn = {0};
  ofn.hwndOwner   = hwnd;
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.lpstrFile   = &lpstrFile[0];
  ofn.nMaxFile    = MAX_PATH -1;
  ofn.lpstrFilter = "TXT\0*.txt\0All Files\0*.*\0\0";
  ofn.lpstrTitle  = "TXT Files";
  ofn.Flags       = OFN_OVERWRITEPROMPT;
  ofn.lpstrDefExt = ".txt";
  return GetOpenFileNameA(&ofn);
  */

  prep_undo(hwnd);
  opfile[0] = 0;
  strcpy(opfilter, OpenFilter);
  len = strlen(opfilter);
  for (i=0; i<len; i++)  if (opfilter[i]=='|')  opfilter[i] = 0;
  opfilter[len] = 0;
  openfile.lStructSize = sizeof(OPENFILENAME);
  openfile.hwndOwner = hwnd;  openfile.hInstance = hinst;
  openfile.lpstrFilter = opfilter;
  openfile.nFilterIndex = FilterType;
  openfile.lpstrFile = opfile;
  openfile.nMaxFile = NAMELEN;
  openfile.lpstrFileTitle = openfile.lpstrCustomFilter = 0;
  openfile.lpstrInitialDir = opendir+256*(NewCopy-1)*(NewCopy>=2);
  openfile.lpfnHook = (DLGPROC)open_dialog;
  openfile.lpTemplateName = MAKEINTRESOURCE(OpDLG);
  openfile.Flags = OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_FILEMUSTEXIST |
                   OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST |
                   OFN_ALLOWMULTISELECT;
  Busy = 1;
  if (GetOpenFileName(&openfile)) {
    Busy = 0;
    FilterType = openfile.nFilterIndex;
    join_names(opfile);
    if (NewCopy>=2)
      return(1);
    strcpy(name, opfile);
    if (!NewCopy) {
      if (find_space(name)) {
        strcpy(name, find_space(opfile)+1);  NewCopy = 1; }
      open_pic_mid(hwnd, opfile, 1); }
    if (NewCopy)
      open_pic_new(hwnd, name);
    return(1); }
  Busy = 0;
  return(0);
}

void open_pic_end(HWND hwnd)
/* Finish loading a graphic file.  This allows for other processing while the
 *  file is being loaded.
 * Enter: HWND hwnd: handle of window.                         5/26/96-DWM */
{
  long i;
  short *p;

  cursor(1);
  if (!newpic) {
    if ((Msg&1) && !titleimage[0])
      MsgBox(hwnd, ErrorMsg[perr>>8], "Error", MB_OK);
    InvalidateRect(hwnd, 0, 0);  cursor(0);
    return; }
  free2(pic);
  pic = newpic;
  oldw = pwidth;  oldh = pheight;  oldi = pinter;
  memcpy(oldspec, pspec, LENPSPEC);
  if (oldspec[0]==5)  memcpy(oldquant, pquant, 128);
  if (titleimage[0]!=1) {
    strcpy(curfile, opfile);
    for (i=0; i<3; i++)
      if (!stricmp(curfile, lastview+i*256))
        break;
    memmove(lastview+256, lastview, 256*i);
    strcpy(lastview, opfile); }
  if (!titleimage[0])
    if (strchr(opfile, '\\'))
      strpath(opendir, opfile);
  set_size();
  preview_invalidate(LastNum, 0, 0);
  LastNum = preview_invalidate(0, curfile, 0);
  if (LastNum>=0 && slide) {
    ((ulong *)(slide->file+LastNum*FE+0x20))[0] |= 0x40000000;
    p = ((short *)(slide->file+LastNum*FE+0x24));
    if (slide->option&0x80) {
      View[0] = View[4] = View[1] = View[5] = 0;
      View[2] = View[6] = pwidth;  View[3] = View[7] = pheight; }
    else {
      View[0] = View[4] = p[0];  View[1] = View[5] = p[1];
      View[2] = View[6] = p[2];  View[3] = View[7] = p[3]; } }
  rescale |= 2;
  if (!IsIconic(hwnd)) {
    reduce_res(hwnd, 0);
    if (!(Lock&9) || Undo[2]<=0)
      set_client(hwnd, OrigX, OrigY, OrigW, OrigH, 1);
    else if (!IsZoomed(hwnd))
      MoveWindow(hwnd, Undo[0], Undo[1], Undo[2], Undo[3], 1);
    InvalidateRect(hwnd, 0, 0);
    zoom(1, 0); }
  else
    rescale = 4;
  set_name(hwnd);
  PalChange = 1;
  prep_undo(hwnd);
  EnableMenuItem(Hmenu, MenuUndo, MFS_GRAYED);
  write_ini(1);
  cursor(0);
  if ((oldspec[1]&0x80) && MultiOpen==1) {
    sprintf(opfile, "\"%s?%d\"", curfile, (oldspec[1]&0x7F)+2);
    open_pic_new(hwnd, opfile); }
  MultiOpen &= 1;
  recheck(Hwnd, 1);
}

void open_pic_mid(HWND hwnd, char *name, long preremove)
/* Load the specified graphics file.  This skip the file dialog box.
 * Enter: HWND hwnd: handle of window.
 *        char *name: name of file to open.
 *        long preremove: non-zero to remove quotes before finding spaces
 *                        between file names.                  5/26/96-DWM */
{
  char temp[NAMELEN];
  long i, part=0;

  if (TimerOn) return;
  if (!strcmp(name, "...")) {
    paste();  return; }
  strcpy(temp, name);
  if (preremove)
    remove_quotes(temp);
  while (temp[0]<=' ' && temp[0])  memmove(temp, temp+1, NAMELEN-1);
  for (i=0; i<strlen(temp); i++)
    if (temp[i]<' ')  temp[i] = 0;
  remove_quotes(temp);
  if (strchr(temp, '?')) {
    sscanf(strchr(temp, '?')+1, "%d", &part);
    part--;
    strchr(temp, '?')[0] = 0; }
  strcpy(opfile, temp);
  LoadPart = 0-((Msg&2) || titleimage[0]);
  cursor(1);
  ppart = (MultiOpen!=0)-1;
  if (part>0)  ppart = part;
  Minpwidth = Minpheight = 0;
  newpic = load_graphic(opfile);
  if (!newpic && (perr>>8)==0x18) {
    slide_open_mid();
    return; }
  if (LoadPart<=0)
    open_pic_end(hwnd);
  else {
    TimerOn = 1;
    SetTimer(hwnd, 1, 0, 0); }
}

void open_pic_new(HWND hwnd, char *name)
/* Start another instance of GV, loading a specified file.
 * Enter: HWND hwnd: handle of window.
 *        char *name: name of file to open.                    6/28/96-DWM */
{
  PROCESS_INFORMATION proc;
  STARTUPINFO start;
  uchar call[NAMELEN], *cmd;

  if (!strchr(name, '\\'))
    getcwd(opendir, 254);
  write_ini(1);
  GetStartupInfo(&start);
  cmd = GetCommandLine();
  if (strlen(cmd)<NAMELEN)  strcpy(call, cmd);  else  strcpy(call, "GV");
  if (find_space(call))  find_space(call)[0] = 0;
  if (strlen(call)+strlen(name)<NAMELEN-4) {
    if (name[0]!='"')  sprintf(call+strlen(call), " %s", name);
    else               sprintf(call+strlen(call), " \"%s\"", name); }
  CreateProcess(0, call, 0, 0, 0, CREATE_DEFAULT_ERROR_MODE, 0, 0, &start,
                &proc);
}

BOOL CALLBACK open_slide_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Set up the slide open dialog.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        6/11/96-DWM */
{
  HWND ctrl;
  long num;

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case IDOK: SlideOpt &= 0xFFFFFFF9;
        SlideOpt |= 2*SendDlgItemMessage(hdlg,OpenRec,BM_GETCHECK,0,0);
        SlideOpt |= 4*SendDlgItemMessage(hdlg,OpenMul,BM_GETCHECK,0,0);
        return(0);
      case HelpHelp: num = PMode;  if (SlideOpt&0x200) num = 2;
          WinHelp(Hwnd, HelpFile, HELP_CONTEXT, HelpOpenSlide+num);
        break; } break;
    case WM_INITDIALOG:
      if ((SlideOpt&0x200) || PMode>1) {
        ctrl = GetDlgItem(hdlg, edt1);  EnableWindow(ctrl, 0);
        ctrl = GetDlgItem(hdlg, lst1);  EnableWindow(ctrl, 0);
        ctrl = GetDlgItem(hdlg, stc3);  EnableWindow(ctrl, 0); }
      if ((SlideOpt&0x200) && PMode<=2)
        SetWindowText(hdlg, "Select New Root Directory");
      else
        SetWindowText(hdlg, Open2Title[PMode]);
      if (PMode>=3) {
        ctrl = GetDlgItem(hdlg, OpenRec);   DestroyWindow(ctrl);
        ctrl = GetDlgItem(hdlg, OpenRec2);  DestroyWindow(ctrl);
        ctrl = GetDlgItem(hdlg, OpenMul);   DestroyWindow(ctrl);
        ctrl = GetDlgItem(hdlg, OpenMul2);  DestroyWindow(ctrl); }
      else {
        SendDlgItemMessage(hdlg, OpenRec, BM_SETCHECK, (SlideOpt&2)!=0, 0);
        SendDlgItemMessage(hdlg, OpenMul, BM_SETCHECK, (SlideOpt&4)!=0, 0); }
      SetFocus(hdlg); }
  return(0);
}

void paste(void)
/* Paste the current clipboard into the window, just as if it was loaded
 *  from a file.                                                6/8/96-DWM */
{
  HGLOBAL gmem=0, gpal=0;
  uchar *src, *dest=0, *buf;
  char text[80];
  long w, h, bits, i, scan, pal, clear=0;
  BITMAP data;

  cursor(1);
  if (OpenClipboard(Hwnd)) {
    if (!gmem) {
      if (IsClipboardFormatAvailable(CF_METAFILEPICT))
        gmem = GetClipboardData(CF_METAFILEPICT);
      if (gmem) {
        gmem = metafile_to_bmp(gmem);
        if (gmem) {
          dest = bmp_to_graphic(gmem);
          if (dest)
            free2(dest);
          else {
            free2(lock2(gmem));
            gmem = 0; } }
        if (gmem)
          clear = 1; } }
    if (!gmem)
      if (IsClipboardFormatAvailable(CF_DIB))
        gmem = GetClipboardData(CF_DIB);
    if (!gmem) {
      if (IsClipboardFormatAvailable(CF_BITMAP))
        gmem = GetClipboardData(CF_BITMAP);
      if (IsClipboardFormatAvailable(CF_PALETTE))
        gpal = GetClipboardData(CF_PALETTE);
      if (gmem) {
        gmem = BitmapToBMP(gmem, gpal, 0);
        if (!gmem) {
          low_memory(HwndP, "Clipboard Access", 0);
          cursor(0);  return; }
        clear = 1; } }
    if (gmem) {
      dest = bmp_to_graphic(gmem);
      if (clear)
        free2(lock2(gmem)); }
    else {
      for (i=1; i<17; i++)
        if (IsClipboardFormatAvailable(i))
          break;
      if (i==CF_DIB || i==CF_BITMAP || i==CF_METAFILEPICT)
        strcpy(text, "The clipboard's owner does not permit access");
      else
        sprintf(text, "Can't read clipboard in format %d.", i);
      if (Msg&1)
        MsgBox(Hwnd, text, "Error", MB_OK); }
    CloseClipboard(); }
  cursor(0);
  prep_undo(Hwnd);
  if (dest) {
    newpic = dest;
    open_pic_end(Hwnd); }
  curfile[0] = 0;
  set_name(Hwnd);
}

void prep_pic(HWND hwnd)
/* Create all the items that Windows needs to show a bitmap.  Global
 *  variables affecting this process are:
 *   Scale: 0 for reduced area/resolution/color depth, 1 for whole image,
 *   rescale: If 0, nothing is done. If <4, it is zeroed before exiting.
 *   oldw, oldh, oldi: size and type of picture.
 *   pic: pointer to picture to scale.
 *   View[0-4]: rectangle defining area to view.  x,y,x,y, not x,y,w,h.
 *   Letter: type of letterboxing to apply.
 *   WindowW, WindowH: destination rectangle size.
 *   CopyMode: 0-letterbox, 1-no letterboxing, 2-do not create BMP, store
 *             results in newpic, pwidth, pheight, and pinter.
 *   Lock: determines if picture is enlarged.  If bits 1 or 2 ane non-zero,
 *         the picture is not scaled up.
 *   PrepTrans: -1 for normal operation, otherwise color number to make
 *              white, with all others being made black.
 * Output variables:
 *   curwidth, curheight, bmpi: size and type of adjusted image.
 *   wpic: bitmap containing adjusted image.
 *   wpal: palette for the bitmap.
 *   newpic: (only if CopyMode==2) pointer to scaled image.
 * Enter: HWND hwnd: handle of window to do this 'for'.         5/6/96-DWM */
{
  char *pic2;
  HDC DC, memDC;
  LOGPALETTE *lpal;
  long *lpic, i, w, x, y, a=0, border, border8, ow, oh, oi, pal, start;
  long depal=0;
  long rw, rh;
  uchar *apic, *bpic, *depalpic;
  short *pv;

  if (!rescale)  return;
  x = oldw;  y = oldh;  pal = oldi;  apic = pic;
  if (x<2) x=2;  if (y<2) y=2;
  if (!Scale) {
    if (View[0]<0 || View[0]>=oldw)  View[0] = 0;
    if (View[1]<0 || View[1]>=oldh)  View[1] = 0;
    if (View[2]<=View[0] || View[2]>oldw)  View[2] = oldw;
    if (View[3]<=View[1] || View[3]>oldh)  View[3] = oldh;
    if (LastNum>=0 && slide)
      if (LastNum<slide->numfile && !(slide->option&0x80)) {
        pv = (short *)(slide->file+FE*LastNum+0x24);
        if (pv[0]!=View[4] || pv[1]!=View[5] || pv[2]!=View[6] ||
            pv[3]!=View[7])
          slide_delete(LastNum, 1);
        pv[0]=View[4];  pv[1]=View[5];  pv[2]=View[6];  pv[3]=View[7]; }
    rw = View[2]-View[0];  rh = View[3]-View[1];
    if (!rw)  rw = 1;  if (!rh)  rh = 1;
    border = LetterColor[Letter&3];
    if (LetterColor[4]>=0 && (Letter&4)) border = LetterColor[4];
    if (LetterColor[5]) {
      border8 = border;
      border = pic[border8*3]*0x10000+pic[border8*3+1]*0x100+
               pic[border8*3+2]; }
    if (PrepTrans!=-1 || PreviewPic)  border = border8 = 0;
    x = WindowW;  y = WindowH;
    if (x<2) x=2;  if (y<2) y=2;
    if (CopyMode) {
      if (x*rh>y*rw)       x = y*rw/rh;
      else if (x*rh<y*rw)  y = x*rh/rw;
      if ((Lock&6) && rw<x && rh<y) {
        x = rw;  y = rh; } }
    if (x!=oldw || y!=oldh || !oldi || View[0]!=0 || View[1]!=0 ||
        View[2]!=oldw || View[3]!=oldh || (Lock&6) || CopyMode ||
        (Interpolate&0x1C00)) {
      if (BitsPixel>8 && oldi && Interpolate) {
        if (bpic=malloc2(oldw*oldh*3)) {
          for (i=0; i<oldw*oldh; i++)  memcpy(bpic+i*3, pic+pic[768+i]*3, 3);
          depal = 1;  depalpic = pic;  pic = bpic;  oldi = 0; } }
      apic = malloc2(x*y*(3-2*oldi)+768*oldi);  a = 1;
      if (!apic) { low_memory(hwnd, "Scaling Picture", 1); return; }
      if (oldi) {
        start = 768+View[0]+View[1]*oldw;
        if ((Lock&6) && !CopyMode && rw<x && rh<y) {
          memset(apic+768, border8, x*y);
          scale_pic(apic+768+((y-rh)/2)*x, pic+start, oldw, x, rw, x, rh, rh,
                    border8, PrepTrans); }
        else
          scale_pic(apic+768, pic+start, oldw, x, rw, x, rh, y, border8,
                    PrepTrans);
        memcpy(apic, pic, 768); }
      else {
        start = (View[0]+View[1]*oldw)*3;
        if ((Lock&6) && !CopyMode && rw<x && rh<y) {
          fill_zone24(apic, border, x*y);
          scale_pic24(apic+((y-rh)/2)*x*3, pic+start, oldw, x, rw, x, rh, rh,
                      border, PrepTrans); }
        else
          scale_pic24(apic, pic+start, oldw, x, rw, x, rh, y, border,
                      PrepTrans);
        bpic = pic; pic = apic;  ow = oldw;  oh = oldh;  oi = oldi;
        oldw = x;  oldh = y;
        reduce_res(hwnd, 1);
        pal = oldi;
        oldi = oi;  oldh = oh;  oldw = ow;  apic = pic; pic = bpic; }
      if (depal) {
        free2(pic);  oldi = 1;  pic = depalpic; } } }
  if (CopyMode==2) {
    newpic = apic;  pwidth = x;  pheight = y;  pinter = pal; }
  else {
    curwidth = x;  curheight = y;
    w = ((x+3)/4)*4;
    if (!pal)  w = ((x*3+3)/4)*4;
    if (wpic)  GlobalFree(wpic);
    wpic = GlobalAlloc(GMEM_MOVEABLE, w*y+1024*pal+40);
    if (!wpic) {
      low_memory(hwnd, "Creating Bitmap", 1);
      if (a) free2(apic); return; }
    pic2 = GlobalLock(wpic);  lpic = (long *)pic2;
    lpic[0] = 40;  lpic[1] = x;  lpic[2] = y;
    ((short *)pic2)[6] = 1;  ((short *)pic2)[7] = 24-16*pal;
    lpic[4] = lpic[6] = lpic[7] = 0;  lpic[5] = w*y;
    lpic[8] = lpic[9] = 256*pal;
    if (pal) {
      for (i=0; i<256; i++) {
        pic2[40+i*4] = apic[i*3+2];
        pic2[41+i*4] = apic[i*3+1];
        pic2[42+i*4] = apic[i*3];
        pic2[43+i*4] = 0; }
      if (PrepTrans!=-1) {
        memset(pic2+40, 0, 1020);
        memset(pic2+1060, 255, 3); }
      for (i=0; i<y; i++)
        memcpy(pic2+1064+w*i, apic+768+x*(y-1-i), x); }
    else
      for (i=0; i<y; i++) {
        memcpy(pic2+40+w*i, apic+x*3*(y-1-i), x*3);
        bgr_to_rgb(pic2+40+w*i, x); }
    if (pal) {
      lpal = LocalAlloc(LPTR, sizeof(LOGPALETTE) + 256*sizeof(PALETTEENTRY));
      if (!lpal) {
        low_memory(hwnd, "Allocating Palette", 1);
        if (a) free2(apic); return; }
      lpal->palVersion = 0x300;  lpal->palNumEntries = 256;
      for (i=0; i<256 && pal; i++) {
        lpal->palPalEntry[i].peRed   = pic2[42+i*4];
        lpal->palPalEntry[i].peGreen = pic2[41+i*4];
        lpal->palPalEntry[i].peBlue  = pic2[40+i*4];
        lpal->palPalEntry[i].peFlags = 0; }
      if (wpal) { DeleteObject(wpal);  wpal = 0; }
      wpal = CreatePalette(lpal);
      LocalFree(lpal); }
    GlobalUnlock(wpic);
    if (a)
      free2(apic); }
  if (rescale<4)  rescale = 0;
  bmpi = pal;
}

HBITMAP prep_pic2(long *rect, long destw, long desth, long noenlarge,
                  long trans, long *pal, HPALETTE *bpal)
/* Create a Windows bitmap.  Global variables affecting this process are:
 *   oldw, oldh, oldi: size and type of picture.
 *   pic: pointer to picture to scale.
 *   Letter: type of letterboxing to apply.
 * The bitmap returned has been allocated using GlobalAlloc and must be
 *  freed using GlobalFree.
 * Enter: long *rect: rectangle defining bounding coordinates within pic to
 *                    use when creating this bitmap.
 *        long destw, desth: size of destination picture.
 *        long noenlarge: 1 if picture may not be enlarged.
 *        long trans: -1 for normal image, color number of black and white
 *                    transparency mask.
 *        long *pal: location to store if this is a palettized image.  Zero
 *                   for no return.
 *        HPALETTE *bpal: location to store windows palette.
 * Exit:  HBITMAP wpic: created bitmap.                        10/3/96-DWM */
{
  long Orescale, Oview[4], Ow, Oh, Ocopy, Olock, Och, Ocw, Obi, i;
  long Oscale, Otrans;
  HBITMAP Owpic, newpic;
  HPALETTE Opal=0;

  Orescale = rescale;  Ow = WindowW;  Oh = WindowH;  Ocopy = CopyMode;
  Olock = Lock;  Ocw = curwidth;  Och = curheight;  Obi = bmpi;
  for (i=0; i<4; i++)  Oview[i] = View[i];
  Oscale = Scale;  Otrans = PrepTrans;  Owpic = wpic;  Opal = wpal;
  rescale = 2;  WindowW = destw;  WindowH = desth;  CopyMode = 0;
  for (i=0; i<4; i++)  View[i] = rect[i];
  Lock = noenlarge*2;  Scale = 0;  PrepTrans = trans;  wpic = 0;  wpal = 0;
  prep_pic(Hwnd);
  newpic = wpic;  if (pal)  pal[0] = bmpi;  bpal[0] = wpal;
  rescale = Orescale;  WindowW = Ow;  WindowH = Oh;  CopyMode = Ocopy;
  Lock = Olock;  curwidth = Ocw;  curheight = Och;  bmpi = Obi;
  for (i=0; i<4; i++)  View[i] = Oview[i];
  Scale = Oscale;  PrepTrans = Otrans;  wpic = Owpic;  wpal = Opal;
  return(newpic);
}

void prep_undo(HWND hwnd)
/* Record the current settings, for they are about to change.
 * Enter: HWND hwnd: window to prep.                           5/30/96-DWM */
{
  long x, y, w, h;
  RECT rect;

  EnableMenuItem(Hmenu, MenuUndo, MFS_ENABLED);
  GetWindowRect(hwnd, &rect);
  x = rect.left;  w = rect.right-x;
  y = rect.top;   h = rect.bottom-y;
  if (x==Undo[0] && y==Undo[1] && w==Undo[2] && h==Undo[3] &&
      !memcmp(Undo+4, View, 8*sizeof(long)))  return;
  memcpy(Undo+UNDOLEN, Undo, UNDOLEN*sizeof(long));
  Undo[0] = x;  Undo[1] = y;  Undo[2] = w;  Undo[3] = h;
  memcpy(Undo+4, View, 8*sizeof(long));
}

void preview(void)
/* If no slide file has yet been loaded, allow the user to select one.
 *  Otherwise, open or switch to the preview window.  The preview window has
 *  separate menu items specifically for it.                    3/2/97-DWM */
{
  PMode = 1;
  if (!slide) {
    slide_open(Hwnd);
    if (!slide) {
      PMode = 0;  return; } }
  slide->option &= ~8;
  if (HwndP) {
    SetForegroundWindow(HwndP); return; }
  HwndP = CreateWindow(WinName2, "Graphic Viewer Preview", WS_VSCROLL|
             WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, HWND_DESKTOP, 0, hinst, 0);
  preview_palette();
  if (WinPlace2.length) {
    if (WinPlace2.showCmd==SW_SHOWMINIMIZED)
      WinPlace2.showCmd = SW_NORMAL;
    SetWindowPlacement(HwndP, &WinPlace2); }
  ShowWindow(HwndP, SW_SHOW);
  recheck(Hwnd, 0);
  PMode = 0;
  SetTimer(HwndP, 3, 333, (TIMERPROC)preview_timer);
  SetTimer(HwndP, 4, 0, (TIMERPROC)preview_pic);
}

void preview_cat_select(long cat)
/* Toggle the state of the category for the selected images.
 * Enter: long cat: category to toggle (0-29) or quick key that was pressed
 *                   (30-39 corresponding to keys '0'-'9').     3/1/97-DWM */
{
  long i, j, num, on=0;
  ulong mask, or=0;

  if (!slide)  return;
  if (cat>=40) { on = 1;  cat -= 10; }
  num = preview_deselect(1);
  if (cat>=30)
    for (i=0; i<30; i++)
      if (((short *)(slide->cat+i*8+4))[0] && slide->cat[i*8+6]==cat-29)
        cat = i;
  if (cat>=30)  return;
  if (!((short *)(slide->cat+cat*8+4))[0])  return;
  mask = ((ulong)0xFFFFFFFF)^(1<<cat);
  if (!(((ulong *)(slide->file+FE*slide->tsel+0x20))[0]&(1<<cat)) || on)
    or = (1<<cat);
  for (i=0; i<slide->numfile; i++)
    if ((slide->file[FE*i+0x2F]&2) || (!num && i==slide->tsel))
      ((ulong *)(slide->file+FE*i+0x20))[0] =
                             (((ulong *)(slide->file+FE*i+0x20))[0]&mask)|or;
  recheck(Hwnd, 0);
  for (i=0; i<PreCoor[0]; i++)
    if ((slide->file[FE*PreCoor[i*2+4]+0x2F]&2) || (!num &&
        PreCoor[i*2+4]==slide->tsel))
      break;
  j = PreCoorMax;
  preview_compute();
  if (j==PreCoorMax) {
    for (i=0; i<PreCoor[0]; i++)
      if ((slide->file[FE*PreCoor[i*2+4]+0x2F]&2) || (!num &&
        PreCoor[i*2+4]==slide->tsel))
      preview_invalidate(PreCoor[i*2+4], 0, 1); }
  else {
    if (i==PreCoor[0])
      if (slide->tsel<PreCoor[4])  i = 0;
    if (i!=PreCoor[0])
      preview_invalidate_end(i); }
}

void preview_char(long key)
/* Process keyboard input in the preview window.  This is for scrolling and
 *  selecting thumbnails.
 * Enter: long key: virtual key code.                          3/19/97-DWM */
{
  long shift=((GetKeyState(VK_SHIFT)&0x8000)!=0);
  long ctrl=((GetKeyState(VK_CONTROL)&0x8000)!=0);
  long cur, new=1e6, sel=0, i, dy;

  if (Down==4 || !slide) return;
  if (ctrl)  shift = 1;
  cur = slide->tsel;
  preview_compute();
  switch (key) {
    case VK_DOWN: new = PreCoor[2]; break;
    case VK_END: new = slide->numfile; break;
    case VK_HOME: new = -slide->numfile; break;
    case VK_LEFT: new = -1; break;
    case VK_NEXT: new = PreCoor[2]*PreCoor[3]; break;
    case VK_PRIOR: new = -PreCoor[2]*PreCoor[3]; break;
    case VK_RIGHT: new = 1; break;
    case VK_SPACE: new = 0; break;
    case VK_UP: new = -PreCoor[2]; }
  if (new==1e6)  return;
  if (((long *)(slide->file+cur*FE+0x40))[0]==-1)
    cur = PreCoor[4];
  for (i=0; i<PreCoor[0] && i<PreCoor[2]*PreCoor[3]; i++)
    if (cur==PreCoor[i*2+4]) break;
  if (i==PreCoor[0] || i==PreCoor[2]*PreCoor[3])  shift = 0;
  else if (key==VK_SPACE)  shift = 1;
  new += ((long *)(slide->file+cur*FE+0x40))[0];
  if (new<0)  new = 0;  if (new>=PreCoorMax)  new = PreCoorMax-1;
  for (i=0; i<slide->numfile; i++)
    if (((long *)(slide->file+i*FE+0x40))[0]==new)
      sel = i;
  if (sel==slide->tsel && !shift)  return;
  if (shift) {
    if (Down!=5) {
      ODown[1] = cur;
      preview_deselect(ctrl!=0);
      Down = 5; }
    for (i=0; i<slide->numfile; i++)
      if ((i<=ODown[1] && i>=sel) || (i<=sel && i>=ODown[1]))
        slide->file[i*FE+0x2F] |= 4;
      else
        slide->file[i*FE+0x2F] &= 0xFB; }
  else
    Down = 0;
  slide->tsel = sel;
  for (i=0; i<PreCoor[0] && i<PreCoor[2]*PreCoor[3]; i++)
    if (sel==PreCoor[i*2+4]) break;
  preview_rectangles(0);
  if (i==PreCoor[0] || i==PreCoor[2]*PreCoor[3]) {
    if (sel>PreCoor[4]) {
      new = (new-PreCoor[2]*PreCoor[3]+PreCoor[2]);
      if (new<0)  new = 0; }
    new = (new/PreCoor[2])*PreCoor[2];
    dy = (new-slide->ttop)/PreCoor[2];
    if (dy)
      SendMessage(HwndP, WM_VSCROLL, SB_THUMBPOSITION+((GetScrollPos(HwndP,
                  SB_VERT)+dy)<<16), 0); }
  recheck(Hwnd, 0);
}

void preview_clear(long set)
/* Set or clear the slide viewed flags.
 * Enter: long set: 0 to clear, 1 to set flags.                3/20/97-DWM */
{
  long i, num;

  if (!slide)  return;
  num = preview_deselect(1);
  for (i=0; i<slide->numfile; i++)
    if ((slide->file[FE*i+0x2F]&2) || (!num && i==slide->tsel)) {
      if (!set)
        ((ulong *)(slide->file+FE*i+0x20))[0] &= 0xBFFFFFFF;
      else
        ((ulong *)(slide->file+FE*i+0x20))[0] |= 0x40000000; }
  slidename[0] = 1;
  for (i=0; i<PreCoor[0]; i++)
    if ((slide->file[FE*PreCoor[i*2+4]+0x2F]&2) || (!num &&
        PreCoor[i*2+4]==slide->tsel))
      preview_invalidate(PreCoor[i*2+4], 0, 1);
}

void preview_compute(void)
/* Compute the coordinates of each picture to show within the preview window.
 *  This also computes how many pictures are visible, and where the scroll
 *  bar should be.                                             3/11/97-DWM */
{
  long w, h, nx, ny, num=0, all=0, i, j, maxpos;
  RECT rect;
  ulong and=0x80000000, cmp=0;

  w = max(50, slide->tx+10);  h = max(20, slide->ty+10);
  if (slide->option&0x4000000)                 h  = 10;
  if (!(slide->option&0x8000000))              h += 12;
  if (!(slide->option&0x10000000))             h += 12;
  if ((slide->option&0x60000000)!=0x60000000)  h += 12;
  if (h<=20)  h = 20;
  GetClientRect(HwndP, &rect);
  nx = rect.right/w;  if (!nx)  nx = 1;
  ny = rect.bottom/h;  if (ny<=0)  ny = 1;
  if (nx*(ny+1)>MAXPREVIEW)  ny = MAXPREVIEW/nx-1;
  if (ny<=0) { nx = MAXPREVIEW/2;  ny = 1; }
  for (i=0; i<30; i++) {
    if (slide->cat[8*i+7]==1) { and |= (1<<i);  cmp |= (1<<i); }
    if (slide->cat[8*i+7]==2)   and |= (1<<i); }
  for (i=0; i<slide->numfile; i++)
    ((long *)(slide->file+i*FE+0x40))[0] = -1;
  if (!(slide->option&0x400)) {
    for (i=0; i<slide->numfile; i++)
      if ((((ulong *)(slide->file+FE*i+0x20))[0]&and)==cmp) {
        ((long *)(slide->file+i*FE+0x40))[0] = num;
        num++; } }
  else {
    for (i=0; i<slide->numfile; i++)
      if (((((ulong *)(slide->file+FE*i+0x20))[0]&and)|cmp)==cmp &&
          ((((ulong *)(slide->file+FE*i+0x20))[0]&cmp) || !cmp)) {
        ((long *)(slide->file+i*FE+0x40))[0] = num;
        num++; } }
  if (!num || (slide->option&0x80000000)) {
    for (i=0; i<slide->numfile; i++)
      ((long *)(slide->file+i*FE+0x40))[0] = i;
    all = 1;  num = slide->numfile; }
  if (slide->ttop!=(slide->ttop/nx)*nx) {
    slide->ttop = (slide->ttop/nx)*nx;
    InvalidateRect(HwndP, 0, 1); }
  maxpos = (num+nx-1)/nx;  maxpos = max(0, maxpos-ny);
  if (slide->ttop<0) slide->ttop = 0;
  if (slide->ttop>maxpos*nx)  slide->ttop = maxpos*nx;
  SetScrollRange(HwndP, SB_VERT, 0, maxpos, 0);
  SetScrollPos(HwndP, SB_VERT, slide->ttop/nx, !ScrollOff);
  j = num = 0;
  if (all) {
    for (i=0; i<slide->numfile; i++, num++)
      if (i>=slide->ttop && j<nx*(ny+1)) {
        PreCoor[4+2*j] = i;  j++; } }
  else if (!(slide->option&0x400)) {
    for (i=0; i<slide->numfile; i++)
      if ((((ulong *)(slide->file+FE*i+0x20))[0]&and)==cmp) {
        num++;
        if (num>slide->ttop && j<nx*(ny+1)) {
          PreCoor[4+2*j] = i;  j++; } } }
  else {
    for (i=0; i<slide->numfile; i++)
      if (((((ulong *)(slide->file+FE*i+0x20))[0]&and)|cmp)==cmp &&
          ((((ulong *)(slide->file+FE*i+0x20))[0]&cmp) || !cmp)) {
        num++;
        if (num>slide->ttop && j<nx*(ny+1)) {
          PreCoor[4+2*j] = i;  j++; } } }
  for (i=0; i<j; i++) {
    ((ushort *)(PreCoor+4+2*i))[2] &= 0x8000;
    ((ushort *)(PreCoor+4+2*i))[2] |= (i%nx)*w;
    ((short *)(PreCoor+4+2*i))[3] = (i/nx)*h; }
  PreCoor[0] = j;
  ((short *)PreCoor)[2] = w;  ((short *)PreCoor)[3] = h;
  PreCoor[2] = nx;  PreCoor[3] = ny;  PreCoorMax = num;
}

void preview_delete(void)
/* Delete the selected files.                                  3/23/97-DWM */
{
  long i, first=-1;
  char name[NAMELEN], *s;

  if (!preview_deselect(1)) {
    if (slide->tsel<0)  return;
    slide->file[slide->tsel*FE+0x2F] |= 6; }
  if (Msg&4) {
    i = preview_deselect(3);
    if (i>1)
      sprintf(name, "Delete %d files?", i);
    else
      strcpy(name, "Delete selected file?");
    if (MsgBox(HwndP, name, "Warning", MB_OKCANCEL)==IDCANCEL)
      return; }
  if (slide->numfile==1) {
    if (Msg&1)
      MsgBox(HwndP, "The last image can not be deleted.", "Error", MB_OK);
    return; }
  for (i=0; i<slide->numfile && slide->numfile; i++)
    if (slide->file[i*FE+0x2F]&2) {
      s = slide->file+i*FE;
      sprintf(name, "%s\\%s", slide->dirname+((long *)(slide->dir+
              ((short *)s)[0]*6))[0], slide->filename+((long *)(s+2))[0]);
      if (first<0) {
        for (first=0; first<PreCoor[0]; first++)
          if (PreCoor[first*2+4]==i)
            break;
        if (i<PreCoor[4])  first = 0;
        if (i>PreCoor[2+PreCoor[0]*2])  first = 1e6; }
      DeleteFile(name);
      slide_delete(i, 0);
      i--; }
  if (slide->tsel>=slide->numfile)  slide->tsel = slide->numfile-1;
  if (first<1e5)
    preview_invalidate_end(first);
}

long preview_deselect(long hold)
/* Deselect all of the items in the preview, or, if the shift or control key
 *  is held down, make the previous selections more permament.
 * Enter: long hold: 0 for clear all, 1 for hold current selections, 2 to
 *                   select all, 3 to count both selected and double selected
 *                   items without changing anything..
 * Exit:  long numsel: the number of files still selected.     3/19/97-DWM */
{
  long i, num=0;

  if (!hold)
    for (i=0; i<slide->numfile; i++)
      slide->file[i*FE+0x2F] &= 0xF9;
  else if (hold!=3)
    for (i=0; i<slide->numfile; i++) {
      if ((((slide->file[i*FE+0x2F]&6) && (slide->file[i*FE+0x2F]&6)!=6) ||
          hold==2) && ((long *)(slide->file+i*FE+0x40))[0]!=-1) {
        slide->file[i*FE+0x2F] = (slide->file[i*FE+0x2F]&0xF9)|2;  num++; }
      else
        slide->file[i*FE+0x2F] &= 0xF9; }
  else
    for (i=0; i<slide->numfile; i++)
      if (slide->file[i*FE+0x2F]&6)  num++;
  return(num);
}

BOOL CALLBACK preview_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the Preview Options dialog box.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.       11/16/96-DWM */
{
  long i;
  char text[270];

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd,HelpFile,HELP_CONTEXT,HelpPreview); break;
      case IDOK: SlideOpt &= 0x01FFFFFF;
        for (i=25; i<=31; i++)
          if (!SendDlgItemMessage(hdlg, PrevOpt25+i-25, BM_GETCHECK, 0, 0))
            SlideOpt |= (1<<i);
        GetDlgItemText(hdlg, PrevWidth, text, 79);
        sscanf(text, "%d", &i);
        if (i>=40 && i<=255)  slide->tx = i;
        GetDlgItemText(hdlg, PrevHeight, text, 79);
        sscanf(text, "%d", &i);
        if (i>=10 && i<=255)  slide->ty = i;
        slide->option = SlideOpt;
        InvalidateRect(HwndP, 0, 1);
      case IDCANCEL: EndDialog(hdlg, 1); return(1);
      default: return(0); } break;
    case WM_INITDIALOG: SlideOpt = slide->option;
      for (i=25; i<=31; i++)
        SendDlgItemMessage(hdlg, PrevOpt25+i-25, BM_SETCHECK,
                           !(SlideOpt&(1<<i)), 0);
      SendDlgItemMessage(hdlg, PrevOpt31b, BM_SETCHECK,
                         (SlideOpt&0x80000000)!=0, 0);
      sprintf(text, "%d", slide->tx);
      SetDlgItemText(hdlg, PrevWidth, text);
      sprintf(text, "%d", slide->ty);
      SetDlgItemText(hdlg, PrevHeight, text);
      SetFocus(hdlg);
      return(1); }
  return(0);
}

void preview_insert(long x, long y, long sel)
/* Insert the selected thumbnails in front of the current thumbnail.  If
 *  called using a mouse, the current thumbnail is first determined based on
 *  the mouse position.
 * Enter: long x, y: mouse coordinates, if sel==0.
 *        long sel: 0 for mouse, 1 to use current selection point, 3 to use
 *                  double selection and to not update the screen afterwards.
 *                                                             3/19/97-DWM */
{
  long w, h, x1, y1, i, j, first, low, high, noupdate=(sel&2);
  RECT rect;
  char temp[FE];

  if (!slide)  return;
  if (!preview_deselect(1+noupdate))  return;
  w = ((short *)PreCoor)[2];  h = ((short *)PreCoor)[3];
  if (!(sel&1)) {
    GetClientRect(HwndP, &rect);
    for (i=0; i<PreCoor[0]; i++) {
      sel = PreCoor[i*2+4];
      x1 = (((ushort *)(PreCoor+i*2+4))[2]&0x7FFF);
      y1 = ((short *)(PreCoor+i*2+4))[3];
      if ((x<x1+w && y<y1+h) || y<y1) break; }
    if (i==PreCoor[0])
      sel++; }
  else
    sel = slide->tsel;
  first = low = high = sel;
  if (sel<0 || sel>slide->numfile)  return;
  for (i=0; i<slide->numfile; i++)
    if (slide->file[i*FE+0x2F]&2) {
      j = i;
      if (i<low)     low = i;
      if (i+1>high)  high = i+1;
      slide->file[i*FE+0x2F] = (slide->file[i*FE+0x2F]&0xF9)|4;
      memcpy(temp, slide->file+i*FE, FE);
      if (i<sel) {
        memmove(slide->file+i*FE, slide->file+i*FE+FE, (sel-i-1)*FE);
        first--;  j--; }
      else {
        memmove(slide->file+(sel+1)*FE, slide->file+sel*FE, (i-sel)*FE);
        sel++; }
      memcpy(slide->file+(sel-1)*FE, temp, FE);
      if (LastNum==i)  LastNum = sel-1;
      else if (LastNum<sel && LastNum>i)  LastNum--;
      else if (LastNum>=sel && LastNum<i) LastNum++;
      else if (LastNum==sel-1)            LastNum++;
      if (NextNum==i)  NextNum = sel-1;
      else if (NextNum<sel && NextNum>i)  NextNum--;
      else if (NextNum>=sel && NextNum<i) NextNum++;
      else if (NextNum==sel-1)            NextNum++;
      i = j; }
  if (noupdate)
    for (i=0; i<slide->numfile; i++)
      if (slide->file[i*FE+0x2F]&4)
        slide->file[i*FE+0x2F] |= 2;
  preview_deselect(1+noupdate);
  if (!noupdate)
    for (i=0; i<PreCoor[0]; i++)
      if (PreCoor[i*2+4]>=low && PreCoor[i*2+4]<high)
        preview_invalidate(PreCoor[i*2+4], 0, 0);
  slidename[0] = 1;
}

long preview_invalidate(long num, char *name, long part)
/* Check if a given picture is currently displayed in the preview window.
 *  If so, update the picture.  Alternately, check if a given file name is
 *  part of the preview file.  If so, return the associated number within the
 *  slide file.  If not, either add it, or not, as indicated.
 * Enter: long num: if name is null, this is the number of the slide to
 *                  update.  If name is not null, this is 0 to report a value
 *                  of -1 when a file is not part of the slide file, or 1 to
 *                  add it into the slide file (if possible).
 *        char *name: null for simple update.  Otherwise, this is the name of
 *                    a file to check if present in the slide file.
 *        long part: portion of the picture to update.  0-all, 1-text only,
 *                   2-text only without erasing the background.  If num is 0
 *                   and name is not null, then this is the image number
 *                   within the file.
 * Exit:  long num: either the number of the file in the slide file, or -1
 *                  not present.                               3/18/97-DWM */
{
  long i, dir, r, oldtime, inum;
  RECT rect, rect2;
  char dname[NAMELEN], fname[NAMELEN], name2[NAMELEN], *new, *file;
  WIN32_FIND_DATA find;

  if (!slide)  return(-1);
  if (!name && HwndP) {
    for (i=0; i<PreCoor[0] && i<MAXPREVIEW; i++)
      if (PreCoor[i*2+4]==num) {
        rect.left = (((ushort *)(PreCoor+i*2+4))[2]&0x7FFF);
        rect.top = ((short *)(PreCoor+i*2+4))[3];
        rect.right = rect.left+((short *)PreCoor)[2];
        rect.bottom = rect.top+((short *)PreCoor)[3];
        if (part) {
          rect.top += 5+(slide->ty*(!(slide->option&0x4000000)));
          rect.left += 5;  rect.right -= 5;  rect.bottom -= 5; }
        InvalidateRect(HwndP, &rect, (part!=2)); }
    return(num); }
  if (!name)  return(-1);
  strcpy(dname, name);
  strcpy(name2, name);
  if (!strchr(dname, '\\'))  return(-1);
  strcpy(fname, strrchr(dname, '\\')+1);
  strrchr(dname, '\\')[0] = 0;
  for (dir=0; dir<slide->numdir; dir++)
    if (!stricmp(dname, slide->dirname+((long *)(slide->dir+dir*6))[0]))
      break;
  if (dir==slide->numdir)  return(-1);
  for (i=0; i<slide->numfile; i++) {
    if (((short *)(slide->file+i*FE))[0]!=dir)  continue;
    if (stricmp(fname, slide->filename+((long *)(slide->file+i*FE+2))[0]))
      continue;
    inum = ((uchar *)(slide->file+i*FE+0xC+1))[0]&0x7F;
    if (slide->file[i*FE+0xC]<0 || slide->file[i*FE+0xC]==2 ||
        slide->file[i*FE+0xC]==5 || slide->file[i*FE+0xC]==9)
      inum += (((short *)(slide->file+i*FE+0xC+0x12))[0]<<7);
    if (inum==part || part<0)
      break; }
  if (i!=slide->numfile) {
    file = slide->file+i*FE;
    oldtime = ((long *)(file+0x3C))[0];
    if (!find_file(name2, 0, 0, &find, 0)) {
      ((long *)(file+0x38))[0] = find.nFileSizeLow+(find.nFileSizeHigh<<16);
      FileTimeToDosDateTime(&find.ftLastWriteTime, (LPWORD)(file+0x3E), (LPWORD)(file+0x3C));
      find_file(0, 0, 0, 0, 0); }
    if (oldtime<((long *)(file+0x3C))[0])
      file[0xC] = -1;
    preview_invalidate(i, 0, 0);
    return(i); }
  if (!num)  return(-1);
  if (!(new=realloc2(slide->file, (slide->numfile+1)*FE)))  return(-1);
  slide->file = new;  file = new+(slide->numfile*FE);
  memset(file, 0, FE);
  if (LastNum>=0)
    ((ulong *)(file+0x20))[0] = ((ulong *)(slide->file+LastNum*FE+0x20))[0];
  ((short *)file)[0] = dir;
  if (!(new=realloc2(slide->filename,slide->lenfilename+strlen(fname)+1)))
    return(-1);
  slide->filename = new;
  ((long *)(file+2))[0] = slide->lenfilename;
  strcpy(slide->filename+slide->lenfilename, fname);
  slide->lenfilename += strlen(fname)+1;
  file[0xC] = -1;
  if (!find_file(name2, 0, 0, &find, 0)) {
    ((long *)(file+0x38))[0] = find.nFileSizeLow+(find.nFileSizeHigh<<16);
    FileTimeToDosDateTime(&find.ftLastWriteTime, (LPWORD)(file+0x3E), (LPWORD)(file+0x3C));
    find_file(0, 0, 0, 0, 0); }
  slidename[0] = 1;
  i = slide->numfile;
  slide->numfile++;
  preview_invalidate_end(PreCoor[0]-1);
  return(i);
}

void preview_invalidate_end(long r)
/* Invalidate from a given entry to the end of the screen.
 * Enter: long r: screen location to begin invalidation.       3/23/97-DWM */
{
  RECT rect, rect2;

  GetClientRect(HwndP, &rect2);
  rect.left = (((ushort *)(PreCoor+r*2+4))[2]&0x7FFF);
  rect.top = ((short *)(PreCoor+r*2+4))[3];
  rect.right = rect2.right;  rect.bottom = rect.top+((short *)PreCoor)[3];
  InvalidateRect(HwndP, &rect, 1);
  rect.left = rect2.left;  rect.top += ((short *)PreCoor)[3];
  rect.bottom = rect2.bottom+((short *)PreCoor)[3];
  InvalidateRect(HwndP, &rect, 1);
}

LRESULT CALLBACK preview_loop(HWND hwnd, ulong msg, WPARAM wp, LPARAM lp)
/* Preview window processing loop.
 * Enter: HWND hwnd: handle of current window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.         5/6/96-DWM */
{
  long x, y, i, j, k;
  HDC hdc;
  RECT rect;

  switch (msg) {
    case WM_CLOSE: GetWindowPlacement(HwndP, &WinPlace2);
      KillTimer(HwndP, 3);  KillTimer(HwndP, 4);
      KillTimer(HwndP, 5);  KillTimer(HwndP, 6);
      HwndP = 0;
      if (prevpal) { DeleteObject(prevpal);  prevpal = 0; }
      return(DefWindowProc(hwnd, msg, wp, lp));
    case WM_COMMAND: Busy = 0;  switch (wp&0xFFFF) {
      case MenuAbout:
        Busy = 1;
        DialogBox(hinst, MAKEINTRESOURCE(AbDLG), hwnd, (DLGPROC)about);
        Busy = 0;
        break;
      case MenuAppend: preview_deselect(0);                 /* Select none */
        InvalidateRect(HwndP, 0, 0); break;
      case MenuCopyFull: copy_preview(1); break;
      case MenuCopyImage: copy_file(1); break;
      case MenuCopyView: copy_preview(0); break;
      case MenuDelete: preview_delete(); break;
      case MenuExit: PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0); break;
      case MenuFind:
        DialogBox(hinst, MAKEINTRESOURCE(FindDLG), HwndP, (DLGPROC)find_dialog);
        break;
      case MenuFindNext: find_next(); break;
      case MenuHelp: WinHelp(Hwnd, HelpFile, HELP_CONTENTS, 0); break;
      case MenuHelpS: WinHelp(Hwnd, HelpFile, HELP_FINDER, 0); break;
      case MenuInfo: info(HwndP, 1); break;
      case MenuKeyDown: preview_char(VK_DOWN); break;
      case MenuKeyEnd: preview_char(VK_END); break;
      case MenuKeyHome: preview_char(VK_HOME); break;
      case MenuKeyInsert: preview_insert(0, 0, 1); break;
      case MenuKeyLeft: preview_char(VK_LEFT); break;
      case MenuKeyPgDown: preview_char(VK_NEXT); break;
      case MenuKeyPgUp: preview_char(VK_PRIOR); break;
      case MenuKeyRight: preview_char(VK_RIGHT); break;
      case MenuKeyUp: preview_char(VK_UP); break;
      case MenuMove: copy_file(0); break;
      case MenuOpen: case MenuPreview:
        PMode = 1;  slide_open(HwndP);  PMode = 0;
        if (!slide) { if (HwndP) SendMessage(HwndP, WM_CLOSE, 0, 0); } break;
      case MenuPrevSave: preview_save(); break;
      case MenuPrint: printer(HwndP, 2); break;
      case MenuPrOptions: Busy = 1;
        DialogBox(hinst, MAKEINTRESOURCE(PrevDLG), HwndP, (DLGPROC)preview_dialog);
        Busy = 0; break;
      case MenuPSetup: printer(HwndP, 1); break;
      case MenuPurge: preview_purge(); break;
      case MenuRefresh: InvalidateRect(HwndP, 0, 1);  break;
      case MenuRename: rename_file(); break;
      case MenuSave: slide_save(HwndP, 0); break;
      case MenuSaveAs: slide_save(HwndP, 1); break;
      case MenuSelectAll: preview_deselect(2);
        InvalidateRect(HwndP, 0, 0); break;
      case MenuShowImage: preview_show(); return(0);
      case MenuSlide: SetForegroundWindow(Hwnd);
        if (LastNum<0)  slide_show(); break;
      case MenuSlAuto: if (slide) slide->option &= 0xFDFFFFFF;
        if (slide) SlideOpt = slide->option;
        Busy = 0;  Down = 0;
        SetTimer(HwndP, 3, 0, (TIMERPROC)preview_timer);
        recheck(Hwnd, 0); return(0);
      case MenuSlClear: preview_clear(0); break;
      case MenuSlMan: if (slide) slide->option |= 0x2000000;
        if (slide) SlideOpt = slide->option;
        recheck(Hwnd, 0); break;
      case MenuSlNext: preview_char(VK_SPACE); break;
      case MenuSlNextGroup: preview_char(VK_SPACE); break;
      case MenuSlOptions: if (!slide) break;
        Busy = 1;
        DialogBox(hinst, MAKEINTRESOURCE(SlOptDLG), HwndP, (DLGPROC)slide_dialog);
        Busy = 0; break;
      case MenuSort: sort(); break;
      case MenuViewed: preview_clear(1); break;
      default: if ((wp&0xFFFF)>=MenuCat && (wp&0xFFFF)<MenuCat+50)
          preview_cat_select((wp&0xFFFF)-MenuCat);
        break; }
      SetTimer(HwndP, 3, 2000, (TIMERPROC)preview_timer);
      break;
    case WM_ENTERMENULOOP: Busy=0; return(DefWindowProc(hwnd,msg,wp,lp));
    case WM_EXITMENULOOP: Busy=0; return(DefWindowProc(hwnd,msg,wp,lp));
    case WM_INITMENU: Busy=1; return(DefWindowProc(hwnd,msg,wp,lp));
    case WM_LBUTTONDBLCLK: if (Down==4) preview_move(wp,lp&0xFFFF,lp>>16,1);
      preview_show();  return(0);
    case WM_LBUTTONDOWN: if (Down==4) break;  Down = 4;
      DownX = lp&0xFFFF;  DownY = (lp>>16);  ODown[0] = 0;
      preview_move(wp, lp&0xFFFF, lp>>16, 2);  cursor(0);
      SetCapture(HwndP); break;
    case WM_LBUTTONUP: if (Down==4) preview_move(wp,lp&0xFFFF,lp>>16,1);
      break;
    case WM_MOUSEMOVE: if (Down==4) preview_move(wp,lp&0xFFFF,lp>>16,0);
      break;
    case WM_PAINT: if (IsIconic(hwnd)) {
        preview_compute();
        return(DefWindowProc(hwnd, msg, wp, lp)); }
      preview_update(hwnd);
      return(0);
    case WM_PALETTECHANGED: PalChange = 1; break;
    case WM_QUERYNEWPALETTE: if (!wpal)  return(0);
      if (PalChange)  InvalidateRect(hwnd, 0, 0);
      PalChange = 0; break;
    case WM_RBUTTONDOWN: Down = 6;  SetCapture(HwndP);  cursor(2); break;
    case WM_RBUTTONUP: if (Down==6) preview_insert(lp&0xFFFF, lp>>16, 0);
      cursor(0);  ReleaseCapture(); break;
    case WM_SIZE: InvalidateRect(hwnd, 0, 1); break;
    case WM_VSCROLL: GetScrollRange(HwndP, SB_VERT, &x, &y);
      i = j = GetScrollPos(HwndP, SB_VERT);
      switch (wp&0xFFFF) {
        case SB_BOTTOM: i = x; break;
        case SB_ENDSCROLL: ScrollOff = 0; break;
        case SB_LINEUP:   if (i>x) i--; break;
        case SB_LINEDOWN: if (i<y) i++; break;
        case SB_PAGEUP:   i-=max(1,PreCoor[3]); if (i<x||i>y) i = x; break;
        case SB_PAGEDOWN: i+=max(1,PreCoor[3]); if (i<x||i>y) i = y; break;
        case SB_THUMBTRACK: ScrollOff = 1;
        case SB_THUMBPOSITION: i = (wp>>16); break;
        case SB_TOP: i = y; }
      SetScrollPos(HwndP, SB_VERT, i, 1);
      if (i!=j) {
        if (slide)  slide->ttop = i*PreCoor[2];
        ScrollOff ++;
        hdc = GetDC(HwndP);
        if ((ODown[0]&3)==1) {
          SetRect(&rect, ODown[1], ODown[2], ODown[3], ODown[4]);
          DrawFocusRect(hdc, &rect);
          ODown[0] = 2; }
        ReleaseDC(HwndP, hdc);
        DownY -= (i-j)*((short *)PreCoor)[3];
        if (i>j) {
          for (k=0; k<PreCoor[0]-PreCoor[2]*(i-j); k++)
            ((ushort *)(PreCoor+k*2+4))[2] = (((ushort *)(PreCoor+(k+
                                          PreCoor[2]*(i-j))*2+4))[2]&0x8000);
          for (; k<PreCoor[0]; k++)
            ((ushort *)(PreCoor+k*2+4))[2] = 0; }
        else {
          for (k=PreCoor[0]-1; k>=PreCoor[2]*(j-i); k--)
            ((ushort *)(PreCoor+k*2+4))[2] = (((ushort *)(PreCoor+(k-
                                          PreCoor[2]*(j-i))*2+4))[2]&0x8000);
          for (; k>=0; k--)
            ((ushort *)(PreCoor+k*2+4))[2] = 0; }
        KillTimer(HwndP, 4);
        SetTimer(HwndP, 3, 2000, (TIMERPROC)preview_timer);
        ScrollWindow(HwndP, 0, (j-i)*((short *)PreCoor)[3], 0, 0);
        ScrollOff --; } break;
    default: return(DefWindowProc(hwnd, msg, wp, lp)); }
  return(0);
}

void preview_move(long flags, long x, long y, long clear)
/* Move the mouse cursor, updating the selected items in the preview
 *  accordingly.
 * Enter: long flags: determine which keys (if any) are down.
 *        short x, y: mouse position.
 *        long clear: 0 for normal operation, 1 for ending operation, 2 for
 *                    beginning operation.                     3/18/97-DWM */
{
  HDC hdc;
  RECT rect;
  long x1, y1, w, h, best, dist, i, n;

  if (x>32768)  x -= 65536;  if (y>32768)  y -= 65536;
  ODown[5] = x;  ODown[6] = y;
  if (ODown[0]&2) {
    ODown[7] |= clear;
    SetTimer(HwndP, 6, 0, (TIMERPROC)preview_move_wait);
    return; }
  KillTimer(HwndP, 6);
  hdc = GetDC(HwndP);
  if (ODown[0]) {
    SetRect(&rect, ODown[1], ODown[2], ODown[3], ODown[4]);
    DrawFocusRect(hdc, &rect);
    ODown[0] = 0; }
  ODown[1] = min(DownX, x);
  ODown[2] = min(DownY, y);
  ODown[3] = max(DownX, x);
  ODown[4] = max(DownY, y);
  if (clear!=1) {
    SetRect(&rect, ODown[1], ODown[2], ODown[3], ODown[4]);
    DrawFocusRect(hdc, &rect);
    ODown[0] = 1; }
  ReleaseDC(HwndP, hdc);
  if (clear==2)
    preview_deselect((flags&(MK_SHIFT|MK_CONTROL))!=0);
  w = ((short *)PreCoor)[2];  h = ((short *)PreCoor)[3];  best = 1e6;
  GetClientRect(HwndP, &rect);
  for (i=0; i<PreCoor[0]; i++) {
    x1 = (((ushort *)(PreCoor+i*2+4))[2]&0x7FFF);
    y1 = ((short *)(PreCoor+i*2+4))[3];
    if (y1>=rect.bottom)  continue;
    if (ODown[1]<x1+w && ODown[3]>=x1 && ODown[2]<y1+h && ODown[4]>=y1)
      ((uchar *)slide->file)[PreCoor[i*2+4]*FE+0x2F] |= 4;
    else
      ((uchar *)slide->file)[PreCoor[i*2+4]*FE+0x2F] &= 0xFB;
    dist = abs(x-x1)+abs(x-x1-(w-1))+abs(y-y1)+abs(y-y1-(h-1));
    if (dist<best) {
      best = dist;  slide->tsel = PreCoor[i*2+4]; } }
  n = 0;
  if (y<0 && !ScrollWait && !clear)
    SendMessage(HwndP, WM_VSCROLL, SB_PAGEUP, 0);
  else if (y<10 && !ScrollWait && !clear)
    SendMessage(HwndP, WM_VSCROLL, SB_LINEUP, 0);
  else if (y>rect.bottom-1 && !ScrollWait && !clear)
    SendMessage(HwndP, WM_VSCROLL, SB_PAGEDOWN, 0);
  else if (y>rect.bottom-1-10 && !ScrollWait && !clear)
    SendMessage(HwndP, WM_VSCROLL, SB_LINEDOWN, 0);
  else n = 1;
  if (!n) {
    ScrollWait = 1;
    SetTimer(HwndP, 5, 500, (TIMERPROC)scroll_wait);
    SendMessage(HwndP, WM_MOUSEMOVE, flags, x+(y<<16)); }
  if (clear==1) {
    for (i=0; i<slide->numfile; i++)
      if ((slide->file[i*FE+0x2F]&6) && (slide->file[i*FE+0x2F]&6)!=6)
        slide->file[i*FE+0x2F] = (slide->file[i*FE+0x2F]&0xF9) | 2;
      else
        slide->file[i*FE+0x2F] &= 0xF9;
    ODown[0] = 0;
    Down = 0;  ReleaseCapture();
    recheck(Hwnd, 0); }
  preview_rectangles(0);
}

VOID CALLBACK preview_move_wait(HWND hwnd, ulong msg, ulong id, long time)
/* Keep calling the preview_move function until it actually occurs.
 * Enter: HWND hwnd: handle of calling window.
 *        ulong msg: ignored.
 *        ulong id: id of calling timer.
 *        long time: time in some windows format.             11/10/96-DWM */
{
  long clear=ODown[7];

  ODown[7] = 0;
  KillTimer(HwndP, 6);
  preview_move(0, ODown[5], ODown[6], clear);
}

long preview_page(HDC hdc, long ww, long wh, long page, long full)
/* Draw the preview to a device context.
 * Enter: HDC hdc: device context for drawing.
 *        long ww, wh: size of the window for drawing.  The DC's upper left
 *                     corner should be zero.
 *        long page: page number to draw.  -1 to return the number of pages
 *                   needed for the specified width and height (hdc is
 *                   ignored).  -2 to return the height required for a single
 *                   page (hdc and wh are ignored).
 *        long full: 0 for selected items only, 1 for all visible items.
 *                   +2 to indicate that this is being sent to a printer.
 * Exit:  long numpages: number of pages needed (if page is -1) or height
 *                       needed (if page is -2).               3/26/97-DWM */
{
  long w, h, num, nx, ny, first, pos=0, x, y, i, j, k, l, pal, w2, paper;
  char *s, text[300], tbl[]="-+xx";
  uchar *buf;
  LPBITMAPINFOHEADER bmp;
  HBRUSH new, old;
  HWND hdlg;

  if (!slide)  return(0);
  paper = full&2;  full &= 1;
  w = ((short *)PreCoor)[2];  h = ((short *)PreCoor)[3];
  num = PreCoorMax;  if (!full) num = preview_deselect(1);
  if (!num)  return(0);
  nx = ww/w;  if (!nx)  nx = 1;
  ny = wh/h;  if (!ny)  ny = 1;
  if (page==-1)  return((num+nx*ny-1)/nx/ny);
  if (page<0)    return(h*((num+nx-1)/nx));
  hdlg = CreateDialog(hinst, MAKEINTRESOURCE(CopyDLG), HwndP, 0);
  if (!paper)  new = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
  else         new = CreateSolidBrush(0xFFFFFF);
  old = SelectObject(hdc, new);
  SelectObject(hdc, GetStockObject(NULL_PEN));
  Rectangle(hdc, 0, 0, ww+1, wh+1);
  first = page*nx*ny;
  if (prevpal)
    SelectPalette(hdc, prevpal, 0);
  if (paper)
    set_font(hdc, FF_SWISS, 12, 0, 0xFFFFFF);
  else
    set_font(hdc, FF_SWISS, 12, GetSysColor(COLOR_WINDOWTEXT),
             GetSysColor(COLOR_WINDOW));
  for (i=0; i<slide->numfile && pos<nx*ny; i++) {
    if (((long *)(slide->file+i*FE+0x40))[0]<0)  continue;
    if (!full && !(slide->file[i*FE+0x2F]&2))    continue;
    if (first) { first--; continue; }
    x = (pos%nx)*w+5;  y = (pos/nx)*h+5;  pos++;
    sprintf(text, "Processing image %d of %d.", pos+page*nx*ny, num);
    SetDlgItemText(hdlg, CopyStat1, text);
    s = slide->file+FE*i;
    if (!(slide->option&0x4000000)) {
      buf = 0;
      if ((BitsPixel>8 || s[0x2E]) && s[0x2C] && s[0x2D] &&
          s[0x2C]<=slide->tx && s[0x2D]<=slide->ty)
        buf = malloc2(((long *)(slide->thumb+((long *)(s+0x30))[0]))[0]+8);
      if (buf) {
        unlzw(buf, slide->thumb+((long *)(s+0x30))[0]+4,
              ((long *)(slide->thumb+((long *)(s+0x30))[0]))[0]+4,
              ((long *)(s+0x30))[1]-4, 8);
        bmp = (LPBITMAPINFOHEADER)buf;
        pal = bmp->biBitCount;  if (pal<=8) pal = (1<<pal); else pal = 0;
        StretchDIBits(hdc, x+(slide->tx-s[0x2C])/2, y+(slide->ty-s[0x2D])/2,
                      bmp->biWidth, bmp->biHeight, 0, 0, bmp->biWidth,
                      bmp->biHeight, buf+bmp->biSize+pal*4, (BITMAPINFO *)bmp,
                      DIB_RGB_COLORS, SRCCOPY);
        free2(buf); }
      y += max(10, slide->ty); }
    if (!(slide->option&0x8000000)) {
      sprintf(text, "%c %s", tbl[((ulong *)(s+0x20))[0]>>30],
              slide->filename+((long *)(s+2))[0]);
      num = (((uchar *)s)[0xC+1]&0x7F);
      if (s[0xC]<0 || s[0xC]==2 || s[0xC]==5 || s[0xC]==9)
        num += (((short *)(s+0xC+0x12))[0]<<7);
      if (s[0xC+1] || num)
        sprintf(text+strlen(text), " - %d", 1+num);
      draw_text(hdc, text, x, y, w-10, 12);
      y += 12; }
    if ((slide->option&0x60000000)!=0x60000000) {
      if (!(slide->option&0x20000000))
        sprintf(text, "%d x %d", ((short *)(s+6))[0], ((short *)(s+6))[1]);
      else  text[0] = 0;
      if (!(slide->option&0x60000000))
        strcat(text, " x ");
      if (!(slide->option&0x40000000))
        sprintf(text+strlen(text), "%d bit", 24-16*((short *)(s+6))[2]);
      if (((short *)(s+6))[0])
        draw_text(hdc, text, x, y, w-10, 12);
      y += 12; }
    if (!(slide->option&0x10000000)) {
      for (l=j=0; l<30; l++)
        if (((short *)(slide->cat+l*8+4))[0])
          if ((((ulong *)(s+0x20))[0]&(1<<l))!=0)
            j++;
      if (j)  w2 = ((w-3)/j)-7;
      else    w2 = 0;
      if (w2>0) for (l=k=0; l<30; l++)
        if (((short *)(slide->cat+l*8+4))[0])
          if ((((ulong *)(s+0x20))[0]&(1<<l))!=0) {
            draw_text(hdc, slide->catname+((long *)(slide->cat+l*8))[0],
                      x+k*(w2+7), y, w2, 12);
            k++; } } }
  set_font(hdc, 0, 0, 0, 0);
  SelectObject(hdc, old);
  DeleteObject(new);
  DestroyWindow(hdlg);
  return(0);
}

void preview_palette(void)
/* Set up a palette for use in preview mode if this is an 8-bit display.
 *                                                             3/11/97-DWM */
{
  LOGPALETTE *lpal;
  long i, j, k;

  if (BitsPixel>8)  return;
  lpal = LocalAlloc(LPTR, sizeof(LOGPALETTE) + 256*sizeof(PALETTEENTRY));
  if (!lpal)  return;
  lpal->palVersion = 0x300;  lpal->palNumEntries = 256;
  UniformPal = 1;
  palettize(0, 0, 0, 1);
  UniformPal = 0;
  for (i=0; i<256; i++) {
    lpal->palPalEntry[i].peRed   = MasterPal[i*3+2];
    lpal->palPalEntry[i].peGreen = MasterPal[i*3+1];
    lpal->palPalEntry[i].peBlue  = MasterPal[i*3];
    lpal->palPalEntry[i].peFlags = 0; }
  if (prevpal) { DeleteObject(prevpal);  prevpal = 0; }
  prevpal = CreatePalette(lpal);
  LocalFree(lpal);
}

void preview_palette_set(void)
/* Obtain the actual system palette used in the preview mode.  This speeds up
 *  drawing the thumbnail pictures.                            3/15/97-DWM */
{
  PALETTEENTRY wp[256];
  long i, n;
  HDC hdc;

  if (PreviewPal!=(uchar *)-1 || GetForegroundWindow()!=HwndP)  return;
  hdc = GetDC(HwndP);
  memset(wp, 0, 256*sizeof(PALETTEENTRY));
  n = GetSystemPaletteEntries(hdc, 0, 256, wp);
  ReleaseDC(HwndP, hdc);
  if (!n)  return;
  if (!(PreviewPal=malloc2(768))) {
    PreviewPal = (uchar *)-1;  return; }
  for (i=0; i<n; i++) {
    PreviewPal[i*3]   = wp[i].peRed;
    PreviewPal[i*3+1] = wp[i].peGreen;
    PreviewPal[i*3+2] = wp[i].peBlue; }
}

VOID CALLBACK preview_pic(HWND hwnd, ulong msg, ulong id, long time)
/* Handle the timer for background preview redrawing.
 * Enter: HWND hwnd: handle of calling window.
 *        ulong msg: ignored.
 *        ulong id: id of calling timer.
 *        long time: time in some windows format.             11/10/96-DWM */
{
  long n, x, y, i, pal;
  uchar *s, *buf;
  LPBITMAPINFOHEADER bmp;
  HDC hdc;
  RECT rect;

  if (!slide || (Down && Down<4) || Busy>1)  return;
  if (slide->option&0x4000000)  return;
  BackUpdate++;  i = BackUpdate = BackUpdate%PreCoor[0];
  while (!(((ushort *)(PreCoor+i*2+4))[2]&0x8000) && i<PreCoor[0]) {
    i++;  BackUpdate++; }
  if (i==PreCoor[0]) {
    KillTimer(HwndP, 4);
    i = 0; }
  if (((ushort *)(PreCoor+i*2+4))[2]&0x8000) {
    n = PreCoor[i*2+4];
    KillTimer(HwndP, 4);
    ((ushort *)(PreCoor+i*2+4))[2] &= 0x7FFF;
    x = ((short *)(PreCoor+i*2+4))[2]+5;
    y = ((short *)(PreCoor+i*2+4))[3]+5;
    s = slide->file+FE*n;
    if (!(slide->option&0x4000000)) {
      buf = 0;
      if ((BitsPixel>8 || s[0x2E]) && s[0x2C] && s[0x2D] &&
          s[0x2C]<=slide->tx && s[0x2D]<=slide->ty)
        buf = malloc2(((long *)(slide->thumb+((long *)(s+0x30))[0]))[0]+8);
      if (buf) {
        unlzw(buf, slide->thumb+((long *)(s+0x30))[0]+4,
              ((long *)(slide->thumb+((long *)(s+0x30))[0]))[0]+4,
              ((long *)(s+0x30))[1]-4, 8);
        bmp = (LPBITMAPINFOHEADER)buf;
        pal = bmp->biBitCount;  if (pal<=8) pal = (1<<pal); else pal = 0;
        hdc = GetDC(HwndP);
        if (prevpal) {
          SelectPalette(hdc, prevpal, 0);
          RealizePalette(hdc); }
        if (Down==4 && (ODown[0]&1)) {
          SetRect(&rect, ODown[1], ODown[2], ODown[3], ODown[4]);
          DrawFocusRect(hdc, &rect); }
        SetDIBitsToDevice(hdc, x+(slide->tx-s[0x2C])/2,
            y+(slide->ty-s[0x2D])/2, bmp->biWidth, bmp->biHeight, 0, 0, 0,
            bmp->biHeight, buf+bmp->biSize+pal*4, (BITMAPINFO *)bmp,
            DIB_RGB_COLORS);
        if (Down==4 && (ODown[0]&1)) {
          SetRect(&rect, ODown[1], ODown[2], ODown[3], ODown[4]);
          DrawFocusRect(hdc, &rect); }
        ReleaseDC(HwndP, hdc);
        free2(buf); } }
    for (i=0; i<PreCoor[0]; i++)
      if (((ushort *)(PreCoor+i*2+4))[2]&0x8000)
        break;
    if (i!=PreCoor[0] || !PreCoor[0])
      SetTimer(HwndP, 4, 0, (TIMERPROC)preview_pic); }
}

void preview_print(HDC hdc, long *a)
/* Print the actual preview.
 * Enter: HDC hdc: printer device context.  StartDoc has already been called.
 *        long *a: array of four values.  These are printing area in dots
 *                 width and height, and dpi horizontal and vertical.  The
 *                 print margins are in the global array PrintMarg.
 *                                                             3/26/97-DWM */
{
  long ew, eh, np, full, i;

  cursor(1);
  if (PrintMarg[4]<16 || PrintMarg[4]>10000)  PrintMarg[4] = 100;
  ew = PrintMarg[4]*a[0]/a[2];  eh = PrintMarg[4]*a[1]/a[3];
  SetMapMode(hdc, MM_ANISOTROPIC);
  SetWindowOrgEx(hdc, 0, 0, 0);
  SetViewportOrgEx(hdc, a[2]*PrintMarg[0], a[3]*PrintMarg[2], 0);
  SetWindowExtEx(hdc, ew, eh, 0);
  SetViewportExtEx(hdc, a[0], a[1], 0);
  full = (PrintSet&0x40)!=0;
  np = preview_page(0, ew, eh, -1, full|2);
  for (i=0; i<np; i++) {
    if (StartPage(hdc)<=0)  return;
    preview_page(hdc, ew, eh, i, full|2);
    EndPage(hdc); }
  cursor(0);
}

void preview_purge(void)
/* Discard the thumbnail pictures from the selected preview files.
 *                                                             3/20/97-DWM */
{
  long i, j, num;
  char *s;

  if (!slide)  return;
  cursor(1);
  num = preview_deselect(1);
  if (num==slide->numfile || !num) {
    if (slide->thumb) {
      free2(slide->thumb);  slide->thumb = 0; }
    slide->lenthumb = 0;
    for (i=0; i<slide->numfile; i++) {
      s = slide->file+FE*i;
      ((long *)(s+0x30))[0] = 0;
      s[0x2C] = s[0x2D] = 0; } }
  else
    for (i=0; i<slide->numfile; i++)
      if ((slide->file[FE*i+0x2F]&2) || !num)
        slide_delete(i, 1);
  if ((!slide->lenthumb || !num) && slide->thumb) {
    free2(slide->thumb);  slide->thumb = 0;
    slide->lenthumb = 0; }
  slidename[0] = 1;
  for (i=0; i<PreCoor[0]; i++)
    if ((slide->file[FE*PreCoor[i*2+4]+0x2F]&2) || !num)
      preview_invalidate(PreCoor[i*2+4], 0, 0);
  cursor(0);
}

void preview_rectangles(HDC pdc)
/* Update the selection rectangles in the preview window.
 * Enter: HDC pdc: Either 0 to get a new dc, or a device context to draw
 *                 the rectangles in.                          3/19/97-DWM */
{
  HDC hdc;
  HPEN news, newb, newk, old;
  long x1, y1, w, h, i, n;
  DWORD clr;

  if (!pdc)  hdc = GetDC(HwndP);
  else       hdc = pdc;
  SelectObject(hdc, GetStockObject(NULL_BRUSH));
  newb = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOW));
  newk = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWTEXT));
  clr = GetSysColor(COLOR_ACTIVECAPTION);
  for (i=0; i<30; i++) {
    if (clr!=GetSysColor(COLOR_WINDOWTEXT) && clr!=GetSysColor(COLOR_WINDOW))
      break;
    clr = GetSysColor(i);
  }
  news = CreatePen(PS_SOLID, 1, clr);
  old = SelectObject(hdc, news);
  w = ((short *)PreCoor)[2];  h = ((short *)PreCoor)[3];
  for (i=0; i<PreCoor[0]; i++) {
    n = PreCoor[i*2+4];
    x1 = (((ushort *)(PreCoor+i*2+4))[2]&0x7FFF);
    y1 = ((short *)(PreCoor+i*2+4))[3];
    if ((slide->file[FE*n+0x2F]&6) && (slide->file[FE*n+0x2F]&6)!=6)
      SelectObject(hdc, news);
    else
      SelectObject(hdc, newb);
    Rectangle(hdc, x1, y1, x1+w, y1+h);
    Rectangle(hdc, x1+1, y1+1, x1+w-1, y1+h-1);
    if (n==slide->tsel) {
      SelectObject(hdc, newk);
      rectangle_dotted(hdc, x1+1, y1+1, x1+w-1, y1+h-1, 4); }
    if (n==LastNum)  SelectObject(hdc, newk);
    else             SelectObject(hdc, newb);
    Rectangle(hdc, x1+2, y1+2, x1+w-2, y1+h-2);
    Rectangle(hdc, x1+3, y1+3, x1+w-3, y1+h-3); }
  SelectObject(hdc, old);
  DeleteObject(newb);
  DeleteObject(news);
  DeleteObject(newk);
  if (!pdc)  ReleaseDC(HwndP, hdc);
}

void preview_save(void)
/* Save the preview images as a series of graphic files.       3/28/97-DWM */
{
  long full, i, j, w, h, scan, pal, ext, err, page;
  HDC hdc, hdc2;
  HBITMAP membmp, bmp, oldbmp;
  uchar *dest, *buf, gspec[SAVELEN+2], gspec2[SAVELEN*2];
  char base[NAMELEN], name[NAMELEN], *root, num[12];

  Busy = 1;
  if (!DialogBox(hinst, MAKEINTRESOURCE(PrevSDLG), HwndP, (DLGPROC)preview_save_dialog)) {
    Busy = 0;
    return;
  }
  Busy = 0;
  if (!save_as(6))
    return;
  full = ((slide->option&0x01000000)!=0);
  page = preview_page(0, w=PrevSaveX, h=PrevSaveY, -1, full);
  gspec[0] = SaveFilt-1;
  gspec[1] = 0;
  memcpy(gspec+2, SaveRec+SAVELEN*(SaveFilt-1), SAVELEN);
  if (gspec[0]==5)
    ((long *)(gspec+0xC))[0] = (long)(gquant);
  strpath(savedir+1024, opfile);
  strcpy(base, opfile);
  for (i=0; i<page; i++) {
    hdc2 = GetDC(HwndP);
    hdc = CreateCompatibleDC(hdc2);
    membmp = CreateCompatibleBitmap(hdc2, w, h);
    ReleaseDC(HwndP, hdc2);
    if (!membmp) {
      DeleteDC(hdc);
      if (Msg&1)  MsgBox(HwndP, "Can't draw preview picture", "Error",MB_OK);
      return; }
    oldbmp = SelectObject(hdc, membmp);
    cursor(1);
    preview_page(hdc, w, h, i, full);
    SelectObject(hdc, oldbmp);
    dest = BitmapToBMP(membmp, 0, hdc);
    DeleteDC(hdc);
    if (!dest) {
      low_memory(HwndP, "Clipboard Allocation", 0);
      DeleteObject(membmp);
      cursor(0);  return; }
    DeleteObject(membmp);
    strcpy(name, base);
    root = strchr(name, '\\');
    if (root)  root++;
    else       root = name;
    if (strchr(root, '.'))  ext = strchr(root, '.')-name;
    else                    ext = strlen(base);
    sprintf(num, "%d", i+1);
    if (ext-(root-name)<=8) {
      while (ext-(root-name)+strlen(num)>8)
        ext--; }
    sprintf(name+ext, "%s%s", num, base+ext);
    memcpy(gspec2, gspec, SAVELEN+2);
    buf = bmp_to_graphic(dest);
    free2(lock2(dest));
    if (buf)
      err = save_graphic(name, buf, gspec2, 1);
    else
      err = 2;
    if (err) {
      if (Msg&1)
        MsgBox(Hwnd, ErrorMsg2[err-1], "Error", MB_OK);
      return; }
    cursor(0); }
}

BOOL CALLBACK preview_save_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the Save Preview Screens dialog box.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        5/26/96-DWM */
{
  char text[80];
  long w, h, num, full;

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd,HelpFile,HELP_CONTEXT,HelpPreviewSave);
        break;
      case IDOK: GetDlgItemText(hdlg, PrevSWidth, text, 79);
        sscanf(text, "%d", &PrevSaveX);
        GetDlgItemText(hdlg, PrevSHeight, text, 79);
        sscanf(text, "%d", &PrevSaveY);
        if (PrevSaveX<10)  PrevSaveX = 10;
        if (PrevSaveY<10)  PrevSaveY = 10;
        slide->option &= 0xFEFFFFFF;
        if (SendDlgItemMessage(hdlg,PrevSOpt2,BM_GETCHECK,0,0))
          slide->option |= 0x01000000;
        SlideOpt = slide->option;
        EndDialog(hdlg, 1);  return(1);
      case IDCANCEL: EndDialog(hdlg, 0);  return(1);
      case PrevSOpt1: case PrevSOpt2: case PrevSHeight: case PrevSWidth:
        w = PrevSaveX;  h = PrevSaveY;
        GetDlgItemText(hdlg, PrevSWidth, text, 79);   sscanf(text, "%d", &w);
        GetDlgItemText(hdlg, PrevSHeight, text, 79);  sscanf(text, "%d", &h);
        if (w<10)  w = 10;  if (h<10)  h = 10;
        num = PreCoorMax;
        full = SendDlgItemMessage(hdlg,PrevSOpt2,BM_GETCHECK,0,0);
        if (!full) num = preview_deselect(1);
        sprintf(text, "%d", num);
        SetDlgItemText(hdlg, PrevSNum1, text);
        sprintf(text, "%d", preview_page(0, w, h, -1, full));
        SetDlgItemText(hdlg, PrevSNum2, text);  return(0);
      default: return(0); } break;
    case WM_INITDIALOG:
      if (slide->tsel<0)  slide->tsel = 0;
      preview_compute();
      if (!preview_deselect(1))
        slide->file[slide->tsel*FE+0x2F] |= 2;
      SendDlgItemMessage(hdlg, PrevSOpt2, BM_SETCHECK,
                         (slide->option&0x01000000)!=0, 0);
      SendDlgItemMessage(hdlg, PrevSOpt1, BM_SETCHECK,
                         !(slide->option&0x01000000), 0);
      sprintf(text, "%d", PrevSaveX);
      SetDlgItemText(hdlg, PrevSWidth, text);
      sprintf(text, "%d", PrevSaveY);
      SetDlgItemText(hdlg, PrevSHeight, text);
      SetFocus(hdlg);
      return(1); }
  return(0);
}

void preview_show(void)
/* Show the currently selected image.                          3/18/97-DWM */
{
  char name[NAMELEN], *s;
  long num;

  if (slide->tsel<0 || slide->tsel>=slide->numfile)  return;
  SetForegroundWindow(Hwnd);
  if (LastNum==slide->tsel)  return;
  s = slide->file+slide->tsel*FE;
  sprintf(name, "\"%s\\%s", slide->dirname+((long *)(slide->dir+
          ((short *)s)[0]*6))[0], slide->filename+((long *)(s+2))[0]);
  num = (((uchar *)s)[0xC+1]&0x7F);
  if (s[0xC]<0 || s[0xC]==2 || s[0xC]==5 || s[0xC]==9)
    num += (((short *)(s+0xC+0x12))[0]<<7);
  if (s[0xC+1] || num)
    sprintf(name+strlen(name), "?%d", num+1);
  strcat(name, "\"");
  MultiOpen |= 2;
  open_pic_mid(Hwnd, name, 0);
}

VOID CALLBACK preview_timer(HWND hwnd, ulong msg, ulong id, long time)
/* Handle the timer for producing the preview thumbnails.
 * Enter: HWND hwnd: handle of calling window.
 *        ulong msg: ignored.
 *        ulong id: id of calling timer.
 *        long time: time in some windows format.             11/10/96-DWM */
{
  static long check=-1, check2=-1, check3=0, refresh=-1, refdelay=0;
  long read=0, w, h, aw, ah, pal, more, i, len, comp, last, rloop, dith, num;
  uchar *s, *pic, *pic2, *new, *buf;
  char name[NAMELEN];
  short *p;
  HBITMAP bmp;
  RECT rect, rect2;

  if (!slide || (Down && Down!=5) || Busy || LoadPart>0)  return;
  if (slide->option&0x6000000)  return;
  for (i=0; i<PreCoor[0]; i++)
    if (((ushort *)(PreCoor+i*2+4))[2]&0x8000)  return;
  KillTimer(HwndP, 3);
  for (rloop=0; rloop<10 && !read; rloop++) {
    comp = -1;  bmp = 0;
    if (!check3) {
      check++;  check = check%slide->numfile;  last = check;
      check3 = 1; }
    else {
      check2++;  check2 = check2%PreCoor[0];  last = PreCoor[check2*2+4];
      check3 = 2; }
    s = slide->file+FE*last;
    if (!(((ulong *)(s+0x20))[0]&0x80000000) || ((char *)s)[0xC]==-1)
      if ((BitsPixel<=8 && !s[0x2E]) || !s[0x2C] || !s[0x2D] ||
          s[0x2C]>slide->tx || s[0x2D]>slide->ty || ((char *)s)[0xC]==-1 ||
          (s[0x2C]!=slide->tx && s[0x2D]!=slide->ty && !(s[0x2F]&1)))
        read = 1;
    if (!read && refresh>=0) {
      refdelay++;
      if (refdelay==100 && refresh<PreCoor[0]) {
        preview_invalidate_end(refresh);
        refresh = -1;  refdelay = 0; } }
    if (read==1) {
      cursor(1);
      sprintf(name, "%s\\%s", slide->dirname+((long *)(slide->dir+
              ((short *)s)[0]*6))[0], slide->filename+((long *)(s+2))[0]);
      LoadPart = -1;
      more = ((char *)s)[0xC];
      num = (((uchar *)s)[0xC+1]&0x7F);
      if (s[0xC]==0xFF || s[0xC]==2 || s[0xC]==5 || s[0xC]==9)
        num += (((short *)(s+0xC+0x12))[0]<<7);
      ppart = num;
      Minpwidth = slide->tx;  Minpheight = slide->ty;
      pic = load_graphic(name);
      slidename[0] = 1;
      if (!pic) {
        ((ulong *)(s+0x20))[0] |= 0x80000000;
        s[0xC] = -2;
        for (i=0; i<PreCoor[0]; i++)
          if (PreCoor[i*2+4]==last)
            comp = i;
        if (comp==-1 && PreCoor[4]>last)  comp = PreCoor[0];
        if (slide->option&0x80000000)     comp = -1;
        read = 0; }
      else {
        ((short *)(s+6))[0] = pwidth;  ((short *)(s+6))[1] = pheight;
        ((short *)(s+6))[2] = pinter;
        memcpy(s+0xC, pspec, LENPSPEC);
        p = (short *)(s+0x24);
        if (!p[2])  p[2] = pwidth;  if (!p[3])  p[3] = pheight;
        if (more==-1 && (pspec[1]&0x80))
          if ((new=realloc2(slide->file, (slide->numfile+1)*FE))) {
            slide->file = new;
            memmove(slide->file+(last+1)*FE, slide->file+last*FE,
                    (slide->numfile-last)*FE);
            slide->numfile ++;
            s = slide->file+FE*last;
            memset(s+FE+0x24, 0, 20);
            ((char *)s)[FE+0xC] = -1;
            num = ((uchar *)pspec)[1]&0x7F;
            if (pspec[0]==2 || pspec[0]==5 || pspec[0]==9)
              num += (((short *)(pspec+0x12))[0]<<7);
            s[FE+0xC+1] = (num+1)&0x7F;
            if (pspec[0]==2 || pspec[0]==5 || pspec[0]==9)
              ((short *)(s+FE+0xC+0x12))[0] = ((num+1)>>7);
            for (i=0; i<PreCoor[0]; i++)
              if (PreCoor[i*2+4]==last)
                comp = i; }
            if (comp==-1 && PreCoor[4]>last)  comp = PreCoor[0]; }
      cursor(0); }
    if (read) {
      cursor(1);
      p = (short *)(s+0x24);
      if (p[0]<0 || p[0]>p[2])  p[0] = 0;   if (p[2]>pwidth)  p[2] = pwidth;
      if (p[1]<0 || p[1]>p[3])  p[1] = 0;   if (p[3]>pheight) p[3] = pheight;
      if (p[0] || p[1] || p[2]!=pwidth || p[3]!=pheight) {         /* crop */
        aw = p[2]-p[0];  if (!pinter)  aw *= 3;
        for (ah=0; ah<p[3]-p[1]; ah++)
          memmove(pic+768*pinter+aw*ah, pic+768*pinter+
                  ((ah+p[1])*pwidth+p[0])*(3-2*pinter), aw);
        pwidth = p[2]-p[0];  pheight = p[3]-p[1]; }
      aw = pwidth;  ah = pheight;  pal = pinter;
      if (pwidth>slide->tx || pheight>slide->ty) {
        w = slide->tx;  h = pheight*w/pwidth;
        if (h>slide->ty) {
          h = slide->ty;  w = pwidth*h/pheight;
          if (w>slide->tx)  w = slide->ty; }
        if (!w)  w = 1;  if (!h)  h = 1;
        pic2 = scale_graphic(pwidth, pheight, pal, pic, w, h, 0);
        free2(pic);
        pic = pic2; }
      else {
        w = aw;  h = ah; }
      if (BitsPixel==8 && pic) {
        preview_palette_set();
        dith = Dither;
        pic2 = palettize_graphic(w, h, pal, pic, 0, PreviewPal);
        Dither = dith;
        free2(pic);
        pic = pic2;  pal = 1; }
      if (pic) {
        bmp = graphic_to_bmp(w, h, pal, pic, 1);  free2(pic);  pic = 0; }
      if (bmp) {
        pic = lock2(bmp);
        i = size2(pic)*1.391+16;
        if (!(pic2=malloc2(i+16+28672))) {
          free2(pic);  pic = 0; }
        else {
          len = lzw(pic2+4, pic, pic2+i+8, size2(pic), i, 8);
          ((long *)pic2)[0] = size2(pic);
          free2(pic);
          pic = pic2; } }
      if (pic) {
        slide_delete(last, 1);
        if (!slide->thumb) {
          slide->thumb = malloc2(len+4);
          slide->lenthumb = 0; }
        else {
          new = realloc2(slide->thumb, slide->lenthumb+len+4);
          if (!new) { free2(pic);  pic = 0; }
          else       slide->thumb = new; } }
      if (pic) {
        memcpy(slide->thumb+slide->lenthumb, pic, len+4);
        free2(pic);
        s[0x2C] = w;  s[0x2D] = h;  s[0x2E] = pal;
        s[0x2F] = (s[0x2F]&0xFE)|(w==aw && h==ah);
        ((long *)(s+0x30))[0] = slide->lenthumb;
        ((long *)(s+0x30))[1] = len+4;
        slide->lenthumb += len+4;
        for (i=0; i<PreCoor[0]; i++)
          if (PreCoor[i*2+4]==last) {
            refdelay = 0;
            rect.left = (((ushort *)(PreCoor+i*2+4))[2]&0x7FFF)+5;
            rect.top = ((short *)(PreCoor+i*2+4))[3]+5;
            rect.right = rect.left+((short *)PreCoor)[2]-10;
            rect.bottom = rect.top+((short *)PreCoor)[3]-10;
            InvalidateRect(HwndP, &rect, 1); } }
      cursor(0); }
    if (comp>=0) {
      if (comp<refresh || refresh==-1)
        refresh = comp;
      preview_compute(); } }
  if (check3==2)  check3 = read;
  SetTimer(HwndP, 3, 10+90*read, (TIMERPROC)preview_timer);
}

void preview_update(HWND hwnd)
/* Update the preview window.  This draws existing bitmaps and prints the
 *  appropriate text.
 * Enter: HWND hwnd: handle for painting.                      3/11/97-DWM */
{
  HDC hdc;
  PAINTSTRUCT paint;
  RECT rect;
  long i, j, k, l, w, h, x, y, n, w2, pal, num;
  char text[300], tbl[]="-+xx";
  uchar *s, *buf;
  LPBITMAPINFOHEADER bmp;

  if (!slide)  return;
  ODown[0] |= 2;
  hdc = BeginPaint(hwnd, &paint);
  if (prevpal) {
    SelectPalette(hdc, prevpal, 0);
    RealizePalette(hdc); }
  preview_compute();
  if (Down==4 && (ODown[0]&1)) {
    SetRect(&rect, ODown[1], ODown[2], ODown[3], ODown[4]);
    DrawFocusRect(hdc, &rect); }
  set_font(hdc, FF_SWISS, 12, GetSysColor(COLOR_WINDOWTEXT),
           GetSysColor(COLOR_WINDOW));
  preview_rectangles(hdc);
  w = ((short *)PreCoor)[2];  h = ((short *)PreCoor)[3];
  for (i=0; i<PreCoor[0]; i++) {
    n = PreCoor[i*2+4];
    x = (((ushort *)(PreCoor+i*2+4))[2]&0x7FFF);
    y = ((short *)(PreCoor+i*2+4))[3];
    if (x+5>paint.rcPaint.right || y+5>paint.rcPaint.bottom ||
        x+w-5<paint.rcPaint.left || y+h-5<paint.rcPaint.top)  continue;
    s = slide->file+FE*n;
    x += 5;  y += 5;
    if (!(slide->option&0x4000000)) {
      buf = 0;
      if ((BitsPixel>8 || s[0x2E]) && s[0x2C] && s[0x2D] &&
          s[0x2C]<=slide->tx && s[0x2D]<=slide->ty) {
        ((ushort *)(PreCoor+i*2+4))[2] |= 0x8000;
        BackUpdate = -1;
        SetTimer(HwndP, 4, 0, (TIMERPROC)preview_pic); }
      y += max(10, slide->ty); }
    if (!(slide->option&0x8000000)) {
      sprintf(text, "%c %s", tbl[((ulong *)(slide->file+FE*n+0x20))[0]>>30],
              slide->filename+((long *)(slide->file+FE*n+2))[0]);
      num = (((uchar *)s)[0xC+1]&0x7F);
      if (s[0xC]<0 || s[0xC]==2 || s[0xC]==5 || s[0xC]==9)
        num += (((short *)(s+0xC+0x12))[0]<<7);
      if (s[0xC+1] || num)
        sprintf(text+strlen(text), " - %d", 1+num);
      draw_text(hdc, text, x, y, w-10, 12);
      y += 12; }
    if ((slide->option&0x60000000)!=0x60000000) {
      if (!(slide->option&0x20000000))
        sprintf(text, "%d x %d", ((short *)(slide->file+FE*n+6))[0],
                ((short *)(slide->file+FE*n+6))[1]);
      else  text[0] = 0;
      if (!(slide->option&0x60000000))
        strcat(text, " x ");
      if (!(slide->option&0x40000000))
        sprintf(text+strlen(text), "%d bit",
                24-16*((short *)(slide->file+FE*n+6))[2]);
      if (((short *)(slide->file+FE*n+6))[0])
        draw_text(hdc, text, x, y, w-10, 12);
      y += 12; }
    if (!(slide->option&0x10000000)) {
      for (l=j=0; l<30; l++)
        if (((short *)(slide->cat+l*8+4))[0])
          if ((((ulong *)(slide->file+FE*n+0x20))[0]&(1<<l))!=0)
            j++;
      if (j)  w2 = ((w-3)/j)-7;
      else    w2 = 0;
      if (w2>0) for (l=k=0; l<30; l++)
        if (((short *)(slide->cat+l*8+4))[0])
          if ((((ulong *)(slide->file+FE*n+0x20))[0]&(1<<l))!=0) {
            draw_text(hdc, slide->catname+((long *)(slide->cat+l*8))[0],
                      x+k*(w2+7), y, w2, 12);
            k++; } } }
  if (Down==4 && (ODown[0]&1)) {
    SetRect(&rect, ODown[1], ODown[2], ODown[3], ODown[4]);
    DrawFocusRect(hdc, &rect); }
  set_font(hdc, 0, 0, 0, 0);
  EndPaint(hwnd, &paint);
  ODown[0] &= 0xFD;
}

BOOL CALLBACK print_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the print layout dialog box.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        6/11/96-DWM */
{
  char text[80];
  long i;
  HWND ctrl;

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd,HelpFile,HELP_CONTEXT,
                             HelpPrint+(PrintSet&0x20)!=0); break;
      case IDCANCEL: PrintSet &= 0xFF;  PrintSet |= 16;
        return(0);
      case IDOK: for (i=0; i<4; i++)
          print_getitem(hdlg, PrMargin1+i, PrintMarg+5+i, 0);
        print_getitem(hdlg, PrDPI, PrintMarg+9, 1);
        if (PrintSet&0x20) {
          PrintSet &= 0xF;
          PrintSet |=0x40*SendDlgItemMessage(hdlg,VRadio2,BM_GETCHECK,0,0); }
        else {
          PrintSet &= 0x41;
          PrintSet |= 2*SendDlgItemMessage(hdlg,MRadio2,BM_GETCHECK,0,0);
          PrintSet |= 4*SendDlgItemMessage(hdlg,CRadio2,BM_GETCHECK,0,0);
          PrintSet |= 8*SendDlgItemMessage(hdlg,VRadio2,BM_GETCHECK,0,0); }
        memmove(PrintMarg, PrintMarg+5, 5*sizeof(float));
      default: return(0); }
    case WM_INITDIALOG: PrintSet = (PrintSet&0xFF)*0x101;
      memmove(PrintMarg+5, PrintMarg, 5*sizeof(float));
      for (i=0; i<4; i++)
        print_setitem(hdlg, PrMargin1+i, PrintMarg[5+i], 0);
      if (PrintSet&0x20) {
        ctrl = GetDlgItem(hdlg, MRadio1);   DestroyWindow(ctrl);
        ctrl = GetDlgItem(hdlg, MRadio2);   DestroyWindow(ctrl);
        ctrl = GetDlgItem(hdlg, CRadio1);   DestroyWindow(ctrl);
        ctrl = GetDlgItem(hdlg, CRadio2);   DestroyWindow(ctrl);
        SetDlgItemText(hdlg, VRadio1, "Print S&election");
        SetDlgItemText(hdlg, VRadio2, "Print &Whole Preview");
        SendDlgItemMessage(hdlg,VRadio1,BM_SETCHECK,!(PrintSet&0x4000), 0);
        SendDlgItemMessage(hdlg,VRadio2,BM_SETCHECK,
                           (PrintSet&0x4000)!=0, 0); }
      else {
        SendDlgItemMessage(hdlg,MRadio1,BM_SETCHECK,!(PrintSet&0x200),0);
        SendDlgItemMessage(hdlg,MRadio2,BM_SETCHECK,(PrintSet&0x200)!=0,0);
        SendDlgItemMessage(hdlg,CRadio1,BM_SETCHECK,!(PrintSet&0x400),0);
        SendDlgItemMessage(hdlg,CRadio2,BM_SETCHECK,(PrintSet&0x400)!=0,0);
        SendDlgItemMessage(hdlg,VRadio1,BM_SETCHECK,!(PrintSet&0x800),0);
        SendDlgItemMessage(hdlg,VRadio2,BM_SETCHECK,(PrintSet&0x800)!=0,0); }
      print_setitem(hdlg, PrDPI, PrintMarg[9], 1);
      if (!(PrintSet&0x100))
        sprintf(text, "Printable Area: %4.2f by %4.2f in", PrintMarg[10],
                PrintMarg[11]);
      else
        sprintf(text, "Printable Area: %1.0f by %1.0f mm",PrintMarg[10]*25.4,
                PrintMarg[11]*25.4);
      SetDlgItemText(hdlg, PrintPage, text);
      SetFocus(hdlg);
      return(0); }
  return(0);
}

void print_getitem(HWND hdlg, long item, float *value, long type)
/* Get the value from a control in the print dialog.  This handles numbers
 *  which are fractions and also switches units.
 * Enter: HWND hdlg: pointer to dialog.
 *        long item: item number to get.
 *        float *value: location to store results.
 *        long type: 0 for inches or mm, 1 for dpi.            6/11/96-DWM */
{
  char text[80];
  long temp;
  float val[3];

  GetDlgItemText(hdlg, item, text, 79);
  if (type) {
    sscanf(text, "%d", &temp);
    if (temp>=0)  value[0] = temp;
    return; }
  temp = sscanf(text, "%f%*c%f%*c%f", val, val+1, val+2);
  if (temp==2 && val[1])  val[0] /= val[1];
  if (temp==3 && val[2])  val[0] += val[1]/val[2];
  temp = PrintSet&1;
  if (toupper(text[strlen(text)-1])=='M')  temp = 3;
  if (toupper(text[strlen(text)-1])=='N')  temp = 2;
  if (text[strlen(text)-1]=='\"')          temp = 2;
  if (temp&1)  val[0] /= 25.4;
  value[0] = val[0];
  if (temp==2 && (PrintSet&1))   PrintSet &= 0xFEFF;
  if (temp==3 && !(PrintSet&1))  PrintSet |= 0x0100;
}

void print_setitem(HWND hdlg, long item, float value, long type)
/* Set a control in the print dialog based on a specified value, usually in
 *  inches or mm.
 * Enter: HWND hdlg: pointer to dialog.
 *        long item: item number to set.
 *        float value: value to write (in inches or dpi).
 *        long type: 0 for inches or mm, 1 for dpi.            6/11/96-DWM */
{
  char text[80];

  if (type)
    sprintf(text, "%g", value);
  else {
    if (!(PrintSet&0x100))
      sprintf(text, "%4.2f in", value);
    else
      sprintf(text, "%1.0f mm", value*25.4); }
  SetDlgItemText(hdlg, item, text);
}

void printer(HWND hwnd, long setup)
/* Open the printing dialog, then print the current image.
 * Enter: long setup: 1 for setup, 0 for actually print, 2 to print preview.
 *                                                              6/8/96-DWM */
{
  HANDLE bmp;
  static HDC hdc;
  LPBITMAPINFOHEADER header;
  LPSTR buf;
  static DOCINFO di;
  long w, h, px, py, pw, ph, i, old, oldr, prev=(setup==2);
  static long a[4];

  setup = (setup==1);
  if (prev)  PrintSet |= 0x20;  else  PrintSet &= 0xDF;
  printdlg.lStructSize = sizeof(PRINTDLG);  printdlg.hwndOwner = hwnd;
  printdlg.hDevMode = devmode;  printdlg.hDevNames = 0;  printdlg.hDC = 0;
  printdlg.Flags = PD_RETURNDC|PD_NOPAGENUMS|PD_NOSELECTION|
                   PD_ENABLEPRINTTEMPLATE|PD_ENABLEPRINTHOOK;
  if (!(Msg&1))
    printdlg.Flags |= PD_NOWARNING;
  printdlg.nFromPage = printdlg.nToPage = 0;
  printdlg.nMinPage = printdlg.nMaxPage = 0;  printdlg.nCopies = 1;
  printdlg.hInstance = hinst;  printdlg.lCustData = 0;
  printdlg.lpfnPrintHook = (LPPRINTHOOKPROC)print_dialog;
  printdlg.lpfnSetupHook = 0;
  printdlg.lpPrintTemplateName = MAKEINTRESOURCE(PrDLG);
  printdlg.lpSetupTemplateName = 0;
  printdlg.hPrintTemplate = printdlg.hSetupTemplate = 0;
  if (setup)
    printdlg.Flags |= PD_PRINTSETUP;
  Busy = 1;
  if (!PrintDlg(&printdlg)) {
    Busy = 0;  return; }
  Busy = 0;
  if (setup) {
    devmode = printdlg.hDevMode;
    DeleteDC(printdlg.hDC);  return; }
  hdc = printdlg.hDC;
  di.cbSize = sizeof(DOCINFO);
  di.lpszDocName = "GV Printing";
  di.lpszOutput = 0;
  if (StartDoc(hdc, &di)<=0) {
    DeleteDC(printdlg.hDC);  return; }
  a[0] = GetDeviceCaps(hdc, HORZRES);
  a[1] = GetDeviceCaps(hdc, VERTRES);
  a[2] = GetDeviceCaps(hdc, LOGPIXELSX);
  a[3] = GetDeviceCaps(hdc, LOGPIXELSY);
  for (i=0; i<4; i++)  if (a[i]<=0)  a[i] = 1;
  PrintMarg[10] = (float)a[0]/a[2];  PrintMarg[11] = (float)a[1]/a[3];
  a[0] -= a[2]*(PrintMarg[0]+PrintMarg[1]);
  a[1] -= a[3]*(PrintMarg[2]+PrintMarg[3]);
  if (prev)
    preview_print(hdc, a);
  else {
    if (StartPage(hdc)<=0) {
      EndDoc(hdc);  DeleteDC(printdlg.hDC);  return; }
    cursor(1);
    old=Scale;  oldr=Reduce;  CopyMode = 1;
    Scale = ((PrintSet&8)!=0);  Reduce = 0;  rescale |= 2;  prep_pic(hwnd);
    w = curwidth;  h = curheight;
    if (!w)  w = 1;  if (!h)  h = 1;
    if (!(PrintSet&2)) {
      pw = a[0];  ph = (float)h/w*a[3]/a[2]*pw;
      if (ph>a[1]) {
        ph = a[1];  pw = (float)w/h*a[2]/a[3]*ph; } }
    else {
      if (!PrintMarg[4])  PrintMarg[4] = 1;
      pw = (float)w*a[2]/PrintMarg[4];
      ph = (float)h*a[3]/PrintMarg[4]; }
    px = (a[0]-pw)/2*(!(PrintSet&4))+a[2]*PrintMarg[0];
    py = (a[1]-ph)/2*(!(PrintSet&4))+a[3]*PrintMarg[2];
    bmp = wpic;
    header = GlobalLock(bmp);
    buf = (LPSTR)header + header->biSize + 1024*bmpi;
    StretchDIBits(hdc, px, py, pw, ph, 0, 0, w, h, buf, (LPBITMAPINFO)header,
                  DIB_RGB_COLORS, SRCCOPY);
    GlobalUnlock(bmp);
    CopyMode = 0;
    Scale = old;  Reduce = oldr;  rescale |= 2;  prep_pic(hwnd);
    EndPage(hdc); }
  EndDoc(hdc);
  DeleteDC(hdc);
  devmode = printdlg.hDevMode;
  cursor(0);
}

void quit(void)
/* Exit the program.                                            5/8/96-DWM */
{
  prep_undo(Hwnd);
  if (HwndP)
    SendMessage(HwndP, WM_CLOSE, 0, 0);
  write_ini(0);
  PostQuitMessage(0);
}

read_ini()
/* Locate and read in values from the INI file.                5/26/96-DWM */
{
  char cmd[MAX_PATH+2], text[256], text2[32], *t2;
  long i, j, k, def;

  memset(cmd, 0, MAX_PATH+2);
  GetModuleFileName(0, cmd, MAX_PATH+1);
  if (strlen(cmd)<MAX_PATH) {
    strcpy(inifile, cmd);
    remove_quotes(inifile);
    if (strchr(inifile, '\\'))  strrchr(inifile, '\\')[0] = 0;
    strcat(inifile, "\\GV.INI"); }
  for (i=0; key[i*3]; i++) {
    GetPrivateProfileString(inihead, key[i*3], key[i*3+1], text, 256, inifile);
    def = 1-(!strcmpi(text, key[i*3+1]));
    switch (i) {
      case 0: sscanf(text, "%d", &Redox); break;
      case 1: Msg = (Msg&0xFFFE)+def; break;
      case 2: Msg = (Msg&0xFFFD)+(!def)*2; break;
      case 3: if (strlen(text)) strcpy(opendir, text); break;
      case 4: Undo[2] = -1;
        sscanf(text, "%d%*c%d%*c%d%*c%d", Undo, Undo+1, Undo+2, Undo+3);
        OrigX = Undo[0];  OrigY = Undo[1]; break;
      case 5: sscanf(text, "%d", &FilterType); break;
      case 6: Reduce = def; break;
      case 7: Dither = def; break;
      case 8: sscanf(text, "%d", &Letter); break;
      case 9: sscanf(text, "%d", &Lock);  Lock &= 0xFFF7; break;
      case 10: sscanf(text, "%f%*c%f*c%f*c%f", &AspectX, &AspectY, AspectXY,
                      AspectXY+1); break;
      case 11: Topmost = def; break;
      case 12: sscanf(text, "%f%*c%f%*c%f%*c%f%*c%f%*c%d", PrintMarg,
                      PrintMarg+1, PrintMarg+2, PrintMarg+3, PrintMarg+4,
                      &PrintSet); break;
      case 13: Msg = (Msg&0xFFFB)+def*4; break;
      case 14: sscanf(text, "%d", &PreviewOpt); break;
      case 15: if (strlen(text)) strcpy(savedir, text); break;
      case 16: sscanf(text, "%d%*c%d", &SaveFilt, &SaveOpt); break;
      case 17: MultiOpen = def; break;
      case 18: if (strlen(text)) strcpy(opendir+256, text); break;
      case 19: if (strlen(text)) strcpy(savedir+256, text); break;
      case 20: if (strlen(text)) strcpy(opendir+512, text); break;
      case 21: if (strlen(text)) strcpy(savedir+512, text); break;
      case 22: if (strlen(text)) strcpy(titleimage+1, text);
        else {
          strcpy(titleimage+1, inifile);
          sprintf(titleimage+1+strlen(titleimage+1)-3, "TIF"); } break;
      case 23: for (j=0; j<sizeof(WINDOWPLACEMENT) && j*2<strlen(text); j++)
          sscanf(text+j*2, "%2X", (int *)(((char *)(&WinPlace.length))+j));
          break;
      case 24: if (strlen(text)) strcpy(opendir+768, text); break;
      case 25: if (strlen(text)) strcpy(savedir+768, text); break;
      case 26: sscanf(text, "%d", &SlideOpt); break;
      case 27: sscanf(text, "%d", &SlideDelay); break;
      case 28: for (j=0; j<sizeof(WINDOWPLACEMENT) && j*2<strlen(text); j++)
          sscanf(text+j*2, "%2X", (int *)(((char *)(&WinPlace2.length))+j));
          break;
      case 29: sscanf(text, "%d", &SlideTx); break;
      case 30: sscanf(text, "%d", &SlideTy); break;
      case 31: sscanf(text, "%d%*c%d%*c%d", &SortVal, &SortDir, &FindType);
        break;
      case 32: if (strlen(text)) strcpy(opendir+1024, text); break;
      case 33: if (strlen(text)) strcpy(lastview,     text); break;
      case 34: if (strlen(text)) strcpy(lastview+256, text); break;
      case 35: if (strlen(text)) strcpy(lastview+512, text); break;
      case 36: if (strlen(text)) strcpy(lastview+768, text); break;
      case 37: if (strlen(text)) strcpy(savedir+1024, text); break;
      case 38: sscanf(text, "%d%*c%d", &PrevSaveX, &PrevSaveY); break;
      case 39: break;  // regnum
      case 40: if (strlen(text) && strlen(text)<255) strcpy(RegName, text);
        break;
      case 41: if (strlen(text) && strlen(text)==8) strcpy(RegPass, text);
        break;
      case 42: Interpolate = def;
        j = -1;  sscanf(text, "0x%x", &j);  if (j<0)  sscanf(text, "%x", &j);
        if (j>=0)  Interpolate = j; break;
      case 43: HalfSizeBatch = def; break;
      case 44: if (strlen(text)) strcpy(savedir+5*256, text); break;
      case 45: MultiSave = def; break;
      default: ; } }
  memcpy(SaveRec, SaveDefault, SAVEFORMATS*SAVELEN);
  for (i=0; i<SAVEFORMATS; i++) {
    sprintf(text2, "SaveOption%d", i);
    GetPrivateProfileString(inihead, text2, "", text, 256, inifile);
    for (j=0; j<SAVELEN; j++) {
      sscanf(text+j*2, "%2X", &k);
      SaveRec[SAVELEN*i+j] = k; } }
  memset(gquant, 0x0A, 128);
  for (i=0; i<4; i++) {
    sprintf(text2, "CustomQuant%d", i);
    GetPrivateProfileString(inihead, text2, "", text, 256, inifile);
    for (j=0; j<32; j++)
      sscanf(text+j*2, "%2X", (int *)(gquant+i*32+j)); }
  for (i=1; i<4; i++) {
    if (!opendir[256*i])  strcpy(opendir+256*i, opendir);
    if (!savedir[256*i])  strcpy(savedir+256*i, savedir); }
  if (savedir[5*256])
    for (i=0; i<5; i++)
      strcpy(savedir+256*i, savedir+5*256);
  if (strchr(titleimage+1, ' ') && !strchr(titleimage+1, '"')) {
    memmove(titleimage+2, titleimage+1, strlen(titleimage+1)+1);
    titleimage[1] = '"';
    strcat(titleimage+1, "\""); }
}

void rectangle_dotted(HDC dc, long x1, long y1, long x2, long y2, long dot)
/* Draw a rectangle composed of dotted lines.
 * Enter: HDC dc: device context to draw within.
 *        long x1, y1: first corner of the rectangle.
 *        long x2, y2: second corner of the rectangle.
 *        long dot: length of dots.                            3/19/97-DWM */
{
  long temp, i, len=dot*2;

  x2--;  y2--;
  if (x1>x2) { temp = x1;  x1 = x2;  x2 = temp; }
  if (y1>y2) { temp = y1;  y1 = y2;  y2 = temp; }
  for (i=x1; i<=x2; i+=len) {
    MoveToEx(dc, i, y1, 0);
    LineTo(dc, min(i+dot, x2), y1);
    MoveToEx(dc, i, y2, 0);
    LineTo(dc, min(i+dot, x2), y2); }
  for (i=y1; i<=y2; i+=len) {
    MoveToEx(dc, x1, i, 0);
    LineTo(dc, x1, min(i+dot, y2));
    MoveToEx(dc, x2, i, 0);
    LineTo(dc, x2, min(i+dot, y2)); }
}

void reduce_res(HWND hwnd, long all)
/* Reduce the picture to 8-bits if the current display is 8-bits or less.
 * Enter: HWND hwnd: handle of window to do this 'for'.
 *        long all: 0 to only do if the Reduce flag is set, 1 for do in any
 *                  case.                                      5/25/96-DWM */
{
  uchar *apic;
  long i, r, g, b;

  if (!oldi && BitsPixel<=8 && (Reduce || all)) {
    if (!(apic=palettize_graphic(oldw, oldh, 0, pic, Dither, 0))) {
      low_memory(hwnd, "Reducing Color Depth\n\nUsing Windows 24 to 8-bit.",
                 0); return; }
    free2(pic);
    pic = apic;
    oldi = 1; }
  if (!oldi && (BitsPixel>8 && BitsPixel<=16) && all && Dither) {
    if (dither16num<0) {
      for (i=0; i<2048; i++)
        dither16tbl[i] = rand()&7;
      dither16num = 0; }
    for (i=0; i<oldw*oldh*3; i+=3) {
      pic[i] = min(255,pic[i]+dither16tbl[dither16num]);
      pic[i+1] = min(255,pic[i+1]+(dither16tbl[dither16num+1]&3));
      pic[i+2] = min(255,pic[i+2]+dither16tbl[dither16num+2]);
      dither16num = (dither16num+3)%2046; } }
  if (!all)
    set_letter_color();
}

void recheck(HWND hwnd, long refresh)
/* Refresh all menu check marks to ensure that they are correct.
 * Enter: HWND hwnd: handle of window.
 *        long refresh: 1 to redraw window afterwards.         5/26/96-DWM */
{
  char text[300], *text2;
  char *spifftbl[]={"off","neutral","dark","medium","bright","dark center","medium center","bright center"};
  HMENU sub, subp=0, hmenup;
  long i, num;

  Busy = 0;  if (Down!=5)  Down = 0;  ReleaseCapture();
  CheckMenuItem(Hmenu, MenuColor,   MF_CHECKED*Reduce);
  CheckMenuItem(Hmenu, MenuDither,  MF_CHECKED*Dither);
  CheckMenuItem(Hmenu, MenuError,   MF_CHECKED*(Msg&1));
  CheckMenuItem(Hmenu, MenuInter,   MF_CHECKED*((Interpolate&~0x1C00)==1));
  CheckMenuItem(Hmenu, MenuInter2,  MF_CHECKED*((Interpolate&~0x1C00) && (Interpolate&~0x1C00)!=1));
  CheckMenuItem(Hmenu, MenuLetBlack,MF_CHECKED*((Letter&3)==0));
  CheckMenuItem(Hmenu, MenuLetCon,  MF_CHECKED*((Letter&3)==3));
  CheckMenuItem(Hmenu, MenuLetFit,  MF_CHECKED*((Letter&8)/8));
  CheckMenuItem(Hmenu, MenuLetGrey, MF_CHECKED*((Letter&3)==1));
  CheckMenuItem(Hmenu, MenuLetWhite,MF_CHECKED*((Letter&3)==2));
  CheckMenuItem(Hmenu, MenuMulti,   MF_CHECKED*(MultiOpen&1));
  CheckMenuItem(Hmenu, MenuSaveMulti,MF_CHECKED*(MultiSave&1));
  CheckMenuItem(Hmenu, MenuSlAuto,  MF_CHECKED*((SlideOpt&8)/8));
  CheckMenuItem(Hmenu, MenuSlide,   MF_CHECKED*(slide!=0));
  sprintf(text, "&Increase Delay -- %d sec\t+", SlideDelay);
  ModifyMenu(Hmenu, MenuSlInc, 0, MenuSlInc, text);
  CheckMenuItem(Hmenu, MenuSlMan,   MF_CHECKED*(!(SlideOpt&8)));
  CheckMenuItem(Hmenu, MenuSole,    MF_CHECKED*((Msg&2)/2));
  sprintf(text, "&Spiff -- %s\tF", spifftbl[(Interpolate&0x1C00)>>10]);
  ModifyMenu(Hmenu, MenuSpiff, 0, MenuSpiff, text);
  CheckMenuItem(Hmenu, MenuSpiff,   MF_CHECKED*((Interpolate&0x1C00)!=0));
  CheckMenuItem(Hmenu, MenuTop,     MF_CHECKED*Topmost);
  CheckMenuItem(Hmenu, MenuWarn,    MF_CHECKED*((Msg&4)/4));
  CheckMenuItem(Hmenu, MenuZ78,     MF_CHECKED*(AspectX==78&&AspectY==110));
  CheckMenuItem(Hmenu, MenuZAspect, MF_CHECKED*(AspectX && AspectY &&
                                               (AspectX!=78||AspectY!=110)));
  CheckMenuItem(Hmenu, MenuZEnlarge,MF_CHECKED*(Redox&1));
  CheckMenuItem(Hmenu, MenuZLock,   MF_CHECKED*(Lock&1));
  CheckMenuItem(Hmenu, MenuZPrev,   MF_CHECKED*((Lock&2)/2));
  CheckMenuItem(Hmenu, MenuZL100,   MF_CHECKED*((Lock&4)/4));
  CheckMenuItem(Hmenu, MenuZReduce, MF_CHECKED*((Redox&2)/2));
  sub = GetSubMenu(Hmenu, MenuSlide-1);
  if (HwndP) {
    hmenup = GetMenu(HwndP);
    if (slide) {
      CheckMenuItem(hmenup, MenuSlAuto,
                    MF_CHECKED*(!(slide->option&0x2000000)));
      CheckMenuItem(hmenup, MenuSlMan,
                    MF_CHECKED*((slide->option&0x2000000)!=0)); }
    subp = GetSubMenu(hmenup, MenuSlide-1); }
  for (i=0; i<30; i++) {
    RemoveMenu(sub, MenuCat+i, MF_BYCOMMAND);
    if (subp)
      RemoveMenu(subp, MenuCat+i, MF_BYCOMMAND); }
  if (slide)
    for (i=0; i<30; i++)
      if (((short *)(slide->cat+i*8+4))[0]) {
        strcpy(text, slide->catname+((long *)(slide->cat+i*8))[0]);
        if (slide->cat[i*8+6])
          sprintf(text+strlen(text), "\t%d", slide->cat[i*8+6]-1);
        AppendMenu(sub, MF_STRING | MF_CHECKED*((((ulong *)(slide->file+FE*
                   max(LastNum,0)+0x20))[0]&(1<<i))!=0&&LastNum>=0),
                   MenuCat+i, text);
        if (subp)
          AppendMenu(subp, MF_STRING | MF_CHECKED*((((ulong *)(slide->file+
                     FE*max(slide->tsel,0)+0x20))[0]&(1<<i))!=0&&
                     slide->tsel>=0), MenuCat+i, text); }
  sub = GetSubMenu(Hmenu, MenuOpen-1);
  for (i=0; i<4; i++)
    RemoveMenu(sub, MenuLastView+i, MF_BYCOMMAND);
  for (i=0; i<4; i++)
    if (lastview[i*256]) {
      text2 = lastview+i*256;
      if (strrchr(text2, '\\'))  text2 = strrchr(text2, '\\')+1;
      sprintf(text, "&%d: %s", i+1, text2);
      AppendMenu(sub, MF_STRING, MenuLastView+i, text); }
  if (refresh) {
    InvalidateRect(hwnd, 0, 0);
    rescale |= 2; }
  num = ((uchar *)oldspec)[1]&0x7F;
  if (oldspec[0]==2 || oldspec[0]==5 || oldspec[0]==9)
    num += (((short *)(oldspec+0x12))[0]<<7);
  EnableMenuItem(Hmenu, MenuImageFirst,MF_GRAYED*!(curfile[0] && num));
  EnableMenuItem(Hmenu, MenuImagePrev, MF_GRAYED*!(curfile[0] && num));
  EnableMenuItem(Hmenu, MenuImageNext, MF_GRAYED*!(curfile[0] && (oldspec[1]&0x80)));
}

void remove_quotes(char *text)
/* If a text string is surrounded by quotes, they are removed.  The string
 *  is first limited to the text before the first space outside of quotes.
 * Enter: char *text: text string to modify.                   11/9/96-DWM */
{
  long i;

  for (i=0; i<strlen(text)-1; i++)
    if (text[i]=='"' && text[i+1]=='"') {
      memmove(text+i, text+1+i, strlen(text)-i);
      i--; }
  if (find_space(text))   find_space(text)[0] = 0;
  if (text[0]=='"' && text[strlen(text)-1]=='"' && strlen(text)>=2) {
    memmove(text, text+1, strlen(text));
    text[strlen(text)-1] = 0; }
}

void remove_quotes2(char *text)
/* If a text string is surrounded by quotes, they are removed.  Spaces do not
 *  affect the name.
 * Enter: char *text: text string to modify.                   11/9/96-DWM */
{
  long i;

  for (i=0; i<strlen(text)-1; i++)
    if (text[i]=='"' && text[i+1]=='"') {
      memmove(text+i, text+1+i, strlen(text)-i);
      i--; }
  if (text[0]=='"' && text[strlen(text)-1]=='"' && strlen(text)>=2) {
    memmove(text, text+1, strlen(text));
    text[strlen(text)-1] = 0; }
  if (strlen(text)>=4)
    if (text[2]=='\\' && text[3]=='\\')
      memmove(text+2, text+3, strlen(text+2));
}

BOOL CALLBACK rename_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the rename dialog box.  Is done is selected,
 *  rename the specified file.
 * Enter: HWND hdlg: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        6/11/96-DWM */
{
  char new[256], oname[NAMELEN], iname[NAMELEN], *s, *newf;

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd, HelpFile, HELP_CONTEXT,HelpRename); break;
      case IDOK: GetDlgItemText(hdlg, RenameNew, new, 256-1);
        s = slide->file+Rename*FE;
        sprintf(oname, "%s\\%s", slide->dirname+((long *)(slide->dir+
                ((short *)s)[0]*6))[0], slide->filename+((long *)(s+2))[0]);
        sprintf(iname, "%s\\%s", slide->dirname+((long *)(slide->dir+
                ((short *)s)[0]*6))[0], new);
        if (!MoveFile(oname, iname)) {
          if (Msg&1)
            MsgBox(hdlg, "Failed to rename file", "Error", MB_OK);
          EndDialog(hdlg, !(Msg&1)); return(Busy=0); }
        newf = realloc2(slide->filename, slide->lenfilename+strlen(new)+1);
        if (!newf) {
          EndDialog(hdlg, 1); return(Busy=0); }
        slide->filename = newf;
        ((long *)(s+2))[0] = slide->lenfilename;
        strcpy(slide->filename+slide->lenfilename, new);
        slide->lenfilename += strlen(new)+1;
        slide_delete(0, 2);
        EndDialog(hdlg, 1);
        preview_invalidate(Rename, 0, 1);
        return(Busy=0);
      case IDCANCEL: EndDialog(hdlg, 0); return(Busy=0);
      default: return(0); }
    case WM_INITDIALOG: Busy = 1;
      SetDlgItemText(hdlg, RenameOrig,
                     slide->filename+((long *)(slide->file+Rename*FE+2))[0]);
      SetDlgItemText(hdlg, RenameNew,
                     slide->filename+((long *)(slide->file+Rename*FE+2))[0]);
      SetFocus(hdlg); break;
    default: return(0); }
  return(0);
}

void rename_file(void)
/* Rename the selected files in a preview.                     3/23/97-DWM */
{
  long i, sel=slide->tsel;

  if (!preview_deselect(1)) {
    if (slide->tsel<0)  return;
    slide->file[slide->tsel*FE+0x2F] |= 6; }
  for (i=0; i<slide->numfile; i++)
    if (slide->file[i*FE+0x2F]&2) {
      Rename = i;
      if (!DialogBox(hinst, MAKEINTRESOURCE(ReDLG), HwndP, (DLGPROC)rename_dialog))
        return; }
  slide->tsel = sel;
}

void rotate(long cw)
/* Rotate the current picture 90 clockwise or counterclockwise.
 * Enter: long cw: 1 for clockwise, -1 for counterclockwise.   9/29/03-DWM */
{
  uchar *new;
  long i, j, s, d;

  if (!cw)  return;
  if (!(new=malloc2(oldw*oldh*(3-2*oldi)+768*oldi))) {
    low_memory(Hwnd, "Rotate", 0);  return; }
  if (cw>0)  cw = 1;  else  cw = -1;
  if (oldi) {
    memcpy(new, pic, 768);
    for (j=0, s=768; j<oldh; j++) {
      d = 768;
      if (cw>0)  d += oldh-1-j;
      else       d += oldh*(oldw-1)+j;
      for (i=0; i<oldw; i++, s++, d+=oldh*cw)
        new[d] = pic[s]; } }
  else {
    for (j=s=0; j<oldh; j++) {
      if (cw>0)  d = oldh-1-j;
      else       d = oldh*(oldw-1)+j;
      d *= 3;
      for (i=0; i<oldw; i++, s+=3, d+=oldh*cw*3)
        memcpy(new+d, pic+s, 3); } }
  i = oldw;  oldw = oldh;  oldh = i;
  free2(pic);
  pic = new;
  set_size();
  rescale |= 2;
  if (!IsIconic(Hwnd)) {
    reduce_res(Hwnd, 0);
    if (!(Lock&9) || Undo[2]<=0)
      set_client(Hwnd, OrigX, OrigY, OrigW, OrigH, 1);
    else if (!IsZoomed(Hwnd))
      MoveWindow(Hwnd, Undo[0], Undo[1], Undo[2], Undo[3], 1);
    InvalidateRect(Hwnd, 0, 0);
    zoom(1, 0); }
  else
    rescale = 4;
  recheck(Hwnd, 1);
}

long save_as(long saveas)
/* Display the save as dialog, then handle its result.
 * Enter: long saveas: 0 for name in opfile[], other for no name.  1 for
 *                     save as, 2 for append file, 3 for batch convert, 4 for
 *                     batch append file, 5 is reserved for the save slide
 *                     function, 6 for preview save.
 * Exit:  long okay: 0 for no file selected, 1 for selected.   6/20/96-DWM */
{
  long len, i, j, err, old, oldr;
  uchar gspec[SAVELEN+2];
  char *name, *ext;

  if (SaveAs=saveas)
    opfile[0] = 0;
  strcpy(opfilter, SaveFilter);
  if (SaveAs==2 || SaveAs==4) {
    opfilter[0] = 0;
    for (i=0; appendtbl[i]>=0; i++) {
      for (j=0, ext=SaveFilter; j<appendtbl[i]*2; j++)
        ext = strchr(ext, '|')+1;
      strcat(opfilter, ext);
      for (j=0, ext=opfilter; j<i*2+2; j++)
        ext = strchr(ext, '|')+1;
      if (ext)
        ext[0] = 0; } }
  len = strlen(opfilter);
  for (i=0; i<len; i++)  if (opfilter[i]=='|')  opfilter[i] = 0;
  openfile.lStructSize = sizeof(OPENFILENAME);
  openfile.hwndOwner = Hwnd;  openfile.hInstance = hinst;
  openfile.lpstrFilter = opfilter;
  openfile.nFilterIndex = SaveFilt;
  if (SaveAs==2 || SaveAs==4)
    for (i=0; appendtbl[i]>=0; i++)
      if (SaveFilt>=appendtbl[i]+1)
        openfile.nFilterIndex = i+1;
  openfile.lpstrFile = opfile;
  openfile.nMaxFile = 256;
  openfile.lpstrFileTitle = openfile.lpstrCustomFilter = 0;
  openfile.lpstrInitialDir = savedir+256*(SaveAs-2)*(SaveAs>=3);
  if (SaveAs==6) {
    openfile.hwndOwner = HwndP;
    openfile.lpstrInitialDir = savedir+1024; }
  openfile.Flags = OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_HIDEREADONLY |
                   OFN_NONETWORKBUTTON | (OFN_PATHMUSTEXIST*(SaveAs!=3)) |
                   (OFN_FILEMUSTEXIST*(SaveAs==2 || SaveAs==4));
  if ((Msg&4) && (SaveAs<=1 || SaveAs==6))
    openfile.Flags |= OFN_OVERWRITEPROMPT;
  openfile.lpfnHook = (DLGPROC)save_dialog;
  openfile.lpTemplateName = MAKEINTRESOURCE(SaDLG);
  Busy = 1;
  if (!GetSaveFileName(&openfile)) { Busy = 0; return(0); }
  Busy = 0;
  cursor(1);
  SaveFilt = openfile.nFilterIndex;
  if (SaveAs==2 || SaveAs==4)
    SaveFilt = appendtbl[SaveFilt-1]+1;
  if (SaveAs>=3 && SaveAs<=5)  return(1);
  name = strrchr(opfile, '\\');
  if (!name)  strrchr(opfile, ':');
  if (!name) name = opfile;  else name++;
  ext = strrchr(name, '.');
  if ((!ext || (SaveOpt&1)) && SaveAs!=2) {
    if (!ext)  ext = name+strlen(name);
    sprintf(ext, ".%s", extlist2[SaveFilt-1]); }
  if (SaveAs==6)  return(1);
  gspec[0] = SaveFilt-1;
  gspec[1] = (SaveAs==2);
  memcpy(gspec+2, SaveRec+SAVELEN*(SaveFilt-1), SAVELEN);
  if (gspec[0]==5)
    ((long *)(gspec+0xC))[0] = (long)(gquant);
  strpath(savedir, opfile);
  old=Scale;  oldr=Reduce;  CopyMode = 2;
  Scale = (!(SaveOpt&2));  Reduce = 0;  rescale |= 2;  prep_pic(Hwnd);
  if (SaveAs<=1 && MultiSave && oldspec[1] && (gspec[0]==1 || gspec[0]==2 ||
      gspec[0]==4 || gspec[0]==5 || gspec[0]==9) && !(SaveOpt&2) &&
      stricmp(opfile, curfile)) {
    multi_save(opfile, gspec);
    if (newpic!=pic)  free2(newpic); }
  else
    err = save_graphic(opfile, newpic, gspec, newpic!=pic);
  CopyMode = 0;  Scale = old;  Reduce = oldr;  rescale |= 2;  prep_pic(Hwnd);
  cursor(0);
  if (err) {
    if (Msg&1)
      MsgBox(Hwnd, ErrorMsg2[err-1], "Error", MB_OK);
    return(0); }
  memcpy(oldspec, gspec, LENPSPEC);
  if (oldspec[0]==5)  memcpy(oldquant, gquant, 128);
  strcpy(curfile, opfile);
  set_name(Hwnd);
  preview_invalidate(LastNum, 0, 0);
  LastNum = preview_invalidate(1, curfile, 0);
  return(1);
}

BOOL CALLBACK save_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the main save dialog box.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        6/11/96-DWM */
{
  HWND ctrl;

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd, HelpFile, HELP_CONTEXT, HelpSave+SaveAs);
        break;
      case IDOK: if (SaveAs<=2)
          SaveOpt &= 1;
        if (SaveAs<=1 || SaveAs==6)
          SaveOpt = SendDlgItemMessage(hdlg,SaveExt,BM_GETCHECK,0,0);
        if (SaveAs<=2)
          SaveOpt |= SendDlgItemMessage(hdlg,SaveView2,BM_GETCHECK,0,0)*2;
        return(0);
      case SaveOptions:
        PreviewOpt = (PreviewOpt&0xFFFE) | SendDlgItemMessage(hdlg,SaveView2,
                      BM_GETCHECK,0,0);
        SaveDialog = SendDlgItemMessage(hdlg,cmb1,CB_GETCURSEL,0,0);
        if (SaveAs==2 || SaveAs==4)  SaveDialog = appendtbl[SaveDialog];
        switch (SaveDialog) {
          case 2: PreviewOpt &= 0xFFF3;  break;
          case 4: PreviewOpt &= 0xFFF7; }
        DialogBox(hinst, MAKEINTRESOURCE(SaDLG0+SaveDialog), hdlg, (DLGPROC)saveopt_dialog);
        if (SaveDialog==2)
          SaveRec[SAVELEN*SaveDialog+1] = 0;
        return(0);
      default: return(0); } break;
    case WM_INITDIALOG:
      if ((SaveAs>=3 && SaveAs<=4) || SaveAs==6) {
        SendDlgItemMessage(hdlg, SaveView1, BM_SETCHECK, 1, 0);
        ctrl = GetDlgItem(hdlg, SaveView1);  EnableWindow(ctrl, 0);
        if (SaveAs==6)  DestroyWindow(ctrl);
        ctrl = GetDlgItem(hdlg, SaveView2);  EnableWindow(ctrl, 0);
        if (SaveAs==6)  DestroyWindow(ctrl); }
      else {
        SendDlgItemMessage(hdlg, SaveView1, BM_SETCHECK, !(SaveOpt&2), 0);
        SendDlgItemMessage(hdlg, SaveView2, BM_SETCHECK, (SaveOpt&2)/2, 0); }
      if (SaveAs==3) {
        ctrl = GetDlgItem(hdlg, edt1);  EnableWindow(ctrl, 0);
        ctrl = GetDlgItem(hdlg, stc3);  EnableWindow(ctrl, 0);
        ctrl = GetDlgItem(hdlg, lst1);  EnableWindow(ctrl, 0);
        SetDlgItemText(hdlg, edt1, "---");
        SendDlgItemMessage(hdlg, SaveExt, BM_SETCHECK, 1, 0); }
      if (SaveAs>=2 && SaveAs<=5) {
        ctrl = GetDlgItem(hdlg, SaveExt);   EnableWindow(ctrl, 0);
        ctrl = GetDlgItem(hdlg, SaveExt2);  EnableWindow(ctrl, 0); }
      else
        SendDlgItemMessage(hdlg, SaveExt, BM_SETCHECK, SaveOpt&1, 0);
      SetWindowText(hdlg, SaveTitle[SaveAs]);
      SetFocus(hdlg);
    default: return(0); }
  return(0);
}

BOOL CALLBACK saveopt_custom(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the custom quantization table dialog box.
 * Enter: HWND hdlg: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        6/11/96-DWM */
{
  HWND ctrl;
  char text[80];
  long i, o, temp;
  static uchar qtbl[128];

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd, HelpFile, HELP_CONTEXT, HelpSaveCustom);
                     break;
      case IDOK: o = 64*SendDlgItemMessage(hdlg, SaveRad2, BM_GETCHECK,0,0);
        for (i=0; i<64; i++) {
          GetDlgItemText(hdlg, SaveQ+i, text, 79);
          sscanf(text, "%d", &temp);
          if (temp>0 && temp<=255)  qtbl[i+o] = temp; }
        memcpy(gquant, qtbl, 128);
        EndDialog(hdlg, 1); break;
      case IDCANCEL: EndDialog(hdlg, 0); break;
      case SaveBest: for (i=0; i<64; i++)
        SetDlgItemText(hdlg, SaveQ+i, "1"); return(0);
      case SaveDef:
        o = 0x45*SendDlgItemMessage(hdlg, SaveRad2, BM_GETCHECK,0,0)+0x19;
        for (i=0; i<64; i++) {
          sprintf(text, "%d", jpghead[i+o]);
          SetDlgItemText(hdlg, SaveQ+i, text); }
        return(0);
      case SaveRad1: case SaveRad2:
        o = (64*SendDlgItemMessage(hdlg, SaveRad2, BM_GETCHECK,0,0))^64;
        if ((!o && (wp&0xFFFF)==SaveRad1) || (o && (wp&0xFFFF)==SaveRad2))
          return(0);
        for (i=0; i<64; i++) {
          GetDlgItemText(hdlg, SaveQ+i, text, 79);
          sscanf(text, "%d", &temp);
          if (temp>0 && temp<=255)  qtbl[i+o] = temp; }
        o ^= 64;
        for (i=0; i<64; i++) {
          sprintf(text, "%d", qtbl[i+o]);
          SetDlgItemText(hdlg, SaveQ+i, text); }
        return(0);
      case SaveWorst: for (i=0; i<64; i++)
        SetDlgItemText(hdlg, SaveQ+i, "255"); return(0);
      default: return(0); } break;

    case WM_INITDIALOG: SetFocus(hdlg);
      SendDlgItemMessage(hdlg, SaveRad1, BM_SETCHECK, 1, 0);
      memcpy(qtbl, gquant, 128);
      for (i=0; i<64; i++) {
        sprintf(text, "%d", qtbl[i]);
        SetDlgItemText(hdlg, SaveQ+i, text); }
      break;
    default: return(0); }
  return(0);
}

BOOL CALLBACK saveopt_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in any of the save options dialog boxes.  The various
 *  save formats and options are listed below.
 *   Format   Colors   Compress Size  DPI Other
 *           b14g8rcy -rljRL ab
 *  0 BMPx2  xxx xx   xx         --    x  Type 40/12
 *  1 CUR    xxx      x         16,32  x  Trans/inv colors, hot spots
 *  2 GIF        x      x        --       Interlacing, 87/89, trans color
 *  3 GRB     x              xx 131       Gamma, shades
 *  4 ICO    xxx      x         16,32  x  Trans/inv colors
 *  5 JPGx2  x  x   x    x       --    x  Quality, decimation
 *  6 PBMx4  xx x x          xx  --
 *  7 PCXx2  xx  x     x         --
 *  8 TGA         x    x         --
 *  9 TIF    xx xxxxx xxx xx     --    x  Strip size, endian
 * Enter: HWND hdlg: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        6/11/96-DWM */
{
  HDC hdc;
  HWND ctrl;
  PAINTSTRUCT paint;
  RECT rect;
  static uchar clr[]={0x37,0x37,0x10,0x02,0x37,0x89,0x2B,0x13,0x20,0x7B};
  static char *clrt[]={"Best possible","1-bit black and white","4-bit Windows colors","8-bit greyscale","8-bit color palette","24-bit RGB","32-bit CMYK","24-bit YCbCr"};
  static uchar com[]={0x03,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x0F};
  char text[80];
  long i, j, temp, init=0, x, y;
  float temp2;

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd, HelpFile, HELP_CONTEXT,
                             HelpSaveBMP+SaveDialog);  break;
      case IDOK: SaveRec[SAVELEN*SaveDialog] =
                        SendDlgItemMessage(hdlg,SaveColors,CB_GETCURSEL,0,0);
        SaveRec[SAVELEN*SaveDialog+1] =
                      SendDlgItemMessage(hdlg,SaveCompress,CB_GETCURSEL,0,0);
        text[0] = 0;
        GetDlgItemText(hdlg, SaveDPI, text, 79);
        sscanf(text, "%d", &temp);  if (temp<=0 || temp>100000)  temp = 160;
        ((short *)(SaveRec+SAVELEN*SaveDialog+2))[0] = temp;
        for (i=0; i<4; i++)
          if (SendDlgItemMessage(hdlg, SaveRad1+i, BM_GETCHECK,0,0))
            SaveRec[SAVELEN*SaveDialog+4] = i;
        SaveRec[SAVELEN*SaveDialog+5] = SendDlgItemMessage(hdlg, SaveRad2b,
                                                            BM_GETCHECK,0,0);
        for (i=0; i<4; i++) {
          GetDlgItemText(hdlg, SaveEdit1+i, text, 79);
          if (SaveDialog==3 && !i) {
            sscanf(text, "%f", &temp2);
            if (temp2<=0 || temp2>1000)  temp2 = 1;
            ((float *)(SaveRec+SAVELEN*SaveDialog+6))[i] = temp2; }
          else {
            sscanf(text, "%d", &temp);
            switch (SaveDialog*4+i) {
              case 13: if (temp<2 || temp>1000)  temp = 2;  break;
              case 20: if (temp<0 || temp>2600)  temp = 50; break;
              case 22: if (temp<2 || temp>64)    temp = 3;  break;
              case 36: if (temp<0 || temp>1e7)   temp = 0;  break; }
            if (i<2)
              ((long *)(SaveRec+SAVELEN*SaveDialog+6))[i] = temp;
            else
              ((short *)(SaveRec+SAVELEN*SaveDialog+6))[i+2] = temp; } }
        if (SaveDialog==2) {
          SaveRec[SAVELEN*SaveDialog] = SendDlgItemMessage(hdlg, SaveCheck,
                                                            BM_GETCHECK,0,0);
          SaveRec[SAVELEN*SaveDialog+12] |= SendDlgItemMessage(hdlg,
                                                SaveKeep, BM_GETCHECK,0,0); }
        if (SaveDialog==5)
          SaveRec[SAVELEN*SaveDialog+5] = SendDlgItemMessage(hdlg, SaveCus,
                                                            BM_GETCHECK,0,0);
      case IDCANCEL: EndDialog(hdlg, 1);
        if (PreviewPic) { free2(PreviewPic);  PreviewPic = 0; }
        if (!wpal)  return(1);
        if (!PalChange)  return(1);
        InvalidateRect(Hwnd, 0, 0);  return(1);
      case SaveCustom:
        if (DialogBox(hinst, MAKEINTRESOURCE(SaDLG5a), hdlg, (DLGPROC)saveopt_custom)==1)
          SendDlgItemMessage(hdlg, SaveCus, BM_SETCHECK, 1, 0);
        SetDlgItemText(hdlg, SaveEdit1, "100");  return(0);
      case SaveDef: memcpy(SaveRec+SAVELEN*SaveDialog,
                           SaveDefault+SAVELEN*SaveDialog, SAVELEN);
        init = 1; break;
      case SaveFit: PreviewOpt ^= 2;
        create_savepic_rect(3, 0, hdlg);  return(0);
      case SaveEdit1: if (SaveDialog==3) {
          GetDlgItemText(hdlg, SaveEdit1, text, 79);
          temp2 = GetScrollPos(GetDlgItem(hdlg, SaveScroll), SB_CTL)*0.01;
          sscanf(text, "%f", &temp2);
          PreviewGamma = temp2;
          SetScrollPos(GetDlgItem(hdlg, SaveScroll), SB_CTL,
                       (long)(temp2*100+0.5), 1);
          create_savepic_rect(2, 0, hdlg);  return(0); }
        if (SaveDialog==5) {
          GetDlgItemText(hdlg, SaveEdit1, text, 79);
          i = GetScrollPos(GetDlgItem(hdlg, SaveScroll), SB_CTL);
          sscanf(text, "%d", &i);
          SetScrollPos(GetDlgItem(hdlg, SaveScroll), SB_CTL, i, 1); }
        if (SaveDialog<1 || SaveDialog>4)  return(0);
        GetDlgItemText(hdlg, SaveEdit1, text, 79);
        temp = PreviewTrans;
        sscanf(text, "%d", &temp);
        if (temp==PreviewTrans)  return(0);
        PreviewTrans = temp;
        if (PreviewOpt&0xC)  return(0);
        create_savepic_rect(3, 0, hdlg);  return(0);
      case SaveEdit2:
        if (SaveDialog!=1 && SaveDialog!=4 && SaveDialog!=3)  return(0);
        GetDlgItemText(hdlg, SaveEdit2, text, 79);
        temp = PreviewInv;
        sscanf(text, "%d", &temp);
        if (temp==PreviewInv)  return(0);
        PreviewInv = temp;
        if (SaveDialog==3) {
          create_savepic_rect(2, 0, hdlg);  return(0); }
        if ((PreviewOpt&0xC)!=4)  return(0);
        create_savepic_rect(3, 0, hdlg);  return(0);
      case SaveEdit3: case SaveEdit4: if (SaveDialog!=1)  return(0);
        GetDlgItemText(hdlg, wp&0xFFFF, text, 79);
        temp = PreviewXY[(wp&0xFFFF)-SaveEdit3];
        sscanf(text, "%d", &temp);
        if (temp==PreviewXY[(wp&0xFFFF)-SaveEdit3])  return(0);
        PreviewXY[(wp&0xFFFF)-SaveEdit3] = temp;
        if ((PreviewOpt&0xC)!=8)  return(0);
        create_savepic_rect(1, 0, hdlg);  return(0);
      case SaveFit1: PreviewOpt = (PreviewOpt&0xFFF3);
        create_savepic_rect(3, 0, hdlg);  return(0);
      case SaveFit2: PreviewOpt = (PreviewOpt&0xFFF3)|4;
        create_savepic_rect(3, 0, hdlg);  return(0);
      case SaveFit3: PreviewOpt = (PreviewOpt&0xFFF3)|8;
        create_savepic_rect(0, 0, hdlg);
        create_savepic_rect(1, &rect, hdlg);
        InvalidateRect(hdlg, &rect, 1); return(0);
      default: return(0); }
    case WM_HSCROLL: GetScrollRange((HWND)lp, SB_CTL, &x, &y);
      i = GetScrollPos((HWND)lp, SB_CTL);
      switch (wp&0xFFFF) {
        case SB_BOTTOM: i = x; break;
        case SB_LINELEFT: if (i>x) i--; break;
        case SB_LINERIGHT: if (i<y) i++; break;
        case SB_PAGELEFT: i -= 10;  if (i<x) i = x; break;
        case SB_PAGERIGHT: i += 10; if (i>y) i = y; break;
        case SB_THUMBPOSITION: case SB_THUMBTRACK: i = (wp>>16); break;
        case SB_TOP: i = y; }
      SetScrollPos((HWND)lp, SB_CTL, i, 1);
      if (SaveDialog==3) {
        sprintf(text, "%4.2f", i*0.01);
        PreviewGamma = i*0.01; }
      else
        sprintf(text, "%d", i);
      SetDlgItemText(hdlg, SaveEdit1, text); break;
    case WM_INITDIALOG: init = 1; SetFocus(hdlg); break;
    case WM_LBUTTONDOWN: if (Down)  return(0);
      if (SaveDialog!=1 && SaveDialog!=2 && SaveDialog!=4)  return(0);
      Down = saveopt_left(hdlg, lp&0xFFFF, (lp>>16));
      if (Down)  SetCapture(hdlg);
      return(0);
    case WM_LBUTTONUP: if (Down==1) { Down = 0;  ReleaseCapture(); } break;
    case WM_MOUSEMOVE:
      if (Down==1) saveopt_left(hdlg, lp&0xFFFF, lp>>16);
      if (Down==2) saveopt_right(hdlg, lp&0xFFFF, (lp>>16), 1); break;
    case WM_PAINT: if (SaveDialog<1 || SaveDialog>4)  break;
      hdc = BeginPaint(hdlg, &paint);
      cursor(1);
      if (SaveDialog!=3)    create_savepic(hdlg, hdc);
      else if (PreviewPic)  create_savepic_grob(hdlg, hdc);
      cursor(0);
      EndPaint(hdlg, &paint); break;
    case WM_RBUTTONDOWN: if (Down || (PreviewOpt&2))  return(0);
      if (SaveDialog!=1 && SaveDialog!=2 && SaveDialog!=4)  return(0);
      Down = saveopt_right(hdlg, lp&0xFFFF, (lp>>16), 0)*2;
      cursor(2);
      if (Down)  SetCapture(hdlg);
      return(0);
    case WM_RBUTTONUP: if (Down==2) {
        Down = 0;  ReleaseCapture();  cursor(0); } break;
    default: return(0); }
  if (init) {
    Down = 0;
    for (i=0; i<8; i++) {
      if (clr[SaveDialog]&(1<<i))
        SendDlgItemMessage(hdlg, SaveColors, CB_ADDSTRING, 0, (long)clrt[i]);
      if (com[SaveDialog]&(1<<i))
        SendDlgItemMessage(hdlg, SaveCompress, CB_ADDSTRING, 0,
                           (long)SaveComp[i]); }
    SendDlgItemMessage(hdlg, SaveColors, CB_SETCURSEL,
                       SaveRec[SAVELEN*SaveDialog], 0);
    SendDlgItemMessage(hdlg, SaveCompress, CB_SETCURSEL,
                       SaveRec[SAVELEN*SaveDialog+1], 0);
    sprintf(text, "%d", ((short *)(SaveRec+SAVELEN*SaveDialog+2))[0]);
    SetDlgItemText(hdlg, SaveDPI, text);
    for (i=0; i<4; i++)
      SendDlgItemMessage(hdlg, SaveRad1+i, BM_SETCHECK,
                         SaveRec[SAVELEN*SaveDialog+4]==i, 0);
    for (i=0; i<2; i++)
      SendDlgItemMessage(hdlg, SaveRad1b+i, BM_SETCHECK,
                         SaveRec[SAVELEN*SaveDialog+5]==i, 0);
    for (i=0; i<4; i++) {
      if (SaveDialog==3 && !i) {
        temp2 = ((float *)(SaveRec+SAVELEN*SaveDialog+6))[i];
        sprintf(text, "%4.2f", temp2); }
      else {
        if (i<2)  temp = ((long *)(SaveRec+SAVELEN*SaveDialog+6))[i];
        else      temp = ((short *)(SaveRec+SAVELEN*SaveDialog+6))[i+2];
        sprintf(text, "%d", temp); }
      SetDlgItemText(hdlg, SaveEdit1+i, text); }
    SendDlgItemMessage(hdlg, SaveFit, BM_SETCHECK, (PreviewOpt&2), 0);
    SendDlgItemMessage(hdlg, SaveFit1, BM_SETCHECK, (PreviewOpt&0xC)==0, 0);
    SendDlgItemMessage(hdlg, SaveFit2, BM_SETCHECK, (PreviewOpt&0xC)==4, 0);
    SendDlgItemMessage(hdlg, SaveFit3, BM_SETCHECK, (PreviewOpt&0xC)==8, 0);
    PreviewTrans = ((long *)(SaveRec+SAVELEN*SaveDialog+6))[0];
    PreviewInv   = ((long *)(SaveRec+SAVELEN*SaveDialog+6))[1];
    PreviewXY[0] = ((short *)(SaveRec+SAVELEN*SaveDialog+6))[4];
    PreviewXY[1] = ((short *)(SaveRec+SAVELEN*SaveDialog+6))[5];
    switch (SaveDialog) {
      case 2: SendDlgItemMessage(hdlg, SaveCheck, BM_SETCHECK,
                                 SaveRec[SAVELEN*SaveDialog], 0);
        SendDlgItemMessage(hdlg, SaveKeep, BM_SETCHECK,
                           SaveRec[SAVELEN*SaveDialog+12]&1, 0);
        sprintf(text, "%d", ((short *)(SaveRec+SAVELEN*SaveDialog+10))[0]);
        SetDlgItemText(hdlg, SaveEdit2, text);
        if (SaveAs==2 || SaveAs==4) {
          ctrl = GetDlgItem(hdlg, SaveEdit3);  EnableWindow(ctrl, 0);
          ctrl = GetDlgItem(hdlg, SaveText1);  EnableWindow(ctrl, 0); }
      case 1: case 4: create_savepic_rect(3, 0, hdlg); break;
      case 3: SetScrollRange(GetDlgItem(hdlg, SaveScroll), SB_CTL, 1, 300,0);
        PreviewGamma = ((float *)(SaveRec+SAVELEN*SaveDialog+6))[0];
        SetScrollPos(GetDlgItem(hdlg, SaveScroll), SB_CTL,
                     (long)(PreviewGamma*100), 1);
        if (!PreviewPic) {
          ppic = pic;  pinter = oldi;  pwidth = oldw;  pheight = oldh;
          pfree = 0;
          if (best_color(0x10, -1, -1)!=4) break;
          PreviewPic = realloc2(ppic, 768+oldw*oldh);
          if (!PreviewPic) { free2(ppic);  break; }
          memmove(PreviewPic+768, PreviewPic, oldw*oldh); }
        create_savepic_rect(2, 0, hdlg); break;
      case 5: SetScrollRange(GetDlgItem(hdlg, SaveScroll), SB_CTL, 0, 200,0);
        SetScrollPos(GetDlgItem(hdlg, SaveScroll), SB_CTL,
                     ((long *)(SaveRec+SAVELEN*SaveDialog+6))[0], 1);
        SendDlgItemMessage(hdlg, SaveCus, BM_SETCHECK,
                           SaveRec[SAVELEN*SaveDialog+5], 0); } }
  return(0);
}

long saveopt_left(HWND hdlg, long x, long y)
/* Determine if the cursor is in the preview window, and, if so, adjust the
 *  appropriate item to the selected value.
 * Enter: HWND hdlg: dialog handle.
 *        long x, y: position in client area.
 * Exit:  long inzone: 0 if not in preview zone, 1 if it is.   10/4/96-DWM */
{
  RECT rect;
  long w, h, iw, ih, ix, iy, fit=(PreviewOpt&2)>>1, clr;
  float scale;
  char text[40];

  create_savepic_rect(0, &rect, hdlg);
  if (x<rect.left || y<rect.top || x>=rect.right || y>=rect.bottom)
    return(0);
  x -= rect.left;  y -= rect.top;
  w = rect.right-rect.left;  h = rect.bottom-rect.top;
  if (!w) w = 1;  if (!h) h = 1;
  iw = PreviewArea[2+fit*4]-PreviewArea[0+fit*4];
  ih = PreviewArea[3+fit*4]-PreviewArea[1+fit*4];
  if (iw<=w && ih<=h) {
    ix = x+PreviewArea[0+fit*4]-(w-iw)/2;
    iy = y+PreviewArea[1+fit*4]-(h-ih)/2; }
  else {
    scale = (float)iw/w;  if (ih/h>scale)  scale = (float)ih/h;
    if (!scale) scale = 1;
    ix = (x-(w-iw/scale)/2)*scale+PreviewArea[0+fit*4];
    iy = (y-(h-ih/scale)/2)*scale+PreviewArea[1+fit*4]; }
  if (ix<0 || ix>=oldw || iy<0 || iy>=oldh)  return(1);
  if (oldi)
    clr = pic[768+iy*oldw+ix];
  else
    clr = pic[(iy*oldw+ix)*3+2]+(pic[(iy*oldw+ix)*3+1]<<8)+
          (pic[(iy*oldw+ix)*3]<<16);
  switch (PreviewOpt&0xC) {
    case 0: if (PreviewTrans==clr)  return(1);
      PreviewTrans = clr;
      sprintf(text, "%d", clr);
      SetDlgItemText(hdlg, SaveEdit1, text); break;
    case 4: if (PreviewInv==clr)  return(1);
      PreviewInv = clr;
      sprintf(text, "%d", clr);
      SetDlgItemText(hdlg, SaveEdit2, text); break;
    case 8: sprintf(text, "%d", ix);
      SetDlgItemText(hdlg, SaveEdit3, text);
      sprintf(text, "%d", iy);
      SetDlgItemText(hdlg, SaveEdit4, text); }
  if (SaveDialog==2)
    SendDlgItemMessage(hdlg, SaveCheck, BM_SETCHECK, 1, 0);
  create_savepic_rect(((PreviewOpt&0xC)<=4), 0, hdlg);
  return(1);
}

long saveopt_right(HWND hdlg, long x, long y, long move)
/* Determine if the cursor is in the preview window, and, if so, scroll the
 *  window.
 * Enter: HWND hdlg: dialog handle.
 *        long x, y: position in client area.
 *        long move: 0 for start, 1 for move.
 * Exit:  long inzone: 0 if not in preview zone, 1 if it is.   10/4/96-DWM */
{
  RECT rect;
  long dx, dy, oldarea[4], w, h;

  create_savepic_rect(0, &rect, hdlg);
  if (x<rect.left || y<rect.top || x>=rect.right || y>=rect.bottom)
    if (!move)
      return(0);
  if (!move) {
    DownX = x;  DownY = y;  return(1); }
  dx = DownX-x;  dy = DownY-y;  DownX = x;  DownY = y;
  if (!dx && !dy)  return(1);
  memcpy(oldarea, PreviewArea, 4*sizeof(long));
  PreviewArea[0] += dx;  PreviewArea[1] += dy;
  PreviewArea[2] += dx;  PreviewArea[3] += dy;
  w = rect.right-rect.left;  h = rect.bottom-rect.top;
  create_savepic_area(w, h);
  if (!(memcmp(oldarea, PreviewArea, 4*sizeof(long))))
    return(1);
  create_savepic_rect(3, 0, hdlg);
  return(1);
}

VOID CALLBACK scroll_wait(HWND hwnd, ulong msg, ulong id, long time)
/* Prevent scrolling too fast.
 * Enter: HWND hwnd: handle of calling window.
 *        ulong msg: ignored.
 *        ulong id: id of calling timer.
 *        long time: time in some windows format.             11/10/96-DWM */
{
  ScrollWait = 0;
  KillTimer(HwndP, 5);
  SendMessage(HwndP, WM_MOUSEMOVE, 0, ODown[5]+(ODown[6]<<16));
}

void set_client(HWND hwnd, long x, long y, long w, long h, long paint)
/* Set the location and size of client area of the window.  The location is
 *  the upper left of the window (including frame).
 * Enter: HWND hwnd: handle of window.
 *        long x, y: location for upper-left corner.
 *        long w, h: size of client area.
 *        long paint: 0-don't redraw, 1-redraw.                10/7/96-DWM */
{
  RECT rect;
  long d=1, inc=0, count=0;

  if (IsZoomed(hwnd))  return;
  MoveWindow(hwnd, x, y, w+extrax, h+extray, paint);
  while (d>0 && count<10) {
    GetClientRect(hwnd, &rect);
    if ((d=(h-(rect.bottom-rect.top)))>0) {
      if (rect.bottom==rect.top)  inc += GetSystemMetrics(SM_CYMENU);
      else                        inc += d;
      count++;
      MoveWindow(hwnd, x, y, w+extrax, h+inc+extray, paint); } }
}

void set_font(HDC dc, long type, long size, long fore, long back)
/* Set a device context to a specific font, foreground, and background color.
 * Enter: HDC dc: dc to set.
 *        long type: font typeface.  Valid values are: FF_SWISS, FF_ROMAN,
 *                   FF_MODERN, and FF_SCRIPT.
 *        long size: point size.  Zero to clear last font.  Negative for
 *                   bold.
 *        long fore, back: colors for the font in BBGGRR format.
 *                                                             1/23/97-DWM */
{
  static HFONT font=(HFONT)-1;
  long bold=0;
  char *name[]={" ","Arial"};

  if (((long)font)!=-1)
    DeleteObject(font);
  font = (HFONT)-1;
  if (!size)  return;
  if (size<0) {
    bold = 600;  size *= -1; }
  font = CreateFont(size, 0, 0, 0, bold, 0, 0, 0, ANSI_CHARSET,
                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                    DEFAULT_PITCH|0x0004|type, name[type==FF_SWISS]);
  SelectObject(dc, font);
  SetTextColor(dc, fore);
  SetBkColor(dc, back);
}

void set_letter_color(void)
/* Compute the contrasting and border colors of the graphic.   5/26/96-DWM */
{
  long i, c, best, r=0, g=0, b=0, t=0;

  LetterColor[5] = oldi;
  if (oldi) {
    for (i=0,best=768; i<256; i++) {
      c = pic[i*3]+pic[i*3+1]+pic[i*3+2];
      if (c<best) { best = c;  LetterColor[0] = i; } }
    for (i=0,best=768; i<256; i++) {
      c = abs(pic[i*3]-128)+abs(pic[i*3+1]-128)+abs(pic[i*3+2]-128);
      if (c<best) { best = c;  LetterColor[1] = i; } }
    for (i=0,best=768; i<256; i++) {
      c = abs(pic[i*3]-255)+abs(pic[i*3+1]-255)+abs(pic[i*3+2]-255);
      if (c<best) { best = c;  LetterColor[2] = i; } }
    for (i=0; i<oldw*oldh; i+=100, t++) {
      r += pic[pic[i+768]*3];
      g += pic[pic[i+768]*3+1];
      b += pic[pic[i+768]*3+2]; }
    if (!t) t = 1;
    r = 255-r/t;  g = 255-g/t;  b = 255-b/t;
    for (i=0,best=768; i<256; i++) {
      c = abs(pic[i*3]-r)+abs(pic[i*3+1]-g)+abs(pic[i*3+2]-b);
      if (c<best) { best = c;  LetterColor[3] = i; } }
    LetterColor[4] = c = pic[768];
    for (i=1; i<oldh-1; i++)
      if (pic[i*oldw+768]!=c || pic[i*oldw+768+oldw-1]!=c)
        LetterColor[4] = -1;
    for (i=0; i<oldw; i++)
      if (pic[i+768]!=c || pic[i+768+oldh*oldw-oldw]!=c)
        LetterColor[4] = -1; }
  else {
    LetterColor[0] = 0;
    LetterColor[1] = 0x7F7F7F;
    LetterColor[2] = 0xFFFFFF;
    for (i=0; i<oldw*oldh; i+=100, t++) {
      r += pic[i*3];
      g += pic[i*3+1];
      b += pic[i*3+2]; }
    if (!t) t = 1;
    r = 255-r/t;  g = 255-g/t;  b = 255-b/t;
    LetterColor[3] = r*0x10000+g*0x100+b;
    if (oldw*oldh==1) {
      LetterColor[4] = -1;  return; }
    LetterColor[4] = c = ((ulong *)pic)[0]&0xFFFFFF;
    for (i=1; i<oldh-1; i++)
      if ((((ulong *)(pic+i*oldw*3))[0]&0xFFFFFF)!=c ||
          (((long *)(pic+(i*oldw+oldw-1)*3))[0]&0xFFFFFF)!=c)
        LetterColor[4] = -1;
    for (i=1; i<oldw-1; i++)
      if ((((ulong *)(pic+i*3))[0]&0xFFFFFF)!=c ||
          (((long *)(pic+(i+oldh*oldw-oldw)*3))[0]&0xFFFFFF)!=c)
        LetterColor[4] = -1; }
}

void set_name(HWND hwnd)
/* Add the name of the graphic file to the window title.
 * Enter: HWND hwnd: handle of window.                         5/26/96-DWM */
{
  static char text[512], *name;
  long num;

  strcpy(text, inihead);
  if (curfile[0]) {
    name = curfile;
    if (strchr(curfile, '\\'))  name = strrchr(curfile, '\\')+1;
    sprintf(text+strlen(text), " - %s", name);
    num = ((uchar *)oldspec)[1]&0x7F;
    if (oldspec[0]==2 || oldspec[0]==5 || oldspec[0]==9)
      num += (((short *)(oldspec+0x12))[0]<<7);
    if (num || oldspec[1])
      sprintf(text+strlen(text), " - %d", num+1); }
  SetWindowText(hwnd, text);
}

void set_size(void)
/* Sets the size of the current window.                        5/26/96-DWM */
{
  long x, y;

  if (!oldw) oldw = 1;  if (!oldh) oldh = 1;
  x = oldw+extrax;  y = oldh+extray;
  if (x>GetSystemMetrics(SM_CXSCREEN)) {
    x = GetSystemMetrics(SM_CXSCREEN);
    y = ((float)oldh*(x-extrax))/oldw+extray; }
  if (y>GetSystemMetrics(SM_CYSCREEN)) {
    y = GetSystemMetrics(SM_CYSCREEN);
    x = ((float)oldw*(y-extray))/oldh+extrax; }
  if (x<MINWIDTH) x = MINWIDTH;
  if (x+OrigX>GetSystemMetrics(SM_CXSCREEN))
    OrigX = GetSystemMetrics(SM_CXSCREEN) - x;
  if (y+OrigY>GetSystemMetrics(SM_CYSCREEN))
    OrigY = GetSystemMetrics(SM_CYSCREEN) - y;
  OrigW = x-extrax;  OrigH = y-extray;
  if (slide && NextPic)  if (LastNum==NextNum)  return;
  View[0] = View[1] = 0;  View[2] = oldw;  View[3] = oldh;
  View[4] = View[5] = 0;  View[6] = oldw;  View[7] = oldh;
}

BOOL CALLBACK slide_cat_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the Slide Category dialog box.  This allows a
 *  category to either be edited or created.  The specific category is given
 *  by the global variable EditCat.
 * Enter: HWND hdlg: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.         3/1/97-DWM */
{
  char *Title[]={"Edit Category","New Category"};
  char *quick[]={"None","0","1","2","3","4","5","6","7","8","9"};
  char text[256], *new;
  long i, pos, len;

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd,HelpFile,HELP_CONTEXT,HelpSlideCat); break;
      case IDOK: slide->cat[EditCat*8+7] = 0;
        if (SendDlgItemMessage(hdlg, SlideShowY, BM_GETCHECK, 0, 0))
          slide->cat[EditCat*8+7] |= 1;
        if (SendDlgItemMessage(hdlg, SlideShowN, BM_GETCHECK, 0, 0))
          slide->cat[EditCat*8+7] |= 2;
        slide->cat[EditCat*8+6] = SendDlgItemMessage(hdlg, SlideQuick,
                                                     CB_GETCURSEL, 0, 0);
        if (slide->cat[EditCat*8+6]!=0)
          for (i=0; i<30; i++)  if (i!=EditCat)
            if (slide->cat[EditCat*8+6]==slide->cat[i*8+6])
              slide->cat[i*8+6] = 0;
        GetDlgItemText(hdlg, SlideName, text, 255);
        if (!strlen(text))
          sprintf(text, "Category %d", EditCat+1);
        if (slide->catname) {
          pos = ((long *)(slide->cat+EditCat*8))[0];
          len = ((short *)(slide->cat+EditCat*8+4))[0];
          memmove(slide->catname+pos, slide->catname+pos+len,
                  slide->lencatname-pos-len);
          for (i=0; i<30; i++)
            if (((long *)(slide->cat+i*8))[0]>pos)
              ((long *)(slide->cat+i*8))[0] -= len;
          slide->lencatname -= len;
          if (!slide->lencatname) {
            free2(slide->catname);  slide->catname = 0; } }
        len = strlen(text)+1;  pos = slide->lencatname;
        if (!slide->catname)
          slide->catname = malloc2(len);
        else {
          new = realloc2(slide->catname, pos+len);
          if (new)
            slide->catname = new;
          else
            len = 0; }
        if (slide->catname)
          memcpy(slide->catname+pos, text, len);
        else
          len = 0;
        ((long *)(slide->cat+EditCat*8))[0] = pos;
        ((short *)(slide->cat+EditCat*8+4))[0] = len;
        slide->lencatname += len;
      case IDCANCEL: EndDialog(hdlg, 1);  return(1);
      case SlideShowN:
        if (SendDlgItemMessage(hdlg, SlideShowN, BM_GETCHECK, 0, 0))
          SendDlgItemMessage(hdlg, SlideShowY, BM_SETCHECK, 0, 0); break;
      case SlideShowY:
        if (SendDlgItemMessage(hdlg, SlideShowY, BM_GETCHECK, 0, 0))
          SendDlgItemMessage(hdlg, SlideShowN, BM_SETCHECK, 0, 0); break;
      default: return(0); } break;
    case WM_INITDIALOG:
      SetWindowText(hdlg, Title[!((short *)(slide->cat+EditCat*8+4))[0]]);
      for (i=0; i<11; i++)
        SendDlgItemMessage(hdlg,SlideQuick,CB_ADDSTRING,0,(long)quick[i]);
      if (((short *)(slide->cat+EditCat*8+4))[0]) {
        SetDlgItemText(hdlg, SlideName, slide->catname+
                       ((long *)(slide->cat+EditCat*8))[0]);
        SendDlgItemMessage(hdlg, SlideQuick, CB_SETCURSEL,
                           slide->cat[EditCat*8+6], 0);
        SendDlgItemMessage(hdlg, SlideShowY, BM_SETCHECK,
                           (slide->cat[EditCat*8+7]&1)!=0, 0);
        SendDlgItemMessage(hdlg, SlideShowN, BM_SETCHECK,
                           (slide->cat[EditCat*8+7]&2)!=0, 0); }
      else
        SendDlgItemMessage(hdlg, SlideQuick, CB_SETCURSEL, 0, 0);
      SetFocus(hdlg);
      return(1); }
  return(0);
}

void slide_cat_list(HWND hdlg, long action)
/* Configure the list of categories, maintaining selections, if any.
 * Enter: HWND hdlg: handle to the dialog which contains this list.
 *        long action: action to take: -1 to delete selected items, 0 for
 *                     refresh entire list, 1 for just refresh yes/never
 *                     check marks, 2 to effect yes to selected items, 3 to
 *                     effect never to selected items.          3/1/97-DWM */
{
  long sel[30], i, num, ind, yeson=0, yesoff=0, neveron=0, neveroff=0, on;
  long and, pos, len, caret;
  char text[300], never[]=" YN";

  memset(sel, 0, 30*sizeof(long));
  num = SendDlgItemMessage(hdlg, SlideCatList, LB_GETCOUNT, 0, 0);
  for (i=0; i<num; i++) {
    ind = SendDlgItemMessage(hdlg, SlideCatList, LB_GETITEMDATA, i, 0);
    sel[ind] = SendDlgItemMessage(hdlg, SlideCatList, LB_GETSEL, i, 0); }
  if (action==-1)
    for (EditCat=0; EditCat<30; EditCat++)  if (sel[EditCat]) {
      if (slide->catname) {
        pos = ((long *)(slide->cat+EditCat*8))[0];
        len = ((short *)(slide->cat+EditCat*8+4))[0];
        memmove(slide->catname+pos, slide->catname+pos+len,
                slide->lencatname-pos-len);
        for (i=0; i<30; i++)
          if (((long *)(slide->cat+i*8))[0]>pos)
            ((long *)(slide->cat+i*8))[0] -= len;
        slide->lencatname -= len;
        if (!slide->lencatname) {
          free2(slide->catname);  slide->catname = 0; } }
      memset(slide->cat+EditCat*8, 0, 8); }
  if (action==2) {
    on = SendDlgItemMessage(hdlg, SlideShowY, BM_GETCHECK, 0, 0);
    and = 0xFE;  if (on)  and = 0xFC;
    for (i=0; i<30; i++)  if (sel[i])
      slide->cat[i*8+7] = (slide->cat[i*8+7]&and)|on; }
  if (action==3) {
    on = SendDlgItemMessage(hdlg, SlideShowN, BM_GETCHECK, 0, 0);
    and = 0xFD;  if (on)  and = 0xFC;
    for (i=0; i<30; i++)  if (sel[i])
      slide->cat[i*8+7] = (slide->cat[i*8+7]&and)|(on*2); }
  if (action>=2)
    caret = SendDlgItemMessage(hdlg, SlideCatList, LB_GETTOPINDEX, 0, 0);
  if (action!=1) {
    SendDlgItemMessage(hdlg, SlideCatList, LB_RESETCONTENT, 0, 0);
    for (i=0; i<30; i++)
      if (((short *)(slide->cat+i*8+4))[0]) {
        strcpy(text, slide->catname+((long *)(slide->cat+i*8))[0]);
        if (slide->cat[i*8+6])
          sprintf(text+strlen(text), "\t%d", slide->cat[i*8+6]-1);
        else
          sprintf(text+strlen(text), "\t");
        sprintf(text+strlen(text), "\t%c", never[slide->cat[i*8+7]]);
        num = SendDlgItemMessage(hdlg, SlideCatList, LB_ADDSTRING, 0,
                                 (long)(text));
        SendDlgItemMessage(hdlg, SlideCatList, LB_SETITEMDATA, num, i);
        SendDlgItemMessage(hdlg, SlideCatList, LB_SETSEL, sel[i], num); } }
  if (action>=2)
    caret = SendDlgItemMessage(hdlg, SlideCatList, LB_SETTOPINDEX, caret, 0);
  for (i=0; i<30; i++) if (sel[i]) {
    if (slide->cat[i*8+7]&1) yeson++;   else yesoff++;
    if (slide->cat[i*8+7]&2) neveron++; else neveroff++; }
  on = (yeson!=0)+(yesoff!=0 && yeson!=0);
  if ((yeson || yesoff) && action!=2)
    SendDlgItemMessage(hdlg, SlideShowY, BM_SETCHECK, on, 0);
  on = (neveron!=0)+(neveroff!=0 && neveron!=0);
  if ((neveron || neveroff) && action!=3)
    SendDlgItemMessage(hdlg, SlideShowN, BM_SETCHECK, on, 0);
}

void slide_cat_select(long cat)
/* Toggle the state of the category for the current image in the slide show.
 * Enter: long cat: category to toggle (0-29) or quick key that was pressed
 *                   (30-39 corresponding to keys '0'-'9').     3/1/97-DWM */
{
  long i, on=0;

  if (!slide || LastNum<0)  return;
  if (cat>=40) { on = 1;  cat -= 10; }
  if (cat>=30)
    for (i=0; i<30; i++)
      if (((short *)(slide->cat+i*8+4))[0] && slide->cat[i*8+6]==cat-29)
        cat = i;
  if (cat>=30)  return;
  if (!((short *)(slide->cat+cat*8+4))[0])  return;
  if (!on)
    ((ulong *)(slide->file+FE*LastNum+0x20))[0] ^= (1<<cat);
  else
    ((ulong *)(slide->file+FE*LastNum+0x20))[0] |= (1<<cat);
  recheck(Hwnd, 0);
  preview_invalidate(LastNum, 0, 0);
}

void slide_clear(long view)
/* Clear the slide viewed flags.
 * Enter: long view: 0 to clear all images, 1 to clear only images which are
 *                   in viewable categories.                  11/12/96-DWM */
{
  long i;
  ulong and=0x80000000, cmp=0;

  if (!slide)  return;
  slidename[0] = 1;
  if (!view) {
    for (i=0; i<slide->numfile; i++)
      ((ulong *)(slide->file+FE*i+0x20))[0] &= 0xBFFFFFFF;
    return; }
  for (i=0; i<30; i++) {
    if (slide->cat[8*i+7]==1) { and |= (1<<i);  cmp |= (1<<i); }
    if (slide->cat[8*i+7]==2)   and |= (1<<i); }
  if (!(slide->option&0x400)) {
    for (i=0; i<slide->numfile; i++)
      if ((((ulong *)(slide->file+FE*i+0x20))[0]&and)==cmp)
        ((ulong *)(slide->file+FE*i+0x20))[0] &= 0xBFFFFFFF; }
  else {
    for (i=0; i<slide->numfile; i++)
      if (((((ulong *)(slide->file+FE*i+0x20))[0]&and)|cmp)==cmp &&
          ((((ulong *)(slide->file+FE*i+0x20))[0]&cmp) || !cmp))
        ((ulong *)(slide->file+FE*i+0x20))[0] &= 0xBFFFFFFF; }
}

void slide_command(long com)
/* Perform one of the slide menu functions which modifies both a slide
 *  setting and a menu setting.
 * Enter: long com: command number: -1-decrease delay, 0-set to manual
 *                  advance, 1-increase delay, 2-set to auto advance.
 *                                                            11/12/96-DWM */
{
  if (slide) {
    SlideDelay = slide->delay;  SlideOpt = slide->option; }
  switch (com) {
    case -1: if (SlideDelay>0)  SlideDelay--; break;
    case 0: SlideOpt &= 0xFFFFFFF7; break;
    case 1: if (SlideDelay<3600)  SlideDelay++; break;
    case 2: SlideOpt |= 8; }
  if (slide) {
    slide->delay = SlideDelay;  slide->option = SlideOpt; }
  recheck(Hwnd, 0);
  slidename[0] = 1;
}

void slide_delete(long num, long part)
/* Delete all or part of a slide entry, including file name and directory
 *  usage.
 * Enter: long num: number of slide to delete.
 *        long part: 0 to delete entire entry, 1 for just thumbnail, 2 to
 *                   eliminate unused names and directories (num is not used
 *                   in this case).                            3/23/97-DWM */
{
  long i, j, l, p, filenamepos, dirnamepos;
  char *s;

  if (!slide || (num<0 && part<2))  return;
  s = slide->file+FE*num;
  if (s[0x2C] && s[0x2D] && ((long *)(s+0x30))[1] && slide->thumb &&
      part!=2) {
    memmove(slide->thumb+((long *)(s+0x30))[0], slide->thumb+
            ((long *)(s+0x30))[0]+((long *)(s+0x30))[1], slide->
            lenthumb-((long *)(s+0x30))[0]-((long *)(s+0x30))[1]);
    for (j=0; j<slide->numfile; j++)
      if (((long *)(slide->file+FE*j+0x30))[0]>((long *)(s+0x30))[0])
        ((long *)(slide->file+FE*j+0x30))[0] -= ((long *)(s+0x30))[1];
    slide->lenthumb -= ((long *)(s+0x30))[1];
    if (!slide->lenthumb) {
      free2(slide->thumb);  slide->thumb = 0; }
    s[0x2C] = s[0x2D] = 0; }
  if (part==1)  return;
  if (part!=2) {
    filenamepos = ((long *)(s+2))[0];
    memmove(slide->file+FE*num, slide->file+FE*(num+1),
            (slide->numfile-num-1)*FE);
    if (LastNum>num)  LastNum--;
    else if (LastNum==num)  LastNum = -1;
    if (NextNum>num)  NextNum--;
    else if (NextNum==num) {
      if (NextPic)  free2(NextPic);  NextPic = 0;  NextNum = -1; }
    slide->numfile--; }
  /*
  slide->file = realloc2(slide->file, slide->numfile*FE);
  if (slide->thumb)
    slide->thumb = realloc2(slide->thumb, slide->lenthumb);
   */
  for (i=0; i<slide->numdir; i++) {
    for (j=0; j<slide->numfile; j++)
      if (((short *)(slide->file+j*FE))[0]==i) break;
    if (j!=slide->numfile)  continue;
    l = ((short *)(slide->dir+i*6+4))[0];
    p = ((long *)(slide->dir+i*6))[0];
    memmove(slide->dirname+p, slide->dirname+p+l, slide->lendirname-l-p);
    for (j=0; j<slide->numdir; j++)
      if (((long *)(slide->dir+j*6))[0]>p)
        ((long *)(slide->dir+j*6))[0] -= l;
    for (j=0; j<slide->numfile; j++)
      if (((short *)(slide->file+j*FE))[0]>i)
        ((short *)(slide->file+j*FE))[0]--;
    slide->lendirname -= l;
    memmove(slide->dir+i*6, slide->dir+i*6+6, (slide->numdir-i-1)*6);
    slide->numdir--;  i--; }
  /*
  slide->dir = realloc2(slide->dir, slide->numdir*6);
  slide->dirname = realloc2(slide->dirname, slide->lendirname);
   */
  for (i=(part!=2?filenamepos:0); i<(part!=2?filenamepos+1:slide->lenfilename) && i<slide->lenfilename;) {
    for (j=0; j<slide->numfile; j++)
      if (((long *)(slide->file+FE*j+2))[0]==i)  break;
    if (j!=slide->numfile) {
      i += strlen(slide->filename+i)+1;  continue; }
    l = strlen(slide->filename+i)+1;
    for (j=0; j<slide->numfile; j++)
      if (((long *)(slide->file+FE*j+2))[0]>i)
        ((long *)(slide->file+FE*j+2))[0] -= l;
    memmove(slide->filename+i,slide->filename+i+l, slide->lenfilename-i-l);
    slide->lenfilename -= l; }
  slide->filename = realloc2(slide->filename, slide->lenfilename);
}

BOOL CALLBACK slide_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the Slide Options dialog box.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.       11/16/96-DWM */
{
  char text[270];
  static char *oldcat=0;
  static long oldcatlen;
  long i, j, val, tabs[]={112,120};

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd,HelpFile,HELP_CONTEXT,HelpSlide); break;
      case IDOK: SlideOpt &= 0xFFFFFA07;
        if (!SendDlgItemMessage(hdlg, SlideOpt5, BM_GETCHECK, 0, 0))
          SlideOpt |= 0x20;
        if (!SendDlgItemMessage(hdlg, SlideOpt7, BM_GETCHECK, 0, 0))
          SlideOpt |= 0x80;
        if (SendDlgItemMessage(hdlg, SlideOpt6, BM_GETCHECK, 0, 0))
          SlideOpt |= 0x40;
        if (!SendDlgItemMessage(hdlg, SlideOpt8, BM_GETCHECK, 0, 0))
          SlideOpt |= 0x100;
        if (SendDlgItemMessage(hdlg, SlideOpt3, BM_GETCHECK, 0, 0))
          SlideOpt |= 8;
        if (SendDlgItemMessage(hdlg, SlideOpt10b, BM_GETCHECK, 0, 0))
          SlideOpt |= 0x400;
        GetDlgItemText(hdlg, SlideSec, text, 79);
        sscanf(text, "%d", &i);
        if (i>=0 && i<=3600)  SlideDelay = slide->delay = i;
        if (SendDlgItemMessage(hdlg, SlideOpt4, BM_GETCHECK, 0, 0))
          SlideOpt |= 0x10;
        slide->option = SlideOpt;
        if (oldcat) { free2(oldcat);  oldcat = 0; }
        EndDialog(hdlg, 1);  recheck(Hwnd, 0);
        for (i=0, val=-1; i<30; i++)
          if (!((short *)(slide->cat+i*8+4))[0])
            val ^= (1<<i);
        for (i=0; i<slide->numfile; i++)
          ((ulong *)(slide->file+FE*i+0x20))[0] &= val;
        if (HwndP)  InvalidateRect(HwndP, 0, 1);
        if (NextPic) {
          if (GetForegroundWindow()!=HwndP || !HwndP) {
            KillTimer(Hwnd, 2);
            slide_next();
            SetTimer(Hwnd, 2, 333, (TIMERPROC)timer); }
          else {
            free2(NextPic);  NextPic = 0; } }
        return(1);
      case IDCANCEL: if (oldcat) {
          memcpy(slide->cat, oldcat, 8*30);
          if (slide->catname) {
            free2(slide->catname);
            slide->catname = 0; }
          if (oldcatlen) {
            slide->catname = malloc2(oldcatlen);
            if (slide->catname)
              memcpy(slide->catname, oldcat+8*30, oldcatlen);
            else
              oldcatlen = 0; }
          slide->lencatname = oldcatlen;
          free2(oldcat); }
        EndDialog(hdlg, 1);  recheck(Hwnd, 0); return(1);
      case SlideCatList: slide_cat_list(hdlg, 1);
        if ((wp>>16)!=LBN_DBLCLK) break;
        i = SendDlgItemMessage(hdlg,SlideShowY,BM_GETCHECK,0,0);
        j = SendDlgItemMessage(hdlg,SlideShowN,BM_GETCHECK,0,0);
        if ((!i && !j) || i>1 || j>1) {
          SendDlgItemMessage(hdlg, SlideShowY, BM_SETCHECK, (i+1)%2, 0);
          slide_cat_list(hdlg, 2); }
        else {
          SendDlgItemMessage(hdlg, SlideShowN, BM_SETCHECK, (j+1)%2, 0);
          slide_cat_list(hdlg, 3); } break;
      case SlideDel: slide_cat_list(hdlg, -1); break;
      case SlideEdit:
        if (!SendDlgItemMessage(hdlg, SlideCatList, LB_GETSELCOUNT, 0, 0))
          break;
        val = SendDlgItemMessage(hdlg, SlideCatList, LB_GETANCHORINDEX, 0,0);
        if (SendDlgItemMessage(hdlg, SlideCatList, LB_GETSEL, val, 0))
          EditCat = SendDlgItemMessage(hdlg, SlideCatList, LB_GETITEMDATA,
                                         val, 0);
        DialogBox(hinst, MAKEINTRESOURCE(SlCatDLG), hdlg, (DLGPROC)slide_cat_dialog);
        slide_cat_list(hdlg, 0); break;
      case SlideNew: for(i=0; i<30; i++)
        if (!((short *)(slide->cat+i*8+4))[0])
          break;
        if (i==30) {
          if (Msg&4)
            MsgBox(hdlg, "There are already 30 categories.\n", "Warning",
                   MB_OK);
          break; }
        EditCat = i;
        DialogBox(hinst, MAKEINTRESOURCE(SlCatDLG), hdlg, (DLGPROC)slide_cat_dialog);
        slide_cat_list(hdlg, 0); break;
      case SlideRef: slide_refresh(1, hdlg);
        if (HwndP)  InvalidateRect(HwndP, 0, 1); Busy = 1; break;
      case SlideRoot: if (slide_root(hdlg))  slide_refresh(0, hdlg);
        if (HwndP)  InvalidateRect(HwndP, 0, 1); Busy = 1; break;
      case SlideShowN:
        SendDlgItemMessage(hdlg, SlideShowN, BM_SETCHECK,(SendDlgItemMessage(
                           hdlg, SlideShowN, BM_GETCHECK, 0, 0)+1)%2, 0);
        slide_cat_list(hdlg, 3); break;
      case SlideShowY:
        SendDlgItemMessage(hdlg, SlideShowY, BM_SETCHECK,(SendDlgItemMessage(
                           hdlg, SlideShowY, BM_GETCHECK, 0, 0)+1)%2, 0);
        slide_cat_list(hdlg, 2); break;
      default: return(0); } break;
    case WM_INITDIALOG: sprintf(text, "Root Directory: %s\\", slide->root);
      SetDlgItemText(hdlg, SlideDir, text);
      SendDlgItemMessage(hdlg, SlideOpt5, BM_SETCHECK, !(SlideOpt&0x20), 0);
      SendDlgItemMessage(hdlg, SlideOpt7, BM_SETCHECK, !(SlideOpt&0x80), 0);
      SendDlgItemMessage(hdlg, SlideOpt6, BM_SETCHECK, (SlideOpt&0x40)!=0,0);
      SendDlgItemMessage(hdlg, SlideOpt8, BM_SETCHECK, !(SlideOpt&0x100), 0);
      SendDlgItemMessage(hdlg, SlideOpt3, BM_SETCHECK, (SlideOpt&8)!=0, 0);
      sprintf(text, "%d", slide->delay);
      SetDlgItemText(hdlg, SlideSec, text);
      SendDlgItemMessage(hdlg, SlideOpt4, BM_SETCHECK,(SlideOpt&0x10)!=0,0);
      SendDlgItemMessage(hdlg, SlideOpt10a, BM_SETCHECK,!(SlideOpt&0x400),0);
      SendDlgItemMessage(hdlg,SlideOpt10b,BM_SETCHECK,(SlideOpt&0x400)!=0,0);
      SendDlgItemMessage(hdlg, SlideCatList, LB_SETTABSTOPS, 2, (long)tabs);
      oldcat = malloc2(8*30+slide->lencatname);
      if (oldcat) {
        memcpy(oldcat, slide->cat, 8*30);
        if (slide->lencatname)
          memcpy(oldcat+8*30, slide->catname, slide->lencatname);
        oldcatlen = slide->lencatname; }
      slide_cat_list(hdlg, 0);
      SetFocus(hdlg);
      return(1); }
  return(0);
}

void slide_fill(SlideList *s, HWND hwnd)
/* Create a new slide data set based on the specified parameters.
 * Enter: SlideList *s: basis of search.
 *        HWND hwnd: owner window.                             11/9/96-DWM */
{
  long ext[]={1,0x1FFFFE,6,8,0x30,0xC0,0x100,0x1E00,0x1E000,0x60000,0x80000,
              0x100000};
  char name[NAMELEN], name2[NAMELEN], *name3, *new, *file, text[80];
  long i, j;
  HWND hdlg;
  WIN32_FIND_DATA find;

  sprintf(name, "%s\\*.*", s->root);
  if (find_file(name, 0x1E00, (s->option&2)/2, 0, name2))
    return;
  find_file(0, 0, 0, 0, 0);
  hdlg = CreateDialog(hinst, MAKEINTRESOURCE(SlDLG), hwnd, (DLGPROC)null_dialog);
  if (s->filter) {
    name[0] = name[1] = 0;  strcpy(name+2, s->root);
    name3 = name+2+strlen(name+2)+1;
    for (i=0; extlist[i]; i++)
      if (ext[s->filter]&(1<<i)) {
        strcpy(name3, extlist[i]);
        name3 += strlen(name3)+1;
        name[1]++; } }
  else
    sprintf(name, "%s\\*.*", s->root);
  if (!(find_file(name, 0x1E00, (s->option&2)/2, &find, name2))) do {
    SetDlgItemText(hdlg, SlideStat2, name2);
    name3 = strrchr(name2, '\\');
    if (!name3)  name3 = name2;  else {  name3[0] = 0;   name3++; }
    if (!s->numfile) {
      if (!(new=malloc2((s->numfile+1)*FE)))  continue; }
    else {
      if (!(new=realloc2(s->file, (s->numfile+1)*FE)))  continue; }
    s->file = new;  file = new+(s->numfile*FE);
    memset(file, 0, FE);
    if (s->option&4)
      file[0xC] = -1;
    for (j=0; j<s->numdir; j++)
      if (!strcmp(s->dirname+((long *)(s->dir+j*6))[0], name2))
        break;
    ((short *)file)[0] = j;
    if (j==s->numdir) {
      if (!s->numdir) {
        s->dir = malloc2(6);  s->dirname = malloc2(strlen(name2)+1);
        if (!s->dir || !s->dirname)  continue; }
      else {
        if (!(new=realloc2(s->dir, s->numdir*6+6)))  continue;
        s->dir = new;
        if (!(new=realloc2(s->dirname, s->lendirname+strlen(name2)+1)))
          continue;
        s->dirname = new; }
      ((long *)(s->dir+6*s->numdir))[0] = s->lendirname;
      ((short *)(s->dir+6*s->numdir+4))[0] = strlen(name2)+1;
      strcpy(s->dirname+s->lendirname, name2);
      s->lendirname += strlen(name2)+1;
      s->numdir++; }
    if (!s->lenfilename) {
      if (!(s->filename=malloc2(strlen(name3)+1)))  continue; }
    else {
      if (!(new=realloc2(s->filename, s->lenfilename+strlen(name3)+1)))
        continue;
      s->filename = new; }
    ((long *)(file+2))[0] = s->lenfilename;
    strcpy(s->filename+s->lenfilename, name3);
    ((long *)(file+0x38))[0] = find.nFileSizeLow+(find.nFileSizeHigh<<16);
    FileTimeToDosDateTime(&find.ftLastWriteTime, (LPWORD)(file+0x3E), (LPWORD)(file+0x3C));
    s->lenfilename += strlen(name3)+1;
    s->numfile++;
    sprintf(text, "Located %d file%c", s->numfile,
            ' '+('s'-' ')*(s->numfile!=1));
    SetDlgItemText(hdlg, SlideStat1, text); }
  while (!(find_file(0,0,0, &find, name2)));
  DestroyWindow(hdlg);
}

long slide_free(long save)
/* Free any memory associated with a slide file.
 * Enter: long save: 0 for no save, 1 for auto save or ask, 2 for save but
 *                   don't free.  +4 for no menu recheck.
 * Exit:  long canceled: 0 for freed, 1 for canceled.          11/9/96-DWM */
{
  long res, rc=save&4;

  if (slide)
    if (slide->tx && slide->ty) {
      SlideTx = slide->tx;  SlideTy = slide->ty; }
  save &= 3;
  if (slidename[0] && save && slide) {
    if ((slide->option&0x100) || !slidename[1])
      res = MsgBox(Hwnd, "The slide show file has changed.  Save these changes?",
                   "Save Slide File?", MB_YESNOCANCEL);
    else res = IDYES;
    if (res==IDCANCEL)  return(1);
    if (res==IDYES)
      if (!slide_save(Hwnd, 0))
        return(1); }
  if (save==2)  return(0);
  if (NextPic) {
    free2(NextPic);  NextPic = 0; }
  if (!slide)  return(0);
  if (slide->catname)   free2(slide->catname);
  if (slide->dir)       free2(slide->dir);
  if (slide->dirname)   free2(slide->dirname);
  if (slide->file)      free2(slide->file);
  if (slide->filename)  free2(slide->filename);
  if (slide->thumb)     free2(slide->thumb);
  free2(slide);  slide = 0;
  if (!rc)  recheck(Hwnd, 0);
  return(0);
}

void slide_new(char *dir)
/* Create a new slide file using the specified directory and the parameters
 *  in SlideOpt.
 * Enter: char *dir: string containing root directory with trailing filename.
 *                   The file name is discarded.               11/9/96-DWM */
{
  KillTimer(Hwnd, 2);
  remove_quotes2(dir);
  if (strrchr(dir, '\\'))  strrchr(dir, '\\')[0] = 0;
  if (slide_free(1))  return;
  if (!(slide=malloc2(sizeof(SlideList)+8*30+strlen(dir)+1))) {
    low_memory(Hwnd, "Creating Slide Information", 0);
    SetTimer(Hwnd, 2, 333, (TIMERPROC)timer);  return; }
  memset(slide, 0, sizeof(SlideList)+8*30);
  slide->cat = ((char *)slide)+sizeof(SlideList);
  slide->root = slide->cat+8*30;
  strcpy(slide->root, dir);
  recheck(Hwnd, 0);
  slide->option = SlideOpt;
  slide->tx = SlideTx;  slide->ty = SlideTy;  slide->ttop = 0;
  slide->delay = SlideDelay;
  slide->filter = max(openfile.nFilterIndex, 2)-2;
  slide_fill(slide, Hwnd);
  if (!slide->numfile) {
    if (Msg&1)
      MsgBox(Hwnd, "No files found.\n", "Error", MB_OK);
    slide_free(0);
    SetTimer(Hwnd, 2, 333, (TIMERPROC)timer);  return; }
  LastNum = -1;
  SlideLast = -SlideDelay*CLOCKS_PER_SEC;
  slidename[0] = 1;  slidename[1] = 0;
  if (!PMode)
    slide_show();
  SetTimer(Hwnd, 2, 333, (TIMERPROC)timer);
}

void slide_next(void)
/* Select and load in the next slide picture.  The next picture is chosen
 *  based on category and order criterion.  If no pictures are available,
 *  the viewed flags are cleared.  If still no pictures are available, the
 *  next picture is shown.  In the event of no pictures, the slide file is
 *  cleared.  When a picture is successfully loaded, its parameters (width,
 *  height, inter, and pspec) are copied to the slide info area.
 *                                                            11/12/96-DWM */
{
  long i, avail=0, view=0, other=0, next=LastNum, more, tw, th, ti, okay;
  long num;
  ulong and=0xC0000000, cmp=0;
  char name[NAMELEN], *s, *new;
  short *p;

  if (!slide || TimerOn)  return;
  for (i=0; i<30; i++) {
    if (slide->cat[8*i+7]==1) { and |= (1<<i);  cmp |= (1<<i); }
    if (slide->cat[8*i+7]==2)   and |= (1<<i); }
  if (!(slide->option&0x400)) {
    for (i=0; i<slide->numfile; i++)
      if ((((ulong *)(slide->file+FE*i+0x20))[0]&and)==cmp)
        avail++;
      else if ((((ulong *)(slide->file+FE*i+0x20))[0]&and&0xBFFFFFFF)==cmp)
        view++;
      else if (!(((ulong *)(slide->file+FE*i+0x20))[0]&0x80000000))
        other++; }
  else {
    for (i=0; i<slide->numfile; i++)
      if (((((ulong *)(slide->file+FE*i+0x20))[0]&and)|cmp)==cmp &&
          ((((ulong *)(slide->file+FE*i+0x20))[0]&cmp) || !cmp))
        avail++;
      else if (((((ulong *)(slide->file+FE*i+0x20))[0]&and&0xBFFFFFFF)|cmp)==
          cmp && ((((ulong *)(slide->file+FE*i+0x20))[0]&cmp) || !cmp))
        view++;
      else if (!(((ulong *)(slide->file+FE*i+0x20))[0]&0x80000000))
        other++; }
  if (!other && !view && !avail) {
    if (HwndP)
      SendMessage(HwndP, WM_CLOSE, 0, 0);
    slide_free(0);  return; }
  if (!avail) { slide_clear(1);  avail = view; }
  if (!avail && !view) {
    and = 0x80000000;  cmp = 0;  avail = view = other; }
  cursor(1);
  Busy++;
  do {
    do {
      if (slide->option&0x10)  next = rand()%slide->numfile;
      else                     next = (next+1)%slide->numfile;
      if (!(slide->option&0x400))
        okay = ((((ulong *)(slide->file+FE*next+0x20))[0]&and)==cmp);
      else
        okay = (((((ulong *)(slide->file+FE*next+0x20))[0]&and)|cmp)==cmp &&
                ((((ulong *)(slide->file+FE*next+0x20))[0]&cmp) || !cmp)); }
    while (!okay);
    if (NextPic && NextNum==next) { cursor(0);  Busy--; return; }
    if (NextPic)  free2(NextPic);  NextPic = 0;
    s = slide->file+FE*next;
    sprintf(name, "%s\\%s", slide->dirname+((long *)(slide->dir+
            ((short *)s)[0]*6))[0], slide->filename+((long *)(s+2))[0]);
    LoadPart = -1;
    more = s[0xC];
    num = (((uchar *)s)[0xC+1]&0x7F);
    if (s[0xC]<0 || s[0xC]==2 || s[0xC]==5 || s[0xC]==9)
      num += (((short *)(s+0xC+0x12))[0]<<7);
    ppart = num;
    Minpwidth = Minpheight = 0;
    NextPic = load_graphic(name);
    if (!NextPic) {
      slidename[0] = 1;
      ((ulong *)(slide->file+FE*next+0x20))[0] |= 0x80000000;
      slide->file[FE*next+0xC] = -2;
      avail--;
      if (!avail) { slide_clear(1);  avail = view; }
      if (!avail && !view) {
        and = 0x80000000;  cmp = 0;  avail = view = other; } } }
  while (!NextPic && avail);
  if (!avail) {
    slide_free(0);  cursor(0);  Busy--;  return; }
  if (!pinter && Reduce && BitsPixel<=16) {
    new = pic;  tw = oldw;  th = oldh;  ti = oldi;
    pic = NextPic;  oldw = pwidth;  oldh = pheight;  oldi = pinter;
    reduce_res(Hwnd, 1);
    NextPic = pic;  pwidth = oldw;  pheight = oldh;  pinter = oldi;
    pic = new;  oldw = tw;  oldh = th;  oldi = ti; }
  NextNum = next;
  ((short *)(s+6))[0] = pwidth;  ((short *)(s+6))[1] = pheight;
  ((short *)(s+6))[2] = pinter;
  memcpy(s+0xC, pspec, LENPSPEC);
  p = (short *)(s+0x24);
  if (!p[2])  p[2] = pwidth;  if (!p[3])  p[3] = pheight;
  cursor(0);
  if (more==-1 && (pspec[1]&0x80)) {
    slidename[0] = 1;
    if (!(new=realloc2(slide->file, (slide->numfile+1)*FE))) {
      Busy--; return; }
    slide->file = new;
    memmove(slide->file+(next+1)*FE, slide->file+next*FE,
            (slide->numfile-next)*FE);
    slide->numfile ++;
    s = slide->file+FE*next;
    memset(s+FE+0x2C, 0, 12);
    s[FE+0xC] = -1;
    num = ((uchar *)pspec)[1]&0x7F;
    if (pspec[0]==2 || pspec[0]==5 || pspec[0]==9)
      num += (((short *)(pspec+0x12))[0]<<7);
    s[FE+0xC+1] = (num+1)&0x7F;
    if (pspec[0]==2 || pspec[0]==5 || pspec[0]==9)
      ((short *)(s+FE+0xC+0x12))[0] = ((num+1)>>7); }
  Busy--;
}

void slide_next_nonmulti(void)
/* Set all files with the same name as this one as viewed, then advance to
 *  the next available slide.                                 11/14/96-DWM */
{
  long name, i, num;
  char *s;

  if (!slide)  return;
  if (LastNum<0 || LastNum>=slide->numfile) { slide_show();  return; }
  s = slide->file+FE*LastNum;
  num = (((uchar *)s)[0xC+1]&0x7F);
  if (s[0xC]<0 || s[0xC]==2 || s[0xC]==5 || s[0xC]==9)
    num += (((short *)(s+0xC+0x12))[0]<<7);
  if (!s[0xC+1] && !num) { slide_show();  return; }
  name = ((long *)(s+2))[0];
  for (i=0; i<slide->numfile; i++) {
    s = slide->file+FE*i;
    if (((long *)(s+2))[0]==name)
      ((long *)(s+0x20))[0] |= 0x40000000; }
  KillTimer(Hwnd, 2);
  slide_next();
  SetTimer(Hwnd, 2, 333, (TIMERPROC)timer);
  if (!slide)  return;
  slide_show();
}

void slide_open(HWND hwnd)
/* If no slide file is present, present a dialog to select a directory or a
 *  slide file.  If a slide file is already present, set to auto advance.
 * A GV Slide file is in the following format:
 *  byte $0: "GVSlideFileV1.0"
 *       10: (long) number of file entries
 *       14: (long) starting location of file entries
 *       18: (long) compressed length of file entries
 *       1C: (long) number of directories
 *       20: (long) starting location of directories
 *       24: (long) length of directory names, see below
 *       28: (long) compressed length of directory names
 *       2C: (long) starting location of file names
 *       30: (long) length of file names
 *       34: (long) compressed length of file names
 *       38: (long) starting location of categories (there are always 30)
 *       3C: (long) length of category names, see below
 *       40: (long) compressed length of category names
 *       44: (long) starting location of thumbnail data
 *       48: (long) length of thumbnail data
 *       4C: (long) slide show option flags (SlideOpt flags)
 *           bit 0: 0-filter to all files, 1-filter to GV slide files
 *               1: 0-no recurse, 1-recurse
 *               2: 0-no multiopen, 1-multiopen
 *               3: 0-manual advance, 1-auto advance
 *               4: 0-sequential display, 1-random display
 *               5: 0-preemptive loading, 1-on-demand loading
 *               6: 0-manual refresh, 1-refresh on loading
 *               7: 0-track view area, 1-don't track view area
 *               8: 0-auto save, 1-manual save
 *               9: 0-new slide file, 1-changing root directory
 *              10: 0-and category include, 1-or category include
 *           Preview option flags
 *              24: 0-save selection, 1-save all
 *              25: 0-make thumbnails, 1-don't make thumbnails
 *              26: 0-show thumbnail, 1-don't show
 *              27: 0-show file name, 1-don't show
 *              28: 0-show categories, 1-don't show
 *              29: 0-show image size, 1-don't show
 *              30: 0-show pixel depth, 1-don't show
 *              31: 0-show only eligible files, 1-show all files
 *       50: (long) minimum time delay (seconds)
 *       54: (short) limiting filter number
 *       56: (2 chars) desired size of thumbnail
 *       58: (long) length of root directory name
 *       5C: (char *) root directory
 *  The file entries are each $48 (72) bytes in length and are of the form:
 *        0: (short) directory number (0 to numdir-1)
 *        2: (long) file name (offset within filename data)
 *        6: (2 shorts) size of original image
 *        A: (short) pinter of original image
 *        C: (20 char) file info block as returned by load_graphic
 *       20: (long) category and viewed data
 *           bit 0-29: category flags
 *                 30: viewed flag
 *                 31: invalid flag - set if this is not a valid graphic file
 *       24: (4 shorts) selected region of picture
 *       2C: (2 chars) size of thumbnail
 *       2E: (char) palettization of thumbnail (0-24 bit, 1-8 bit)
 *       2F: (char) scale and select flags
 *           bit 0: 0-scaled from orginal, 1-actual size
 *               1: 0-not selected, 1-selected
 *               2: 0-selection not changing, 1-selection changing
 *       30: (long) location of thumbnail (offset within thumbnail data)
 *       34: (long) length of thumbnail
 *       38: (long) length of file
 *       3C: (short) file time.  Bits 0-4: seconds/2 (0-29), 5-10: minute
 *                   (0-59), 11-15: hour (0-23).
 *       3E: (short) file date.  Bits 0-4: day (0-31), 5-8: month (1-12),
 *                   9-15: year-1980
 *       40: (long) position in preview or -1 for not in preview
 *       44: (long) used for sort
 *  The directories have a header followed by a text strike of each name:
 *        0: (long) location of text (offset from start of text strike)
 *        4: (short) length of text (including terminating null)
 *    6*numdir: text strike
 *  The file names are stored as a text strike.
 *  The categories have a header followed by a text strike of each name:
 *        0: (long) location of text (offset from start of text strike)
 *        4: (short) length of text (including terminating null).  Zero for
 *           category not defined.
 *        6: (char) Associated quick key, 0 for none, 1-10 for '0'-'9'.
 *        7: (char) Show property: 0-doesn't matter, 1-include, 2-exclude
 *     8*30: text strike (note that there are exactly 30 categories)
 *  The thumbnail data is referenced by the file entries.  Each thumbnail is
 *   of the form:
 *        0: lzw compressed RGB picture of the size specified in file entry.
 *  Within the file, file, directory, and category names are LZW compressed.
 *   This is both to save space and to keep undesirable file names hidden.
 * SlideOpt contains the same flags as the slide show file.
 * Enter: HWND hwnd: handle of top window.                    10/30/96-DWM */
{
  long len, i;
  FILE *fptr;
  char check[16];

  if (slide_free(2))
    return;
  strcpy(opfile, "----");
  sprintf(opfilter, SlideFilter, OpenFilter);
  len = strlen(opfilter);
  for (i=0; i<len; i++)  if (opfilter[i]=='|')  opfilter[i] = 0;
  openfile.lStructSize = sizeof(OPENFILENAME);
  openfile.hwndOwner = hwnd;  openfile.hInstance = hinst;
  openfile.lpstrFilter = opfilter;
  openfile.nFilterIndex = 3-(SlideOpt&1)*2;
  openfile.lpstrFile = opfile;
  openfile.nMaxFile = NAMELEN;
  openfile.lpstrFileTitle = openfile.lpstrCustomFilter = 0;
  openfile.lpstrInitialDir = opendir+768;
  openfile.lpfnHook = (DLGPROC)open_slide_dialog;
  openfile.lpTemplateName = MAKEINTRESOURCE(OpDLG);
  SlideOpt &= 0xFFFFFDFF;
  openfile.Flags = OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_HIDEREADONLY |
                   OFN_NONETWORKBUTTON;
  Busy = 1;
  if (GetOpenFileName(&openfile)) {
    Busy = 0;
    SlideOpt = (SlideOpt&0xFFFFFFFE) | (openfile.nFilterIndex==1);
    if (!(fptr=fopen(opfile, "rb"))) {
      strpath2(opendir+768, opfile);
      slide_free(0);
      slide_new(opfile);
      return; }
    fread(check, 1, 16, fptr);
    fclose(fptr);
    if (!memcmp(check, slidecheck, 16)) {
      slide_free(0);
      slide_open_mid(); }
    else
      open_pic_mid(hwnd, opfile, 1); }
  Busy = 0;
}

void slide_open_mid(void)
/* Open the slide file specified in opfile.                   11/16/96-DWM */
{
  FILE *fptr;
  char check[16], csi[]="Creating internal slide record", *comp, *s;
  char name[NAMELEN];
  long head[19], i, oldlen=FE;

  KillTimer(Hwnd, 2);
  if (slide_free(1))  return;
  strpath(opendir+768, opfile);
  if (!(fptr=fopen(opfile, "rb"))) {
    if (Msg&1)  MsgBox(Hwnd, "Can't read slide file.", "Error", MB_OK);
    return; }
  fread(check, 1, 16, fptr);
  if (memcmp(check, slidecheck, 16)) {
    if (Msg&1)  MsgBox(Hwnd, "Invalid slide file.", "Error", MB_OK);
    fclose(fptr);  return; }
  cursor(1);
  fread(head, 4, 19, fptr);
  if (!(slide=malloc2(sizeof(SlideList)+8*30+head[18]))) {
    low_memory(Hwnd, csi, 0);  fclose(fptr);
    return; }
  memset(slide, 0, sizeof(SlideList)+8*30);
  slide->cat = ((char *)slide)+sizeof(SlideList);
  slide->root = slide->cat+8*30;
  fread(slide->root, 1, head[18], fptr);
  slide->catname = slide->thumb = 0;
  slide->file = malloc2(head[0]*FE);  slide->dir = malloc2(head[3]*6);
  slide->dirname = malloc2(head[5]);  slide->filename = malloc2(head[8]);
  if (head[11])  slide->catname = malloc2(head[11]);
  if (head[14])  slide->thumb = malloc2(head[14]);
  if (!slide->file || !slide->dir || !slide->dirname || !slide->filename ||
      (!slide->catname && head[11]) || (!slide->thumb && head[14])) {
    low_memory(Hwnd, csi, 0);  fclose(fptr);
    slide_free(0);  return; }
  slide->numfile = head[0];
  fseek(fptr, head[1], SEEK_SET);
  if (!(comp=malloc2(head[2]))) {
    low_memory(Hwnd, csi, 0);  slide_free(0);  fclose(fptr);  return; }
  fread(comp, 1, head[2], fptr);
  memset(slide->file, 0xFF, head[0]*FE);
  unlzw(slide->file, comp, head[0]*oldlen, head[2], 8);
  free2(comp);
  for (i=head[0]*FE-1; i>=0; i--)
    if (((uchar *)slide->file)[i]!=0xFF)  break;
  oldlen = (i+head[0])/head[0];
  if (oldlen<FE)
    for (i=head[0]-1; i>=0; i--) {
      memmove(slide->file+i*FE, slide->file+i*oldlen, oldlen);
      memset(slide->file+i*FE+oldlen, 0, FE-oldlen); }
  slide->numdir = head[3];
  slide->lendirname = head[5];
  fseek(fptr, head[4], SEEK_SET);
  fread(slide->dir, 6, head[3], fptr);
  if (!(comp=malloc2(head[6]))) {
    low_memory(Hwnd, csi, 0);  slide_free(0);  fclose(fptr);  return; }
  fread(comp, 1, head[6], fptr);
  unlzw(slide->dirname, comp, head[5], head[6], 8);
  free2(comp);
  slide->lenfilename = head[8];
  fseek(fptr, head[7], SEEK_SET);
  if (!(comp=malloc2(head[9]))) {
    low_memory(Hwnd, csi, 0);  slide_free(0);  fclose(fptr);  return; }
  fread(comp, 1, head[9], fptr);
  unlzw(slide->filename, comp, head[8], head[9], 8);
  free2(comp);
  slide->lencatname = head[11];
  fseek(fptr, head[10], SEEK_SET);
  fread(slide->cat, 8, 30, fptr);
  if (head[11]) {
    if (!(comp=malloc2(head[12]))) {
      low_memory(Hwnd, csi, 0);  slide_free(0);  fclose(fptr);  return; }
    fread(comp, 1, head[12], fptr);
    unlzw(slide->catname, comp, head[11], head[12], 8);
    free2(comp); }
  slide->lenthumb = head[14];
  if (head[14]) {
    fseek(fptr, head[13], SEEK_SET);
    fread(slide->thumb, 1, head[14], fptr); }
  slide->option = head[15]|1;
  slide->delay = head[16];
  slide->filter = ((short *)(head+17))[0];
  slide->tx = ((uchar *)(head+17))[2];
  slide->ty = ((uchar *)(head+17))[3];
  slide->ttop = 0;
  if (slide->tx<40) slide->tx = 40;
  if (slide->ty<10) slide->ty = 10;
  fclose(fptr);
  SlideOpt = slide->option;  SlideTx = slide->tx;  SlideTy = slide->ty;
  strcpy(slidename+1, opfile);
  slidename[0] = 0;
  recheck(Hwnd, 0);
  cursor(0);
  for (i=0; i<slide->numfile; i++)
    ((uchar *)slide->file)[i*FE+0x2F] &= 0xF9;
  s = slide->file;
  sprintf(name, "%s\\%s", slide->dirname+((long *)(slide->dir+
          ((short *)s)[0]*6))[0], slide->filename+((long *)(s+2))[0]);
  if (find_file(name, 0x1E00, (slide->option&2)/2, 0, name)) {
    find_file(0, 0, 0, 0, 0);
    if (!slide_root(Hwnd)) {
      slide_free(0);
      return; }
    slide_refresh(0, Hwnd);
    sprintf(name, "%s\\*.*", slide->root);
    if (find_file(name, 0x1E00, (slide->option&2)/2, 0, name)) {
      find_file(0, 0, 0, 0, 0);
      if (Msg&1)
        MsgBox(Hwnd, "No files found.\n", "Error", MB_OK);
      slide_free(0);
      return; }
    find_file(0, 0, 0, 0, 0); }
  else {
    find_file(0, 0, 0, 0, 0);
    if (slide->option&0x40)
      slide_refresh(0, Hwnd); }
  slide_show();
  SetTimer(Hwnd, 2, 333, (TIMERPROC)timer);
}

void slide_refresh(long remove, HWND hwnd)
/* Refresh the slide list.  This rescans the files in the root directory,
 *  adding new files and either marking missing files as unviewable or
 *  removing them.
 * Enter: long remove: 0 to mark missing files as unviewable, 1 to remove
 *                     them.
 *        HWND hwnd: owner window.                            11/29/96-DWM */
{
  SlideList *slide2, *slide3;
  long i, j, d, dir, found, l, p;
  long dirlen=0, dirnamelen=0, filelen=0, filenamelen=0;
  char *name, *name2, *new, *file;
  uchar *s;

  slide->ttop = slide->tsel = 0;
  if (HwndP)  InvalidateRect(HwndP, 0, 1);
  if (!(slide2=malloc2(sizeof(SlideList)+8*30+strlen(slide->root)+1))) {
    low_memory(Hwnd, "Creating Slide Information", 0);  return; }
  memset(slide2, 0, sizeof(SlideList)+8*30);
  slide2->cat = ((char *)slide2)+sizeof(SlideList);
  slide2->root = slide2->cat+8*30;
  strcpy(slide2->root, slide->root);
  slide2->option = slide->option;
  slide2->filter = slide->filter;
  slide_fill(slide2, hwnd);
  if (!slide2->numfile) {
    if (Msg&1)
      MsgBox(Hwnd, "No files found.\n", "Error", MB_OK);
    slide3 = slide;  slide = slide2;  slide_free(4);  slide = slide3;
    return; }
  for (i=0; i<slide->numfile; i++)
    ((ulong *)(slide->file+FE*i+0x20))[0] |= 0x80000000;
  d = slide->numdir;
  for (i=0; i<slide2->numfile; i++) {
    name2 = slide2->dirname+((long *)(slide2->dir+
                             ((short *)(slide2->file+FE*i))[0]*6))[0];
    name = slide2->filename+((long *)(slide2->file+FE*i+2))[0];
    for (dir=0; dir<d; dir++)
      if (!strcmp(slide->dirname+((long *)(slide->dir+dir*6))[0], name2))
        break;
    if (dir==d) {
      if (slide->numdir*6+6 > dirlen) {
        dirlen = (slide->numdir*6+6) * 2;
      }
      if (!(new=realloc2(slide->dir, dirlen)))  continue;
      slide->dir = new;
      if (slide->lendirname+strlen(name2)+1 > dirnamelen) {
        dirnamelen = (slide->lendirname+strlen(name2)+1) * 2;
      }
      if (!(new=realloc2(slide->dirname, dirnamelen)))
        continue;
      slide->dirname = new;
      ((long *)(slide->dir+6*slide->numdir))[0] = slide->lendirname;
      ((short *)(slide->dir+6*slide->numdir+4))[0] = strlen(name2)+1;
      strcpy(slide->dirname+slide->lendirname, name2);
      slide->lendirname += strlen(name2)+1;
      slide->numdir++;  d++;
      found = 0; }
    else
      for (j=found=0; j<slide->numfile; j++) {
        if (!(((ulong *)(slide->file+FE*j+0x20))[0]&0x80000000))  continue;
        if (strcmp(name2, slide->dirname+
            ((long *)(slide->dir+((short *)(slide->file+FE*j))[0]*6))[0]))
          continue;
        if (!strcmp(slide->filename+((long *)(slide->file+FE*j+2))[0],
                    name)) {
          ((ulong *)(slide->file+FE*j+0x20))[0] &= 0x7FFFFFFF;
          if (((ulong *)(slide->file+FE*j+0x3C))[0]<
              ((ulong *)(slide2->file+FE*i+0x3C))[0])
            slide->file[FE*j+0xC] = -1;
          memcpy(slide->file+FE*j+0x38, slide2->file+FE*i+0x38, 8);
          found = 1; } }
    if (!found) {
      if ((slide->numfile+1)*FE > filelen) {
        filelen = ((slide->numfile+1)*FE) * 2;
      }
      if (!(new=realloc2(slide->file, filelen)))  continue;
      slide->file = new;  file = new+(slide->numfile*FE);
      memcpy(file, slide2->file+i*FE, FE);
      ((short *)file)[0] = dir;
      if (slide->lenfilename+strlen(name)+1 > filenamelen) {
        filenamelen = (slide->lenfilename+strlen(name)+1) * 2;
      }
      if (!(new=realloc2(slide->filename, filenamelen)))
        continue;
      slide->filename = new;
      ((long *)(file+2))[0] = slide->lenfilename;
      strcpy(slide->filename+slide->lenfilename, name);
      slide->lenfilename += strlen(name)+1;
      slide->numfile++; } }
  slide3 = slide;  slide = slide2;  slide_free(4);  slide = slide3;
  slidename[0] = 1;
  if (remove) {
    for (i=0; i<slide->numfile && slide->numfile>=1; i++)
      if (((ulong *)(slide->file+FE*i+0x20))[0]&0x80000000) {
        slide_delete(i, 0);
        i--; } }
}

long slide_root(HWND hwnd)
/* Get a new root directory for a slide file.
 * Enter: HWND hwnd: handle of calling window.
 * Exit:  long new: 0 for no change, 1 for changed.           12/28/96-DWM */
{
  long len, i, ol, nl, l;
  FILE *fptr;
  SlideList *s2;
  char *name;

  strcpy(opfile, "----");
  strcpy(opfilter, OpenFilter);
  len = strlen(opfilter);
  for (i=0; i<len; i++)  if (opfilter[i]=='|')  opfilter[i] = 0;
  openfile.lStructSize = sizeof(OPENFILENAME);
  openfile.hwndOwner = hwnd;  openfile.hInstance = hinst;
  openfile.lpstrFilter = opfilter;
  openfile.nFilterIndex = slide->filter+1;
  openfile.lpstrFile = opfile;
  openfile.nMaxFile = NAMELEN;
  openfile.lpstrFileTitle = openfile.lpstrCustomFilter = 0;
  openfile.lpstrInitialDir = slide->root;
  openfile.lpfnHook = (DLGPROC)open_slide_dialog;
  openfile.lpTemplateName = MAKEINTRESOURCE(OpDLG);
  openfile.Flags = OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_HIDEREADONLY |
                   OFN_NONETWORKBUTTON;
  Busy = 1;  SlideOpt = slide->option|0x200;
  if (!GetOpenFileName(&openfile)) { Busy = 0;  return(0); }
  Busy = 0;
  strpath(opendir+768, opfile);
  remove_quotes(opfile);
  if (strrchr(opfile, '\\'))  strrchr(opfile, '\\')[0] = 0;
  if (!(s2=malloc2(sizeof(SlideList)+8*30+strlen(opfile)+1))) {
    low_memory(hwnd, "Creating New Root Information", 0);  return(0); }
  memcpy(s2, slide, sizeof(SlideList)+8*30);
  s2->cat = ((char *)s2)+sizeof(SlideList);
  s2->root = s2->cat+8*30;
  strcpy(s2->root, opfile);
  s2->filter = max(openfile.nFilterIndex, 1)-1;
  s2->option = SlideOpt;
  slidename[0] = 1;
  ol = strlen(slide->root);  nl = strlen(s2->root);
  free2(slide);  slide = s2;
  if (!slide->numdir)  return(1);
  if (!(name=malloc2(slide->lendirname+slide->numdir*(nl-ol)))) {
    low_memory(hwnd, "Creating New Root Information", 0);  return(0); }
  for (i=l=0; i<slide->numdir; i++) {
    sprintf(name+l, "%s%s", slide->root,
            slide->dirname+((long *)(slide->dir+i*6))[0]+ol);
    ((long *)(slide->dir+i*6))[0] = l;
    ((short *)(slide->dir+i*6+4))[0] = strlen(name+l)+1;
    l += strlen(name+l)+1; }
  free2(slide->dirname);
  slide->dirname = name;
  slide->lendirname = l;
  return(1);
}

long slide_save(HWND hwnd, long saveas)
/* Save the current slide show.  See slide_open for the file format.
 * Enter: HWND hwnd: handle of topmost window.
 *        long saveas: 0 for save, 1 for save as.
 * Exit:  long okay: 0 for cancelled, 1 for okay.             11/15/96-DWM */
{
  char *name, *ext;
  char serr[]="Saving slide file -- File now invalid";
  long head[19], max;
  uchar *work, *comp;
  FILE *fptr;

  if ((!saveas && !slidename[0]) || !slide)  return(0);
  Busy = 1;
  strcpy(opfile, slidename+1);
  if (saveas || !slidename[1]) {
    if (opfile[0])
      if (strchr(opfile, '\\')) {
        strcpy(opfile, strrchr(opfile, '\\')+1);
        strpath(savedir+768, slidename+1); }
    memcpy(opfilter, "GV - GV Slide file\0*.GV\0", 25);
    openfile.lStructSize = sizeof(OPENFILENAME);
    openfile.hwndOwner = hwnd;  openfile.hInstance = hinst;
    openfile.lpstrFilter = opfilter;
    openfile.nFilterIndex = 1;
    openfile.lpstrFile = opfile;
    openfile.nMaxFile = 256;
    openfile.lpstrFileTitle = openfile.lpstrCustomFilter = 0;
    openfile.lpstrInitialDir = savedir+768;
    openfile.Flags = OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_HIDEREADONLY |
                     OFN_NONETWORKBUTTON;
    if (Msg&4)  openfile.Flags |= OFN_OVERWRITEPROMPT;
    openfile.lpfnHook = (DLGPROC)save_dialog;
    openfile.lpTemplateName = MAKEINTRESOURCE(StdSaveDLG);
    SaveAs = 5;
    if (!GetSaveFileName(&openfile)) { Busy = 0; return(0); }
    ext = strrchr(opfile, '.');
    if (!ext)  ext = opfile+strlen(opfile);
    strcpy(ext, ".GV");
    strpath(savedir+768, opfile); }
  cursor(1);
  if (!(work=malloc2(28*1024))) {
    low_memory(hwnd, "Allocating work space", 0);  Busy = 0; return(0); }
  if (!(fptr=fopen(opfile, "w+b"))) {
    if (Msg&1)  MsgBox(hwnd, "Can't write to slide file.", "Error", MB_OK);
    cursor(0);
    free2(work);  Busy = 0; return(0); }
  fwrite(slidecheck, 1, 16, fptr);
  fwrite(head, 4, 19, fptr);
  fwrite(slide->root, 1, strlen(slide->root)+1, fptr);
  head[18] = strlen(slide->root)+1;
  head[0] = slide->numfile;
  head[1] = ftell(fptr);
  if (!(comp=malloc2(max=(long)(slide->numfile*FE*1.391+16)))) {
    low_memory(hwnd, serr, 0);  free2(work);  fclose(fptr);  Busy = 0;
    return(0); }
  slide->file[slide->numfile*FE-1] = 0;
  head[2] = lzw(comp, slide->file, work, slide->numfile*FE, max, 8);
  fwrite(comp, 1, head[2], fptr);
  free2(comp);
  head[3] = slide->numdir;
  head[4] = ftell(fptr);
  head[5] = slide->lendirname;
  if (!(comp=malloc2(max=(long)(slide->lendirname*1.391+16)))) {
    low_memory(hwnd, serr, 0);  free2(work);  fclose(fptr);  Busy = 0;
    return(0); }
  head[6] = lzw(comp, slide->dirname, work, slide->lendirname, max, 8);
  fwrite(slide->dir, 6, slide->numdir, fptr);
  fwrite(comp, 1, head[6], fptr);
  free2(comp);
  head[7] = ftell(fptr);
  head[8] = slide->lenfilename;
  if (!(comp=malloc2(max=(long)(slide->lenfilename*1.391+16)))) {
    low_memory(hwnd, serr, 0);  free2(work);  fclose(fptr);
    Busy = 0;  return(0); }
  head[9] = lzw(comp, slide->filename, work, slide->lenfilename, max, 8);
  fwrite(comp, 1, head[9], fptr);
  free2(comp);
  head[10] = ftell(fptr);
  head[11] = slide->lencatname;
  if (!(comp=malloc2(max=(long)(slide->lencatname*1.391+16)))) {
    low_memory(hwnd, serr, 0);  free2(work);  fclose(fptr);
    Busy = 0;  return(0); }
  if (slide->lencatname)
    head[12] = lzw(comp, slide->catname, work, slide->lencatname, max, 8);
  else
    head[12] = 0;
  fwrite(slide->cat, 8, 30, fptr);
  if (head[12])
    fwrite(comp, 1, head[12], fptr);
  head[13] = ftell(fptr);
  head[14] = slide->lenthumb;
  if (slide->lenthumb)
    fwrite(slide->thumb, 1, slide->lenthumb, fptr);
  head[15] = slide->option;
  head[16] = slide->delay;
  ((short *)(head+17))[0] = slide->filter;
  ((uchar *)(head+17))[2] = slide->tx;
  ((uchar *)(head+17))[3] = slide->ty;
  fseek(fptr, 16, SEEK_SET);
  fwrite(head, 4, 19, fptr);
  fclose(fptr);
  free2(work);
  slidename[0] = 0;
  strcpy(slidename+1, opfile);
  cursor(0);
  Busy = 0;
  return(1);
}

void slide_show(void)
/* Show the next slide in the sequence.  If no picture has yet been loaded,
 *  load one now.                                             11/12/96-DWM */
{
  char *s;
  short *p;
  long mo=MultiOpen, i;
  RECT rect;

  if (!slide || TimerOn)  return;
  if (!NextPic) {
    KillTimer(Hwnd, 2);
    slide_next();
    SetTimer(Hwnd, 2, 333, (TIMERPROC)timer); }
  if (!slide)  return;
  s = slide->file+FE*NextNum;
  newpic = NextPic;
  sprintf(opfile, "%s\\%s", slide->dirname+((long *)(slide->dir+
          ((short *)s)[0]*6))[0], slide->filename+((long *)(s+2))[0]);
  pwidth = ((short *)(s+6))[0];  pheight = ((short *)(s+6))[1];
  pinter = ((short *)(s+6))[2];
  memcpy(pspec, s+0xC, LENPSPEC);
  p = (short *)(s+0x24);
  preview_invalidate(LastNum, 0, 0);
  preview_invalidate(slide->tsel, 0, 0);
  LastNum = NextNum;
  if (GetForegroundWindow()!=HwndP)  slide->tsel = LastNum;
  if (slide->option&0x80) {
    View[0] = View[4] = View[1] = View[5] = 0;
    View[2] = View[6] = pwidth;  View[3] = View[7] = pheight; }
  else {
    View[0] = View[4] = p[0];  View[1] = View[5] = p[1];
    View[2] = View[6] = p[2];  View[3] = View[7] = p[3]; }
  prep_undo(Hwnd);
  MultiOpen = 0;  titleimage[0] = 2;
  open_pic_end(Hwnd);
  MultiOpen = mo; titleimage[0] = 0;
  NextPic = 0;
  LastNum = NextNum;
  if (GetForegroundWindow()!=HwndP)  slide->tsel = LastNum;
  ((ulong *)(slide->file+FE*NextNum+0x20))[0] |= 0x40000000;
  slidename[0] = 1;
  SlideLast = clock();
  recheck(Hwnd, 0);
  preview_invalidate(LastNum, 0, 0);
}

void slide_show_group(void)
/* Skip ahead to the next file whose name is substantially different than the
 *  current file.  The file name must be more than 3 characters different.
 *                                                               5/29/15-DWM */
{
  long i, num, j, same;
  char *s, *name, *name2;

  if (!slide)  return;
  if (LastNum<0 || LastNum>=slide->numfile) { slide_show();  return; }
  s = slide->file+FE*LastNum;
  name = slide->filename+((long *)(s+2))[0];
  for (i=LastNum+1; i<slide->numfile; i++) {
    s = slide->file+FE*i;
    name2 = slide->filename+((long *)(s+2))[0];
    for (j=0; j<strlen(name) && j<strlen(name2); j++) {
      if (name[j]!=name2[j]) {
        same = j;
        break; } }
    for (j=0; j<strlen(name) && j<strlen(name2); j++) {
      if (name[strlen(name)-1-j]!=name2[strlen(name2)-1-j]) {
        same += j;
        break; } }
    if (same+3>=strlen(name) && same+3>=strlen(name2))
      LastNum += 1;
    else
      break; }
  KillTimer(Hwnd, 2);
  slide_next();
  SetTimer(Hwnd, 2, 333, (TIMERPROC)timer);
  if (!slide)  return;
  slide_show();
}

void sort(void)
/* Sort the selected items in the preview by any of a variety of different
 *  metrics.                                                   3/22/97-DWM */
{
  long num, first=0, i, sel=0;

  Busy = 1;
  for (i=0; i<slide->numfile; i++)
    if (slide->file[i*FE+0x2F]&2) {
      slide->tsel = i; break; }
  num = preview_deselect(1);
  if (num<2)
    for (i=0, sel=1; i<slide->numfile; i++)
      if (((long *)(slide->file+i*FE+0x40))[0]>=0)
        slide->file[i*FE+0x2F] |= 6;
  if (!DialogBox(hinst, MAKEINTRESOURCE(SortDLG), HwndP, (DLGPROC)sort_dialog)) {
    preview_rectangles(0);
    Busy = 0;
    return; }
  preview_insert(0, 0, 3);
  if (!sel)
    for (i=0; i<slide->numfile; i++)
      if (slide->file[i*FE+0x2F]&4)
        slide->file[i*FE+0x2F] = (slide->file[i*FE+0x2F]&0xF9)|2;
  num = preview_deselect(3);
  for (i=0; i<slide->numfile; i++)
    if (slide->file[i*FE+0x2F]&2) {
      first = i;  break; }
  if (!DialogBox(hinst, MAKEINTRESOURCE(Sort2DLG), HwndP, (DLGPROC)sort2_dialog)) {
    Busy = 0;
    return; }
  qsort((char *)(slide->file+first*FE), (size_t)num, (size_t)FE, sort_compare);
  slide->tsel = first;
  preview_deselect(1);
  InvalidateRect(HwndP, 0, 1);
  if (NextPic) { free2(NextPic);  NextPic = 0;  NextNum = -1; }
  if (curfile[0])  LastNum = preview_invalidate(0, curfile, 0);
  else             LastNum = -1;
  Busy = 0;
}

int sort_compare(const char *s1, const char *s2)
/* Compare two preview images based on either a string (such as file name),
 *  or on a previously calculated metric.
 * Enter: char *s1, *s2: pointers to the file records for each image.
 * Exit:  long comp: <0 if s1<s2, 0 if s1==s2, >0 if s1>s2.    3/22/97-DWM */
{
  long val;

  if (SortDir) {
    if (SortVal<=2)
      val = stricmp((char *)(((long *)(s1+0x40))[0]),
                    (char *)(((long *)(s2+0x40))[0]));
    else
      val = (((long *)(s1+0x40))[0]>((long *)(s2+0x40))[0])-
            (((long *)(s1+0x40))[0]<((long *)(s2+0x40))[0]);
    if (val)  return(val);
    return((((long *)(s1+0x44))[0]>((long *)(s2+0x44))[0])-
           (((long *)(s1+0x44))[0]<((long *)(s2+0x44))[0]));
  }
  if (SortVal<=2)
    val = stricmp((char *)(((long *)(s2+0x40))[0]),
                  (char *)(((long *)(s1+0x40))[0]));
  else
    val = (((long *)(s2+0x40))[0]>((long *)(s1+0x40))[0])-
          (((long *)(s2+0x40))[0]<((long *)(s1+0x40))[0]);
  if (val)  return(val);
  return((((long *)(s2+0x44))[0]>((long *)(s1+0x44))[0])-
         (((long *)(s2+0x44))[0]<((long *)(s1+0x44))[0]));
}

BOOL CALLBACK sort_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Handle the controls in the sort dialog box.
 * Enter: HWND hdlg: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        6/11/96-DWM */
{
  static char *method[]={"File name","File extension","Directory name","File size (bytes)","Date of last modification","Time","Date and time","Image format","Image size (pixels)","Width","Height","Aspect ratio (width:height)",
         "DPI","Bit depth","Number of categories","Viewed flag","Compression efficiency (bits/pixel)","Intensity (greyscale brightness)","Contrast","Hue (hexcone model)","Saturation (hexcone model)","Hue weighted by saturation",
         "Value (hexcone model)","Red","Green","Blue", 0};
  long i;

  switch (msg) {
    case WM_COMMAND: switch (wp&0xFFFF) {
      case HelpHelp: WinHelp(Hwnd, HelpFile, HELP_CONTEXT, HelpSort);  break;
      case IDOK: SortVal=SendDlgItemMessage(hdlg,SortList,CB_GETCURSEL,0,0);
        SortDir = SendDlgItemMessage(hdlg, SortUp, BM_GETCHECK, 0, 0);
        EndDialog(hdlg, 1); return(1);
      case IDCANCEL: EndDialog(hdlg, 0); return(0);
      default: return(0); }
    case WM_INITDIALOG: for (i=0; method[i]; i++)
        SendDlgItemMessage(hdlg, SortList, CB_ADDSTRING, 0, (long)method[i]);
      SendDlgItemMessage(hdlg, SortList, CB_SETCURSEL, SortVal, 0);
      SendDlgItemMessage(hdlg, SortUp, BM_SETCHECK, SortDir, 0);
      SendDlgItemMessage(hdlg, SortDown, BM_SETCHECK, SortDir^1, 0);
      SetFocus(hdlg); break;
    default: return(0); }
  return(0);
}

BOOL CALLBACK sort2_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp)
/* Calculate the sort metrics for the actual sorting.
 * Enter: HWND hwnd: handle of current dialog window.
 *        long msg: message to process.
 *        WPARAM wp, LPARAM lp: parameters for message.        3/22/97-DWM */
{
  static long num, cur, left, all;
  static char *text;
  long i, j, k, val, w, h, len, H, V, N, S, bin[256], bin2[256], inc;
  char *s, *n;
  uchar *buf, *pic2, *pic;

  switch (msg) {
    case WM_INITDIALOG: if (!(text=malloc2(80))) {
        low_memory(HwndP, "Sort Processing", 0);
        EndDialog(hdlg, 0);  return(0); }
      SetTimer(hdlg, 0, 0, 0);
      SetFocus(hdlg);
      num = left = preview_deselect(3);  cur = all = 0;
      return(1);
    case WM_TIMER: for (i=0; i<100 && left && cur<slide->numfile; i++) {
        if (slide->file[cur*FE+0x2F]&2) {
          if (!i) {
            sprintf(text, "%4.2f%% left to process.", 100.*left/num, cur);
            SetDlgItemText(hdlg, SortStat, text); }
          s = slide->file+cur*FE;
          ((long *)(s+0x44))[0] = cur;
          switch (SortVal) {
            case 0: val = (long)(slide->filename+((long *)(s+2))[0]); break;
            case 1: n = slide->filename+((long *)(s+2))[0];
              if (strchr(n, '.'))  n = strrchr(n, '.');
              val = (long)(n);  break;
            case 2: val = (long)(slide->dirname+((long *)(slide->dir+
                                          ((short *)s)[0]*6))[0]); break;
            case 3: val = ((long *)(s+0x38))[0]; break;
            case 4: val = ((ushort *)(s+0x3E))[0]; break;
            case 5: val = ((ushort *)(s+0x3C))[0]; break;
            case 6: val = ((ulong *)(s+0x3C))[0]>>1; break;
            case 7: val = s[0xC]; break;
            case 8: val = ((short *)(s+6))[0]*((short *)(s+6))[1]; break;
            case 9: val = ((short *)(s+6))[0]; break;
            case 10: val = ((short *)(s+6))[1]; break;
            case 11: if (!((short *)(s+6))[1])  val = 0;
              else val=65536*((short *)(s+6))[0]/((short *)(s+6))[1]; break;
            case 12: val = ((long *)(s+0xC+4))[0]; break;
            case 13: val = 1-s[0xA]; break;
            case 14: val = 0;
              for (j=0; j<30; j++)
                if (((ulong *)(s+0x20))[0]&(1<<j))  val++; break;
            case 15: val = ((((ulong *)(s+0x20))[0]&0x40000000)!=0); break;
            case 16: j = ((short *)(s+6))[0]*((short *)(s+6))[1];
              if (j)  val = 65536.*((long *)(s+0x38))[0]*8/j;
              else    val = 0x7FFFFFFF; break;
            /** Additional scalar sort preparations go here **/
            default: break; }
          if (SortVal>=17) {
            buf = pic = 0;
            if (s[0x2C] && s[0x2D])
              buf = malloc2(((long *)(slide->thumb+
                                               ((long *)(s+0x30))[0]))[0]+8);
            if (buf) {
              unlzw(buf, slide->thumb+((long *)(s+0x30))[0]+4,
                    ((long *)(slide->thumb+((long *)(s+0x30))[0]))[0]+4,
                    ((long *)(s+0x30))[1]-4, 8);
              i += 19;
              buf = unlock2(buf);
              pic2 = bmp_to_graphic(buf);
              free2(lock2(buf));
              pic = depalettize_graphic(w=pwidth, h=pheight, pinter, pic2);
              free2(pic2); }
            val = 0;  memset(bin, 0, 256*sizeof(long));
            if (pic) {
              len = w*h*3;
              for (S=0; S<len; S+=3)
                switch (SortVal) {
                  case 17: val += (pic[S]*0.3+pic[S+1]*0.59+pic[S+2]*0.11);
                    break;
                  case 18: if (S!=len-1)
                    val += abs((pic[S]*0.3+pic[S+1]*0.59+pic[S+2]*0.11)-
                               (pic[S+3]*0.3+pic[S+4]*0.59+pic[S+5]*0.11));
                    break;
                  case 19: case 21: V = max(max(pic[S], pic[S+1]), pic[S+2]);
                    N = min(min(pic[S], pic[S+1]), pic[S+2]);
                    if (V==N)  V--;
                    if (SortVal==19 || !V)  inc = 1;
                    else                    inc = 255*(V-N)/V;
                    if (pic[S]==V && pic[S+1]>=pic[S+2])
                      bin[(long)(42.6*((float)pic[S+1]-pic[S+2])/
                          (V-N))] += inc;
                    else if (pic[S]==V)
                      bin[(long)(42.6*(6+((float)pic[S+1]-pic[S+2])/
                          (V-N)))] += inc;
                    else if (pic[S+1]==V)
                      bin[(long)(42.6*(2+((float)pic[S+2]-pic[S])/
                          (V-N)))] += inc;
                    else
                      bin[(long)(42.6*(4+((float)pic[S]-pic[S+1])/
                          (V-N)))] += inc;  break;
                  case 20: V = max(max(pic[S], pic[S+1]), pic[S+2]);
                    N = min(min(pic[S], pic[S+1]), pic[S+2]);
                    if (V)  val += 255*(V-N)/V;  break;
                  case 22: val+=max(max(pic[S], pic[S+1]), pic[S+2]); break;
                  case 23: val += pic[S]-max(pic[S+1],pic[S+2]); break;
                  case 24: val += pic[S+1]-max(pic[S],pic[S+2]); break;
                  case 25: val += pic[S+2]-max(pic[S],pic[S+1]); }
              if (len && SortVal!=19)
                val = (65536.*val/(w*h));
              if (SortVal==19 || SortVal==21) {
                memset(bin2, 0, 256*sizeof(long));
                for (j=5; j<256+5; j++)  for (k=-5; k<=5; k++)
                  bin2[j%256] += bin[(j+k)%256];
                for (j=1, val=0; j<256; j++)
                  if (bin2[j]>bin2[val])  val = j; }
              free2(pic); } }
          ((long *)(s+0x40))[0] = val;
          left--; }
        cur++; }
      if (left && cur<slide->numfile) break;
    case WM_COMMAND: if (msg==WM_COMMAND && (wp&0xFFFF)!=IDCANCEL) break;
      KillTimer(hdlg, 0);
      free2(text);
      EndDialog(hdlg, (msg!=WM_COMMAND || (wp&0xFFFF)!=IDCANCEL)); }
  return(0);
}

void start_ctl3d(void)
/* Start up the faux-3D control functions, if available.       10/2/96-DWM */
{
  FARPROC reg, sub;
  char name[]="CTL3D32.DLL";

  if (!windows_file(name))  return;
  if (!(hctl3d=LoadLibrary(name)))
    return;
  reg = GetProcAddress(hctl3d, "Ctl3dRegister");
  sub = GetProcAddress(hctl3d, "Ctl3dAutoSubclass");
  if ((*reg)(hinst))
    (*sub)(hinst);
}

void strpath(char *dest, char *src)
/* If the source is different from the destination, copy the source to the
 *  destination.  Then, remove the trailing filename from the destination.
 *  This works properly for the c:\ type condition.
 * Enter: char *dest: location to store result.
 *        char *src: location of initial string.  Can be the same as dest.
 *                                                            11/15/96-DWM */
{
  if (src!=dest)
    strcpy(dest, src);
  remove_quotes(dest);
  if (strchr(dest, '\\'))
    strrchr(dest, '\\')[0] = 0;
  if (dest[strlen(dest)-1]==':')
    strcat(dest, "\\");
}

void strpath2(char *dest, char *src)
/* If the source is different from the destination, copy the source to the
 *  destination.  Then, remove the trailing filename from the destination.
 *  This works properly for the c:\ type condition.  Spaces are not removed.
 * Enter: char *dest: location to store result.
 *        char *src: location of initial string.  Can be the same as dest.
 *                                                            11/15/96-DWM */
{
  if (src!=dest)
    strcpy(dest, src);
  remove_quotes2(dest);
  if (strchr(dest, '\\'))
    strrchr(dest, '\\')[0] = 0;
  if (dest[strlen(dest)-1]==':')
    strcat(dest, "\\");
}

void test(char *data)
/* Standard test message entry point.  This routine allows diagnostic
 *  messages to be printed by non-Windows source routines.
 * Enter: char *data: string to print in a dialog.             10/1/96-DWM */
{
  FILE *fptr;

/**/  fptr = fopen("C:\\TEMP\\TEST.TXT", "at");
  fprintf(fptr, "%s\n", data);
  fclose(fptr);
  return; /**/
/**   SetWindowText(Hwnd, data); /**/
/**   debug(data); /**/
}

VOID CALLBACK timer(HWND hwnd, ulong msg, ulong id, long time)
/* Handle a system wide timer.  This is primarily for the slide show
 *  functions.
 * Enter: HWND hwnd: handle of calling window.
 *        ulong msg: ignored.
 *        ulong id: id of calling timer.
 *        long time: time in some windows format.             11/10/96-DWM */
{
  long t;

  if (!slide || TimerOn || Down || Busy)  return;
  t = clock();
  if (((slide->option&0x20) || t-SlideLast<0.5*CLOCKS_PER_SEC) &&
      t-SlideLast<slide->delay*CLOCKS_PER_SEC)
    return;
  if (GetForegroundWindow()==HwndP)
    return;
  KillTimer(Hwnd, 2);
  if (!NextPic && !(slide->option&0x20))  slide_next();
  if (!slide) {
    SetTimer(Hwnd, 2, 333, (TIMERPROC)timer); return; }
  if (t-SlideLast<slide->delay*CLOCKS_PER_SEC || !(slide->option&8)) {
    SetTimer(Hwnd, 2, 333, (TIMERPROC)timer); return; }
  slide_show();
  SetTimer(Hwnd, 2, 333, (TIMERPROC)timer);
}

void topmost(void)
/* Toggle the window from being a topmost window to a normal window.
 *                                                              6/8/96-DWM */
{
  RECT rect;

  Topmost ^= 1;  recheck(Hwnd, 0);
  GetWindowRect(Hwnd, &rect);
  if (Topmost)
    SetWindowPos(Hwnd, HWND_TOPMOST, rect.left, rect.top,
                 rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW);
  else
    SetWindowPos(Hwnd, HWND_NOTOPMOST, rect.left, rect.top,
                 rect.right-rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW);
}

void undo(HWND hwnd)
/* Undo the last zoom, resize, or scroll command.
 * Enter: HWND hwnd: pointer to window.                        5/30/96-DWM */
{
  long x, y, w, h;
  RECT rect;

  GetWindowRect(hwnd, &rect);
  x = rect.left;  w = rect.right-x;
  y = rect.top;   h = rect.bottom-y;
  prep_undo(hwnd);
  memcpy(View, Undo+UNDOLEN+4, 8*sizeof(long));
  MoveWindow(hwnd, Undo[UNDOLEN], Undo[UNDOLEN+1], Undo[UNDOLEN+2],
             Undo[UNDOLEN+3], 1);
  rescale |= 2;  InvalidateRect(Hwnd, 0, 0);
}

APIENTRY WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR argv, int winmode)
/* Enter: HINSTANCE inst: number identifying this program.
 *        HINSTANCE prev: 0.
 *        LPSTR argv: single string containing command line parameters.
 *        int winmode: how window is displayed at start.        5/4/96-DWM */
{
  HWND hwnd;
  MSG msg;
  WNDCLASS wcl;
  HACCEL acc;
  HDC hdc;
  RECT rect;
  long x, y;

  opendir[0] = savedir[0] = opendir[256] = savedir[256] = opendir[512] =
              savedir[512] = opendir[768] = savedir[768] = opendir[1024] =
              savedir[1024] = savedir[1280] = 0;
  RegName[0] = RegPass[0] = 0;
  memset(lastview, 0, 1024);
  memset(oldspec, 0, sizeof(LENPSPEC));
  titleimage[0] = titleimage[1] = 0;
  WinPlace.length = WinPlace2.length = 0;
  read_ini();
  hinst = wcl.hInstance = inst;
  wcl.lpszClassName = WinName;
  wcl.lpfnWndProc = (WNDPROC)main_loop;
  wcl.style = 0;
  wcl.hIcon = LoadIcon(inst, MAKEINTRESOURCE(ICON1));
  wcl.hCursor = LoadCursor(0, IDC_ARROW);
  wcl.lpszMenuName = "MainMenu";
  wcl.cbClsExtra = wcl.cbWndExtra = 0;
  wcl.hbrBackground = GetStockObject(BLACK_BRUSH);
  if (!RegisterClass(&wcl))  return(0);
  wcl.lpszClassName = WinName2;
  wcl.lpfnWndProc = (WNDPROC)preview_loop;
  wcl.style = CS_DBLCLKS;
  wcl.lpszMenuName = "PrevMenu";
  wcl.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  if (!RegisterClass(&wcl))  return(0);
  extrax = GetSystemMetrics(SM_CXFRAME)*2;
  extray = GetSystemMetrics(SM_CYFRAME)*2+GetSystemMetrics(SM_CYMENU)+
           GetSystemMetrics(SM_CYCAPTION);
  pwidth = 200;  pheight = 10;  pinter = 1;
  oldw = pwidth;  oldh = pheight;  oldi = pinter;
  opfile = malloc2(NAMELEN);
  pic = malloc2(oldw*oldh+768);
  if (!pic || !opfile)  exit(0);
  curfile[0] = 0;
  memset(pic, 0, oldw*oldh+768);
  set_size();
  WindowW = OrigW;  WindowH = OrigH;  x = OrigW+extrax;  y = OrigH+extray;
  if ((Lock&1) && Undo[2]>0) {
    OrigX = Undo[0];  OrigY = Undo[1];  x = Undo[2];  y = Undo[3]; }
  hwnd = CreateWindow(WinName, "Graphic Viewer", WS_OVERLAPPEDWINDOW, OrigX,
                      OrigY, x, y, HWND_DESKTOP, 0, inst, 0);
  Hwnd = hwnd;  Hmenu = GetMenu(hwnd);
  if (WinPlace.length && ((Lock&1)||(WinPlace.showCmd==SW_SHOWMAXIMIZED))) {
    SetWindowPlacement(hwnd, &WinPlace);
    winmode = WinPlace.showCmd; }
  if (!(Lock&1) || Undo[2]<=0)
    set_client(hwnd, OrigX, OrigY, x-extrax, y-extray, 1);
  DragAcceptFiles(Hwnd, 1);
  hdc = GetDC(hwnd);
  BitsPixel = GetDeviceCaps(hdc, BITSPIXEL);
  ReleaseDC(hwnd, hdc);
  acc = LoadAccelerators(inst, "MainMenu");
  start_ctl3d();
  SetTimer(Hwnd, 2, 333, (TIMERPROC)timer);
  srand((time(0)<<16)+(time(0)>>16));
  memset(oldspec, 0, LENPSPEC);
  if (argv) if (strlen(argv))
    open_pic_mid(hwnd, argv, 0);
  else if (titleimage[1]) {
    titleimage[0] = 1;
    open_pic_mid(hwnd, titleimage+1, 0);
    titleimage[0] = 0; }
  if (!Topmost)
    ShowWindow(hwnd, winmode);
  else {
    GetWindowRect(Hwnd, &rect);
    if (winmode==SW_SHOWDEFAULT)
      ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    else
      ShowWindow(hwnd, winmode|x);
    SetWindowPos(Hwnd, HWND_TOPMOST, rect.left, rect.top, rect.right-
           rect.left, rect.bottom-rect.top, SWP_SHOWWINDOW|SWP_NOACTIVATE); }
  if (argv) if (strlen(argv)) {
    while (argv[0]<=' ' && argv[0])  argv++;
    if (find_space(argv))  argv = find_space(argv);
    else                   argv = argv+strlen(argv);
    while (argv[0]<=' ' && argv[0])  argv++;
    if (argv[0])  open_pic_new(hwnd, argv); }
  recheck(hwnd, 0);
  UpdateWindow(hwnd);
  InvalidateRect(hwnd, 0, 0);
  prep_undo(hwnd);
  EnableMenuItem(Hmenu, MenuUndo, MFS_GRAYED);
  while (GetMessage(&msg, 0, 0, 0)) {
    hwnd = Hwnd;
    if (GetForegroundWindow()==HwndP)  hwnd = HwndP;
    if (!TranslateAccelerator(hwnd, acc, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg); } }
  WinHelp(Hwnd, HelpFile, HELP_QUIT, 0);
  KillTimer(Hwnd, 2);
  DragAcceptFiles(Hwnd, 0);
  if (pic) { free2(pic);  pic = 0; }
  free2(opfile);
  if (wpic) { GlobalFree(wpic);  wpic = 0; }
  if (wpal) { DeleteObject(wpal);  wpal = 0; }
  set_font(0, 0, 0, 0, 0);
  end_ctl3d();
  return(msg.wParam);
}

long windows_file(char *name)
/* Determine if a specified file is present in the WINDOWS/SYSTEM or
 *  SYSTEM32 path.  This is usually used with DLL files.
 * Enter: char *name: name of file to search for.  Wildcards are allowed.
 * Exit:  long present: 0 for non found, 1 for present.        10/2/96-DWM */
{
  char path[256], *cmd;
  FILE *fptr;
  WIN32_FIND_DATA find;
  HANDLE temp;
  long i;

  path[0] = 0;
  for (i=0; i<3; i++) {
    switch (i) {
      case 0: cmd = GetCommandLine();
        if (strlen(cmd)<256)  strcpy(path, cmd);
        if (strchr(path, '\\'))  strrchr(path, '\\')[1] = 0; break;
      case 1: GetSystemDirectory(path, 255);
        if (path[strlen(path)-1]!='\\')  strcat(path, "\\"); break;
      case 2: GetWindowsDirectory(path, 255);
        if (path[strlen(path)-1]!='\\')  strcat(path, "\\"); }
    if (strlen(path)+strlen(name)<255)  strcat(path, name);
    if ((temp=FindFirstFile(path, &find))!=INVALID_HANDLE_VALUE) {
      FindClose(temp);
      return(1); } }
  return(0);
}

void write_ini(long offset)
/* Write values to the INI file.
 * Enter: long offset: 1 to stagger window starting position.  5/26/96-DWM */
{
  long i, j, x=OrigX, y=OrigY;
  char *text, text2[32], t2[256];
  RECT rect;

  if (!(Lock&1)) { if (x<0)  x = 0;  if (y<0)  y = 0; }
  if (offset && !(Lock&1)) {
    x += 24;  if (x>400) x = 0;
    y += 24;  if (y>300) y = 0; }
  GetWindowRect(Hwnd, &rect);
  if (Lock&1) {
    x = rect.left;  y = rect.top; }
  for (i=0; key[i*3]; i++) {
    text = key[i*3+1];
    switch (i) {
      case 0: text = t2;  sprintf(t2, "%d", Redox); break;
      case 1: if (Msg&1)  text = key[i*3+2]; break;
      case 2: if (!(Msg&2))  text = key[i*3+2]; break;
      case 3: text = opendir; break;
      case 4: text = t2;  sprintf(t2, "%d,%d,%d,%d", x, y, rect.right-
                                  rect.left, rect.bottom-rect.top); break;
      case 5: text = t2;  sprintf(t2, "%d", FilterType); break;
      case 6: if (Reduce) text = key[i*3+2]; break;
      case 7: if (Dither) text = key[i*3+2]; break;
      case 8: text = t2;  sprintf(t2, "%d", Letter); break;
      case 9: text = t2;  sprintf(t2, "%d", Lock); break;
      case 10:text = t2;  sprintf(t2, "%g,%g,%g,%g", AspectX, AspectY,
                                  AspectXY[0], AspectXY[1]); break;
      case 11: if (Topmost)  text = key[i*3+2]; break;
      case 12: text = t2;  sprintf(t2, "%g,%g,%g,%g,%g,%d", PrintMarg[0],
                      PrintMarg[1], PrintMarg[2], PrintMarg[3], PrintMarg[4],
                      PrintSet); break;
      case 13: if (Msg&4)  text = key[i*3+2]; break;
      case 14: text = t2;  sprintf(t2, "%d", PreviewOpt); break;
      case 15: text = savedir; break;
      case 16: text = t2;  sprintf(t2, "%d,%d", SaveFilt, SaveOpt); break;
      case 17: if (MultiOpen)  text = key[i*3+2]; break;
      case 18: text = opendir+256; break;
      case 19: text = savedir+256; break;
      case 20: text = opendir+512; break;
      case 21: text = savedir+512; break;
      case 22: text = titleimage+1; break;
      case 23: text = t2;  t2[0] = 0;
        GetWindowPlacement(Hwnd, &WinPlace);
        for (j=0; j<sizeof(WINDOWPLACEMENT); j++)
          sprintf(t2+strlen(t2), "%02X", ((uchar *)(&WinPlace.length))[j]);
        break;
      case 24: text = opendir+768; break;
      case 25: text = savedir+768; break;
      case 26: text = t2;  sprintf(t2, "%d", SlideOpt); break;
      case 27: text = t2;  sprintf(t2, "%d", SlideDelay); break;
      case 28: text = t2;  t2[0] = 0;
        for (j=0; j<sizeof(WINDOWPLACEMENT); j++)
          sprintf(t2+strlen(t2), "%02X", ((uchar *)(&WinPlace2.length))[j]);
        break;
      case 29: text = t2;  sprintf(t2, "%d", SlideTx); break;
      case 30: text = t2;  sprintf(t2, "%d", SlideTy); break;
      case 31: text = t2;  sprintf(t2, "%d,%d,%d", SortVal, SortDir,
                                   FindType); break;
      case 32: text = opendir+1024; break;
      case 33: text = lastview;     break;
      case 34: text = lastview+256; break;
      case 35: text = lastview+512; break;
      case 36: text = lastview+768; break;
      case 37: text = savedir+1024; break;
      case 38: text = t2;  sprintf(t2, "%d,%d", PrevSaveX, PrevSaveY); break;
      case 39: continue; // regnum
      case 40: text = RegName; break;
      case 41: text = RegPass; break;
      case 42: if (Interpolate==1) text = key[i*3+2];
        if (Interpolate>1) {  text = t2;  sprintf(t2, "0x%X", Interpolate); }
        break;
      case 43: if (HalfSizeBatch) text = key[i*3+2]; break;
      case 44: text = savedir+5*256; break;
      case 45: if (MultiSave)  text = key[i*3+2]; break;
      default: continue; }
    if (i!=4 || !IsIconic(Hwnd))
      WritePrivateProfileString(inihead, key[i*3], text, inifile); }
  for (i=0; i<SAVEFORMATS; i++) {
    sprintf(text2, "SaveOption%d", i);
    for (j=0; j<SAVELEN; j++)
      sprintf(t2+j*2, "%02X", SaveRec[SAVELEN*i+j]);
    WritePrivateProfileString(inihead, text2, t2, inifile); }
  for (i=0; i<4; i++) {
    sprintf(text2, "CustomQuant%d", i);
    for (j=0; j<32; j++)
      sprintf(t2+j*2, "%02X", gquant[i*32+j]);
    WritePrivateProfileString(inihead, text2, t2, inifile); }
}

void zoom(float mag, long style)
/* Zoom the current window in the specified manner.
 * Enter: float *mag: magnitude of zoom.
 *        long style: 0-relative magnitude, 1-absolute magnitude, 2-step
 *                    magnitude.                                6/3/96-DWM */
{
  float zy=1, zx=1;
  RECT rect, r2;
  long i, w, h, mx, my, cx, cy, sx, sy, del=0;

  if (IsIconic(Hwnd))  return;
  sx = mx = GetSystemMetrics(SM_CXFULLSCREEN)-extrax;
  sy = my = GetSystemMetrics(SM_CYFULLSCREEN)-extrax-
            GetSystemMetrics(SM_CYMENU);
  GetClientRect(Hwnd, &rect);
  GetWindowRect(Hwnd, &r2);
  if (style==2 && !mag)  del = 1;
  if (!(View[2]-View[0]+del))  View[2]++;
  if (!(View[3]-View[1]+del))  View[3]++;
  if (rect.right)  zx = (float)rect.right/ (View[2]-View[0]+del);
  if (rect.bottom) zy = (float)rect.bottom/(View[3]-View[1]+del);
  if (zy<zx)  zx = zy;
  if (zx>1 && (Lock&6) && mag!=1)  zx = 1;
  switch (style+(style==2&&mag)) {
    case 0: zy = zx*mag; break;
    case 1: zy = mag; break;
    case 2: for (i=-100; i<100; i++) {
      if (!i)  continue;
      zy = 1./i;  if (zy<0)  zy = -i;
      if (zy<=zx) break; } break;
    case 3: for (i=-100; i<100; i++) {
      if (!i)  continue;
      zy = i;  if (zy<0)  zy = -1./i;
      if (zy>=zx+0.02)  break; } }
  if (((Lock&2) && zy>1) || (Lock&4))  zy = 1;
  w = (View[6]-View[4])*zy;
  h = (View[7]-View[5])*zy;
  if (w<MINWIDTH) w = MINWIDTH;
  if (h<2)        h = 2;
  if (Lock&9) {
    mx = rect.right;  my = rect.bottom; }
  memcpy(View, View+4, 4*sizeof(long));
  cx = (View[0]+View[2])/2;  cy = (View[1]+View[3])/2;
  if (Focus[0])  cx = Focus[0];  if (Focus[1])  cy = Focus[1];
  if (!zy)  zy = 1;
  if (w>mx) {
    View[0] = cx-0.5*mx/zy;
    if (View[0]<0)  View[0] = 0;
    if (View[0]+(float)mx/zy>oldw)  View[0] = oldw-(float)mx/zy;
    if (View[0]<View[4])  View[0] = View[4];
    View[2] = View[0]+(float)mx/zy;
    w = mx; }
  if (h>my) {
    View[1] = cy-0.5*my/zy;
    if (View[1]<0)  View[1] = 0;
    if (View[1]+(float)my/zy>oldh)  View[1] = oldh-(float)my/zy;
    if (View[1]<View[5])  View[1] = View[5];
    View[3] = View[1]+(float)my/zy;
    h = my; }
  mx = r2.left;  my = r2.top;
  if (mx+w>sx)  mx = sx-w;  if (mx<0)  mx = 0;
  if (my+h>sy)  my = sy-h;  if (my<0)  my = 0;
  if (!(Lock&9))
    set_client(Hwnd, mx, my, w, h, 1);
  rescale |= 2;  InvalidateRect(Hwnd, 0, 0);
  Focus[0] = Focus[1] = -1;
}
