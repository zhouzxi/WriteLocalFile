/**********************************************************************
* 版权所有 (C)2015, Zhou Zhaoxiong。
*
* 文件名称：WriteLocalFile.c
* 文件标识：无
* 内容摘要：按照时间和大小生成文件
* 其它说明：无
* 当前版本：V1.0
* 作    者：Zhou Zhaoxiong
* 完成日期：20150721
*
**********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 数据类型重定义
typedef signed   int        INT32;
typedef signed   char       INT8;
typedef unsigned char       UINT8;
typedef unsigned short int  UINT16;
typedef unsigned int        UINT32;
typedef long     int        LONG;
typedef unsigned char       BOOL;

// 字段最大长度
#define MAX_RET_BUF_LEN     (1024)

//参数类型
#define MML_INT8_TYPE       0
#define MML_INT16_TYPE      1
#define MML_INT32_TYPE      2
#define MML_STR_TYPE        3

#define  TRUE         (BOOL)1
#define  FALSE        (BOOL)0

typedef struct
{
    UINT8   second;     /* 0-59 */
    UINT8   minute;     /* 0-59 */
    UINT8   hour;       /* 0-23 */
    UINT8   day;        /* 1-31 */
    UINT8   month;      /* 1-12 */
    UINT16  year;       /* 1994-2099 */
    UINT8   week;       /* 1-7 */
    UINT8   Count10ms;  /* 0-99 */
} ClockStruc;

// 全局变量
UINT32  g_iSeqNo         = 1;     // 序列号初始化为1
INT8    g_szLastDate[20] = {0};   // 日期时间初始化为空
INT8    g_szCtlFile[500]   = {0};

// 函数声明
void CurrentTime(ClockStruc *ptTime);
void WriteToLocalFile(UINT8 *pszContentLine);
void GetValueFromStr(UINT16 iSerialNum, UINT8 iContentType, UINT8 *pSourceStr, UINT8 *pDstStr, UINT8 cIsolater, UINT32 iDstStrSize);
void GetCtlInfo();
void WriteCtrFileForNewDay(INT8 *pszCurDate);
void WriteCtrFileBySize(INT8 *pszCurDate);


/**********************************************************************
* 功能描述：主函数
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期        版本号     修改人            修改内容
* -------------------------------------------------------------------
* 20150721        V1.0     Zhou Zhaoxiong        创建
***********************************************************************/
INT32 main()
{
    UINT8  szContentLine[1000] = {0};

    // 先获取控制文件中的日期时间和序列号
    GetCtlInfo();

    // 第一次写文件
    // 拼装写本地文件的内容
    snprintf(szContentLine, sizeof(szContentLine)-1, "123|ab|\r\n");
    // 将文件内容写入本地文件
    WriteToLocalFile(szContentLine);

    // 第二次写文件
    // 拼装写本地文件的内容
    snprintf(szContentLine, sizeof(szContentLine)-1, "56|ef|\r\n");
    // 将文件内容写入本地文件
    WriteToLocalFile(szContentLine);

    // 第三次写文件
    // 拼装写本地文件的内容
    snprintf(szContentLine, sizeof(szContentLine)-1, "21|ba|\r\n");
    // 将文件内容写入本地文件
    WriteToLocalFile(szContentLine);

    return 0;                  // main函数返回0
}


/**********************************************************************
* 功能描述：获取当前时间
* 输入参数：pTime-时间结构体
* 输出参数：pTime-时间结构体
* 返 回 值：无
* 其它说明：无
* 修改日期        版本号     修改人            修改内容
* -------------------------------------------------------------------
* 20150721        V1.0     Zhou Zhaoxiong        创建
***********************************************************************/
void CurrentTime(ClockStruc *ptTime)
{
    LONG    dt           = 0;
    struct  tm      *tm1 = NULL;
    struct  timeval  tp  = {0};

    // get real clock from system
    gettimeofday(&tp, NULL);
    dt  = tp.tv_sec;
    tm1 = localtime(&dt);
    ptTime->Count10ms = tp.tv_usec / 10000;
    ptTime->year      = (UINT16)(tm1->tm_year + 1900);
    ptTime->month     = (UINT8)tm1->tm_mon + 1;
    ptTime->day       = (UINT8)tm1->tm_mday;
    ptTime->hour      = (UINT8)tm1->tm_hour;
    ptTime->minute    = (UINT8)tm1->tm_min;
    ptTime->second    = (UINT8)tm1->tm_sec;
    ptTime->week      = (UINT8)tm1->tm_wday;
    if (ptTime->week == 0)   // Sunday
    {
        ptTime->week = 7;
    }
}


