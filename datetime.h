#ifndef _DATETIME_H_
#define _DATETIME_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#define DATE_LENGTH 19
class DateTime {
private:
    int year, month, day, hour, min, sec;
    char strDate[20];

    void set(int year_, int month_, int day_, int hour_, int min_, int sec_);

public:
    DateTime(int year_ = 1899, int month_ = 12, int day_ = 31, int hour_ = 23, int min_ = 59, int sec_ = 59);
    int ReadTxt(const char* str);

    const char* str() const;
    void SetRand();
    void setString(const char date[20]);
    int Year() const;

    bool operator<=(const DateTime& d) const;
    bool operator<(const DateTime& d) const;
    bool operator==(const DateTime& d) const;
    DateTime& operator=(const DateTime& datetime);
};

#endif

