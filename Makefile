LEVEL=../..

LIBRARYNAME=llvm-array-check-pass

LOADABLE_MODULE=1

#files to be compiled
SOURCES:= ArrayBoundsCheckPass.cpp RunTimeBoundsChecking.cpp

include $(LEVEL)/Makefile.common
