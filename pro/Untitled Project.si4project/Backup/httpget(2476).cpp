#include"httpget.h" 

using namespace std;
#if 1
#define POSTURL "pet.qinqingonline.com:80/feedplan/getFeedPlanForDev"  
#define POSTFIELDS "devid=310023001301920001"  
#define FILENAME "log1.txt"  
#endif
extern char devid_get[20];

size_t WriteMemoryCallback1(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)data;

    mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory) 
    {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }
    return realsize;
}




int httpget_GetTime(char *PlanRecv)
{
	int ret = 0;
  CURL *curl;
  CURLcode res;
char  Url_http[128]={0};
  curl_global_init(CURL_GLOBAL_DEFAULT);
 
  curl = curl_easy_init();
  if(curl) {
//	sprintf(Url_http,"pet.qinqingonline.com:80/feedplan/getFeedPlanForDev?devid=%s",devid_get);
  //  curl_easy_setopt(curl,CURLOPT_URL,"pet.qinqingonline.com:80/feedplan/getFeedPlanForDev?devid=310023001301920001");
    curl_easy_setopt(curl,CURLOPT_URL,"http://pet.qinqingonline.com/devs/devGetTime");
 
#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */ 


    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
 
#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */ 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback1);  //得到请求结果后的回调函数

    MemoryStruct oDataChunk;  //请求结果的保存格式
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &oDataChunk);
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
//	printf("this is ++++++++++++++++++%s\n",oDataChunk.memory);
	strncpy(PlanRecv,oDataChunk.memory,oDataChunk.size);
	printf("------------PlanRecv is %s\n",PlanRecv);
    /* Check for errors */ 
    if(res != CURLE_OK)
      {fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
		ret = -1;
    	}
 /*long res_code=0;
    res=curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &res_code);

    //正确响应后，请请求转写成本地文件的文件
    if(( res == CURLE_OK ) && (res_code == 200 || res_code == 201))
    {
	printf("this is ++++++++++++++++++%s\n",oDataChunk.memory);
	}*/
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
 
  curl_global_cleanup();
 
  return ret;

}

#if 1
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {  
    FILE *fptr = (FILE*)userp;  
    if(fwrite(buffer, size, nmemb, fptr)>0)
    {
	fflush(fptr);
	printf("web is buffer\n",buffer);
	}
	else
	fclose(fptr);
	
	
}    
int post(char * param) {  
    CURL *curl;  
    CURLcode res;
	char p_param[10240]={0};
    FILE *fptr; 
	//if(strncpy(p_param,param,strlen(param))!=NULL)
	//	printf("strncpy ok\n");
    struct curl_slist *http_header = NULL;  
  
    if ((fptr = fopen(FILENAME, "w")) == NULL) {  
        fprintf(stderr, "fopen file error: %s\n", FILENAME);  
        exit(1);  
    }    
    curl = curl_easy_init();  
	if(curl)
	{
	 // char *raw="devid=310023001301920001";
	 
	  curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.220:8080/Dev/getDevOptsRes");
	  sprintf(p_param,"devId=310023003100230001&opt_res=%s",param);
	  printf("p_param is %s\n",p_param);
	  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(p_param ));
	  /* Now specify the POST data */
	  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, p_param );
	  /* 长参数一定要设置CURLOPT_POSTFIELDSIZE，否则会截断 */
	  
	  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1); /* 打开debug */
	  res = curl_easy_perform(curl);
	  /* Check for errors */
	  if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
	
	  /* always cleanup */
	  curl_easy_cleanup(curl);
	  
	}
	curl_global_cleanup();

	return 0;
}  
  
#endif


size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)data;

    mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory) 
    {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }
    return realsize;
}


/*
webresult used to get websever response
logtxt is final the data you want to sent to webdatabase;


*/

int httpget_uploadlogtxt( char *webresult,char *logtxt)
{
  CURL *curl;
  CURLcode res;
 char  Url_http[10240]={0};
  curl_global_init(CURL_GLOBAL_DEFAULT);
 char *logtxt_2 = logtxt;
  curl = curl_easy_init();
  if(curl) {
	sprintf(Url_http,"http://192.168.1.220:8080/Dev/getDevOptsRes?devId=310023003100230001&opt_res=%s",logtxt_2);
  //  curl_easy_setopt(curl,CURLOPT_URL,"pet.qinqingonline.com:80/feedplan/getFeedPlanForDev?devid=310023001301920001");
    curl_easy_setopt(curl,CURLOPT_URL,Url_http);
 
#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */ 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
 
#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */ 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);  //得到请求结果后的回调函数

    MemoryStruct oDataChunk;  //请求结果的保存格式
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &oDataChunk);
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
//	printf("this is ++++++++++++++++++%s\n",oDataChunk.memory);
	strncpy(webresult,oDataChunk.memory,oDataChunk.size);
	printf("------------PlanRecv is %s\n",webresult);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 /*long res_code=0;
    res=curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &res_code);

    //正确响应后，请请求转写成本地文件的文件
    if(( res == CURLE_OK ) && (res_code == 200 || res_code == 201))
    {
	printf("this is ++++++++++++++++++%s\n",oDataChunk.memory);
	}*/
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
 
  curl_global_cleanup();
 
  return 0;
}


