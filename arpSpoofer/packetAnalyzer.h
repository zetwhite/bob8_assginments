#include "util.h" 
#include "arphelper.h"
#include "ethernethelper.h" 
#include "iphelper.h" 
#include "tcphelper.h"
#include "udphelper.h"

#ifndef PA_H
#define PA_H
#define NO 0 
int isArp(struct pcap_pkthdr* header, const u_char* packet){
	int32_t index = 0;
	int32_t version = 0;

	void* ether;
	ethernetHeader* etherHeader;
	index = readEthernet(packet, &ether, &version);
	if(index >= 0 && version == ethernetV2){
		etherHeader = (ethernetHeader*)ether;
		version = etherHeader->type;
	}
	if(version == ARP)
		return index; 
	else 
	 	return -1; 
}

bool isArpReply(uint8_t sender_mac[6], uint8_t sender_ip[4], struct pcap_pkthdr* header, const u_char* packet){
	int32_t index = 0;
	if((index = isArp(header, packet)) != -1){
	 	arpHeader arpHeader;
   	index += readArp(packet+index, &arpHeader);
		if(memcmp(arpHeader.senderIpAddr, sender_ip,4) == 0 && arpHeader.operation == 0x2){
			memcpy(sender_mac, arpHeader.senderMacAddr, 6);
			return true;  
		}   
  }
	return false;
}

int ethernetSourceCheck(uint8_t* get_addr){
	for(int i= 0; i < sessionSize; i++){
		if(memcmp(get_addr, sessions[i].senderMac, 6)==0) 
			return i; 
	}
	return -1; 
}

int arpCheck(uint8_t* targetIp){
	for(int i = 0; i < sessionSize; i++)
		if(memcmp(targetIp, sessions[i].targetIp, 4)==0) 
			return i; 
	return -1; 
}

int RelayAnalysis(struct pcap_pkthdr* header, const u_char* packet, int* sigNum){
	int32_t index = 0; 
	int version = 0; 
	void* ether; 
	ethernetHeader* etherHeader; 
	index = readEthernet(packet, &ether, &version);
	if(index >= 0 && version == ethernetV2){
		etherHeader = (ethernetHeader*) ether; 
		printEthernet(etherHeader); 
	}
	if((*sigNum = ethernetSourceCheck(etherHeader->sourceMacAddr))== -1) 
		return -1; // just pass it! may be need infection, 	

	if(etherHeader->type == ARP){
		arpHeader _arp; 
		index += readArp(packet+index, &_arp);
		if((*sigNum = arpCheck(_arp.recvIpAddr))!=-1){
			return -2; //hurry up! need infection!
		}
	}
	//check dest ip 
	else if(etherHeader->type == IP){
		ipHeaderV4 ip; 
		index += readIp(packet+index, &ip); 
		printIp(&ip); 
	}
	return *sigNum; 
}

void analyzePacket(struct pcap_pkthdr* header, const u_char* packet){
    printf("========================================\n");
    printf("============ PACKET CAPTURED ===========\n");
    printf("========================================\n");
    printf("packet total size : %u\n", header->caplen);
    int32_t index = 0;
    int32_t version = 0;

    //----------------datalink layer---------------------
    void* ether;
    ethernetHeader* etherHeader;
    index = readEthernet(packet, &ether, &version);
    if(index >= 0 && version == ethernetV2){
        etherHeader = (ethernetHeader*)ether;
        version = etherHeader->type;
        printEthernet(etherHeader);
    }
    else{
        printf("\tsorry. Ethernet IEEE802.3 is not provided\n");
        return;
    }

    //----------------network layer---------------------
    if(version == ARP){
        arpHeader arpHeader;
        index += readArp(packet+index, &arpHeader);
        version = NO;
        printArp(&arpHeader);
        return;
    }
    else if(version == IP){
        ipHeaderV4 ipHeader;
        index += readIp(packet+index, &ipHeader);
        version = ipHeader.protocol;
        printIp(&ipHeader);
    }
    else {
        printf("\tsorry. this protocol is not provided\n");
    }
   //--------------transport layer---------------------
    if(version == TCP){
        tcpHeader tcp;
        index += readTcp(packet + index, &tcp);
        printTcp(&tcp);
    }
    else if(version == UDP){
        udpHeader udp;
        index += readUdp(packet + index, &udp);
        printUdp(&udp);
    }
    else {
        printf("\tsorry, this protocol is not provided\n");
    }

    //------------ data ----------------------------------
    int dataSize = header->caplen - index;
    if(dataSize > 10) dataSize = 10;
    printf("\t===== data =====\n\t");
    printf("length : %d\n\t", dataSize);
    for(int i = 0; i < dataSize; i++)
        printf("%02x ", packet[index + i]);
    printf("\n");
}
#endif 
