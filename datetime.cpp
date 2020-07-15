#include "datetime.h"

DateTime::DateTime(int year_, int month_, int day_, int hour_, int min_, int sec_) {
    set(year_, month_, day_, hour_, min_, sec_);
}

void DateTime::set(int year_, int month_, int day_, int hour_, int min_, int sec_) {
    bool correctDate = (1899 <= year_ && year_ <= 2100 && 1 <= month_ && month_ <= 12 && 1 <= day_ && day_ <= 31
                        && 0 <= hour_ && hour_ <= 23 && 0 <= min_ && min_ <= 59 && 0 <= sec_ && sec_ <= 59);

    if (!correctDate)
        throw "invalid argument";

    year = year_;
    month = month_;
    day = day_;
    hour = hour_;
    min = min_;
    sec = sec_;

    snprintf(strDate, 20, "%4d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min, sec);
}

void DateTime::setString(const char date[20]) {
    memcpy(strDate, date, 20);
}

const char* DateTime::str() const {
    return strDate;
}

void DateTime::SetRand() {
    int year_ = 1900 + rand() % 201,
        month_ = 1 + rand() % 12,
        day_ = 1 + rand() % 31,
        hour_ = rand() % 24,
        min_ = rand() % 60,
        sec_ = rand() % 60;

     set(year_, month_, day_, hour_, min_, sec_);
}

bool DateTime::operator<(const DateTime& d) const {
    return year < d.year
            || (year == d.year && month < d.month)
            || (year == d.year && month == d.month && day < d.day)
            || (year == d.year && month == d.month && day == d.day && hour < d.hour)
            || (year == d.year && month == d.month && day == d.day && hour == d.hour && min < d.min)
            || (year == d.year && month == d.month && day == d.day && hour == d.hour && min == d.min && sec < d.sec);
}

bool DateTime::operator==(const DateTime& d) const {
    return (year == d.year && month == d.month && day == d.day && hour == d.hour && min == d.min && sec == d.sec);
}

bool DateTime::operator<=(const DateTime& d) const {
    return (*this < d) || (*this == d);
}

DateTime& DateTime::operator=(const DateTime& dt){
    set(dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec);
    return *this;
}

int DateTime::ReadTxt(const char* str) {
    int year_, month_, day_, hour_, min_, sec_;
    int dateLen = 0;

    if (sscanf(str, "%4d-%02d-%02d %02d:%02d:%02d%n", &year_, &month_, &day_, &hour_, &min_, &sec_, &dateLen) != 6)
        throw "wrong format of date/time string(should be 'yyyy-mm-dd hh:mm:ss')";
    if(dateLen != 19)
        throw "too much spaces in your date";
    set(year_, month_, day_, hour_, min_, sec_);
    return dateLen;
}

int DateTime::Year() const {
    return year;
}