int httpget_GET_noreturn(char *url)
{
  CURL *curl;
  CURLcode res;
  char* url_local;
  url_local = url;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl,CURLOPT_URL,url_local);
//    curl_easy_setopt(curl, CURLOPT_URL, "www.baidu.com");

#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return 0;
}

////
size_t WriteCallback(char *data, size_t size, size_t nmemb, string *writerData)
{
unsigned long sizes = size * nmemb;
    if (writerData == NULL) return 0;
    writerData->append(data, sizes);
    return sizes;
}
/*
this fun have two action
1:download upgrade data
2:down music
3:testupgrade

*/
string httpget_GetUpgrade(const char *version,int action,const char *devid)
{
 std::string str; 
 CURL *curl;
  CURLcode res;
char  url[128]={0};
  curl_global_init(CURL_GLOBAL_DEFAULT);
if(action == 1)
sprintf(url,"pet.qinqingonline.com/devupgrade/getDevUpdateFile?filename=%s",version);  
if(action ==2)
sprintf(url,"pet.qinqingonline.com:80/devs/getAudioFile?devid=%s&filename=%s",devid,version);
if(action == 3)
sprintf(url,"pet.qinqingonline.com/devupgrade/getDevUpdateFiletest?filename=%s",version);  
//url ="pet.qinqingonline.com/devupgrade/getDevUpdateFile?filename=custompacket.20200815(alpha)";
curl = curl_easy_init();
  if(curl) {
       // sprintf(Url_http,"pet.qinqingonline.com:80/feedplan/getFeedPlanForDev?devid=%s",devid_get);
  //  curl_easy_setopt(curl,CURLOPT_URL,"pet.qinqingonline.com:80/feedplan/getFeedPlanForDev?devid=310023001301920001");
    curl_easy_setopt(curl,CURLOPT_URL,url);
#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

 curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);  //得到请求结果后的回调函数

   // MemoryStruct oDataChunk;  //请求结果的保存格式
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 /*long res_code=0;
    res=curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &res_code);

    //正确响应后，请请求转写成本地文件的文件
    if(( res == CURLE_OK ) && (res_code == 200 || res_code == 201))
    {
        printf("this is ++++++++++++++++++%s\n",oDataChunk.memory);
        }*/
    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return str;
}


//////
int httpget_GetNewVersion(char *PlanRecv)
{
  CURL *curl;
  CURLcode res;
char  Url_http[128]={0};
  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
   #if 0
     memcpy(Url_http,"http://pet.qinqingonline.com/devupgrade/getdevfileversion",57);
   #endif
        memcpy(Url_http,"http://pet.qinqingonline.com/devupgrade/getdevfileversiontest",61);
  //  curl_easy_setopt(curl,CURLOPT_URL,"pet.qinqingonline.com:80/feedplan/getFeedPlanForDev?devid=310023001301920001");
    curl_easy_setopt(curl,CURLOPT_URL,Url_http);
#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);  //得到请求结果后的回调函数
 MemoryStruct oDataChunk;  //请求结果的保存格式
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &oDataChunk);
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
        strncpy(PlanRecv,oDataChunk.memory,oDataChunk.size);
//      printf("------------PlanRecv is %s\n",PlanRecv);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 /*long res_code=0;
    res=curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &res_code);

    //正确响应后，请请求转写成本地文件的文件
    if(( res == CURLE_OK ) && (res_code == 200 || res_code == 201))
    {
        printf("this is ++++++++++++++++++%s\n",oDataChunk.memory);
        }*/
    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return 0;
}

//////////
size_t WDownload(void *buffer, size_t size, size_t nitems, void *outstream)
{
//      char *p =(char *)data;
//      memcpy(p,ptr,size);
printf("nitem is %d  size is %d \n",nitems,size);
//      return 0;
int written = fwrite(buffer, size, nitems, (FILE*)outstream);
printf("written = %d\n",written);
return written;
}
int HttpDownloadfile(char *filename,char *devid)
{
	char url[256]={0};
  CURL *curl;
  CURLcode res;
  FILE* pFile = fopen( "/mnt/mtd/music.g711", "w" );
	printf("sprintf before\n");
sprintf(url,"pet.qinqingonline.com:80/devs/getAudioFile?devid=%s&filename=%s",devid,filename);
	printf("sprintf after\n");
  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
   // curl_easy_setopt(curl,CURLOPT_URL,"pet.qinqingonline.com:80/feedplan/getFeedPlanForDev?devid=310023001301920001");
    curl_easy_setopt(curl,CURLOPT_URL,url);
curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WDownload);  //得到请求结果后的回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)pFile );
    res = curl_easy_perform(curl);
 if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
  curl_easy_cleanup(curl);
  }
        fclose(pFile ) ;

  curl_global_cleanup();

  return 0;
}
