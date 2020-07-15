#ifndef _PHONESERVICE_H_
#define _PHONESERVICE_H_

#include "datetime.h"
#include <sys/stat.h>
#include <fcntl.h>
#include<vector>
#include <unistd.h>

#define PHONE_LEN 11
#define SERVICE_LEN 44
class PhoneService {
public:
    char phone[12];
    int service;
    DateTime datetime;
    double sum;

public:
    PhoneService(char* phone_ = NULL, int srvce = 0, DateTime date = DateTime(), double sum_ = 0.);

    void SetRand();

    int ReadBin(int fd);
    int WriteBin(int fd) const;

    void WriteStr(char* buff) const;
    void ReadStr(char* buff);

    void WriteTxt(FILE* fout) const;
};
#endif
