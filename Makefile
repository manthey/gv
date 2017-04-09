CL_OPTS=/O2 /GL /nologo 
LINK_OPTS=
RC_OPTS=/n /nologo

all: gv.exe

gv.exe: gload.obj gsave.obj gv.obj gvlib.obj mem.obj process.obj gv.res
	cl $(CL_OPTS) $(LINK_OPTS) gload.obj gsave.obj gv.obj gvlib.obj mem.obj process.obj gv.res /link /OUT:gv.exe advapi32.lib comdlg32.lib gdi32.lib kernel32.lib shell32.lib user32.lib 

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

clean: 
	rm *.obj
	rm *.res
