CL_OPTS=/O2 /GL /nologo 
CLLIB_OPTS=/O2 /nologo
LINK_OPTS=/LTCG /nologo
LIB_OPTS=/nologo
RC_OPTS=/n /nologo

all: gv.exe gvlib.lib


gv.exe: gload.obj gsave.obj gv.obj gvlib.obj mem.obj process.obj gv.res
	link $(LINK_OPTS) /OUT:gv.exe gload.obj gsave.obj gv.obj gvlib.obj mem.obj process.obj gv.res advapi32.lib comdlg32.lib gdi32.lib kernel32.lib shell32.lib user32.lib 

gv.res: gv.rc gvrc.h 
	rc $(RC_OPTS) gv.rc

gload.obj: gload.c gload.h mem.h process.h 
	cl /c $(CL_OPTS) gload.c
gsave.obj: gsave.c gload.h mem.h 
	cl /c $(CL_OPTS) gsave.c
gv.obj: gv.c gv.h gload.h gvlib.h gvrc.h 
	cl /c $(CL_OPTS) gv.c
gvlib.obj: gvlib.c gvlib.h gload.h mem.h
	cl /c $(CL_OPTS) gvlib.c
mem.obj: mem.c mem.h
	cl /c $(CL_OPTS) mem.c
process.obj: process.c process.h mem.h
	cl /c $(CL_OPTS) process.c


gvlib.lib: lib.obj
	lib $(LIB_OPTS) /out:gvlib.lib lib.obj
	IF EXIST "C:\P\LIB" copy gvlib.lib C:\P\LIB\gvlib.lib 
	IF EXIST "C:\P\LIB" copy gvlib.h C:\P\LIB\gvlib.h
	
lib.obj: lib.c gload.c gsave.c gvlib.c process.c gload.h gvlib.h process.h
	cl /c $(CLIB_OPTS) lib.c


clean: 
	rm *.obj
	rm *.res
