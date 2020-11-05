#ifndef INI_FILE_H
#define INI_FILE_H

#define MAX_CFG_BUF                 512
#define MAX_MSQ_BUF                 100

#define CFG_OK                      0
#define CFG_SECTION_NOT_FOUND       -1
#define CFG_KEY_NOT_FOUND           -2
#define CFG_ERR                     -10
#define CFG_ERR_FILE                -11
#define CFG_ERR_OPEN_FILE           -12
#define CFG_ERR_CREATE_FILE         -13
#define CFG_ERR_READ_FILE           -14
#define CFG_ERR_WRITE_FILE          -15
#define CFG_ERR_FILE_FORMAT         -16
#define CFG_ERR_SYSTEM              -17
#define CFG_ERR_SYSTEM_CALL         -18
#define CFG_ERR_INTERNAL            -19
#define CFG_ERR_EXCEED_BUF_SIZE     -20

#define COPYF_OK                    0
#define COPYF_ERR_OPEN_FILE         -30
#define COPYF_ERR_CREATE_FILE       -31
#define COPYF_ERR_READ_FILE         -32
#define COPYF_ERR_WRITE_FILE        -33

class CIniFile
{
    public:
        CIniFile();
        ~CIniFile();

        int  ConfigGetKey(const char *CFG_file, const char *section, const char *key, char *buf);
        int  ConfigSetKey(const char *CFG_file, const char *section, const char *key, const char *buf);
        int  WriteTask(const char *CFG_file, const char *key, const char *buf);
        int  WifiConfig(const char *CFG_file, const char *key, const char *value);
        int  MsqMsgParse(char* text, char str[][MAX_MSQ_BUF]);

    private:
        char m_cfg_ssl;
        char m_cfg_ssr;         /* 项标志符Section Symbol --可根据特殊需要进行定义更改，如 { }等*/
        char m_cfg_nts;                 /*注释符*/

        int m_cfg_section_line_no;
        int m_cfg_key_line_no;
        int m_cfg_key_lines;

        char* strtrimr(char * buf);
        char* strtriml(char * buf);
        int  SplitKeyValue(char *buf, char **key, char **val);
        int  FileGetLine(FILE *fp, char *buffer, int maxlen);
        int  FileCopy(char *source_file, const char *dest_file);
};

#endif
