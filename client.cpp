#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <vector>

#include <cinttypes>
#include <cstring>
#include "phoneservice.h"
#include "dataBase.h"

int makeConnection(const char* hostIP, uint16_t port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if(inet_pton(AF_INET, hostIP, &addr.sin_addr) < 1) {
        fprintf(stderr, "Wrong host IP format.\n");
        return -1;
    }

    int socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_ < 0) {
        fprintf(stderr, "Socket creation error.\n");
        return -1;
    }

    if(connect(socket_, (struct sockaddr *)&addr, sizeof addr) == -1) {
        fprintf(stderr, "%4d - Connect error.\n", socket_);
        if(close(socket_))
            fprintf(stderr, "%4d - Close error.\n", socket_);
        return -1;
    }
    
    return socket_;
}   

void closeConnection(int socket_) {
    if(shutdown(socket_, 2) == -1) {
        fprintf(stderr, "%4d - Shutdown error.\n", socket_);
        if(close(socket_))
            fprintf(stderr, "%4d - Close error.\n", socket_);
        throw -2;
    }

    if(close(socket_)) {
        fprintf(stderr, "%4d - Close error.\n", socket_);
        throw -2;
    }

    printf("%4d - shutdowned and closed.\n", socket_);
}

bool sendAll(int socket_, const char* msg, uint32_t len) {
    ssize_t status;

    for(uint32_t cur = 0; len - cur;) {
        status = write(socket_, msg + cur, len - cur);
        if(status > 0) {
            cur += status;
        }
        else
            return false;
    }
    
    return true;
}

void sendQuery(int socket_, const char* query) {
    uint32_t len = strlen(query) + 1;

    if(!sendAll(socket_, (const char*)&len, sizeof len)) {
        fprintf(stderr, "%4d - Length sending error(can't send all data).\n", socket_);
        throw -1;
    }

    if(!sendAll(socket_, query, len)) {
        fprintf(stderr, "%4d - Query sending error(can't send all data).\n", socket_);
        throw -1;
    }
}

bool getAll(int socket_, char* buff, uint32_t len) {
    ssize_t status;

    for(uint32_t cur = 0; len - cur;) {
        status = read(socket_, &buff[0] + cur, len - cur);
        if(status > 0) 
            cur += status;
        else 
            return false;
    }

    return true;
}

ResultStatus getStatus(char status) {
    if(status == 0)
        return SUCCESS;

    if(status == 1)
        return INCORRECT_FILTERS;
        
    if(status == 2)
        return INCORRECT_SYNTAX;
    
    if(status == 3)
        return UNKNOWN_QUERY_TYPE;

    if(status == 4)
        return INCORRECT_FROM_QUERY;

    return SERVER_ERROR; //SERVER ERRORS
}

Result getResult(int socket_) {
    std::vector<char> buff(5);
    Result res;

    if(!getAll(socket_, &buff[0], 5)) {
        fprintf(stderr, "%4d - Length reading error(can't read all data).\n", socket_);
        throw -1;
    }

    res.status = getStatus(buff[0]);

    if(res.status != SUCCESS) {
        return res;
    }
    else {
        buff.resize(SERVICE_LEN);
        PhoneService serv;

        uint32_t len;
        memcpy(&len, &buff[1], sizeof len);

        for(uint32_t i = 0; i < len; i++) {
            if(!getAll(socket_, &buff[0], SERVICE_LEN)) {
                fprintf(stderr, "%4d - Result reading error(can't read all data).\n", socket_);
                throw -1;
            }

            serv.ReadStr(&buff[0]);
            res.resList.push_back(serv);
        }

        buff.resize(sizeof(uint32_t));
        if(!getAll(socket_, &buff[0], sizeof(uint32_t))) {
            fprintf(stderr, "%4d - Total amount reading error.\n", socket_);
            throw -1;
        }

        uint32_t total;
        memcpy(&total, &buff[0], sizeof total);

        res.total = total;
    }
    
    return res;
}