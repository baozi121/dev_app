#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "IniFile.h"

CIniFile::CIniFile()
{
    m_cfg_ssl = '[';
    m_cfg_ssr = ']';
    m_cfg_nts = '#';
}

CIniFile::~CIniFile()
{
}

/**********************************************************************
 * 函数名称： strtrimr
 * 功能描述： 去除字符串右边的空字符
 * 访问的表： 无
 * 修改的表： 无
 * 输入参数： char * buf 字符串指针
 * 输出参数： 无
 * 返 回 值    ： 字符串指针
 ***********************************************************************/
char* CIniFile::strtrimr(char * buf)
{
    int len,i;
    char * tmp = NULL;
    len = strlen(buf);
    tmp = (char*)malloc(len);

    memset(tmp,0x00,len);
    for(i = 0;i < len;i++)
    {
        if (buf[i] !=' ')
            break;
    }
    if (i < len) {
        strncpy(tmp,(buf+i),(len-i));
    }
    strncpy(buf,tmp,len);
    free(tmp);
    return buf;
}

/**********************************************************************
 * 函数名称： strtriml
 * 功能描述： 去除字符串左边的空字符
 * 访问的表： 无
 * 修改的表： 无
 * 输入参数： char * buf 字符串指针
 * 输出参数： 无
 * 返 回 值    ： 字符串指针
 ***********************************************************************/
char* CIniFile::strtriml(char * buf)
{
    int len,i;
    char * tmp=NULL;
    len = strlen(buf);
    tmp = (char*)malloc(len);
    memset(tmp,0x00,len);
    for(i = 0;i < len;i++)
    {
        if (buf[len-i-1] !=' ')
            break;
    }
    if (i < len) {
        strncpy(tmp,buf,len-i);
    }
    strncpy(buf,tmp,len);
    free(tmp);
    return buf;
}

/**********************************************************************
 * 函数名称： FileGetLine
 * 功能描述： 从文件中读取一行
 * 访问的表： 无
 * 修改的表： 无
 * 输入参数： FILE *fp 文件句柄；int maxlen 缓冲区最大长度
 * 输出参数： char *buffer 一行字符串
 * 返 回 值    ： 实际读的长度
 ***********************************************************************/
int  CIniFile::FileGetLine(FILE *fp, char *buffer, int maxlen)
{
    int  i, j;
    char ch1;

    for(i = 0, j = 0; i < maxlen; j++)
    {
        if(fread(&ch1, sizeof(char), 1, fp) != 1)
        {
            if(feof(fp) != 0)
            {
                if(j == 0) return -1;               /* 文件结束 */
                else break;
            }
            if(ferror(fp) != 0) return -2;        /* 读文件出错 */
            return -2;
        }
        else
        {
            if(ch1 == '\n' || ch1 == 0x00) break; /* 换行 */
            if(ch1 == '\f' || ch1 == 0x1A)        /* '\f':换页符也算有效字符 */
            {
                buffer[i++] = ch1;
                break;
            }
            if(ch1 != '\r') buffer[i++] = ch1;    /* 忽略回车符 */
        }
    }
    buffer[i] = '\0';
    return i;
}

/**********************************************************************
 * 函数名称： FileCopy
 * 功能描述： 文件拷贝
 * 访问的表： 无
 * 修改的表： 无
 * 输入参数： void *source_file　源文件　void *dest_file　目标文件
 * 输出参数： 无
 * 返 回 值    ： 0 -- OK,非0－－失败
 ***********************************************************************/
