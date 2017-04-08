#define ulong unsigned long
#define uchar unsigned char

#define FE           72
#define MAXPREVIEW 2000
#define MINWIDTH    (GetSystemMetrics(SM_CXMIN)-extrax)
#define NAMELEN    4096
#define SAVEFORMATS  10
#define SAVELEN      18
#define SUBVERSION   14
#define UNDOLEN      12
#define VERSION       1

typedef struct {
  long lencatname, lendirname, lenfilename, lenthumb, numdir, numfile;
  char *cat, *catname, *dir, *dirname, *file, *filename, *root;
  uchar *thumb;
  long delay, filter, tx, ty, ttop, tsel;
  ulong option; } SlideList;

BOOL CALLBACK about            (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK batch_dialog     (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK find_dialog      (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK grey_dialog      (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK null_dialog      (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK open_dialog      (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK open_slide_dialog(HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK preview_save_dialog(HWND hdlg,ulong msg,WPARAM wp, LPARAM lp);
BOOL CALLBACK register_dialog  (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK save_dialog      (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK saveopt_dialog   (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK slide_dialog     (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK sort_dialog      (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK sort2_dialog     (HWND hdlg, ulong msg, WPARAM wp, LPARAM lp);
VOID CALLBACK preview_move_wait(HWND hwnd, ulong msg, ulong id, long time);
VOID CALLBACK preview_pic      (HWND hwnd, ulong msg, ulong id, long time);
VOID CALLBACK preview_timer    (HWND hwnd, ulong msg, ulong id, long time);
VOID CALLBACK scroll_wait      (HWND hwnd, ulong msg, ulong id, long time);
VOID CALLBACK timer            (HWND hwnd, ulong msg, ulong id, long time);
LRESULT CALLBACK main_loop     (HWND hwnd, ulong msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK preview_loop  (HWND hwnd, ulong msg, WPARAM wp, LPARAM lp);
void    create_savepic_area(long w, long h);
void    create_savepic_cross(HWND hdlg, HBITMAP wpic);
void    create_savepic_grob(HWND hdlg, HDC hdc);
void    create_savepic_rect(long num, RECT *rect, HWND hdlg);
void    cursor         (long wait);
void    draw_bmp       (HDC hdc, long x, long y, long w, long h, long pal,
                        HANDLE bmp);
void    draw_text      (HDC dc, char *text, long x, long y, long w, long h);
long    find_file      (char *name, long type, long recurse,
                        WIN32_FIND_DATA *find, char *path);
char   *find_space     (char *text);
void    low_memory     (HWND hwnd, char *text, long end);
void    map_dialog_rect(HWND hdlg, long x1, long y1, long x2, long y2,
                        long *out);
HANDLE  metafile_to_bmp(HANDLE gmem);
long    MsgBox         (HWND hwnd, char *text, char *title, ulong style);
void    multi_save     (char *name, uchar *spec);
void    no_letter      (HWND hwnd);
long    open_pic       (HWND hwnd);
void    open_pic_mid   (HWND hwnd, char *name, long preremove);
void    open_pic_new   (HWND hwnd, char *name);
void    open_pic_end   (HWND hwnd);
void    paste          (void);
void    prep_pic       (HWND hwnd);
HBITMAP prep_pic2      (long *rect, long destw, long desth, long noenlarge,
                        long trans, long *pal, HPALETTE *bpal);
void    prep_undo      (HWND hwnd);
void    preview        (void);
void    preview_compute(void);
long    preview_deselect(long hold);
long    preview_invalidate(long num, char *name, long part);
void    preview_invalidate_end(long r);
long    preview_page   (HDC hdc, long ww, long wh, long page, long full);
void    preview_palette(void);
void    preview_palette_set(void);
void    preview_purge  (void);
void    preview_rectangles(HDC pdc);
void    preview_save   (void);
void    preview_show   (void);
void    preview_update (HWND hwnd);
void    print_getitem  (HWND hdlg, long item, float *value, long type);
void    print_setitem  (HWND hdlg, long item, float value, long type);
void    printer        (HWND hwnd, long setup);
void    quit           (void);
void    recheck        (HWND hwnd, long refresh);
void    rectangle_dotted(HDC dc, long x1, long y1, long x2, long y2, long dot);
void    remove_quotes  (char *text);
void    remove_quotes2 (char *text);
void    rotate         (long cw);
long    save_as        (long saveas);
long    saveopt_left   (HWND hdlg, long x, long y);
long    saveopt_right(HWND hdlg, long x, long y, long move);
void    set_client     (HWND hwnd, long x, long y, long w, long h, long paint);
void    set_font       (HDC dc, long type, long size, long fore, long back);
void    set_letter_color(void);
void    set_name       (HWND hwnd);
void    set_size       (void);
void    slide_cat_list (HWND hdlg, long action);
void    slide_clear    (long view);
void    slide_command  (long com);
void    slide_delete   (long num, long part);
void    slide_fill     (SlideList *s, HWND hwnd);
long    slide_free     (long save);
void    slide_new      (char *dir);
void    slide_next     (void);
void    slide_next_nonmulti(void);
void    slide_open     (HWND hwnd);
void    slide_open_mid (void);
void    slide_refresh  (long remove, HWND hwnd);
long    slide_root     (HWND hwnd);
long    slide_save     (HWND hwnd, long saveas);
void    slide_show     (void);
void    slide_show_group(void);
void    sort           (void);
int     sort_compare   (const char *s1, const char *s2);
void    start_ctl3d    (void);
void    strpath        (char *dest, char *src);
void    strpath2       (char *dest, char *src);
void    test           (char *data);
void    topmost        (void);
void    undo           (HWND hwnd);
long    windows_file   (char *name);
void    write_ini      (long offset);
void    zoom           (float mag, long style);

#define debug(x) MessageBox(Hwnd, (x), "--", MB_OK)

extern FARPROC pDSM_Entry;
extern HBITMAP wpic;
extern HANDLE devmode;
extern HINSTANCE hctl3d, hinst;
extern HMENU Hmenu;
extern HPALETTE prevpal, wpal;
extern HWND Hwnd, HwndP;
extern OPENFILENAME openfile;
extern PRINTDLG printdlg;
extern WINDOWPLACEMENT WinPlace, WinPlace2;
extern char *BatchTitle[], curfile[], *ErrorMsg[], *ErrorMsg2[], *extlist[],
       *extlist2[], HelpFile[], inifile[], inihead[], *key[], lastview[],
       No[], oldspec[], opendir[], OpenFilter[], *OpenTitle[], *opfile,
       opfilter[], opchar, RegName[], RegPass[], *SaveComp[],
       savedir[], SaveFilter[], *SaveTitle[], slidecheck[], SlideFilter[],
       slidename[], titleimage[], WinName[], WinName2[], Yes[];
extern float AspectX, AspectY, AspectXY[], PreviewGamma, PrintMarg[];
extern long appendtbl[], BackUpdate, BitsPixel, bmpi, Busy, CopyMode,
       curheight, curwidth, dither16num, Down, DownX, DownY, EditCat, extrax,
       extray, FilterType, FindType, Focus[], HalfSizeBatch, LastNum, Letter,
       LetterColor[], Lock, Msg, MultiOpen, MultiSave, NewCopy, NextNum,
       ODown[], oldh, oldi, oldw, OrderForm[], OrigH, OrigW, OrigX, OrigY,
       PalChange, PMode, PreCoor[], PreCoorMax, PrepTrans, PreviewArea[],
       PreviewCoor[], PreviewInv, PreviewOpt, PreviewTrans, PreviewXY[],
       PrevSaveX, PrevSaveY, PrintSet, Redox, Reduce,
       Rename, rescale, SaveAs, SaveDialog, SaveFilt, SaveOpt,
       Scale, ScrollOff, ScrollWait, Shade, SlideDelay, SlideLast, SlideTx,
       SlideTy, SortDir, SortVal, TimerOn, Topmost, Undo[], View[], WindowW,
       WindowH;
extern SlideList *slide;
extern uchar dither16tbl[], gquant[], *newpic, *NextPic, oldquant[], *pic,
       *PreviewPal, *PreviewPic, SaveDefault[], SaveRec[];
extern ulong SlideOpt;
