#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "util/log/logmgr.h"

CLogMgr gs_logmgr;
CLogMgr& GetDefaultLogMgr()
{
    return gs_logmgr;
}
CLogMgr::CLogMgr()
{

}

CLogMgr::~CLogMgr()
{

}

int CLogMgr::AddFileCat( int iLogMinLevel, int iLogMaxLevel, int iMaxRollSize, int iMaxRollCount, const char * prefix, const char * postfix )
{
    LogFileCatConf conf;
    conf.m_iMinLevel = iLogMinLevel;
    conf.m_iMaxLevel = iLogMaxLevel;
    m_LogCats.push_back(conf);
    
    int iLast = m_LogCats.size() - 1;
    int iRet = m_LogCats[iLast].m_LogFile.InitLogFile(iMaxRollSize, iMaxRollCount, prefix, postfix);
    if (iRet < 0)
    {
        m_LogCats.pop_back(); 
        return -1;
    }
    //printf("~LogFileCatConf\n");
    return 0;
}

int CLogMgr::ClearAllFileCats()
{
    for (size_t i = 0 ; i < m_LogCats.size(); ++i)
    {
        m_LogCats[i].m_LogFile.Close();
    }
    m_LogCats.clear();
    return 0;
}

static const char* const g_LogLevelDes[] = 
{
    "TRACE", 
    "DEBUG",
    "INFO",
    "ERROR",
    "FATAL",
};



int CLogMgr::Log( const char * fname, const char * ffunc, int fline, LOGLEVEL lv, const char* format, ... )
{
    static const int buflen = 1024*1024;
    static char buf[buflen];
    static char prefixbuf[1024];
    int prefixoffset = 0;
    bool isInited = false;


    // 一般cats不会多于10个，所以小循环遍历下
    for (size_t i = 0 ; i < m_LogCats.size(); ++i)
    {
        if (lv >= m_LogCats[i].m_iMinLevel && lv <= m_LogCats[i].m_iMaxLevel)
        {
            if (!isInited)
            {
                isInited = true;
                int sublen = 0;

                timeval timestamp;
                gettimeofday(&timestamp, NULL);
                tm vtm;
                localtime_r(&timestamp.tv_sec, &vtm);

                char strTime[64];
                strftime(strTime, sizeof(strTime), "%Y%m%d %T", &vtm);
                sublen = snprintf(prefixbuf + prefixoffset, sizeof(prefixbuf) - prefixoffset,"[%s.%06d] [%-5s] ", strTime, (int)timestamp.tv_usec, g_LogLevelDes[lv]);
                if(sublen > 0 && sublen <= (int)(sizeof(prefixbuf) - prefixoffset))
                {
                    prefixoffset += sublen;
                }	

                if(fline != -1)
                {
                    sublen = snprintf(prefixbuf + prefixoffset, sizeof(prefixbuf) - prefixoffset,"[%s:%d (%s)] ", fname, fline, ffunc);
                    if(sublen > 0 && sublen <= (int)(sizeof(prefixbuf) - prefixoffset))
                    {
                        prefixoffset += sublen;
                    }	
                }

                memcpy(buf, prefixbuf, prefixoffset);
            }

            va_list va;
            va_start(va, format);
            int count = vsnprintf(buf + prefixoffset, buflen - prefixoffset, format, va);
            buf[prefixoffset+count] = '\n';
            m_LogCats[i].m_LogFile.Write(buf, prefixoffset+count+1);
            va_end(va);
        }
    }
    return 0;
}

int CLogMgr::Log( LOGLEVEL lv, const char* format, ... )
{
    va_list va;
    va_start(va, format);
    int iRet = Log( "", "", -1, lv, format, va );
    va_end(va);

    return iRet;
}