/**********************************************************************
 * 功能描述： 把内容写到本地文件中
 * 输入参数： pszContentLine-一条文件记录
 * 输出参数： 无
 * 返 回 值： 无
 * 其它说明： 无
 * 修改日期            版本号            修改人           修改内容
 * ----------------------------------------------------------------------
 * 20150721             V1.0          Zhou Zhaoxiong        创建
 ************************************************************************/
void WriteToLocalFile(UINT8 *pszContentLine)
{
    FILE  *fpCtlFile           = NULL;
    INT8   szLocalFile[500]    = {0};
    FILE  *fp                  = NULL;
    INT8   szCurDate[20]       = {0};
    INT8   szBuf[200]          = {0};

    ClockStruc  tClock = {0};

    if (NULL == pszContentLine)
    {
        printf("WriteToLocalFile: input parameter is NULL.\n");
        return;
    } 

    CurrentTime(&tClock);     // 获取当前时间
    snprintf(szCurDate, sizeof(szCurDate) - 1, "%04d%02d%02d", tClock.year, tClock.month, tClock.day);

    if (strncmp(szCurDate, g_szLastDate, 8) != 0 || g_szLastDate[0] == 0)   // 日期不匹配或日期为空, 都当做新的一天处理
    {
        // 新的一天, 需重置序列号为1, 并记录当前日期到控制文件
        WriteCtrFileForNewDay(szCurDate);
    }

    snprintf(szLocalFile, sizeof(szLocalFile)-1, "%s/zhouzhaoxiong/zzx/File_%d.r", getenv("HOME"), g_iSeqNo);
    fp = fopen(szLocalFile, "a+");
    if (fp == NULL)
    {
         printf("WriteToLocalFile: open local file failed, file=%s\n", szLocalFile);
         return;
    }

    printf("WriteToLocalFile: LocalFile=%s, ContentLine=%s\n", szLocalFile, pszContentLine);

    fputs(pszContentLine, fp);
    fflush(fp);

    if (ftell(fp) >= 10)            // 当前文件大于10个字节, 则写新文件
    {
        WriteCtrFileBySize(szCurDate);
    }

    fclose(fp);
    fp = NULL;

    return;
}


/**********************************************************************
*功能描述：获取字符串中某一个字段的数据
*输入参数：iSerialNum-字段编号(为正整数)
           iContentType-需要获取的内容的类型
           pSourceStr-源字符串
           pDstStr-目的字符串(提取的数据的存放位置)
           cIsolater-源字符串中字段的分隔符
           iDstStrSize-目的字符串的长度
*输出参数：无
*返 回 值：无
*其它说明：无
*修改日期        版本号            修改人         修改内容
* --------------------------------------------------------------
* 20150721       V1.0          Zhou Zhaoxiong       创建
***********************************************************************/
void GetValueFromStr(UINT16 iSerialNum, UINT8 iContentType, UINT8 *pSourceStr, UINT8 *pDstStr, UINT8 cIsolater, UINT32 iDstStrSize)
{
    UINT8  *pStrBegin                 = NULL;
    UINT8  *pStrEnd                   = NULL;
    UINT8   szRetBuf[MAX_RET_BUF_LEN] = {0}; //截取出的字符串放入该数组中
    UINT8  *pUINT8                    = NULL;
    UINT16 *pUINT16                   = NULL;
    UINT32 *pUINT32                   = NULL;
    UINT32  iFieldLen                 = 0;     //用于表示每个字段的实际长度

    if (pSourceStr == NULL)           //对输入指针的异常情况进行判断
    {
        return;
    }
    //字段首
    pStrBegin = pSourceStr;
    while (--iSerialNum != 0)
    {
        pStrBegin = strchr(pStrBegin, cIsolater);
        if (pStrBegin == NULL)
        {
            return;
        }
        pStrBegin ++;
    }

    //字段尾
    pStrEnd = strchr(pStrBegin, cIsolater);
    if (pStrEnd == NULL)
    {
        return;
    }

    iFieldLen = (UINT16)(pStrEnd - pStrBegin);
    if(iFieldLen >= MAX_RET_BUF_LEN) //进行异常保护, 防止每个字段的值过长
    {
        iFieldLen = MAX_RET_BUF_LEN - 1;
    }

    memcpy(szRetBuf, pStrBegin, iFieldLen);

    //将需要的字段值放到pDstStr中去
    switch (iContentType)
    {
        case MML_STR_TYPE:                        //字符串类型
        {
            strncpy(pDstStr, szRetBuf, iDstStrSize);
            break;
        }

        case MML_INT8_TYPE:                       //字符类型
        {
            pUINT8   = (UINT8 *)pDstStr;
            *pDstStr = (UINT8)atoi(szRetBuf);
            break;
        }

        case MML_INT16_TYPE:                      // short int类型
        {
            pUINT16  = (UINT16 *)pDstStr;
            *pUINT16 = (UINT16)atoi(szRetBuf);
            break;
        }

        case MML_INT32_TYPE:                      // int类型
        {
            pUINT32  = (UINT32 *)pDstStr;
            *pUINT32 = (UINT32)atoi(szRetBuf);
            break;
        }

        default:                                  // 一定要有default分支
        {
            return;
        }
    }

    return;
}


