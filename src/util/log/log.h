#ifndef __UTIL_LOG_H
#define __UTIL_LOG_H


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


static const int k_iMaxFileName = 255;

class CLogFile
{
public:
    CLogFile();
    ~CLogFile();
    int InitLogFile(int iRollSize, int iRollCountMax, const char * filenameprefix, const char * filenamepostfix);
    int Write(const char* buff, size_t iSize);
    int Close();
protected:
    
    int CheckFileRoll();
    int InitFileRoll();
    int GetFileName(char* pPath, size_t zuPathLen, uint32_t uRollID, time_t tNow);
    int OpenFile(const char* pPath, bool isClear = false);
    
private:
    time_t m_tCurFileTime;
    uint32_t m_uCurSize;
    uint32_t m_uCurRollIdx;
    
    FILE* m_pFile;

    uint32_t m_uRollSize;
    uint32_t m_uRollCountMax;
    uint32_t m_uRollTime;
    char m_FileNamePrefix[k_iMaxFileName];
    char m_FileNamePostfix[k_iMaxFileName];

};
#endif
