#ifndef ARPHELPER_H
#define ARPHELPER_H
#include "util.h"

typedef struct arpHeader{
    uint16_t hardwareType;
    uint16_t protocolType;
    uint8_t hardwareLength;
    uint8_t protocolLength;
    uint16_t operation;
    uint8_t senderMacAddr[6];
    uint8_t senderIpAddr[4];
    uint8_t recvMacAddr[6];
    uint8_t recvIpAddr[4];
} arpHeader;

int readArp(const u_char* packet, arpHeader* arp);
void printOPcode(arpHeader* arp);
void printArp(arpHeader* arp);

int readArp(const u_char* packet, arpHeader* arp){
    arp->hardwareType = my_ntohs(packet);
    arp->protocolType = my_ntohs(packet+2);
    arp->hardwareLength = packet[4];
    arp->protocolLength = packet[5];
    arp->operation = my_ntohs(packet+6);
    memcpy(arp->senderMacAddr, packet+8, 6);
    memcpy(arp->senderIpAddr, packet+14, 4);
    memcpy(arp->recvMacAddr, packet+18, 6);
    memcpy(arp->recvIpAddr, packet+24, 4);
    return 28;
}

void printOPcode(arpHeader* arp){
    if(arp->operation == 1)
        printf("Request");
    else if(arp -> operation == 2)
        printf("Reply");
    else
        printf("ERROR");
    printf("(%d)\n", arp->operation);
}

void printArp(arpHeader* arp){
    printf("\t===== arp header ===== \n");
    printf("\top code : ");
    printOPcode(arp);
    printHexValue("\tsource mac : ", arp->senderMacAddr,6, ':');
    printDecValue("\tsource ip : ", arp->senderIpAddr, 4, '.');
    printHexValue("\tdest mac : ", arp->recvMacAddr, 6, ':');
    printDecValue("\tdest ip : ", arp->recvIpAddr, 4, ':');
}
#endif
