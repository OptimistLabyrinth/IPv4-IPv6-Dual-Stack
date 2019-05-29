#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include <unistd.h>
#include <netinet/in.h>





int LOG(const char*, ...);
class IPv6Data;
class IPv4Data;
class Server;
bool RunServer(const char* arg);





int LOG(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
    printf("\n");
}





class IPv6Data {
public:
    IPv6Data() : socket_(0), serverAddress_(sockaddr_in6()), len_(sizeof(sockaddr_in6)), connSocket_(0) {
        ResetServerAddress();
        ResetRDBuffer();
        ResetWRBuffer();
    }
    void ResetServerAddress() { memset(&serverAddress_, 0, len_); }
    void ResetClientAddress() { memset(&clientAddress_, 0, len_); }
    void ResetRDBuffer() { memset(RDBuffer_, 0, sizeof(RDBuffer_)); }
    void ResetWRBuffer() { memset(WRBuffer_, 0, sizeof(WRBuffer_)); }

    int socket_;
    sockaddr_in6 serverAddress_;
    sockaddr_in6 clientAddress_;
    socklen_t len_;
    int connSocket_;
    char RDBuffer_[4096];
    char WRBuffer_[4096];
};

class IPv4Data {
public:
    IPv4Data() : socket_(0), serverAddress_(sockaddr_in()), len_(sizeof(sockaddr_in)), connSocket_(0) {
        ResetServerAddress();
        ResetRDBuffer();
        ResetWRBuffer();
    }
    void ResetServerAddress() { memset(&serverAddress_, 0, len_); }
    void ResetClientAddress() { memset(&clientAddress_, 0, len_); }
    void ResetRDBuffer() { memset(RDBuffer_, 0, sizeof(RDBuffer_)); }
    void ResetWRBuffer() { memset(WRBuffer_, 0, sizeof(WRBuffer_)); }

    int socket_;
    sockaddr_in serverAddress_;
    sockaddr_in clientAddress_;
    socklen_t len_;
    int connSocket_;
    char RDBuffer_[4096];
    char WRBuffer_[4096];
};

class Server {
public:
    Server();
    explicit Server(unsigned short port);
    ~Server();

    bool StartIPv6Socket();
    static void* Accept6Helper(void* context);
    void* Accept6();

    bool StartIPv4Socket();
    static void* Accept4Helper(void* context);
    void* Accept4();

private:
    unsigned short port_;
    IPv6Data data6_;
    IPv4Data data4_;
    int optionValue_;
    pthread_t ipv6ThreadID_;
    pthread_t ipv4ThreadID_;
};

Server::Server() : port_((unsigned short) 5000), data6_(IPv6Data()), data4_(IPv4Data()), optionValue_(1) {
    LOG("    Server Constructor( ) default");
}

Server::Server(unsigned short port) :
    port_(port), data6_(IPv6Data()), data4_(IPv4Data()), optionValue_(1)
{
    LOG("    Server Constructor( ) with port number");
}

Server::~Server() {
    if (0 <= read(data6_.socket_, data6_.RDBuffer_, 1))
    {
        shutdown(data6_.socket_, SHUT_RDWR);
        close(data6_.socket_);
    }
    if (0 <= read(data4_.socket_, data4_.RDBuffer_, 1))
    {
        shutdown(data4_.socket_, SHUT_RDWR);
        close(data4_.socket_);
    }
    LOG("    Server Destructor( )");
}

