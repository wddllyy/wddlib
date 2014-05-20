#ifndef __UTIL_LOGMGR_H
#define __UTIL_LOGMGR_H

#include <vector>
#include <string.h>
#include "util/log/log.h"

//分级
enum LOGLEVEL
{
    LOGLV_TRACE = 0,
    LOGLV_DEBUG = 1,
    LOGLV_INFO  = 2,
    LOGLV_ERROR = 3,
    LOGLV_FATAL = 4,
    LOGLV_COUNT = 5
};



struct LogFileCatConf 
{
    int m_iMinLevel;
    int m_iMaxLevel;
    CLogFile m_LogFile;
};


// 考虑做统一缓存和
class CLogMgr
{
public:
    CLogMgr();
    ~CLogMgr();

    int AddFileCat(int iLogMinLevel, int iLogMaxLevel, int iMaxRollSize, int iMaxRollCount, const char * prefix, const char * postfix);
    int ClearAllFileCats();
    int Log(LOGLEVEL lv, const char* format, ...);
    int Log(const char * fname, const char * ffunc, int fline, LOGLEVEL lv, const char* format, ...);
public:
    typedef std::vector<LogFileCatConf> LogCatVec;

private:
    LogCatVec m_LogCats;
};

extern CLogMgr gs_logmgr;
CLogMgr& GetDefaultLogMgr();


#define LOG(lv, fmt, args...)        gs_logmgr.Log(basename(__FILE__), __FUNCTION__, __LINE__, lv, fmt, ##args)
#define LOG_FATAL(fmt, args...)      gs_logmgr.Log(basename(__FILE__), __FUNCTION__, __LINE__, LOGLV_FATAL, fmt, ##args)
#define LOG_ERROR(fmt, args...)      gs_logmgr.Log(basename(__FILE__), __FUNCTION__, __LINE__, LOGLV_ERROR, fmt, ##args)
#define LOG_INFO(fmt, args...)       gs_logmgr.Log(basename(__FILE__), __FUNCTION__, __LINE__, LOGLV_INFO, fmt, ##args)
#define LOG_DEBUG(fmt, args...)      gs_logmgr.Log(basename(__FILE__), __FUNCTION__, __LINE__, LOGLV_DEBUG, fmt, ##args)
#define LOG_TRACE(fmt, args...)      gs_logmgr.Log(basename(__FILE__), __FUNCTION__, __LINE__, LOGLV_TRACE, fmt, ##args)

#endif
