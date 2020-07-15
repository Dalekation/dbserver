#include "server.h"

void Server::startServer(uint32_t port, const char* file_name) {
    if(db.loadDatabase(file_name) < 0) {
        fprintf(stderr, "Can't read database from %s.\n", file_name);
        throw -1;
    }

    struct sockaddr_in addr;
    pollfd listen_sock;
    int listen_socket_, option = 1;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    listen_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_socket_ < 0) {
        fprintf(stderr, "Can't create listen socket.\n");
        throw -1;
    }

    if(setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR, &option, sizeof option) < 0) {
        fprintf(stderr, "%4d - Setsockopt error.\n", listen_socket_);
        throw -1;
    }

    if(fcntl(listen_socket_, F_SETFL, O_NONBLOCK) < 0) {
        fprintf(stderr, "%4d - Nonblock set error.\n", listen_socket_);
        throw -1;
    }

    if(bind(listen_socket_, (struct sockaddr*)&addr, sizeof addr) < 0) {
        fprintf(stderr, "%4d - Bind error.\n", listen_socket_);
        throw -1;
    }

    if(listen(listen_socket_, 5) == -1) {
        fprintf(stderr, "%4d - Listen error.\n", listen_socket_);
        throw -1;
    }

    listen_socket = listen_socket_;

    listen_sock.fd = listen_socket_;
    listen_sock.events = POLLIN;
    listen_sock.revents = 0;

    active_sockets[0] = listen_sock;
}

void Server::listenAndServe() {
    nfds_t sock_amount = 1;
    for(std::map<int,Connection*>::iterator it = conns.begin(); it != conns.end();) {
        if (!it->second->active) {
            delete it->second;
            it = conns.erase(it);
        }
        else {
            short e = 0;

            if(it->second->canRcv())
                e |= POLLIN;

            if(it->second->canSnd())
                e |= POLLOUT;

            if(e) {
                active_sockets[sock_amount].fd = it->first;
                active_sockets[sock_amount].events = e;
                sock_amount++;
            }

            it++;
        }
    }

    switch(poll(&active_sockets[0], sock_amount, 1 * 1000)) {
        case 0:
            return;
        case -1:
            if(errno != EINTR) 
                fprintf(stderr, "Poll error.\n");
            break;
        default:
            if(active_sockets[0].revents & POLLIN) 
                accept_(); 
                
            for(size_t i = 1; i < sock_amount; i++) {
                if(active_sockets[i].revents & POLLIN) 
                    conns[active_sockets[i].fd]->receive_(db);
                    
                if(active_sockets[i].revents & POLLOUT)
                    conns[active_sockets[i].fd]->send_();
            }
    }
}

void Server::accept_() {
    struct sockaddr_in client;
    socklen_t addrlen;
    int new_sock, option = 1;

    addrlen = sizeof client;

    new_sock = accept(listen_socket, (struct sockaddr*)&client, &addrlen);
    if(new_sock < 0) {
        if(new_sock == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            fprintf(stderr, "Accept error.\n");
        return;
    }

    if(setsockopt(new_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof option) < 0) {
        fprintf(stderr, "%4d - Setsockopt error.\n", new_sock);
        throw -1;
    }

    if(fcntl(new_sock, F_SETFL, O_NONBLOCK) < 0) {
        fprintf(stderr, "%4d - Nonblock set error.\n", new_sock);
        throw -1;
    }

    if(conns.size() < active_sockets.size() - 1) {
        printf("%4d - New connection.\n", new_sock);
        conns[new_sock] = new Connection(new_sock);
    }
    else {
        fprintf(stderr, "%4d - Socket storage limit exceeded.\n", new_sock);
        Connection con(new_sock); //called to use destructor and close it safely
    }
}

void Connection::receive_(DataBase& db) {
    ssize_t status;
    uint32_t len;

    if(!canRcv())
        return;

    status = read(fd, &ibuff[0] + icur, iall - icur);

    if(status > 0) {
        icur += status;
    }
    else {
        if(!status) {
            close_();
            return;
        }
        if(status == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            fprintf(stderr, "%4d - Read error.\n", fd);
            close_();
            return;
        }
    }

    if(icur == iall) {
        if(iall == sizeof len) {
            memcpy(&len, &ibuff[0], sizeof len);
            ibuff.resize(len);
            iall = len;
            icur = 0;
        }
        else {
            result_ = db.Perform(&ibuff[0], session);

            obuff[0] = setStatus(result_.status);

            if(result_.status == SUCCESS) 
                len = result_.resList.size();
            else 
                len = 0;

            oall = sizeof len + 1;
            memcpy(&obuff[1], &len, sizeof len);
        }
    }   
}

void Connection::send_() {
    ssize_t status;

    if(!canSnd())
        return;

    status = write(fd, &obuff[0] + ocur, oall - ocur);

    if(status > 0) {
        ocur += status;
    }
    else {
        if(!status) {
            close_();
            return;
        }
        if(status == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            fprintf(stderr, "%4d - Write error.\n", fd);
            close_();
            return;
        }
    }

    if(ocur == oall) {
        if(result_.status == SUCCESS) {
            if(!result_.resList.empty()) {
                oall = SERVICE_LEN;
                ocur = 0;

                obuff.resize(oall);

                PhoneService serv = result_.resList.front();

                serv.WriteStr(&obuff[0]);

                result_.resList.pop_front();
            }
            else {
                if(oall == 5 || oall == SERVICE_LEN) {
                    uint32_t res = result_.total;

                    oall = sizeof res;
                    ocur = 0;

                    obuff.resize(oall);

                    memcpy(&obuff[0], &res, sizeof res);
                }
                else {
                    oall = 0;
                    ocur = 0;
                    iall = 4;
                    icur = 0;
                    ibuff.resize(4);
                    obuff.resize(5);
                }
            }
        }
        else {
                oall = 0;
                ocur = 0;
                iall = 4;
                icur = 0;
                ibuff.resize(4);
                obuff.resize(5);
        }
    }

}

bool Connection::canRcv() {
    return active && iall > icur;
}

bool Connection::canSnd() {
    return active && oall > ocur;
}

void Connection::close_() {
    if(active) {
        if(shutdown(fd, 2) < 0)
            fprintf(stderr, "%4d - Shutdown error.\n", fd);

        if(close(fd))
            fprintf(stderr, "%4d - Close error.\n", fd);

        active = false;
        printf("%4d - Shutdowned and closed.\n", fd);
    }
}

char setStatus(ResultStatus status) {
    if(status == SUCCESS)
        return 0;

    if(status == INCORRECT_FILTERS)
        return 1;
        
    if(status == INCORRECT_SYNTAX)
        return 2;
    
    if(status == UNKNOWN_QUERY_TYPE)
        return 3;

    if(status == INCORRECT_FROM_QUERY)
        return 4;

    return 5; //SERVER ERRORS
}

