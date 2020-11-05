
#include /opt/pjproject/config.mk
PJBASE= /home/share/pjproject-1.6
include $(PJBASE)/build.mak

sources:=$(wildcard *.cpp)
objects:=$(sources:.cpp=.o)
depends:=$(sources:.cpp=.d)

PWD := $(shell pwd)

#CC      = $(PJ_CC)
#CC      = $(CC_NAME)
#CPPFLAGS= $(CFLAGS)
CXX = arm_v5t_le-g++
LIB_PATH = $(TargetDir)/lib

LDLIBS += $(PJ_LDLIBS)

#CFLAGS = -O2 -g -Wall 
CFLAGS += $(PJ_CFLAGS)
#CFLAGS += -I/opt/crosstool/arm-linux/gcc-3.4.4-glibc-2.3.5/arm-linux/arm-linux/include
#CFLAGS += -I/opt/crosstool/arm-linux/gcc-3.4.4-glibc-2.3.5/arm-linux/lib/gcc/arm-linux/3.4.4/include/

#LDFLAGS = -L/opt/crosstool/arm-linux/gcc-3.4.4-glibc-2.3.5/arm-linux/arm-linux/lib
LDFLAGS += -L$(LIB_PATH)
LDFLAGS += $(PJ_LDFLAGS)

CPPFLAGS = $(CFLAGS)

.PHONY:all
.PHONY:depend
all: sip_app

sip_app: $(depends) $(objects) Sip.h IniFile.h Socket.h
	$(CXX) $(objects) $(LDFLAGS) $(LDLIBS) -o sip_app

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(CPPFLAGS) $< > $@;

-include $(depends)

.PHONY:clean
clean:
	rm -rf *.o *.d sip_app



#test: main.cpp hw_lib.h
#	$(CXX) -o $@ $< $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)
#	$(CXX) -o $@ $^ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)
#	$(CC) -o $@ $^ $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)

#clean:
#	rm -f *.o test
