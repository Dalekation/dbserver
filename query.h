#ifndef _QUERY_H_
#define _QUERY_H_

#include <vector>
#include <stdio.h>
#include "phoneservice.h"

class DataBase;

enum QueryType {
    UNKNOWN,
    SELECT,
    SELECT_FROM,
    DELETE,
    INSERT,
    SAVE,
    CLEAR
};

enum ResultStatus {
    OPEN_ERROR = -9,
    READ_ERROR,
    WRITE_ERROR,
    ALLOCATION_ERROR,
    INCORRECT_FROM_QUERY,
    UNKNOWN_QUERY_TYPE,
    INCORRECT_SYNTAX,
    INCORRECT_FILTERS,
    SERVER_ERROR,
    SUCCESS
};


void PrintType(FILE* fout, QueryType type);
void PrintResultStatus(FILE* fout, ResultStatus res);
void PrintStatusToStr(char* buff, ResultStatus res);
size_t StatusMsgLen(ResultStatus res);
unsigned int skip(const char* str);

struct DateTimePairs {
    DateTime min;
    DateTime max;

    DateTimePairs(DateTime min_ = DateTime(), DateTime max_ = DateTime()) {
        min = min_;
        max = max_;
    }

    void Print(FILE* fout) const {
        fprintf(fout, "[%s, %s]", min.str(), max.str());
    }
};

struct Filter {
    std::vector<int> services;
    std::vector<DateTimePairs> periods;

    void PrintFilter(FILE* fout) const;
};


class Query {
private:
    QueryType type;
    Filter filters;
    int from;
    PhoneService insValue;

    const char* ParseFilters(const char* query, Filter &filters_, ResultStatus& res);
    ResultStatus ParseFromQuery(const char* query, int& from_);
    ResultStatus ParseInsert(const char* query, PhoneService& insertVal);
    ResultStatus ParseSelect(const char* query);
    ResultStatus ParseDelete(const char* query);
public:
    Query() {
        type = UNKNOWN;
        from = -1;
    }

    void SetInsert(const PhoneService& insValue_) {
        type = INSERT;
        insValue = insValue_;
    }

    void SetSelectFrom(const Filter& filters_, int from_) {
        type = SELECT_FROM;
        from = from_;
        filters = filters_;
    }

    void SetSelect(const Filter& filters_) {
        type = SELECT;
        filters = filters_;
    }

    void SetDelete(const Filter& filters_) {
        type = DELETE;
        filters = filters_;
    }

    QueryType Type() const {
        return type;
    }

    const Filter& Filters() const {
        return filters;
    }

    int From() const {
        return from;
    }

    const PhoneService& Value() const {
        return insValue;
    }

    ResultStatus Parse(const char* query);

    void Print(FILE* fout) const;

    friend class DataBase;
};

#endif
