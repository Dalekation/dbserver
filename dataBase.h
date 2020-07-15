#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "query.h"
#include <list>
#include <map>

class Connsection;

class Result {
private:
    ResultStatus status;    
    uint32_t total;
    std::list<PhoneService> resList;

public:
    Result() {
        status = SUCCESS;
        total = 0;
    }

    ResultStatus Status() const {
        return status;
    }

    const std::list<PhoneService>& resultList() const {
        return resList;
    }

    void printResult(FILE* fout) const; 

    friend class DataBase; 
    friend class Connection;
    friend Result getResult(int socket_);
};

class Session {
private:
    std::map<int, Result> results;
    int next;

    Session() {
        next = 1;
    }
public:
    void Clear() {
        results.clear();
    }

    const std::map<int, Result>& Storage() const { 
        return results;
    }

    int Next() const {
        return next;
    }

    friend class DataBase;
    friend class Connection;
};

class DataBase {
private:
    std::list<PhoneService> phoneServices;
    char* fileName;

    void PerformSelect(const Query& query, Result& result, Session& session);
    void PerformSelectFrom(const Query& query, Result& result, Session& session);
    void PerformDelete(const Query& query, Result& result);

    void Print() const;
public:
    DataBase() {
        fileName = NULL;
    }

    const std::list<PhoneService>& DBstorage() const {
        return phoneServices;
    }

    const char* FileName() const {
        return fileName;
    }

    Result Perform(const char* query, Session& session);
    Result Perform(const Query& query, Session& session);
    ResultStatus loadDatabase(const char* file);

    ResultStatus Save();
};

#endif
