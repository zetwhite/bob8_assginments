#ifndef TCPHELPER_H
#define TCPHELPER_H
#include "util.h"

typedef struct tcpHeader{
    uint16_t sourcePort;
    uint16_t destPort;
    uint32_t seqNumber;
    uint32_t ackNumber;
    uint8_t headerLength;
    uint8_t reserved;
    uint8_t TCPFLAGS;
    uint16_t window;
    uint16_t checksum;
} tcpHeader;

int readTcp(const u_char* packet, tcpHeader* tcp){
    tcp->sourcePort = my_ntohs(packet);
    tcp->destPort = my_ntohs(packet + 2);
    tcp->seqNumber = my_ntohl(packet + 4);
    tcp->ackNumber = my_ntohl(packet + 8);
    tcp->headerLength = packet[12]>>4;
    tcp->reserved = my_ntohs(packet+12) & 0xFC0;
    tcp->TCPFLAGS = my_ntohs(packet+12) & 0x3F;
    tcp->window = my_ntohs(packet + 14);
    tcp->checksum = my_ntohs(packet + 16);
    return tcp->headerLength* 4;
}

void printTcp(tcpHeader* tcp){
    printf("\t===== tcp header =====\n");
    printf("\tsource port : %u\n", tcp->sourcePort);
    printf("\tdest port : %u\n", tcp->destPort);
    //printf("\tseq number : %u\n", tcp->seqNumber);
    //printf("\tack number : %u\n", tcp->ackNumber);
}
#endif // TCPHELPER_H