int  CIniFile::FileCopy(char *source_file, const char *dest_file)
{
    FILE *fp1, *fp2;
    char buf[1024+1];
    int  ret;

    if((fp1 = fopen(source_file, "r")) == NULL)
        return COPYF_ERR_OPEN_FILE;
    ret = COPYF_ERR_CREATE_FILE;

    if((fp2 = fopen(dest_file, "w")) == NULL) goto copy_end;

    while(1)
    {
        ret = COPYF_ERR_READ_FILE;
        memset(buf, 0x00, 1024+1);
        if(fgets((char *)buf, 1024, fp1) == NULL)
        {
            if(strlen(buf) == 0)
            {
                if(ferror(fp1) != 0) goto copy_end;
                break;                                   /* 文件尾 */
            }
        }
        ret = COPYF_ERR_WRITE_FILE;
        if(fputs((char *)buf, fp2) == EOF) goto copy_end;
    }
    ret = COPYF_OK;
copy_end:
    if(fp2 != NULL) fclose(fp2);
    if(fp1 != NULL) fclose(fp1);
    return ret;
}

/**********************************************************************
 * 函数名称： SplitKeyValue
 * 功能描述： 分离key和value
 *　　　　　　key=val
 *          jack   =   liaoyuewang
 *          |      |   |
 *          k1     k2  i
 * 访问的表： 无
 * 修改的表： 无
 * 输入参数： char *buf
 * 输出参数： char **key;char **val
 * 返 回 值    ： 1 --- ok
 *                0 --- blank line
 *                      -1 --- no key, "= val"
 *                  -2 --- only key, no '='
 ***********************************************************************/
int  CIniFile::SplitKeyValue(char *buf, char **key, char **val)
{
    int  i, k1, k2, n;

    if((n = strlen((char *)buf)) < 1) return 0;
    for(i = 0; i < n; i++)
        if(buf[i] != ' ' && buf[i] != '\t') break;
    if(i >= n) return 0;
    if(buf[i] == '=') return -1;
    k1 = i;
    for(i++; i < n; i++)
        if(buf[i] == '=') break;
    if(i >= n) return -2;
    k2 = i;
    for(i++; i < n; i++)
        if(buf[i] != ' ' && buf[i] != '\t') break;
    buf[k2] = '\0';
    *key = buf + k1;
    *val = buf + i;
    return 1;
}

/**********************************************************************
 * 函数名称： ConfigGetKey
 * 功能描述： 获得key的值
 * 访问的表： 无
 * 修改的表： 无
 * 输入参数： void *CFG_file　文件；void *section　项值；void *key　键值
 * 输出参数： void *buf　key的值
 * 返 回 值： 0 --- ok 非0 --- error
 ***********************************************************************/
