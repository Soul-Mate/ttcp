#ifndef TTCP_TCP_STREAM_H
#define TTCP_TCP_STREAM_H

#include "common.h"
#include "socket.h"
#include <memory>
class TcpStream :nocopyable
{
public:
    explicit TcpStream(Socket&& socket);

    int receiveAll(void* buf, size_t len);

    int sendAll(const void* buf, size_t len);

    static std::unique_ptr<TcpStream> connect(const InetAddress&);

private:
    Socket __sock;
};


#endif /*TTCP_TCP_STREAM_H*/