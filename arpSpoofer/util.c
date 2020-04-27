#include "util.h"

uint16_t my_ntohs(const u_char* num) {
   // int _num = *num;
    uint16_t _num = *(const uint16_t*)num;
    return _num << 8 | _num >> 8;
}

uint32_t my_ntohl(const u_char* num) {
    uint32_t _num = *(const uint32_t*)num;
    return _num << 24 | _num >> 24 | (_num&0xff00) << 8 | (_num&0xff0000) >> 8;
}

void my_mac_converter(uint8_t* mac){
	for(int i =0 , j = 6; i < 3, j >=3; i++, j--){
		uint8_t tmp = mac[i]; 
		mac[i] = mac[j];
		mac[j] = tmp;  
	}
}

void printHexValue(const char* msg, uint8_t* start, int32_t size, const char delim){
  printf("%s", msg);
  for(int i = 0; i < size-1; i++)
    printf("%02x%c", start[i], delim);
  printf("%02x\n", start[size-1]);
}

void printDecValue(const char* msg, uint8_t* start, int32_t size, const char delim){
  printf("%s", msg);
  for(int i = 0; i <size-1; i++)
    printf("%d%c", start[i], delim);
  printf("%d\n", start[size-1]);
}

bool getIp(uint8_t ip[4], char* dev){
	int sock; 
	struct ifreq ifr; 
	struct sockaddr_in *sin; 
	sock = socket(AF_INET, SOCK_STREAM, 0); 
	if(sock < 0){
		printf("util.h _ getIp _ socket error\n"); 
		return false;  
	}

	strcpy(ifr.ifr_name, dev); 
	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0){
		printf("util.h _ getIp _ iotcl error\n"); 
		return false; 
	}
	sin = (struct sockaddr_in*)&ifr.ifr_addr; 
	memcpy(ip, &(sin->sin_addr), 4); 
	return true; 
}

bool getMac(uint8_t mac_address[6],char* dev){
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) { printf("socek error\n");};

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { printf("getmac_ erro set ioctl\n");  }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }
        else { printf("getmac _ error ioctl\n"); }
    }


    if (success) memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
    if (success) return true; 
}


bool getgateway(uint8_t addr[4]){
	long destination, gateway; 
	char iface[IF_NAMESIZE]; 
	char buf[4096]; 
	FILE* file; 

	memset(iface, 0, sizeof(iface)); 
	memset(buf, 0, sizeof(buf)); 
	
	file = fopen("/proc/net/route", "r"); 
	if(file < 0){
		printf("util.h _ getgatewat _ file open error");
		return 0;  
	}

	while(fgets(buf, sizeof(buf), file)){
		if(sscanf(buf, "%s %lx %lx", iface, &destination, &gateway) == 3){
			if(destination == 0){
				memcpy(addr,&gateway, 4); 
				fclose(file); 
				return true; 
			}
		}
	}
	return false; 
}

void usage() {
  printf("syntax: pcap_test <interface> <sender ip> <target ip> <sender ip> <target ip>...\n");
	printf("sample: pcap_test wlan0 192.168.2.121 192.168.2.1\n");
	printf("==================================================\n");
	printf("sender means the one you want to change arp table!\n");
	printf("target means the one you wnat to write sender's arp table!\n");
}

void setSignal(int num){
  pthread_mutex_lock(&mutex);
	threadSignal[num] = 1;
	pthread_mutex_unlock(&mutex);
}
void  resetSignal(int num){
  pthread_mutex_lock(&mutex);
	threadSignal[num] = 0;
	pthread_mutex_unlock(&mutex);
} 
