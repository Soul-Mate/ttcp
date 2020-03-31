#ifndef TTCP_MESSAGE_H
#define TTCP_MESSAGE_H

struct SessionMessage
{
    int32_t number;
    int32_t length;
} __attribute__ ((__packed__));

struct PayloadMessage
{
    uint32_t length;
    char data[0];
};


#endif /* TTCP_MESSAGE_H */