int  CIniFile::ConfigGetKey(const char *CFG_file, const char *section, const char *key, char *buf)
{
    FILE *fp;
    char buf1[MAX_CFG_BUF + 1], buf2[MAX_CFG_BUF + 1];
    char *key_ptr, *val_ptr;
    int  line_no, n, ret;

    line_no = 0;
    m_cfg_section_line_no = 0;
    m_cfg_key_line_no = 0;
    m_cfg_key_lines = 0;

    if((fp = fopen(CFG_file, "rb")) == NULL) return CFG_ERR_OPEN_FILE;

    while(1)                                       /* 搜找项section */
    {
        ret = CFG_ERR_READ_FILE;
        n = FileGetLine(fp, buf1, MAX_CFG_BUF);
        if(n < -1) goto r_cfg_end;
        ret = CFG_SECTION_NOT_FOUND;
        if(n < 0) goto r_cfg_end;                    /* 文件尾，未发现 */
        line_no++;
        n = strlen(strtriml(strtrimr(buf1)));
        if(n == 0 || buf1[0] == m_cfg_nts) continue;       /* 空行 或 注释行 */
        ret = CFG_ERR_FILE_FORMAT;
        if(n > 2 && ((buf1[0] == m_cfg_ssl && buf1[n-1] != m_cfg_ssr)))
            goto r_cfg_end;
        if(buf1[0] == m_cfg_ssl)
        {
            buf1[n-1] = 0x00;
            if(strcmp(buf1+1, section) == 0)
                break;                                   /* 找到项section */
        }
    }
    m_cfg_section_line_no = line_no;
    while(1)                                       /* 搜找key */
    {
        ret = CFG_ERR_READ_FILE;
        n = FileGetLine(fp, buf1, MAX_CFG_BUF);
        if(n < -1) goto r_cfg_end;
        ret = CFG_KEY_NOT_FOUND;
        if(n < 0) goto r_cfg_end;                    /* 文件尾，未发现key */
        line_no++;
        m_cfg_key_line_no = line_no;
        m_cfg_key_lines = 1;
        n = strlen(strtriml(strtrimr(buf1)));
        if(n == 0 || buf1[0] == m_cfg_nts) continue;       /* 空行 或 注释行 */
        ret = CFG_KEY_NOT_FOUND;
        if(buf1[0] == m_cfg_ssl) goto r_cfg_end;
        if(buf1[n-1] == '+')                         /* 遇+号表示下一行继续  */
        {
            buf1[n-1] = 0x00;
            while(1)
            {
                ret = CFG_ERR_READ_FILE;
                n = FileGetLine(fp, buf2, MAX_CFG_BUF);
                if(n < -1) goto r_cfg_end;
                if(n < 0) break;                         /* 文件结束 */
                line_no++;
                m_cfg_key_lines++;
                n = strlen(strtrimr(buf2));
                ret = CFG_ERR_EXCEED_BUF_SIZE;
                if(n > 0 && buf2[n-1] == '+')            /* 遇+号表示下一行继续 */
                {
                    buf2[n-1] = 0x00;
                    if(strlen(buf1) + strlen(buf2) > MAX_CFG_BUF)
                        goto r_cfg_end;
                    strcat(buf1, buf2);
                    continue;
                }
                if(strlen(buf1) + strlen(buf2) > MAX_CFG_BUF)
                    goto r_cfg_end;
                strcat(buf1, buf2);
                break;
            }
        }
        ret = CFG_ERR_FILE_FORMAT;
        if(SplitKeyValue(buf1, &key_ptr, &val_ptr) != 1)
            goto r_cfg_end;
        strtriml(strtrimr(key_ptr));
        if(strcmp(key_ptr, key) != 0)
            continue;                                  /* 和key值不匹配 */
        strcpy(buf, val_ptr);
        break;
    }
    ret = CFG_OK;
r_cfg_end:
    if(fp != NULL) fclose(fp);
    return ret;
}

/**********************************************************************
 * 函数名称： ConfigSetKey
 * 功能描述： 设置key的值
 * 访问的表： 无
 * 修改的表： 无
 * 输入参数： void *CFG_file　文件；void *section　项值；
 *           void *key　键值；void *buf　key的值
 * 输出参数： 无
 * 返 回 值：   0 --- ok 非0 --- error
 ***********************************************************************/
