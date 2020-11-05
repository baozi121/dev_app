#ifndef _HTTPGET_H_
#define _HTTPGET_H_
#include <stdio.h>
//#include </opt/curl7.71/include/curl/curl.h>
#include "curl.h"
#include <stdlib.h>
#include "easy.h"
#include <string.h>
#include <iostream>
#include <string>
using namespace std;
struct MemoryStruct
{
    char *memory;
    size_t size;
    MemoryStruct()
    {
        memory = (char *)malloc(1);
        size = 0;
    }
    ~MemoryStruct()
    {
        free(memory);
        memory = NULL;
        size = 0;
    }
};
	
int httpget_uploadlogtxt( char *webresult,char *logtxt);

int httpget_GET_noreturn(char *url);


//string httpget_GetUpgrade(const char *version);
string httpget_GetUpgrade(const char *version,int action,const char *devid);
int httpget_GetNewVersion(char *PlanRecv);
int HttpDownloadfile(char *filename,char *devid);
int httpget_GetTime(char *PlanRecv);
int post(char * param);

#endif
