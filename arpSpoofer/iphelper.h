#ifndef IPHELPER_H
#define IPHELPER_H
#include <pcap.h>
#include "util.h"

#define ICMP 1
#define TCP 6
#define UDP 17

typedef struct ipHeaderV4{
    uint8_t version;
    uint8_t headerLength;
    uint8_t typeOfService;
    uint16_t totalLength;
    uint16_t identification;
    bool flag[3];
    uint16_t fagmentationOffset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint8_t sourceIpAddr[4];
    uint8_t destIpAddr[4];
} ipHeaderV4;

int readIp(const u_char* packet, ipHeaderV4* ip);
void printProtocol(ipHeaderV4* ip);
void printIp(ipHeaderV4* ip);

int readIp(const u_char* packet, ipHeaderV4* ip){
    ip->version = packet[0] >> 4;
    ip->headerLength = packet[0] & 0x0f ;
    ip->typeOfService = packet[1];
    ip->totalLength = my_ntohs(packet+2);
    ip->identification = my_ntohs(packet + 4);
    ip->flag[0] = packet[6] & 0b10000000;
    ip->flag[1] = packet[6] & 0b01000000;
    ip->flag[2] = packet[6] & 0b00100000;
    ip->fagmentationOffset = my_ntohs(packet+6)&0x1FFF;
    ip->ttl = packet[8];
    ip->protocol = packet[9];
    ip->checksum = my_ntohs(packet+10);
    memcpy(ip->sourceIpAddr, packet+12, 4);
    memcpy(ip->destIpAddr, packet+16, 4);
    return ip->headerLength*4;
}

void printProtocol(ipHeaderV4* ip){
    if(ip->protocol == ICMP)
        printf("ICMP");
    else if(ip->protocol == TCP)
        printf("TCP");
    else if(ip->protocol == UDP)
        printf("UDP");
    printf("(%d)\n", ip->protocol);
}

void printIp(ipHeaderV4* ip){
    printf("\t===== ip header =====\n");
    printf("\tprotocol : ");
    printProtocol(ip);
    printf("\tip header size : %d\n", ip->headerLength);
    printDecValue("\tsource ip : ", ip->sourceIpAddr, 4, '.');
    printDecValue("\tdest ip : ", ip->destIpAddr, 4, '.');
}
#endif // IPHELPER_H
