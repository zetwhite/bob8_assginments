#ifndef UTILSET_H
#define UTILSET_H
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <sys/socket.h> 
#include <sys/ioctl.h> 
#include <net/if.h> 
#include <netinet/in.h> 
#include <netinet/ip.h>
#include <arpa/inet.h> 
#include <netinet/ether.h> 
#include <netdb.h>
#include <unistd.h> 
#include <stdbool.h>  
#include <pthread.h> 
#endif

#ifndef UTIL_H
#define UTIL_H

typedef struct sessionInfo{
  uint8_t senderIp[4];
	uint8_t senderMac[6];
	uint8_t targetIp[4];
	uint8_t targetMac[6];
} sessionInfo;


//====== pthread global =========
pthread_mutex_t mutex;
int* threadSignal;
sessionInfo* sessions;  
pcap_t* handle;
int sessionSize;
int* sessionNumber; 
uint8_t ip[4];
uint8_t mac[6];
//===============================

uint16_t my_ntohs(const u_char* num);
uint32_t my_ntohl(const u_char* num);
void printHexValue(const char* msg, uint8_t* start, int32_t size, const char delim);
void printDecValue(const char* msg, uint8_t* start, int32_t size, const char delim);
bool getIp(uint8_t ip[4], char* dev); 
bool getMac(uint8_t mac[6], char* dev); 
bool getgateway(uint8_t addr[4]); 
void usage(); 
//int basicSetting(int argc, char* argv[]); 
void setSignal(int num); 
void resetSignal(int num); 
#endif
