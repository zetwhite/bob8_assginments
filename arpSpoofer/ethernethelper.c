#include "ethernethelper.h"

int readEthernet(const u_char* packet, void** ether, int32_t* version){
    int16_t type = my_ntohs(packet+12);
    if(type > etherVerdelim){
        *ether = malloc(sizeof(ethernetHeader));
        *version = ethernetV2;
    }
    else{
        *version = ethernetIEEE802;
        //printf("\tethernetIEEE 802.3 is not supported\n");
        return -1;
    }
    ethernetHeader* etherT = (ethernetHeader*) *ether;
    memcpy((etherT)->destinationMacAddr, packet, 6);
    memcpy((etherT)->sourceMacAddr, packet+6,6);
    etherT->type = type;
    return 14;
}

void printType(ethernetHeader* ether){
    if(ether->type == IP)
        printf("IP");
    else if(ether->type == ARP)
        printf("ARP");
    else
        printf("unknown");
    printf("(%04x)\n", ether->type);
}

void printEthernet(ethernetHeader* ether){
    if(ether->type >= etherVerdelim){
        printf("\t====== ethernet v2 ======\n");
        printHexValue("\tdest mac : ", ether->destinationMacAddr, 6, ':');
        printHexValue("\tsource mac : ", ether->sourceMacAddr, 6, ':');
        printf("\ttype : ");
        printType(ether);
    }
    else{
        printf("\t===== ethernet IEEE802.3 =====\n");
        printf("\tsorry... this format is not available\n");
    }
}
