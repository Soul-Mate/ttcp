#include "inet_address.h"
InetAddress::InetAddress(const StringArg& ip, in_port_t port) throw() {
    bzero(&__sockaddr, sizeof(sockaddr_in));
    __sockaddr.sin_family = AF_INET;
    __sockaddr.sin_port = htons(port);
    int code = ::inet_pton(AF_INET, ip.get_c_str(), &__sockaddr.sin_addr.s_addr);
    if (code == 0) {
        throw InetAddressException("Not in presentation format");
    }

    if (code == -1) {
        throw strerror(errno);
    }
}

InetAddress::InetAddress(in_port_t port, bool loopback) {
    bzero(&__sockaddr, sizeof(sockaddr_in));
    __sockaddr.sin_port = htons(port);
    __sockaddr.sin_family = AF_INET;
    // TODO 为什么还需要调用htonl? INADDR_LOOPBACK和INADDR_ANY的类型是什么？
    __sockaddr.sin_addr.s_addr = htonl(loopback? INADDR_LOOPBACK : INADDR_ANY);
}

InetAddress::InetAddress(const sockaddr_in& addr) {
    std::memcpy(&__sockaddr, &addr, sizeof(addr));
}

std::string InetAddress::toIP() const {
    char buf[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &__sockaddr.sin_addr.s_addr, buf, INET_ADDRSTRLEN);
    return std::string(std::move(buf));
}

std::string InetAddress::toEndpoint() const {
    char buf[24]; // 255.255.255.255:65535  4*4+5+1 = 22
    ::inet_ntop(AF_INET, &__sockaddr.sin_addr.s_addr, buf, INET_ADDRSTRLEN);
    size_t ipEnd = std::strlen(buf);
    snprintf(buf + ipEnd, sizeof buf - ipEnd, ":%u", ntohs(__sockaddr.sin_port));
    return buf;
}