#include "query.h"

ResultStatus Query::Parse(const char* query) {
    const char* iter = query;
    int state = 0;
    

    iter += skip(iter);

    sscanf(iter, "select%n", &state); //включает select from
    if (state != 0) {
        iter += state;
        iter += skip(iter);

        return ParseSelect(iter);
    }

    sscanf(iter, "delete%n", &state);
    if (state != 0) {
        iter += state;
        iter += skip(iter);
        
        return ParseDelete(iter);
    }

    sscanf(iter, "clear%n", &state);
    if (state != 0) {
        iter += state;
        iter += skip(iter);

        if(iter[0] != '\0') 
            return INCORRECT_SYNTAX;

        type = CLEAR;

        return SUCCESS;
    }

    sscanf(iter, "insert%n", &state);
    if (state != 0) {
        iter += state;
        iter += skip(iter);

        PhoneService insertVal_;
        ResultStatus res;

        res = ParseInsert(iter, insertVal_);
        if(res == SUCCESS) {
            insValue = insertVal_;
            type = INSERT;
        }

        return res;
    }

    sscanf(iter, "save%n", &state);
    if (state != 0) {
        iter += state;
        iter += skip(iter);

        if(iter[0] != '\0') 
            return INCORRECT_SYNTAX;
        
        type = SAVE;

        return SUCCESS;
    }
    
    return UNKNOWN_QUERY_TYPE;
}

ResultStatus Query::ParseInsert(const char* query, PhoneService& insertVal) {
    const char* iter = query;
    bool modified = true, marker[4] = {false, false, false, false};
    int state = 0;

    while(modified) {
        modified = false;
        if(!marker[0]) {
            sscanf(iter, "phone=%11[0-9]%n", insertVal.phone, &state);
            if (state == 17) {
                iter += state;

                iter += skip(iter);

                modified = true;
                marker[0] = true;

                state = 0;
            }
        }

        if(!marker[1]) {
            sscanf(iter, "service=%d%n", &insertVal.service, &state);
            if (state != 0) {
                iter += state;
                iter += skip(iter);

                modified = true;
                marker[1] = true;

                state = 0;
            }
        }

        if(!marker[2]) {
            sscanf(iter, "sum=%lf%n", &insertVal.sum, &state);
            if (state != 0) {
                iter += state;
                iter += skip(iter);

                modified = true;
                marker[2] = true;

                state = 0;
            }
        }

        if(!marker[3]) {
            sscanf(iter, "datetime=%n", &state);
            if (state != 0) {
                iter += state;

                try {
                    state = insertVal.datetime.ReadTxt(iter);
                }
                catch(const char* msg) {
                    return INCORRECT_SYNTAX;
                }

                iter += state;
                iter += skip(iter);
                
                modified = true;
                marker[3] = true;

                state = 0;
            }
        }
    }

    if(iter[0] == '\0' && marker[0] && marker[1] && marker[2] && marker[3])
        return SUCCESS;
    else 
        return INCORRECT_SYNTAX;
}

ResultStatus Query::ParseSelect(const char* query) {
    const char* iter = query;
    int from_ = -1;
    Filter filters_;
    ResultStatus res;

    iter = ParseFilters(iter, filters_, res);
    if (res == SUCCESS){
        res = ParseFromQuery(iter, from_);
        if(res == SUCCESS) {
            if(from_ != -1) {
                type = SELECT_FROM;
                from = from_;
            }
            else {
                type = SELECT;
            }

            filters = filters_;
        }
    }

    return res;
}

ResultStatus Query::ParseDelete(const char* query) {
    const char* iter = query;
    Filter filters_;
    ResultStatus res;

    iter = ParseFilters(iter, filters_, res);
    if (res == SUCCESS) {
        if(iter[0] == '\0') {
            filters = filters_;
            type = DELETE;
            return res;
        }
            
        return INCORRECT_SYNTAX;
    } 

    return res;  
}

const char* Query::ParseFilters(const char* query, Filter &filters_, ResultStatus& res) {
    const char* iter = query;

    int serviceNum = -1, state = 0;
    bool modified = true;
    Filter filter;
    DateTimePairs date;

    while(modified) {
        modified = false;

        sscanf(iter, "service=%d%n", &serviceNum, &state);
        if (state != 0) {
            filters_.services.push_back(serviceNum);

            iter += state;
            iter += skip(iter);

            modified = true;
            state = 0;
        }

        sscanf(iter, "period=[%n",  &state);
        if (state != 0) {
            iter += state;
            try {
                iter += date.min.ReadTxt(iter);

                if(iter[0] != ',') {
                    res = INCORRECT_SYNTAX;
                    return NULL;
                }
                
                iter++;
                iter += date.max.ReadTxt(iter);
                if(date.max < date.min) {
                    res = INCORRECT_FILTERS;
                    return NULL;
                }
            }
            catch (const char* msg) {
                res = INCORRECT_FILTERS;
                return NULL;
            }

            if (iter[0] != ']')  {
                res = INCORRECT_SYNTAX;
                return NULL;
            }
            

            filters_.periods.push_back(date);

            iter++;
            iter += skip(iter);

            modified = true;
            state = 0;
        }
        
    }

    res = SUCCESS;
    return iter;
}

