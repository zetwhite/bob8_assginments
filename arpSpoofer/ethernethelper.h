#ifndef ETHERNETHELPER_H
#define ETHERNETHELPER_H
#include "util.h"

#define etherVerdelim 0x05DC

#define ethernetV2 1
#define ethernetIEEE802 2

#define ARP 0x0806
#define IP 0x0800


typedef struct ethernetHeader{
    uint8_t destinationMacAddr[6];
    uint8_t sourceMacAddr[6];
    int16_t type;
} ethernetHeader;

int readEthernet(const u_char* packet, void** ether, int32_t* version);
void printType(ethernetHeader* ether);
void printEthernet(ethernetHeader* ether);
#endif // ETHERNETHELPER_H
