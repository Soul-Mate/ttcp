package main

import (
	"flag"
	"fmt"
	"log"
	"net"
	"time"
)

var (
	client = flag.String("t", "server", "client or server")
	host = flag.String("host", "127.0.0.1", "listen host (default 127.0.0.1)")
	port = flag.Int("port", 19999, "listen port (default 19999)")
	number = flag.Int("number", 1000, "buffer count (default 1000)")
	length = flag.Int("length", 102400, "buffer length (default 102400)")
)

func main() {
	flag.Parse()
	if *client == "server" {
		ln, err := net.Listen("tcp", fmt.Sprintf("%s:%d", *host, *port))
		if err != nil {
			log.Fatal(err)
		}
	
		conn, err := ln.Accept()
		if err != nil {
			log.Fatal(err)
		}
	
		receive(conn)
		
	} else {
		conn, err := net.Dial("tcp", fmt.Sprintf("%s:%d", *host, *port))
		if err != nil {
			log.Fatal(err)
		}

		transmit(conn, *number, *length)
	}
}

func receive(conn net.Conn) {
	defer conn.Close()
	sessionMsg := NewSessionMessage(0, 0)
	err := sessionMsg.Read(conn)
	if err != nil {
		log.Fatal(err)
	}

	totalMib := float64(1.0 * sessionMsg.Number * sessionMsg.Length  / 1024 / 1024)
	fmt.Printf("receive buffer length = %d\nreceive number of buffers = %d\n", sessionMsg.Length, sessionMsg.Number)
	fmt.Printf("%.3f MiB in total\n", totalMib)

	payloadMsg := NewPayloadMessage()
	var i uint32
	start := time.Now()
	for i = 0; i < sessionMsg.Number; i++ {
		if err = payloadMsg.ReadBySession(conn, sessionMsg); err != nil {
			log.Fatal(err)
		}

		if err = payloadMsg.WriteAck(conn); err != nil {
			log.Fatal(err)
		}
	}
	elasped := time.Since(start)

	fmt.Printf("%.3f seconds\n%.3f MiB/s\n", elasped.Seconds(), totalMib/elasped.Seconds())
}

func transmit(conn net.Conn, number, length int) {
	defer conn.Close()
	sessionMsg := NewSessionMessage(uint32(number), uint32(length))
	err := sessionMsg.Write(conn)
	if err != nil {
		log.Fatal(err)
	}

	payloadMsg := NewPayloadMessage()
	payloadMsg.Fill(uint32(length))
	for i := 0; i < number; i++ {
		if err = payloadMsg.Write(conn); err != nil {
			log.Fatal(err)
		}

		ack, err := payloadMsg.ReadAck(conn)
		if err != nil {
			log.Fatal(err)
		}

		if ack != payloadMsg.Length {
			log.Fatal("ack error, server tall ack length not equal send payload message length")
		}
	}
}
