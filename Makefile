COMPILER = gcc 
CFLAGS   = -g -c -Wall -pedantic
LIBS     = -lpthread -lm -lmhash -lcommon -L. 
OUTFLAGS = -o
LIBRARIAN = ar
LIBOPTS  = -cvq
PUTS     = echo
APPOBJS  = linkl.o transportl.o netl.o nfdapp.o 
DELETE   = rm
common_lib:
	$(COMPILER) $(CFLAGS)  cfparser.c $(OUTFLAGS)  cfparser.o
	$(COMPILER) $(CFLAGS)  statehc.c  $(OUTFLAGS)  sstatehc.o
	$(COMPILER) $(CFLAGS)  packeth.c  $(OUTFLAGS)  packth.o
	$(PUTS) "Building common libraries  .... " 	
	$(LIBRARIAN) $(LIBOPTS) libcommon.a cfparser.o sstatehc.o packth.o
	$(PUTS) "Building the rest .... "
	$(COMPILER) $(CFLAGS)  linkl.c $(OUTFLAGS) linkl.o
	$(COMPILER) $(CFLAGS)  trasportl.c $(OUTFLAGS) transportl.o
	$(COMPILER) $(CFLAGS)  netl.c $(OUTFLAGS) netl.o 
	$(COMPILER) $(CFLAGS)  nfdapp.c $(OUTFLAGS) nfdapp.o 
	$(COMPILER)  $(APPOBJS) $(OUTFLAGS) nfdapp $(LIBS)

clean:
	$(DELETE) *.o
	$(DELETE) libcommon.a
