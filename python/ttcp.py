import socket
import sys
import struct
import time

class AckMessage():
    def size(self):
        return 4
    
    def pack(self, value):
        return struct.pack('=I', socket.htonl(value))
    
    def unpack(self, data):
        return socket.ntohl(struct.unpack('=I', data)[0])
    
class SessionMessage():
    def __init__(self, number = 0, length = 0):
        self.number = number
        self.length = length
    
    def pack(self):
        return struct.pack('=2I', socket.htonl(self.number), socket.htonl(self.length))

    def unpack(self, data):
        self.number = socket.ntohl(struct.unpack('=I', data[:4])[0])
        self.length = socket.ntohl(struct.unpack('=I', data[4:8])[0])

    def message_length(self):
        return 8

class PayloadMessage():
    def __init__(self, length = 0):
        self.data = b''
        for i in range (length):
            self.data += "0123456789ABCDEF"[i%16].encode()
        self.length = length
    
    def pack(self):
        return struct.pack('=I%ds' % self.length, socket.htonl(self.length), self.data) 

    def unpack_length(self, data):
        self.length = socket.ntohl(struct.unpack('=I', data[:4])[0])

    def length_size(self):
        return 4

    def message_length(self):
        return 4 + self.length

    def unpack_ack(self, ack_data):
        return socket.ntohl(struct.unpack('=I', ack_data)[0])

class TcpStream():
    def __init__(self, so):
        self.so = so
    
    def send_all(self, buf, n):
        sendn = 0
        nw = 0
        while sendn < n:
            nw = self.so.send(buf[sendn:n])
            if nw == 0:
                return 0
            
            if nw < 0:
                return -1

            if nw > 0:
                sendn += nw
        
        return sendn

    def recv_all(self, n):
        return self.so.recv(n, socket.MSG_WAITALL)

    def close(self):
        self.so.close()

def transmit(server_socket, number, length):
    # send session message
    sessionMsg = SessionMessage(number, length)
    server_socket.send(sessionMsg.pack())
    
    # paylaod message
    ackMsg = AckMessage()
    stream = TcpStream(server_socket)
    payloadMsg = PayloadMessage(length)
    
    for _ in range(number):
        # send paylaod message to server
        nw = stream.send_all(payloadMsg.pack(), payloadMsg.message_length())
        if nw < 0:
            print ("stream.send_all payload message fail")
            stream.close()
            return

        if nw == 0:
            print ("stream.send_all payload message fail, server closed.")
            stream.close()
            return

        if nw != payloadMsg.message_length():
            print("stream.send_all payload message fail, actually need send: %d, but strem send: %d" % (payloadMsg.message_length(), nw))
            stream.close()
            return
       
        # recv ack message from server
        data = stream.recv_all(ackMsg.size())
        if len(data) == 0:
            print("stream.recv_all ack message fail,server closed")
            stream.close()
            return
        ack = ackMsg.unpack(data)
        if ack != length:
            print("check ack fail, actually need send length: %d, but server ack recv length: %d" % (length, ack))
            stream.close()
            return

    stream.close()

def receive(server_socket, client_socket):
    # recv session message and unpack from transmit
    sessionMsg = SessionMessage()
    stream = TcpStream(client_socket)
    data = stream.recv_all(sessionMsg.message_length())
    if len(data) != sessionMsg.message_length():
        stream.close()
        print("stream.recv_all error, need recv: %d, give: %d" % sessionMsg.message_length(), len(data))
        return
    
    sessionMsg.unpack(data)

    print("receive buffer length = %d\nreceive number of buffers = %d" % (sessionMsg.length, sessionMsg.number))
    totalMib = sessionMsg.number * sessionMsg.length * 1.0 / 1024 / 1024
    print("%.3f MiB in total" % (totalMib))

    ackMsg = AckMessage()

    # recv payload message and unpack from transmit
    payloadMsg = PayloadMessage()
    start_time = time.time()
    for _ in range (sessionMsg.number):
        # recv payload message length data and unpack it
        data = stream.recv_all(payloadMsg.length_size())
        nr = len(data)
        if nr == 0:
            print("client closed.")
            stream.close()
            return
        
        if nr < 0:
            print("stream.recv_all payload message length data fail.")
            stream.close()
            return
        
        if nr != payloadMsg.length_size():
            print("stream.recv_all payload message length data fail.")
            stream.close()
            return
        
        payloadMsg.unpack_length(data)

        # recv payload message data data and unpack it
        data = stream.recv_all(payloadMsg.length)
        nr = len(data)
        if nr == 0:
            print("client closed.")
            stream.close()
            return

        if nr < 0:
            print("stream.recv_all payload message data data fail.")
            stream.close()
            return

        if nr != payloadMsg.length:
            print("stream.recv_all payload message data data fail, need %d length data, give %d" % payloadMsg.length, nr)

        # send ack of recive data length to client
        nw = stream.send_all(ackMsg.pack(payloadMsg.length), ackMsg.size())
        if nw == 0:
            print("stream.send_all ack message fail, client closed")
            stream.close()
            return
        
        if nw < 0:
            print("stream.send_all ack message fail")
            stream.close()
            return

        if nw != ackMsg.size():
            print("stream.send_all ack message fail, need %d send, give %d" % nw, ackMsg.size())
            stream.close()
            return

    end_time = time.time()
    elapsed = end_time - start_time
    print("%.3f seconds\n%.3f MiB/s\n" % (elapsed, totalMib / elapsed))
    stream.close()

def main(argv):
    if len(argv) < 3:
        print ("Usage:\n -t (client or server)")
        return

    if argv[1] != "-t":
        print ("Usage:\n -t (client or server)")
        return

    if argv[2] == "server":
        host = ""
        port = 0
        for i in range(len(argv)):
            if argv[i] == "-h" and i + 1 < len(argv):
                host = argv[i+1]
            if argv[i] == "-p" and i + 1 < len(argv):
                port = int(argv[i+1])

        if host == "":
            print ("Usage:\n -t server -h host")
            return

        if port == 0:
            print ("Usage:\n -t server -h %s -p port" % (host))
            return

        print ("ttcp listen server %s:%d" % (host, port))
        
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.TCP_NODELAY, 1)
        server_socket.bind((host, port))
        server_socket.listen(5)
        (client_socket, client_address) = server_socket.accept()
        receive(server_socket, client_socket)
        server_socket.close()
  
    if argv[2] == "client":
        host = ""
        port = 0
        number = 1000
        length = 1024
        for i in range(len(argv)):
            if argv[i] == "-h" and i + 1 < len(argv):
                host = argv[i+1]
            if argv[i] == "-p" and i + 1 < len(argv):
                port = int(argv[i+1])
            if argv[i] == "-n" and i + 1 < len(argv):
                number = int(argv[i+1])
            if argv[i] == "-b" and i + 1 < len(argv):
                length = int(argv[i+1])

        if host == "":
            print ("Usage:\n -t client -h host")
            return

        if port == 0:
            print ("Usage:\n -t client -h %s -p port" % (host))
            return
        
        print ("ttcp connection server %s:%d:\n  message number: %d count\n  message length: %d byte\n  total: %.3f Mib" % (host, port, number, length, 
            float(1.0 * number * length / 1024 / 1024)) )

        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.TCP_NODELAY, 1)
        server_socket.connect((host, port))
        transmit(server_socket, number, length)

if __name__ == "__main__":
    main(sys.argv)