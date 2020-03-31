#include "tcp_stream.h"

TcpStream::TcpStream(Socket&& socket)
 : __sock(std::move(socket))
{}

int TcpStream::receiveAll(void *buf, size_t len)
{
    int read_n = recv(__sock.get_sockfd(), buf, len, MSG_WAITALL);
    if (read_n == 0) {
        // TODO对端关闭
    }
    if (read_n < 0) {
        perror("TcpStream::receiveAll");
        abort();
    }

    return read_n;
}

int TcpStream::sendAll(const void* buf, size_t len)
{
    int write_n = 0;
    int wn = 0;
    while (write_n < len) {
        wn = send(__sock.get_sockfd(), static_cast<const char*>(buf) + write_n, len, 0);
        if (wn == -1) {
            perror("TcpStream::sendAll");
            abort();
        }

        if (wn == 0) {
            break;
        }

        write_n += wn;
    }
}

std::unique_ptr<TcpStream> TcpStream::connect(const InetAddress& serverAddr) 
{
    Socket s(std::move(Socket::createTCP()));
    s.connect(serverAddr);
    
    return std::unique_ptr<TcpStream>(new TcpStream(std::move(s)));
}