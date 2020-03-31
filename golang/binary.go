package main

import (
	"encoding/binary"
	"fmt"
	"io"
	"net"
)

type SessionMessage struct {
	Number uint32
	Length uint32
}

func NewSessionMessage(number, length uint32) *SessionMessage {
	return &SessionMessage{
		Number: number,
		Length: length,
	}
}

func (s *SessionMessage) Read(conn net.Conn) error {
	bufsize := 8
	buf := make([]byte, bufsize)
	readn := 0
	for readn < bufsize {
		nr, err := conn.Read(buf[readn:bufsize])
		if err != nil {
			if err == io.EOF {
				return fmt.Errorf("read SessionMessage fail, client closed")
			}

			panic(err)
		}

		if nr > 0 {
			readn += nr
		}
	}

	s.Number = binary.BigEndian.Uint32(buf[:4])
	s.Length = binary.BigEndian.Uint32(buf[4:])
	return nil
}

func (s *SessionMessage) Write(conn net.Conn) error {
	session := make([]byte, 8)
	binary.BigEndian.PutUint32(session[:4], s.Number)
	binary.BigEndian.PutUint32(session[4:], s.Length)
	nw, err := conn.Write(session)
	if err != nil {
		panic(err)
	}

	if nw != 8 {
		return fmt.Errorf("write session message to server error")
	}

	return nil
}

type PayloadMessage struct {
	Length uint32
	Data   []byte
}

func NewPayloadMessage() *PayloadMessage {
	return new(PayloadMessage)
}

func (p *PayloadMessage) Fill(length uint32) {
	p.Data = make([]byte, 4 + length)
	p.Length = length
	var i uint32
	for i = 0; i < p.Length; i++ {
		p.Data[4+i] = "0123456789ABCDEF"[i%16]
	}

	binary.BigEndian.PutUint32(p.Data[:4], p.Length)
}

func (p *PayloadMessage) ReadBySession(conn net.Conn, msg *SessionMessage) error {
	buf := make([]byte, 4)
	nr, err := conn.Read(buf)
	if err != nil {
		if err == io.EOF {
			return err
		}

		panic(err)
	}

	if nr == 0 {
		return fmt.Errorf("read PayloadMessage Length error, client closed")
	}

	p.Length = binary.BigEndian.Uint32(buf)
	if p.Length != msg.Length {
		return fmt.Errorf("read PayloadMessage Length error, payload message length %d != session message length %d", p.Length, msg.Length)
	}

	readn, needn := 0, int(p.Length)
	buf = make([]byte, p.Length)
	for readn < needn {
		nr, err = conn.Read(buf)
		if err != nil {
			if err == io.EOF {
				return err
			}
		}

		readn += nr
	}

	if readn != needn {
		return fmt.Errorf("read PayloadMessage data error, need %d, give %d", needn, readn)
	}

	return nil
}

func (p *PayloadMessage) Write(conn net.Conn) error {
	needn := 4 + int(p.Length)
	writen := 0
	for writen < needn {
		nw, err := conn.Write(p.Data[writen:needn])
		if err != nil {
			panic(err)
		}

		writen += nw
	}

	if writen != needn {
		return fmt.Errorf("write payload message error")
	}

	return nil
}

func (p *PayloadMessage) WriteAck(conn net.Conn) error {
	ack := make([]byte, 4)
	binary.BigEndian.PutUint32(ack, p.Length)
	nr, err := conn.Write(ack)
	if err != nil {
		panic(err)
	}

	if nr != 4 {
		return fmt.Errorf("write ack to client error, need write 4 != %d", nr)
	}

	return nil
}

func (p *PayloadMessage) ReadAck(conn net.Conn) (uint32, error) {
	ack := make([]byte, 4)
	nr, err := conn.Read(ack)
	if err != nil {
		panic(err)
	}

	if nr != 4 {
		return 0, fmt.Errorf("write ack to client error, need write 4 != %d", nr)
	}

	return binary.BigEndian.Uint32(ack), nil
}