#include <iostream>
#include <string>
#include <chrono>
#include "common.h"
#include "message.h"
#include "accept.h"
#include "inet_address.h"

using std::cout;
using std::endl;
using std::string;

struct commandLine
{
   StringArg host;
   StringArg port;
   int bsNumber;
   int bsLength; 
   bool isClinet;
   commandLine() :bsNumber(0), bsLength(0), isClinet(false) {}
   ~commandLine() = default;
};

commandLine* parseCommandLine(int& argc, const char* argv[]) 
{
   if (argc < 3) {
      cout << "Usage -t client/server" << endl;
      return nullptr;
   }

   if (strcmp(argv[1], "-t") != 0) {
      cout << "Usage -t [client/server]" << endl;
      return nullptr;
   }

   if (strcmp(argv[2], "client") == 0) {
      if (argc < 6) {
         cout << "Usage -t client -s host -p port (default -n 1000 -b 1024)" << endl;
         return nullptr;
      }

      
      StringArg host, port;
      int bsCount = 0, bsLength = 0;
      for (int i = 3; i < argc; i++) {
         if (strcmp(argv[i], "-s") == 0 && i+ 1 < argc) {
            host = argv[++i];
         } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = argv[++i];
         } else if (strcmp(argv[i], "-n") == 0 && i+ 1 < argc) {
            bsCount = atoi(argv[++i]);
         } else if (strcmp(argv[i], "-b") == 0 && i+ 1 < argc) {
            bsLength = atoi(argv[++i]);
         }
      }

      if (port.empty()) {
         cout << "-p port argument empty" << endl;
         return nullptr;
      }

      if (host.empty()) {
         host = "0.0.0.0"; // INADDR_ANY
      }

      if (bsCount == 0) {
         bsCount = 1000;
      }

      if (bsLength == 0) {
         bsLength = 1024;
      }

      struct commandLine* line = new commandLine;
      line->host = host;
      line->port = port;
      line->bsNumber = bsCount;
      line->bsLength = bsLength;
      line->isClinet = true;

      cout << "host: " << line->host.get_c_str()
         << ", port: " << line->port.get_c_str()
         << ", bs count: " << line->bsNumber 
         << ", bs length: " << line->bsLength
         << ", is client: " << line->isClinet << endl;

      return line;
   }
   
   if (strcmp(argv[2], "server") == 0) {
      if (argc < 6) {
         cout << "Usage -t client -s host -p port (default -n 1000 -b 1024)" << endl;
         return nullptr;
      }
      StringArg host, port;
      for (int i = 3; i < argc; i++) {
         if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            host = argv[i+1];
         } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = argv[i+1];
         }
      }

      if (port.empty()) {
         cout << "-p port argument empty" << endl;
         return nullptr;
      }

      if (host.empty()) {
         host = "0.0.0.0"; // INADDR_ANY
      }

      struct commandLine* line = new commandLine;
      line->host = host;
      line->port = port;
      line->isClinet = false;
      cout << "host: " << line->host.get_c_str()
         << ", port: " << line->port .get_c_str()
         << ", is client: " << line->isClinet << endl;
      return line;
   }

   cout << "invalid -t argument" << endl;

   return nullptr;
}

void transmit(struct commandLine* linePtr) 
{
   try {
      InetAddress serverAddr(linePtr->host, static_cast<in_port_t>(linePtr->port.toi()));
      std::unique_ptr<TcpStream> stream = TcpStream::connect(serverAddr);
      struct SessionMessage sessionMessage {0,0};
      sessionMessage.number = htonl(linePtr->bsNumber);
      sessionMessage.length = htonl(linePtr->bsLength);

      stream->sendAll(&sessionMessage, sizeof sessionMessage);
   
      int payloadMessageLen = sizeof(uint32_t) + linePtr->bsLength;
      PayloadMessage* payloadMessage = static_cast<PayloadMessage*>(::malloc(payloadMessageLen)); 
      assert(payloadMessage);

      payloadMessage->length = htonl(linePtr->bsLength);
      for (int i = 0; i < linePtr->bsLength; i++) {
         payloadMessage->data[i] = "0123456789ABCDEF"[i % 16];
      }

      double totalMib = 1.0 * linePtr->bsNumber * linePtr->bsLength / 1024 / 1024;
      std::printf("%.3f MiB in total\n", totalMib);

      int nr, wr;
      uint32_t ack = 0;
      for (int i = 0; i < linePtr->bsNumber; i++) {
         wr = stream->sendAll(payloadMessage, payloadMessageLen);
         if (wr != payloadMessageLen) {
            perror("sendAll error");
            abort();
         }

         nr = stream->receiveAll(&ack, sizeof ack);
         if (nr != sizeof ack) {
            perror("receiveAll error");
            abort();
         }

         ack = ntohl(ack);
         if (ack != linePtr->bsLength) {
            perror("server receiveAll error, ack not euqal");
            abort();
         }
      }
   } catch (InetAddressException& e) {
      perror(e.what());
   } catch (SocketConnectException& e) {
      perror(e.what());
   }
}

void receive(struct commandLine* linePtr) 
{
   Acceptor acceptor(InetAddress(linePtr->host, 
      static_cast<in_port_t>(linePtr->port.toi())));
   
   std::unique_ptr<TcpStream> stream = acceptor.accept();

   struct SessionMessage sessionMessage{0, 0};
   int read_n = stream->receiveAll(&sessionMessage, sizeof sessionMessage);
   sessionMessage.number = ntohl(sessionMessage.number);
   sessionMessage.length = ntohl(sessionMessage.length);
   std::printf("receive buffer length = %d\nreceive number of buffers = %d\n",
         sessionMessage.length, sessionMessage.number);

   int payloadLen = sizeof(uint32_t) + sessionMessage.length;
   PayloadMessage *payloadMessage = static_cast<PayloadMessage*>(::malloc(payloadLen));

   double totalMib = 1.0 * sessionMessage.number  * sessionMessage.length / 1024 / 1024;
   std::printf("%.3f MiB in total\n", totalMib);
   
   int nr, wr;
   uint32_t ack;
   auto before = std::chrono::system_clock::now();
   for (int i = 0; i < sessionMessage.number; i++) {
      nr = stream->receiveAll(&payloadMessage->length, sizeof payloadMessage->length);
      if (nr != sizeof payloadMessage->length) {
         cout << "receive error" << endl;
      }

      payloadMessage->length = ntohl(payloadMessage->length);
      if (payloadMessage->length != sessionMessage.length) {
         cout << "receive PayloadMessage length error" << endl;
         return;         
      }

      nr = stream->receiveAll(payloadMessage->data,  payloadMessage->length);
      if (nr != payloadMessage->length) {
         cout << "receive error" << endl;
         return;         
      }

      ack = htonl(payloadMessage->length);
      wr = stream->sendAll(&ack, sizeof ack);
      if (wr != sizeof ack) {
          cout << "send ack error" << endl;
         return;  
      }
   }

   std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - before;
   std::printf("%.3f seconds\n%.3f MiB/s\n", elapsed, totalMib / elapsed.count());
}
// -t client -s host -p port -n 10 -b 1000
// -t server -s host -p port
int main(int argc, const char* argv[]) 
{
   
   struct commandLine* linePtr = parseCommandLine(argc, argv);
   if (linePtr == nullptr) {
      return 0;
   }
   
   if (linePtr->isClinet) {
      transmit(linePtr);
   }
   
   if (!linePtr->isClinet) {
      receive(linePtr);
   }
   
   delete linePtr;
}