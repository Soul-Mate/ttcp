#include "accept.h"

Acceptor::Acceptor(const InetAddress& saddr) 
 : __listenSock(std::move(Socket::createTCP()))
{
    __listenSock.setRefuse(true);
    __listenSock.bind(saddr);
    __listenSock.listen(128);
}

std::unique_ptr<TcpStream> Acceptor::accept()
{
    socklen_t peerLen;
    int peerfd = ::accept(__listenSock.get_sockfd(), (sockaddr*)&__peer.getSockAddrNonConst(), &peerLen);
    if (peerfd < 0) {
        perror("Acceptor::accept");
        
    } else {
        return std::unique_ptr<TcpStream>(new TcpStream(Socket(peerfd)));
    }
}