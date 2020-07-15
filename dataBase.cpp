#include "dataBase.h"
#include <iterator>


Result DataBase::Perform(const Query& query, Session& session) {
    Result result;
    
    if(query.type == SELECT) {
        PerformSelect(query, result, session);
        
        return result;
    }

    if(query.type == SELECT_FROM) {
        PerformSelectFrom(query, result, session);
        
        return result;
    }

    if(query.type == DELETE) {
        PerformDelete(query, result);

        return result;
    }

    if(query.type == INSERT) {
        phoneServices.push_back(query.insValue);
        result.total = phoneServices.size();
        return result;
    }

    if(query.type == CLEAR) {
        result.total = session.results.size();
        session.Clear();
        return result;
    }

    if(query.type == SAVE) {
        result.status = Save();
        result.total = phoneServices.size();

        return result;
    }

    result.status = UNKNOWN_QUERY_TYPE;

    return result;
}

void DataBase::PerformSelect(const Query& query, Result& result, Session& session) {
    bool dateCheck = false, servCheck = false, needToChDate = true, needToChServ = true;
    if(query.filters.services.size() == 0) {
        needToChServ = false;
        servCheck = true;
    }

    if(query.filters.periods.size() == 0) {
        needToChDate = false;
        dateCheck = true;
    }

    for (std::list<PhoneService>::iterator iter = phoneServices.begin(); iter != phoneServices.end(); iter++) {
        if(needToChServ)
            for(size_t  i = 0; i < query.filters.services.size(); i++) {
                if(iter->service == query.filters.services[i]) {
                    servCheck = true;
                    break;
                }
            }

        if(needToChDate)
            for(size_t  i = 0; i < query.filters.periods.size(); i++) {
                if(query.filters.periods[i].min <= iter->datetime && iter->datetime <= query.filters.periods[i].max) {
                    dateCheck = true;
                    break;
                }
            }

        if(servCheck && dateCheck) {
            result.resList.push_back(*iter);
            result.total++;
        }

        if(needToChServ)
            servCheck = false;

        if(needToChDate)
            dateCheck = false;
    }

    session.results[session.next] = result;
    session.next++;
}

void DataBase::PerformSelectFrom(const Query& query, Result& result, Session& session) {
    if(query.from >= session.next) {  
        result.status = INCORRECT_FROM_QUERY;
        return;
    }

    try {
        session.results.at(query.from);

        std::list<PhoneService>::iterator iter = session.results[query.from].resList.begin(),
                                            border = session.results[query.from].resList.end();

        bool dateCheck = false, servCheck = false, needToChServ = true, needToChDate = true;

        if(query.filters.services.size() == 0) {
            needToChServ = false;
            servCheck = true;
        }

        if(query.filters.periods.size() == 0) {
            needToChDate = false;
            dateCheck = true;
        }

        for (; iter != border; iter++) {
            if(needToChServ)
                for(size_t  i = 0; i < query.filters.services.size(); i++) {
                    if(iter->service == query.filters.services[i]) {
                        servCheck = true;
                        break;
                    }
                }

            if(needToChDate)
                for(size_t  i = 0; i < query.filters.periods.size(); i++) {
                    if(query.filters.periods[i].min <= iter->datetime && iter->datetime <= query.filters.periods[i].max) {
                        dateCheck = true;
                        break;
                    }
                }

            if(servCheck && dateCheck) {
                result.resList.push_back(*iter);
                result.total++;
            }

            if(needToChServ)
                servCheck = false;

            if(needToChDate)
                dateCheck = false;
        }

        session.results[session.next] = result;
        session.next++;

    }
    catch(const std::out_of_range& exep) {
        result.status = INCORRECT_FROM_QUERY;
    }
}

void DataBase::PerformDelete(const Query& query, Result& result) {
    std::list<PhoneService>::iterator iter = phoneServices.begin(), border = phoneServices.end();
    size_t startSize = phoneServices.size();
    bool dateCheck = false, servCheck = false, needToChServ = true, needToChDate = true;

    if(query.filters.services.size() == 0) {
        needToChServ = false;
        servCheck = true;
    }

    if(query.filters.periods.size() == 0) {
        needToChDate = false;
        dateCheck = true;
    }

    while(iter != border) {
        if(needToChServ)
            for(size_t  i = 0; i < query.filters.services.size(); i++) {
                if(iter->service == query.filters.services[i]) {
                    servCheck = true;
                    break;
                }
            }

        if(needToChDate)
            for(size_t  i = 0; i < query.filters.periods.size(); i++) {
                if(query.filters.periods[i].min <= iter->datetime && iter->datetime <= query.filters.periods[i].max) {
                    dateCheck = true;
                    break;
                }
            }

        if(servCheck && dateCheck) 
            iter = phoneServices.erase(iter);
        else 
            iter++;

        if(needToChServ)
            servCheck = false;

        if(needToChDate)
            dateCheck = false;
    }

    result.total = startSize - phoneServices.size();
}

ResultStatus DataBase::loadDatabase(const char* file) {
    PhoneService serv;
    int status, fd;

    fd = open(file, O_RDONLY);
    if (fd == -1) 
        return OPEN_ERROR;
    

    while(1) {  
        status = serv.ReadBin(fd);

        if (status == 0)
                break;
        else if (status == -1) {
            phoneServices.clear();
            close(fd);

            return READ_ERROR;
        }
        else
            phoneServices.push_back(serv);
    }

    size_t len = strlen(file) + 1;

    fileName = (char*) malloc(len * sizeof(char));
    if(fileName == NULL) {
        return ALLOCATION_ERROR;
    }
    
    memcpy(fileName, file, len);

    close(fd);

    return SUCCESS;
}

void Result::printResult(FILE* fout) const {
    printf("\n");
    if(status != SUCCESS) {
        PrintResultStatus(fout, status);
        return;
    }

    if(!resList.empty()) {
        fprintf(fout, "| Phone number | service | date and time       | sum          |\n");
        fprintf(fout, "|--------------|---------|---------------------|--------------|\n");
        for(std::list<PhoneService>::const_iterator iter = resList.begin(); iter != resList.end(); iter++) 
            iter->WriteTxt(fout);
    }

    fprintf(fout, "Total: %u\n", total);
}


Result DataBase::Perform(const char* query, Session& session) {
    Query query_;
    ResultStatus res;

    res = query_.Parse(query);
    if(res != SUCCESS) {
        Result result;
        result.status = res;

        return result;
    }

    return Perform(query_, session);
}

ResultStatus DataBase::Save() {
    int fd;

    fd = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) 
        return OPEN_ERROR;
    

    int state = 0;

    for(std::list<PhoneService>::iterator iter = phoneServices.begin(); iter != phoneServices.end(); iter++) {
        state = iter->WriteBin(fd);
        if(state == -1) {
            close(fd);
            return WRITE_ERROR;
        }
    }

    close(fd);
    
    return SUCCESS;
}

//helping functions

void DataBase::Print() const {
    for(std::list<PhoneService>::const_iterator iter = phoneServices.begin(); iter != phoneServices.end(); iter++)
        iter->WriteTxt(stdout);
    

    printf("%s\n", fileName);
    
    /*if(session.results.size() == 0)
        printf("session is empty\n");
    else {
        printf("Session::\n");

        for(std::map<int, Result>::const_iterator iter = session.results.begin(); iter != session.results.end(); iter++)
            iter->second.printResult(stdout);
            
        
    }
    printf("Next:: %d\n", session.next);*/
}
