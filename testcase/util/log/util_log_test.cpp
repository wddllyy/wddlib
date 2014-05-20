#include <stdio.h>
#include <time.h>
#include <stdlib.h>
//#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include "util/log/log.h"
#include "util/log/logmgr.h"

int util_logfile_test(int argc, char *argv[])
{
    CLogFile logfile;

    logfile.InitLogFile(atoi(argv[1]), atoi(argv[2]), argv[3], argv[4]);


    for (int i =0 ; i <  atoi(argv[5]); ++i)
    {
        struct tm stTm;
        time_t tNow = time(NULL);
        localtime_r(&tNow, &stTm);

        char strTime[128];
        strftime(strTime, sizeof(strTime), "%Y%m%d %T", &stTm);

        char str [4096];
        snprintf(str,sizeof(str),"%s (%s:%d [%s]): %s\n", strTime, basename(__FILE__),__LINE__,__FUNCTION__, "lalalalaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        logfile.Write(str, strlen(str));
        sleep(1);
    }
    
    logfile.Close();
    
    return 0;
}

int util_logmgr_test(int argc, char *argv[])
{
    CLogMgr mgr;
    mgr.AddFileCat(LOGLV_TRACE, LOGLV_FATAL, 200, 5, "logmgr", "log");
    mgr.AddFileCat(LOGLV_ERROR, LOGLV_FATAL, 200, 3, "logmgr", "error");



    for (int i =0 ; i <  10; ++i)
    {
        mgr.Log(basename(__FILE__), __FUNCTION__, __LINE__, LOGLV_ERROR ,"lalalalaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        mgr.Log(LOGLV_DEBUG ,"lalalalaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        
    }
    GetDefaultLogMgr().AddFileCat(LOGLV_TRACE, LOGLV_FATAL, 200, 5, "defaultlogmgr", "log");
    LOG_ERROR("lalala %d", 5);

    mgr.ClearAllFileCats();
    return 0;
}

int main(int argc, char* argv[])
{
    //util_logfile_test(argc, argv);
    util_logmgr_test(argc, argv);
    return 0;
}