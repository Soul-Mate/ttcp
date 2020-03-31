#include "inet_address.h"
#include "common.h"
#include <iostream>
#include <exception>
int main()
{
    try {
        StringArg ip("127.0.0");
        // std::cout << ip.get_c_str() << std::endl;
        InetAddress addr(ip, 9999);
    } catch(InetAddressException& e) {
        std::cout << e.what() << std::endl;
    }
}