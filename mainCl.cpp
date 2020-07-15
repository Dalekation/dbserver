#include <stdio.h>
#include <vector>
#include <cinttypes>
#include <stdlib.h>
#include <cerrno>
#include <csignal>
#include "dataBase.h"
int toUInt16(const char *str, uint16_t *p);
void Usage(const char* msg);
void readInput(std::vector<char>& buff);
int makeConnection(const char* hostIP, uint16_t port);
int closeConnection(int socket_);
int sendQuery(int socket_, const char* query);
bool sendAll(int socket_, const char* msg, uint32_t len);
Result getResult(int socket_);
bool getAll(int socket_, char* buff, uint32_t len);
ResultStatus getStatus(char status);

int signalIgnoring();

void handler(int signo) {
    (void)signo;
}

int signalIgnoring() {
    struct sigaction a;
    a.sa_handler = handler;
    a.sa_flags   = 0;
    sigemptyset(&a.sa_mask);

    if(sigaction(SIGPIPE, &a, 0) == -1) {
        fprintf(stderr, "Sigaction(SIGPIPE) error.\n");
        return -1;
    }

    return 0;
}

int main(int ac, char* av[]) {
    if(ac != 1 && ac != 3) {
        fprintf(stderr, "Wrong amount of command line parametres.\n");
        Usage(av[0]);
        return -1;
    }

    int status = 0;
    int sock;
    const char* hostIP = "127.0.0.1";
    uint16_t port = 8000;

    if(ac == 3) {
        if(toUInt16(av[2], &port) < 0) {
            fprintf(stderr, "Wrong format of port number(should be short unsigned int).\n");
            Usage(av[0]);
            return -1;
        }
        hostIP = av[1];
    }

    if(signalIgnoring() != 0)
        return -1;
    
    sock = makeConnection(hostIP, port);
    if(sock < 0) 
        return -1;

    try {
        std::vector<char> buff;
        
        while(1) {
            printf("> ");
            readInput(buff);
            
            if(!feof(stdin)) {
                sendQuery(sock, &buff[0]);
                Result res = getResult(sock);
                res.printResult(stdout);
            }
            else {
                printf("\nFinished successfully.\n");
                break;
            }
            buff.clear();
        }
    }
    catch(std::bad_alloc& e) {
        fprintf(stderr, "Allocation error.\n");
        status = -1;
    }
    catch(int e) {
        status = -1;
        if(e == -1) {
            if(close(sock) < 0) {
                fprintf(stderr, "%4d - Can't close socket.\n", sock);
                return -1;
            }
            else 
                printf("%4d - closed.\n", sock); 
        }
    }

    if(status == 0)
        closeConnection(sock);

    return status;
}

void readInput(std::vector<char>& buff) {
    int c;

    c = fgetc(stdin);

    while(c != '\n' && c != EOF) {
        buff.push_back(c);

        c = fgetc(stdin);
    }
    
    if(c != EOF)
        buff.push_back('\0');
}

void Usage(const char* msg) {
    fprintf(stderr, "Usage: %s [<hostIP> <port>]\n", msg);
}

int toUInt16(const char *str, uint16_t *p) {
    long  l;
    char *e;

    errno = 0;
    l = strtol(str, &e, 10);

    if(!errno && *e == '\0' && 0 <= l && l <= 65535) {
        *p = (uint16_t)l;
        return 0;
    }
    else
        return -1;
}