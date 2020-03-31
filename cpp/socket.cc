#include "socket.h"
#include <iostream>
#include <algorithm>
Socket::Socket(Socket&& that) {
    std::cout << "move constructor for Socket" << std::endl;
    std::swap(__sockfd, that.__sockfd);
}

Socket& Socket::operator=(Socket&& that) {
    std::cout << "move assign operator for Socket" << std::endl;
    if (this == &that) {
        return *this;
    }

    std::swap(__sockfd, that.__sockfd);
    return *this;
}

Socket::~Socket() {
    std::cout << "Socket::~Socket" <<  std::endl;
    close(__sockfd);
}

Socket Socket::createTCP() {
    std::cout << "Socket::createTCP" << std::endl;
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    return Socket(sockfd);
}

void Socket::bind(const InetAddress& addr) {
    std::cout << "Socket::bind" << std::endl;
    const auto saddr = addr.getSockAddr();
    int ret = ::bind(__sockfd, (const sockaddr*)&saddr,  sizeof saddr);
    if (ret) {
        perror("Socket::bind");
        abort();
    }    
}

void Socket::listen(int backlog) {
    std::cout << "Socket::listen" << std::endl;
    int ret = ::listen(__sockfd, backlog);
    if (ret) {
        perror("Socket::listen");
        abort();
    }
}

int Socket::connect(const InetAddress& serverAddr) throw() {
    int code = ::connect(__sockfd, (const struct sockaddr*)&serverAddr.getSockAddr(), sizeof serverAddr.getSockAddr());
    if (code == -1) {
        // TODO 更详细的错误处理
        throw SocketConnectException();
    }
}

void Socket::setRefuse(bool refuse) 
{
    int optval = refuse? 1 : 0;
    int ret = ::setsockopt(__sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    if (ret) {
        perror("Socket::setRefuse");
    }
}