bool Server::StartIPv6Socket() {
    if ((data6_.socket_ = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        LOG("ERROR on IPv6 socket( )");
        exit(1);
    }
    LOG("    IPv6 socket created = %d", data6_.socket_);

    data6_.serverAddress_.sin6_family = AF_INET6;
    data6_.serverAddress_.sin6_port = htons(port_);
    data6_.serverAddress_.sin6_addr = in6addr_any;

    if (setsockopt(data6_.socket_, SOL_SOCKET, SO_REUSEADDR, (void*) &optionValue_, sizeof(optionValue_)) != 0) {
        perror("ERROR on IPv6 setsockopt - [ SO_REUSEADDR ]");
        exit(1);
    }
    if (setsockopt(data6_.socket_, SOL_SOCKET, SO_REUSEPORT, (void*) &optionValue_, sizeof(optionValue_)) != 0) {
        perror("ERROR on IPv6 setsockopt - [ SO_REUSEPORT ]");
        exit(1);
    }
    if (setsockopt(data6_.socket_, IPPROTO_IPV6, IPV6_V6ONLY, (void*) &optionValue_, sizeof(optionValue_)) != 0) {
        perror("ERROR on IPv6 setsockopt - [ IPV6_V6ONLY ]");
        exit(1);
    }
    LOG("    IPv6 setsockopt success");

    if (bind(data6_.socket_, (sockaddr*) &data6_.serverAddress_, data6_.len_ ) != 0) {
        perror("ERROR on IPv6 bind");
        exit(1);
    }
    LOG("    IPv6 bind OK");

    if (listen(data6_.socket_, 100) != 0) {
        perror("ERROR on IPv6 listen");
        exit(1);
    }
    LOG("    IPv6 listen OK");

    pthread_create(&ipv6ThreadID_, NULL, Server::Accept6Helper, (void*) this);
}

void* Server::Accept6Helper(void *context) {
    LOG("    Accept 6 Helper !");
    return ((Server*) context)->Accept6();
}

void* Server::Accept6() {
    while (true) {
        data6_.ResetClientAddress();
        if ((data6_.connSocket_ = accept(data6_.socket_, (sockaddr *) &data6_.clientAddress_, &data6_.len_)) < 0) {
            perror("ERROR on IPv6 accept");
            exit(1);
        }
        LOG("    IPv6 Accept( ) OK");
        shutdown(data6_.connSocket_, SHUT_RDWR);
        close(data6_.connSocket_);
        LOG("    -> IPv6 Close( ) OK");
    }

    return NULL;
}

bool Server::StartIPv4Socket() {
    if ((data4_.socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        LOG("ERROR on IPv4 socket( )");
        exit(1);
    }
    LOG("    IPv4 socket created = %d", data4_.socket_);

    data4_.serverAddress_.sin_family = AF_INET;
    data4_.serverAddress_.sin_port = htons(port_);
    data4_.serverAddress_.sin_addr.s_addr = INADDR_ANY;

    if (setsockopt(data4_.socket_, SOL_SOCKET, SO_REUSEADDR, (void*) &optionValue_, sizeof(optionValue_)) != 0) {
        perror("ERROR on IPv4 setsockopt - [ SO_REUSEADDR ]");
        exit(1);
    }
    if (setsockopt(data4_.socket_, SOL_SOCKET, SO_REUSEPORT, (void*) &optionValue_, sizeof(optionValue_)) != 0) {
        perror("ERROR on IPv4 setsockopt - [ SO_REUSEPORT ]");
        exit(1);
    }
    LOG("    IPv4 setsockopt success");

    if (bind(data4_.socket_, (sockaddr*) &data4_.serverAddress_, data4_.len_) != 0) {
        perror("ERROR on IPv4 bind");
        exit(1);
    }
    LOG("    IPv4 bind OK");

    if (listen(data4_.socket_, 100) != 0) {
        perror("ERROR on IPv4 listen");
        exit(1);
    }
    LOG("    IPv4 listen OK");

    pthread_create(&ipv4ThreadID_, NULL, Server::Accept4Helper, (void*) this);
}

void* Server::Accept4Helper(void *context) {
    LOG("    Accept 4 Helper !");
    return ((Server*) context)->Accept4();
}

void* Server::Accept4() {
    while (true) {
        data4_.ResetClientAddress();
        if ((data4_.connSocket_ = accept(data4_.socket_, (sockaddr *) &data4_.clientAddress_, &data4_.len_)) < 0) {
            perror("ERROR on IPv4 accept");
            exit(1);
        }
        LOG("    IPv4 Accept( ) OK");
        shutdown(data4_.connSocket_, SHUT_RDWR);
        close(data4_.connSocket_);
        LOG("    -> IPv4 Close( ) OK");
    }

    return NULL;
}





bool RunServer(const char* arg) {
    Server server((unsigned short) atoi(arg));
    LOG("server created");

    server.StartIPv6Socket();
    LOG("version 6 stack setup OK");

    server.StartIPv4Socket();
    LOG("version 4 stack setup OK");

    while (true) {
        pause();
    }

    return true;
}

int main(int argc, const char* argv[]) {
    LOG("main( ) begin");

    if (1 < argc) {
        RunServer(argv[1]);
    } else {
        RunServer("5000");
    }

    return 0;
}