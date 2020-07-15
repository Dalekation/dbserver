#ifndef _SERVER_H_
#define _SERVER_H_

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <cerrno>
#include <csignal>

#include "dataBase.h"

char setStatus(ResultStatus status);

class Connection {
public:
    Connection(int fd_ = 0): fd(fd_), active(true), icur(0), iall(4), ocur(0), oall(0), ibuff(4), obuff(5)
    {
    }

    ~Connection() {
        close_();
    }

    bool canRcv();
    bool canSnd();

    void receive_(DataBase& db);
    void send_();

    void close_();
private:
    int fd;
    bool active;

    uint32_t icur, iall, ocur, oall;
    std::vector<char>  ibuff, obuff;
    
    Session session;
    Result result_;

    friend class Server;
};

class Server {
public:
    Server(size_t maxSockets = 100): active_sockets(maxSockets) 
    {
    }
    ~Server() {
        for(std::map<int,Connection*>::iterator it = conns.begin(); it != conns.end(); it++) {
            delete it->second;
        }
    }
    void listenAndServe();
    void startServer(uint32_t port, const char* file_name);
    void accept_();

    int listenSocket() {
        return listen_socket;
    }
private:
    int listen_socket;

    std::vector<pollfd> active_sockets;

    std::map<int, Connection*> conns;
    DataBase db;
};

#endif
