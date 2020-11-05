DIR=/opt/jthread_jrtplib
LIB_PATH =$(DIR)/lib
#LIB=-lcurl -lrt -lcrypto -lssl 
LIB= -lcurl
PWD=/mnt/hgfs/linux_share/labbox/dev_app
#CFLAGS +=-static
CFLAGS += $(PJ_CFLAGS)
CFLAGS += -I/home/baozi/montavista/pro/devkit/arm/v5t_le/include
CFLAGS += -I/home/baozi/montavista/pro/devkit/arm/v5t_le/lib/gcc/armv5tl-montavista-linux-gnueabi/4.2.0/include

CFLAGS += -I$(DIR)/include/PnCppLib
CFLAGS += -I$(DIR)/include/jthread
CFLAGS += -I$(DIR)/include/jrtplib3
CFLAGS += -I$(DIR)/include/tinyxml
CFLAGS += -I$(DIR)/include/curl


#CFLAGS += -I$(DIR)/include/dfdulib

CFLAGS += -I$(PWD)
CFLAGS += -I./Client_socket
CFLAGS += -I./Media
CFLAGS += -I./Work_Order
CFLAGS += -I./FTP
CFLAGS += -I./Ini
CFLAGS += -I./Msq
CFLAGS += -I./Manage
CFLAGS += -I./RTC
CFLAGS += -I./Sysinfo
CFLAGS += -I./Xml

sources+=$(PWD)/main.cpp
sources+=$(PWD)/comm.h
sources+=$(PWD)/Client_socket/*.cpp
sources+=$(PWD)/Ini/*.cpp
sources+=$(PWD)/Msq/*.cpp
sources+=$(PWD)/Manage/*.cpp
sources+=$(PWD)/Media/*.cpp
sources+=$(PWD)/Work_Order/*.cpp
sources+=$(PWD)/FTP/*.cpp
sources+=$(PWD)/RTC/*.cpp
sources+=$(PWD)/Sysinfo/*.cpp
sources+=$(PWD)/Xml/*.cpp
sources+=$(PWD)/httpget.cpp



CC=arm_v5t_le-g++

dev_app_pn:$(sources)
	$(CC) -DDEBUG -Wall -O2 -o $@ $(CFLAGS) $^ -L$(LIB_PATH)  $(LIB) -ljrtp -lpthread -lpncpp -ltinyxml -ljthread 

clean:
	rm -rf dev_app_* *.o
install:
	cp dev_app_pn /home/baozi/filesys_test/opt/baozi/ -f
