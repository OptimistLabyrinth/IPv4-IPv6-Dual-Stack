#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>





int LOG(const char* format, ...);
class IPv6Client;
class IPv4Client;





int LOG(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
    printf("\n");
}





class IPv6Client {
public:
    IPv6Client();
    explicit IPv6Client(unsigned short port);
    ~IPv6Client();

    void Run();
    void ResetAddress();

private:
    unsigned short port_;
    int socket_;
    sockaddr_in6 address_;
    socklen_t len_;
    int optionValue_;
};

IPv6Client::IPv6Client() :
    port_((unsigned short) 5000), socket_(0), address_(sockaddr_in6()), len_(sizeof(sockaddr_in6)), optionValue_(1)
{
    LOG("    IPv6 Constructor( )");
}

IPv6Client::IPv6Client(unsigned short port) :
    port_(port), socket_(0), address_(sockaddr_in6()), len_(sizeof(sockaddr_in6)), optionValue_(1)
{
    LOG("    IPv6 Constructor( ) with port number");
}

IPv6Client::~IPv6Client() {
    if (0 <= read(socket_, NULL, 1)) {
        shutdown(socket_, SHUT_RDWR);
        close(socket_);
    }
    if (0 <= read(socket_, NULL, 1)) {
        shutdown(socket_, SHUT_RDWR);
        close(socket_);
    }
    LOG("    IPv6 Destrcutor( )");
}

void IPv6Client::Run() {
    if ((socket_ = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("ERROR on IPv6 socket( )");
        exit(1);
    }
    LOG("    IPv6 socket created");

    ResetAddress();
    address_.sin6_family = AF_INET6;
    address_.sin6_port = htons(port_);
    if (inet_pton(AF_INET6, "fd15:4ba5:5a2b:1008:b935:6d65:5d68:5481", (void*) &address_.sin6_addr) != 1) {
        perror("ERROR on IPv6 inet_pton( )");
        exit(1);
    }

    if (setsockopt(socket_, SOL_SOCKET, SO_REUSEPORT, (void*) &optionValue_, sizeof(optionValue_)) != 0) {
        perror("ERROR on IPv6 setsockopt( )");
        exit(1);
    }
//    LOG("    IPv6 setsockopt OK");

    if (connect(socket_, (sockaddr*) &address_, len_) != 0) {
        perror("ERROR on IPv6 connect( )");
        exit(1);
    }
//    LOG("    IPv6 connect OK");

    {
        char buf[8192];
        memset(buf, 0, sizeof(buf));
        inet_ntop(AF_INET6, &address_.sin6_addr, buf, sizeof(buf));
        LOG("    remote host IP: \"%s\"", buf);
    }

    shutdown(socket_, SHUT_RDWR);
    close(socket_);
//    LOG("    IPv6 close OK");
}

void IPv6Client::ResetAddress() { memset(&address_, 0, len_); }





class IPv4Client {
public:
    IPv4Client();
    explicit IPv4Client(unsigned short port);
    ~IPv4Client();

    void Run();
    void ResetAddress();

private:
    unsigned short port_;
    int socket_;
    sockaddr_in address_;
    socklen_t len_;
    int optionValue_;
};

IPv4Client::IPv4Client() :
        port_((unsigned short) 5000), socket_(0), address_(sockaddr_in()), len_(sizeof(sockaddr_in)), optionValue_(1)
{
    LOG("    IPv4 Client Constructor( )");
}

IPv4Client::IPv4Client(unsigned short port) :
    port_(port), socket_(0), address_(sockaddr_in()), len_(sizeof(sockaddr_in)), optionValue_(1)
{
    LOG("    IPv4 Client Constructor( ) with port number");
}

IPv4Client::~IPv4Client() {
    if (0 <= read(socket_, NULL, 1)) {
        shutdown(socket_, SHUT_RDWR);
        close(socket_);
    }
    if (0 <= read(socket_, NULL, 1)) {
        shutdown(socket_, SHUT_RDWR);
        close(socket_);
    }
    LOG("    IPv4 Destrcutor( )");
}

void IPv4Client::Run() {
    if ((socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("ERROR on IPv4 socket( )");
        exit(1);
    }
    LOG("    IPv4 socket created");

    ResetAddress();
    address_.sin_family = AF_INET;
    address_.sin_port = htons(port_);
    if (inet_pton(AF_INET, "192.168.199.129", (void*) &address_.sin_addr) != 1) {
        perror("ERROR on IPv4 inet_pton( )");
        exit(1);
    }

    if (setsockopt(socket_, SOL_SOCKET, SO_REUSEPORT, (void*) &optionValue_, sizeof(optionValue_)) != 0) {
        perror("ERROR on IPv4 setsockopt( )");
        exit(1);
    }
//    LOG("    IPv4 setsockopt OK");

    if (connect(socket_, (sockaddr*) &address_, len_) != 0) {
        perror("ERROR on IPv4 connect( )");
        exit(1);
    }
//    LOG("    IPv4 connect OK");

    {
        char buf[8192];
        memset(buf, 0, sizeof(buf));
        inet_ntop(AF_INET, &address_.sin_addr, buf, sizeof(buf));
        LOG("    remote host IP: \"%s\"", buf);
    }

    shutdown(socket_, SHUT_RDWR);
    close(socket_);
//    LOG("    IPv4 close OK");
}

void IPv4Client::ResetAddress() { memset(&address_, 0, len_); }





bool RunClient(const char* arg) {
    LOG("called RunClient( )");

    IPv6Client client6((unsigned short) atoi(arg));
    LOG("version 6 client ready");

    IPv4Client client4((unsigned short) atoi(arg));
    LOG("version 4 client ready");

    int i = 0;
    while (true) {
        printf("which one ? ");
        std::cin >> i;

        if (i == 6) {
            client6.Run();
        }
        else if (i == 4) {
            client4.Run();
        }
        else {
            std::cin.ignore();
        }
    }
}

int main(int argc, const char* argv[]) {
    if (1 < argc) {
        RunClient(argv[1]);
    } else {
        RunClient("5000");
    }
    return 0;
}