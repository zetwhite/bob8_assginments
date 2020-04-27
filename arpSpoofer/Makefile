CC = gcc
TARGET = arpSpoofer

$(TARGET) : main.o util.o ethernethelper.o packetAnalyzer.h packetmaker.h arphelper.h iphelper.h arpSpoofer.h
	$(CC) -o $@ $^ -lpcap -lpthread 

main.o : main.c 
	$(CC) -c -o $@ $^ 

#arphelper.o : arphelper.c
#	$(CC) -c -o $@ $^ 

ethernethelper.o : ethernethelper.c
	$(CC) -c -o $@ $^ 

#iphelper.o : iphelper.c
#	$(CC) -c -o $@ $^ 

util.o : util.c
	$(CC) -c -o $@ $^ 

clean: 
	rm *.o $(TARGET)
