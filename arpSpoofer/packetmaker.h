#include "arphelper.h" 
#include "ethernethelper.h"

#ifndef PACKET_MAKER_H 
#define PACKET_MAKER_H
typedef struct arpPacket{
	ethernetHeader etherh; 
	arpHeader arph; 
}arpPacket; 

bool sendArp(pcap_t* handle, const uint8_t* sender_mac, const uint8_t* sender_ip, const uint8_t* target_mac, const uint8_t* target_ip, bool isRequest) {
	arpPacket _arpPacket;

	ethernetHeader* _eth = &(_arpPacket.etherh);  
	memcpy(_eth->sourceMacAddr, sender_mac, 6);
	
	//check unknown 
	uint8_t tmp[6] = {0,};  
	if(memcmp(target_mac,tmp,6) == 0)   
		memset(_eth->destinationMacAddr, 0xff,6); 
	else 
		memcpy(_eth->destinationMacAddr, target_mac, 6); 
	_eth->type = htons(ARP); 
	
	arpHeader* _arp = &(_arpPacket.arph); 
	_arp->hardwareType = ntohs(0x1); 
	_arp->protocolType = ntohs(0x0800); 
	_arp->hardwareLength = 6; 
	_arp->protocolLength = 4; 
	if(isRequest)
	_arp->operation = ntohs(0x1); 
	else 
	_arp->operation = ntohs(0x2); 	

	memcpy(_arp->senderMacAddr, sender_mac, 6); 
	memcpy(_arp->senderIpAddr, sender_ip, 4); 
	memcpy(_arp->recvMacAddr, target_mac, 6); 
	memcpy(_arp->recvIpAddr, target_ip, 4); 
  /*print packet in raw data for debugging 
	printf("size is = %d\n", sizeof(arpPacket)); 
	unsigned char* packet = (unsigned char*) malloc(sizeof(unsigned char)*sizeof(arpPacket));  
	memcpy(packet, &_arpPacket, sizeof(arpPacket)); 

	for(int i= 0; i < sizeof(arpPacket); i++){
		printf("%02x ", packet[i]); 
		fflush(stdout);  
	} */
	
	pthread_mutex_lock(&mutex); 
	int res = (pcap_inject(handle, (u_char*)&_arpPacket, sizeof(arpPacket)) == sizeof(arpPacket));
  pthread_mutex_unlock(&mutex); 
	return res; 
}
#endif