/**********************************************************************
* 功能描述：获取控制文件中的时间和序列号
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期        版本号     修改人            修改内容
* -------------------------------------------------------------------
* 20150721        V1.0     Zhou Zhaoxiong        创建
***********************************************************************/
void GetCtlInfo()
{
    FILE *fpCtlFile   = NULL;
    INT8  szBuf[500]  = {0};

    // 控制文件存放在当前用户的etc目录下, 命名为CtrFile.txt
    snprintf(g_szCtlFile, sizeof(g_szCtlFile) - 1, "%s/etc/CtrFile.txt", getenv("HOME"));
    fpCtlFile = fopen(g_szCtlFile, "r");
    if (fpCtlFile != NULL)    // 文件打开成功
    {
        fgets(szBuf, sizeof(szBuf), fpCtlFile);    // 读取文件内容

        GetValueFromStr(1, MML_STR_TYPE,   szBuf, (UINT8 *)g_szLastDate, '|', sizeof(g_szLastDate));       // 获取日期字段内容

        GetValueFromStr(2, MML_INT32_TYPE, szBuf, (UINT8 *)&g_iSeqNo,    '|', sizeof(g_iSeqNo));           // 获取序列号字段内容

        fclose(fpCtlFile);
        fpCtlFile = NULL;
    }

    return;
}


/**********************************************************************
* 功能描述：新的一天, 重置序列号为1, 并记录当前日期到控制文件
* 输入参数：pszCurDate-当前时间
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期        版本号     修改人            修改内容
* -------------------------------------------------------------------
* 20150721        V1.0     Zhou Zhaoxiong        创建
***********************************************************************/
void WriteCtrFileForNewDay(INT8 *pszCurDate)
{
    FILE  *fpCtlFile  = NULL;
    INT8   szBuf[200] = {0};

    if (pszCurDate == NULL)
    {
        return;
    }

    g_iSeqNo = 1;                       // 重置序列号为1

    strcpy(g_szLastDate, pszCurDate);   // 对全局时间变量赋值, 防止其为空而导致所有记录写入到一个文件中

    fpCtlFile = fopen(g_szCtlFile, "w");
    if (fpCtlFile != NULL)
    {
        sprintf(szBuf, "%s|%d|", pszCurDate, g_iSeqNo);
        fputs(szBuf, fpCtlFile);
        fclose(fpCtlFile);
        fpCtlFile = NULL;

        printf("WriteCtrFileForNewDay: a new day is coming or the program has just initialized, write control file successfully! file=%s, content=%s\n", g_szCtlFile, szBuf);
    }
    else
    {
        printf("WriteCtrFileForNewDay: write control file failed! file=%s\n", g_szCtlFile);
    }

    return;
}


/**********************************************************************
* 功能描述：当前文件大于规定大小, 则记录日期和序列号到控制文件中
* 输入参数：pszCurDate-当前时间
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期        版本号     修改人            修改内容
* -------------------------------------------------------------------
* 20150721        V1.0     Zhou Zhaoxiong        创建
***********************************************************************/
void WriteCtrFileBySize(INT8 *pszCurDate)
{
    FILE  *fpCtlFile  = NULL;
    INT8   szBuf[200] = {0};

    if (pszCurDate == NULL)
    {
        return;
    }

    g_iSeqNo ++;     // 序列号加1
    if (g_iSeqNo > 999999)      // 序列号最大值为999999
    {
        g_iSeqNo = 1;
    }

    fpCtlFile = fopen(g_szCtlFile, "w");
    if (fpCtlFile != NULL)
    {
        sprintf(szBuf, "%s|%d|", pszCurDate, g_iSeqNo);
        fputs(szBuf, fpCtlFile);
        fclose(fpCtlFile);
        fpCtlFile = NULL;

        printf("WriteCtrFileBySize: file size has reached max size, write control file successfully! file=%s, content=%s\n", g_szCtlFile, szBuf);
    }
    else
    {
        printf("WriteCtrFileBySize: write control file failed! file=%s\n", g_szCtlFile);
    }

    return;
}
