#ifndef TTCP_ACCEPT_H
#define TTCP_ACCEPT_H

#include "common.h"
#include "socket.h"
#include "tcp_stream.h"
#include "inet_address.h"
#include <memory>

class Acceptor :nocopyable
{
private:
    typedef sockaddr_in PeerAddr;

public:
    explicit Acceptor(const InetAddress& saddr);
    
    ~Acceptor() = default;

    std::unique_ptr<TcpStream> accept();

    inline const InetAddress& getPeerAddr() const { return __peer; };

private:
    Socket __listenSock;
    
    InetAddress __peer;
};

#endif /* TTCP_ACCEPT_H */