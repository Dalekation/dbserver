#include "phoneservice.h"

PhoneService::PhoneService(char* phone_, int srvce, DateTime date, double sum_) {
    if(phone_ == NULL)
        memset(phone, '\0', 12);
    else {
        size_t len = strlen(phone);
        if(len != 11) {
            throw "Wrong number format";
        }

        memcpy(phone, phone_, 12);
    }

    service = srvce;
    datetime = date;
    sum = sum_;
}

void PhoneService::SetRand() {
    for(int i = 0; i < 11; i++)
        phone[i] = '0' + rand() % 10;

    phone[11] = '\0';

    service = rand() % 5;
    sum = (rand() % 50000) / 100.;

    datetime.SetRand();
}

void PhoneService::WriteTxt(FILE* fout) const{
    fprintf(fout, "| %-12s | %-7d | %-s | %-12.2f |\n", phone, service, datetime.str(), sum);
}

int PhoneService::WriteBin(int fd) const {
    size_t dateLen = strlen(datetime.str()) + 1;
    size_t length = sizeof phone + sizeof service + sizeof sum + dateLen;

    std::vector<char> buff(length);

    memcpy(&buff[0], phone, sizeof phone);
    length = sizeof phone;

    memcpy(&buff[0] + length, &service, sizeof service);
    length += sizeof service;

    memcpy(&buff[0] + length, datetime.str(), dateLen);
    length += dateLen;

    memcpy(&buff[0] + length, &sum, sizeof sum);

    ssize_t status = write(fd, &buff[0], buff.size());
    if (status != (ssize_t)buff.size())
        return -1;

    return buff.size();
}

int PhoneService::ReadBin(int fd) {
    size_t dateLen = strlen(datetime.str()) + 1;
    size_t length = sizeof phone + sizeof service + dateLen + sizeof sum;

    std::vector<char> buff(length);
    char date_buff[20];

    ssize_t status = read(fd, &buff[0], length);
    if (status == 0)
        return 0;
    if (status != (ssize_t)length)
        return -1;

    length = 0;
    memcpy(phone, &buff[0], sizeof phone);

    length += sizeof phone;
    memcpy(&service, &buff[0] + length, sizeof service);

    length += sizeof service;
    memcpy(date_buff, &buff[0] + length, dateLen);

    datetime.ReadTxt(date_buff);

    length += dateLen;
    memcpy(&sum, &buff[0] + length, sizeof sum);

    return buff.size();
}

void PhoneService::WriteStr(char* buff) const {
    size_t dateLen = strlen(datetime.str()) + 1;

    size_t length = 0;
    memcpy(buff, phone, sizeof phone);
    length = sizeof phone;

    memcpy(buff + length, &service, sizeof service);
    length += sizeof service;

    memcpy(buff + length, datetime.str(), dateLen);
    length += dateLen;

    memcpy(buff + length, &sum, sizeof sum);
}

void PhoneService::ReadStr(char* buff) {
    size_t dateLen = strlen(datetime.str()) + 1;
    char dateBuff[20];

    size_t length = 0;
    memcpy(phone, buff, sizeof phone);
    length = sizeof phone;

    memcpy(&service, buff + length, sizeof service);
    length += sizeof service;

    memcpy(dateBuff, buff + length, dateLen);
    datetime.ReadTxt(dateBuff);
    
    length += dateLen;

    memcpy(&sum, buff + length, sizeof sum);
}

