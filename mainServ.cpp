#include "server.h"

void Usage (char* av);
int signalIgnoring();

volatile sig_atomic_t quit;

void handler(int signo) {
    if(signo == SIGINT) 
        quit = 1;
}

int signalIgnoring() {
    struct sigaction a;
    a.sa_handler = handler;
    a.sa_flags   = 0;
    sigemptyset(&a.sa_mask);

    if(sigaction(SIGINT, &a, 0) == -1) {
        fprintf(stderr, "Sigaction(SIGINT) error.\n");
        return -1;
    }

    if(sigaction(SIGPIPE, &a, 0) == -1) {
        fprintf(stderr, "Sigaction(SIGPIPE) error.\n");
        return -1;
    }

    return 0;
}

int main(int ac, char* av[]) {
    if(ac != 2) {
        fprintf(stderr, "Wrong amount of prameters.\n");
        Usage(av[0]);
        return -1;
    }

    int status = 0;
    Server serv;

    try {
        if(signalIgnoring() != 0)
            throw -1;
        serv.startServer(8000, av[1]);

        while(!quit) {
            serv.listenAndServe();
        }
    }
    catch(std::bad_alloc& e) {
        fprintf(stderr, "Allocation error.\n");
        status = -1;
    }
    catch(const char* msg) {
        fprintf(stderr, "%s\n", msg);
        status = -1;
    }
    catch(int e) {
        status = e;
    }

    if(close(serv.listenSocket())) {
        fprintf(stderr, "%4d - Close error.\n", serv.listenSocket());
        status = -1;
    }

    printf("Done.\n");

    return status;
}

void Usage (char* av) {
    fprintf(stderr, "Usage:: %s filename\n", av);
}
