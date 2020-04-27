#ifndef UDPHELPER_H
#define UDPHELPER_H
#include "util.h"

typedef struct udpHeader{
    uint16_t sourcePort;
    uint16_t destPort;
    uint16_t length;
    uint16_t checksum;
} udpHeader;

int readUdp(const u_char* packet, udpHeader* udp){
    udp->sourcePort = my_ntohs(packet);
    udp->destPort = my_ntohs(packet + 2);
    udp->length = my_ntohs(packet + 4);
    udp->checksum = my_ntohs(packet + 6);
    return 8;
}

void printUdp(udpHeader* udp){
    printf("\t===== udp ===== \n");
    printf("\tsource port : %u\n", udp->sourcePort);
    printf("\tdestination port : %u\n", udp->destPort);
    return;
}

#endif // UDPHELPER_H