int  CIniFile::ConfigSetKey(const char *CFG_file, const char *section, const char *key, const char *buf)
{
    FILE *fp1, *fp2;
    char buf1[MAX_CFG_BUF + 1];
    int  line_no, line_no1, n, ret, ret2;
    char *tmpfname;

    ret = ConfigGetKey(CFG_file, section, key, buf1);
    if(ret <= CFG_ERR && ret != CFG_ERR_OPEN_FILE) return ret;
    if(ret == CFG_ERR_OPEN_FILE || ret == CFG_SECTION_NOT_FOUND)
    {

        if((fp1 = fopen(CFG_file, "a")) == NULL)

            return CFG_ERR_CREATE_FILE;

        if(fprintf(fp1, "%c%s%c\n", m_cfg_ssl, section, m_cfg_ssr) == EOF)
        {
            fclose(fp1);
            return CFG_ERR_WRITE_FILE;
        }
        if(fprintf(fp1, "%s=%s\n", key, buf) == EOF)
        {
            fclose(fp1);
            return CFG_ERR_WRITE_FILE;
        }
        fclose(fp1);
        return CFG_OK;
    }
    if((tmpfname = tmpnam(NULL)) == NULL)
        return CFG_ERR_CREATE_FILE;

    if((fp2 = fopen(tmpfname, "w")) == NULL)

        return CFG_ERR_CREATE_FILE;
    ret2 = CFG_ERR_OPEN_FILE;

    if((fp1 = fopen(CFG_file, "rb")) == NULL) goto w_cfg_end;

    if(ret == CFG_KEY_NOT_FOUND)
        line_no1 = m_cfg_section_line_no;
    else /* ret = CFG_OK */
        line_no1 = m_cfg_key_line_no - 1;
    for(line_no = 0; line_no < line_no1; line_no++)
    {
        ret2 = CFG_ERR_READ_FILE;
        n = FileGetLine(fp1, buf1, MAX_CFG_BUF);
        if(n < 0) goto w_cfg_end;
        ret2 = CFG_ERR_WRITE_FILE;
        if(fprintf(fp2, "%s\n", buf1) == EOF) goto w_cfg_end;
    }
    if(ret != CFG_KEY_NOT_FOUND)
        for( ; line_no < line_no1+m_cfg_key_lines; line_no++)
        {
            ret2 = CFG_ERR_READ_FILE;
            n = FileGetLine(fp1, buf1, MAX_CFG_BUF);
            if(n < 0) goto w_cfg_end;
        }
    ret2 = CFG_ERR_WRITE_FILE;
    if(fprintf(fp2, "%s=%s\n", key, buf) == EOF) goto w_cfg_end;
    while(1)
    {
        ret2 = CFG_ERR_READ_FILE;
        n = FileGetLine(fp1, buf1, MAX_CFG_BUF);
        if(n < -1) goto w_cfg_end;
        if(n < 0) break;
        ret2 = CFG_ERR_WRITE_FILE;
        if(fprintf(fp2, "%s\n", buf1) == EOF) goto w_cfg_end;
    }
    ret2 = CFG_OK;
w_cfg_end:
    if(fp1 != NULL) fclose(fp1);
    if(fp2 != NULL) fclose(fp2);
    if(ret2 == CFG_OK)
    {
        ret = FileCopy(tmpfname, CFG_file);
        if(ret != 0) return CFG_ERR_CREATE_FILE;
    }
    remove(tmpfname);
    return ret2;
}

int  CIniFile::WriteTask(const char *CFG_file, const char *key, const char *buf)
{
    FILE *fp=NULL;
    if(strcmp(buf,"null")==0)
        return CFG_OK;

    if((fp = fopen(CFG_file, "a")) == NULL)
        return CFG_ERR_CREATE_FILE;

    if(fprintf(fp, "%s:%s\n", key, buf) == EOF)
    {
        fclose(fp);
        return CFG_ERR_WRITE_FILE;
    }
    fclose(fp);
    return CFG_OK;
}

int  CIniFile::WifiConfig(const char *CFG_file, const char *key, const char *value)
{
    FILE *fp=NULL;
    char wifi_cfg[256];                                                                                                        
    
    sprintf(wifi_cfg,"ctrl_interface=/var/run/wpa_supplicant\n"
            "network={\n"
            "ssid=\"%s\"\n"
            "psk=\"%s\"\n"
            "key_mgmt=WPA-EAP WPA-PSK IEEE8021X NONE\n"
            "pairwise=TKIP CCMP\n"
            "group=CCMP TKIP WEP104 WEP40\n"
            "}",key,value);

    if((fp = fopen(CFG_file, "w+")) == NULL){
        printf("Open wifi configure file error");
        return CFG_ERR_OPEN_FILE;
    }

    fprintf(fp,"%s\n",wifi_cfg);
    fclose(fp);
    fp = NULL;
    return CFG_OK;
}

int  CIniFile::MsqMsgParse(char* text, char str[][MAX_MSQ_BUF])
{
    int i;
    char* p;

    if((p=strtok(text,","))==NULL || strlen(p)>MAX_MSQ_BUF){
        printf("wrong message or the message is too long!\n");
        return 1;
    }
    strcpy(str[0],p);

    for(i=1;;i++){
        if((p=strtok(NULL,","))==NULL || strlen(p)>MAX_MSQ_BUF){
            printf("wrong message or the message is too long!\n");
            return i;
        }
        strcpy(str[i],p);
    }
    return i;
}