ResultStatus Query::ParseFromQuery(const char* query, int& from_) {
    const char* iter = query;
    int froM = -1, state = 0;

    sscanf(iter, "from %d%n", &froM, &state);
    if(state != 0) {
        if(froM <= 0)
            return INCORRECT_FROM_QUERY;
                
        from_ = froM;
    }

    iter += state;
    iter += skip(iter);

    if (iter[0] != '\0') 
        return INCORRECT_SYNTAX;

    return SUCCESS;
}

void Query::Print(FILE* fout) const {
    PrintType(fout, type);

    if(type == SELECT || type == SELECT_FROM || type == DELETE) {
        filters.PrintFilter(fout);
    }

    if(type == SELECT_FROM) {
        fprintf(fout, "from %d\n", from);
    }

    if(type == INSERT) 
        insValue.WriteTxt(fout);
    
}

void PrintType(FILE* fout, QueryType type) {
    if (type == UNKNOWN) { 
        fprintf(fout, "unknown\n");
        return;
    }
    
    if (type == SELECT) { 
        fprintf(fout, "select\n");
        return;
    }

    if (type == SELECT_FROM) { 
        fprintf(fout, "select from\n");
        return;
    }
    
    if (type == DELETE) { 
        fprintf(fout, "delete\n");
        return;
    }

    if (type == CLEAR) { 
        fprintf(fout, "clear\n");
        return;
    }

    if (type == SAVE) { 
        fprintf(fout, "save\n");
        return;
    }

    if (type == INSERT) { 
        fprintf(fout, "insert\n");
        return;
    }
    
}

void PrintResultStatus(FILE* fout, ResultStatus res) {
    if (res == INCORRECT_FROM_QUERY) { 
        fprintf(fout, "INCORRECT_FROM_QUERY\n");
        return;
    }
    
    if (res == UNKNOWN_QUERY_TYPE) { 
        fprintf(fout, "UNKNOWN_QUERY_TYPE\n");
        return;
    }

    if (res == INCORRECT_SYNTAX) { 
        fprintf(fout, "INCORRECT_SYNTAX\n");
        return;
    }

    if(res == INCORRECT_FILTERS) {
        fprintf(fout, "INCORRECT_FILTERS\n");
        return;
    }

    if(res == SUCCESS) {
        fprintf(fout, "SUCCESS\n");
        return;
    }

    if(res == OPEN_ERROR) {
        fprintf(fout, "can't open file");
        return;
    }

    if(res == READ_ERROR) {
        fprintf(fout, "can't read from file");
        return;
    }

    if(res == WRITE_ERROR) {
        fprintf(fout, "can't write to file");
        return;
    }
    
    if(res == ALLOCATION_ERROR) {
        fprintf(fout, "ALLOCATION_ERROR\n");
        return;
    }

    if(res == SERVER_ERROR) {
        fprintf(fout, "SERVER_ERROR\n");
        return;
    }
}

void PrintStatusToStr(char* buff, ResultStatus res) {
    if (res == INCORRECT_FROM_QUERY) { 
        memcpy(buff, "INCORRECT FROM QUERY", strlen("INCORRECT FROM QUERY") + 1);
        return;
    }
    
    if (res == UNKNOWN_QUERY_TYPE) { 
        memcpy(buff, "UNKNOWN QUERY TYPE", strlen("UNKNOWN QUERY TYPE") + 1);
        return;
    }

    if (res == INCORRECT_SYNTAX) { 
        memcpy(buff, "INCORRECT SYNTAX", strlen("INCORRECT SYNTAX") + 1);
        return;
    }

    if(res == INCORRECT_FILTERS) {
        memcpy(buff, "INCORRECT FILTERS", strlen("INCORRECT FILTERS") + 1);
        return;
    }

    if(res == SUCCESS) {
        memcpy(buff, "SUCCESS", strlen("SUCCESS") + 1);
        return;
    }

    memcpy(buff, "ERROR ON SERVER", strlen("ERROR ON SERVER") + 1);
}

size_t StatusMsgLen(ResultStatus res) {
    if (res == INCORRECT_FROM_QUERY)
        return strlen("INCORRECT FROM QUERY") + 1;
    
    if (res == UNKNOWN_QUERY_TYPE) 
        return strlen("UNKNOWN QUERY TYPE") + 1;

    if (res == INCORRECT_SYNTAX)
        return strlen("INCORRECT SYNTAX") + 1;

    if(res == INCORRECT_FILTERS) 
        return strlen("INCORRECT FILTERS") + 1;

    if(res == SUCCESS)
        return strlen("SUCCESS") + 1;

    return strlen("ERROR ON SERVER") + 1;
}

void Filter::PrintFilter(FILE* fout) const {
        fprintf(fout, "services:");
        if(services.size() == 0) {
            fprintf(fout, "empty\n");
        }
        else {    
            for(size_t i = 0; i < services.size(); i++) {
                fprintf(fout, " %d", services[i]);
            }

            fprintf(fout, "\n");
        }

        fprintf(fout, "periods:");
        if(periods.size() == 0) {
            fprintf(fout, "empty\n");
        }
        else {    
            for(size_t i = 0; i < periods.size(); i++) {
                fprintf(fout, " ");
                periods[i].Print(fout);
            }
            
            fprintf(fout, "\n");
        }
    }


unsigned int skip(const char* str) {
    unsigned int counter = 0;

    while(*(str + counter) == ' ') 
        counter++;
    
    return counter;
}
