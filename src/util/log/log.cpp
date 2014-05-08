#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include "util/log/log.h"

static int64_t GetDiffDays(time_t secOfTime1, time_t secOfTime2, int32_t timezone = 28800);
static int64_t GetDiffDays(time_t secOfTime1, time_t secOfTime2, int32_t timezone)
{
    return (secOfTime1 + timezone) / 86400 - (secOfTime2 + timezone) / 86400;
}



CLogFile::CLogFile()
{
    m_pFile = NULL;
    m_uCurSize = 0;
    m_uCurRollIdx = 0;

    m_uRollSize = 0;
    m_uRollCountMax = 0;
    m_uRollTime = 0;
    m_FileNamePrefix[0]=0;
    m_FileNamePostfix[0]=0;
}

CLogFile::~CLogFile()
{

}

int CLogFile::InitLogFile( int iRollSize, int iRollCountMax, const char * filenameprefix, const char * filenamepostfix )
{
    m_uRollSize = iRollSize;
    m_uRollCountMax = iRollCountMax;
    strncpy(m_FileNamePrefix, filenameprefix, k_iMaxFileName-1);
    strncpy(m_FileNamePostfix, filenamepostfix, k_iMaxFileName-1);

    return InitFileRoll();
}

int CLogFile::Write( const char* buff, size_t iSize )
{
    CheckFileRoll();
    //printf("this:%p CLogFile::Write  %s -> %s\n",this , m_FileNamePostfix, buff);
    if (m_pFile)
    {
        size_t count = fwrite(buff, sizeof(char), iSize, m_pFile);
        m_uCurSize += count;
        // TODO:优化
        fflush(m_pFile);
    }
    return 0;
}

int CLogFile::CheckFileRoll()
{
    time_t tNow;
    time(&tNow);

    bool isChangeFile = false;

    int iDiff = GetDiffDays(tNow, m_tCurFileTime);
    if (iDiff != 0)
    {
        isChangeFile = true;
        // 跨天roll清零
        m_uCurRollIdx = 0;
    }
    else
    {
        if (m_uCurSize >= m_uRollSize)
        {
            if (m_uRollCountMax == 0)
            {
                m_uRollCountMax = 1;
            }
            m_uCurRollIdx = (m_uCurRollIdx + 1) % m_uRollCountMax;
            isChangeFile = true;
            //printf("m_uCurSize:%d >= m_uRollSize:%d \n", m_uCurSize, m_uRollSize);
        }
    }

    if (isChangeFile)
    {
        char strPath[k_iMaxFileName];
        GetFileName(strPath, k_iMaxFileName, m_uCurRollIdx, tNow);
        //printf("new filename : %s\n", strPath);
        int iRet = OpenFile(strPath, true);
        return iRet;
    }
    return 0;
}

int CLogFile::InitFileRoll()
{
    char strPath[k_iMaxFileName];
    time_t tNow;
    time(&tNow);
    time_t tLastModify = 0; // Get rid of warning
    int iLastRollID = -1;
    for (uint32_t i = 0; i < m_uRollCountMax; ++i)
    {
        GetFileName(strPath, k_iMaxFileName, i, tNow);
        struct stat buf;
        int iRet = stat(strPath, &buf);
        if (iRet < 0)
        {
            continue;
        }
        if (iLastRollID == -1)
        {
            tLastModify = buf.st_mtime;
            iLastRollID = i;
        }
        else if(tLastModify < buf.st_mtime)
        {
            tLastModify = buf.st_mtime;
            iLastRollID = i;
        }
    }

    if( -1 == iLastRollID )
    {
        m_uCurRollIdx = 0;
    }
    else
    {
        m_uCurRollIdx =	iLastRollID;
    }
    GetFileName(strPath, k_iMaxFileName, m_uCurRollIdx, tNow);
    int iRet = OpenFile(strPath);
    return iRet;
}

int CLogFile::GetFileName( char* pPath, size_t zuPathLen, uint32_t uRollID, time_t tNow )
{
    struct tm stTm;
    localtime_r(&tNow, &stTm);

    char strTime[128];
    strftime(strTime, sizeof(strTime), "%Y%m%d", &stTm);

    if (uRollID == 0)
    {
        snprintf(pPath, zuPathLen, "%s_%s.%s", m_FileNamePrefix, strTime, m_FileNamePostfix);
    }
    else
    {
        snprintf(pPath, zuPathLen, "%s_%s.%s.%u", m_FileNamePrefix, strTime, m_FileNamePostfix, uRollID);
    }
    return 0;
}


#define	OS_DIRSEP '/'
static int MakeDir(char* strDir)
{
    int iRet = mkdir(strDir, 0755);
    if( 0 == iRet || EEXIST == errno )// 已经成功创建目录或者目录本身存在
    {
        return 0;
    }

    char* strPos = strrchr(strDir, OS_DIRSEP);
    if( strPos == NULL )
    {
        return -1;
    }

    strPos[0] =	'\0';
    //去创建上级目录
    iRet = MakeDir(strDir);

    //加上最尾目录本身
    strPos[0] = OS_DIRSEP;
    if( iRet < 0 )
    {
        return -1;
    }
    // 创建目录本身
    iRet = mkdir(strDir, 0755);
    return iRet;
}

static int MakeDirFullPath(const char* strPath)
{
    char strDir[k_iMaxFileName];
    int iRet = 0;

    strncpy(strDir, strPath, sizeof(strDir));

    // 去除文件名本身
    char* strPos = strrchr(strDir, OS_DIRSEP);
    if(!strPos)
    {
        strPos = strrchr(strDir, '/');
    }

    if( strPos )
    {
        strPos[0] = '\0';
        iRet = MakeDir(strDir);
    }

    return iRet;
}
int CLogFile::OpenFile(const char* pPath, bool isClear )
{
    if (m_pFile != NULL)
    {
        //printf("this:%p close oldfile : %p\n", this, m_pFile);
        Close();
    }
    m_pFile = fopen(pPath, "a+");
    //printf("this:%p fopen file : %p  %s\n", this, m_pFile, pPath);
    if( m_pFile == NULL )
    {
        if(ENOENT != errno)
        {
            return -1;
        }
        int iRet = MakeDirFullPath(pPath);
        if( iRet < 0 )
        {
            return -1;
        }
        m_pFile = fopen(pPath, "a+");
    }
    if (m_pFile == NULL)
    {
        return -1;
    }

    //printf("this:%p open file : %p  %s\n", this, m_pFile, pPath);

    if (isClear)
    {
        ftruncate(fileno(m_pFile), 0);
        fseek(m_pFile, 0, SEEK_SET);
    }
    fseek(m_pFile, 0, SEEK_END);
    m_uCurSize = ftell(m_pFile);
    m_tCurFileTime = time(NULL);
    // TODO 文件缓冲区管理
    return 0;
}

int CLogFile::Close()
{
    if (m_pFile != NULL)
    {
        //printf("this:%p close oldfile : %p\n", this, m_pFile);
        const char *str = "close file\n";
        fwrite(str, sizeof(char), strlen(str), m_pFile);
        fflush(m_pFile);
        fclose(m_pFile);
        m_pFile = NULL;
        m_tCurFileTime = 0;
        m_uCurSize = 0;
    }
    return 0;
}
