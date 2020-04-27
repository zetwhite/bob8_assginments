#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

extern void debug(const char *fmt, ...);
extern void *sbrk(intptr_t increment);

typedef struct block{
	size_t prevSize; 
	size_t size; 
	
	struct block* prev; 
	struct block* next; 
}__attribute__((packed)) BLK; 

#define BLK_PREV_SZ_IDX 0 
#define BLK_SZ_IDX 8 
#define BLK_PREV_IDX 16 
#define BLK_NEXT_IDX 24 

#define PAGE_TRIM_SZ(size) (((size)%(PAGE_SZ)) ? (((size)&(0xfffffffffffff000)) + PAGE_SZ) : (size))

#define PAGE_SZ 0x1000
#define BLK_HEADER_SZ 0x10 
#define EMPTY NULL

#define F_BIN_START_SZ 0x20 
#define F_BIN_COUNT 7
#define F_BIN_INTERVAL 0x10 

#define S_BIN_START_SZ 0x90
#define S_BIN_COUNT 50 
#define S_BIN_INERVAL 0x10 

#define L_BINS_SZ 0x410 

#define F_IDX_2_SZ(idx) (F_BIN_START_SZ + (idx)*F_BIN_INTERVAL) 
#define F_SZ_2_IDX(size) ((size - F_BIN_START_SZ + 1) / F_BIN_INTERVAL)  
#define F_TRIM_SZ(size) (((size)%(F_BIN_INTERVAL))?(((size)&(0xfffffffffffffff0)) + F_BIN_INTERVAL): (size)) 

#define S_IDX_2_SZ(idx) (S_BIN_START_SZ + (idx)*S_BIN_INTERVAL) 
#define S_SZ_2_IDX(size) ((size - S_BIN_START_SZ + 1) / S_BIN_INTERVAL)  
#define S_TRIM_SZ(size) (((size)%(S_BIN_INTERVAL))?(((size)&(0xfffffffffffffff0)) + S_BIN_INTERVAL): (size)) 

BLK* fastBins[F_BIN_COUNT] = {EMPTY, EMPTY, }; 
BLK* smallBins[S_BIN_COUNT] = {EMPTY, EMPTY, }; 
BLK* largeBins = EMPTY;  

BLK* unsortedBins = EMPTY; 
BLK* unsortedBinsEnd = EMPTY; 
long long unsortMaxSize = 0; 

uint8_t leftSize; 
void* allocStartPtr; 
void* allocBackupPtr; 
uint8_t setting = 0; 


inline void initSet(){
	allocStartPtr = sbrk(PAGE_SZ);
	allocBackupPtr = allocStartPtr + PAGE_SZ;
   	leftSize = PAGE_SZ;
	return;
}


void error(const char * errmsg){
	debug("ERROR! %s\n", errmsg);
	abort(); 
}


inline void* myInternalSbrk(size_t size){
	if(!setting){
		initSet();
	  	setting = 1;
	}
	if(leftSize <= size + sizeof(size_t)) {
		size_t trimSz = PAGE_TRIM_SZ(size);
		allocBackupPtr = sbrk(trimSz) + trimSz;
		leftSize += trimSz;
	}
	void* newAlloc = allocStartPtr;
	allocStartPtr += size;
	leftSize -= size;
	return newAlloc;
}

inline void* allocFromFast(size_t index){
	BLK* newAllocPtr = fastBins[index]; 
	fastBins[index] = fastBins[index]->next;

	int size = F_IDX_2_SZ(index);
	setBlockSize(newAllocPtr, size, 1); 
	return (void*)newAllocPtr; 
}


void* allocFromSmall(size_t index){
	BLK* newAllocPtr = smallBins[index]; 
	smallBins[index] = smallBins[index]->next; 
	return newAllocPtr + BLK_HEADER_SZ; 
}

inline void unlink(BLK* block){
	if(block == unsortedBins && block == unsortedBinsEnd){
		unsortedBins = (unsortedBinsEnd = EMPTY); 
	}
	else if(block == unsortedBins) {
		unsortedBins = block->next;
		unsortedBins->prev = EMPTY; 
	}
	else if(block == unsortedBinsEnd){
		unsortedBinsEnd = block->prev; 
		unsortedBinsEnd->next = EMPTY;
	}
	else{
		block->prev->next = block->next; 
		block->next->prev = block->prev; 
	}
	return; 
}

inline void setBlockSize(BLK* block, size_t size, int inUsed){
	block->size = size;
	if(inUsed){ 
		*(size_t*)(((void*)block) + size) = size|0x1; 
	}
	else 
		*(size_t*)(((void*)block) + size) = size&0xfffffffffffffffe; 
	return; 
}

void* allocFromUnsorted(size_t size){
	BLK* here = unsortedBinsEnd;
	
	void* newAllocPtr; 
	size_t totalSize;
	BLK* leftPtr; 
	size_t leftSize; 
	int count = 0; 

	while(here != EMPTY){
		if((here->size) >= size){
			newAllocPtr = here; 
			totalSize = here->size; 
			leftPtr = (void*)here + size; 
			leftSize = totalSize - size; 
			break; 
		}
		here = here->prev; 
		count ++;
		if(count > 200) {
			here = EMPTY; 
			break; 
		}
	}

	if(here ==  EMPTY) {
		return EMPTY; 
	}
	else if(leftSize < F_BIN_START_SZ){
		unlink(newAllocPtr); 
		*(size_t*)(newAllocPtr + totalSize) = size|0x1; 
		return newAllocPtr; 
	}
	else if(leftSize < S_BIN_START_SZ){ //split to fast bin 

		setBlockSize(newAllocPtr, size, 1); 
		setBlockSize(leftPtr, leftSize, 0); 
		unlink(newAllocPtr); 
		insertFastBins(F_SZ_2_IDX(leftSize), leftPtr); 
		return newAllocPtr; 
	}
	else{//just split 
		setBlockSize(newAllocPtr, size, 1); 
		setBlockSize(leftPtr, leftSize, 0);
		leftPtr->next = ((BLK*)newAllocPtr)->next; 
		leftPtr->prev = newAllocPtr; 
		((BLK*)newAllocPtr)->next = leftPtr;

		if(leftPtr->next != EMPTY) leftPtr->next->prev = leftPtr; 
		if(newAllocPtr == unsortedBinsEnd) unsortedBinsEnd = leftPtr; 	
		unlink(newAllocPtr); 
		return newAllocPtr; 
	}
	return EMPTY;
}


void dumpFast(size_t index){
	debug("fastBins[%d]_size(0x%x) : ", index, F_IDX_2_SZ(index)); 
	BLK* here = fastBins[index]; 
	while(here != EMPTY){
		debug("%p -> ", here); 
		here = here->next; 
	}
	debug("END\n"); 
	return; 
}

void dumpReverseUnsorted(){
	debug("unosorted Bins Reverse : "); 
	BLK* here = unsortedBinsEnd; 
	while(here != EMPTY){
		debug("0x%x(0x%x) -> " , here, here->size); 
		here = here -> prev; 
	}
	debug("END\n"); 
	return ;
}

void dumpUnsorted(){
	debug ("unsorted Bins : "); 
	BLK* here = unsortedBins; 
	while(here != EMPTY){
		debug ("0x%x(0x%x) -> ", here, here->size); 
		here = (here -> next); 
	}
	debug("END\n"); 
	return; 
}


void dumpChunk(BLK* ptr){
	debug("[+] dumpChunk at 0x%x -----------------\n", ptr); 
	debug("  prevSize :\t 0x%x\n", ptr->prevSize);
	debug("  size :    \t 0x%x\n", ptr->size); 
	debug("  prev :    \t 0x%p\n", ptr->prev); 
	debug("  next :    \t 0x%p\n", ptr->next); 
	debug("---------------------------------------\n"); 
}


void *myalloc(size_t size)
{

	size += BLK_HEADER_SZ; 
	void* newAllocPtr; 

	if(size < F_BIN_START_SZ) {
		size = F_BIN_START_SZ; 
	}
	size = F_TRIM_SZ(size); 

	if(size < S_BIN_START_SZ && fastBins[F_SZ_2_IDX(size)]!= EMPTY) { //allocate fast bin! 
		size_t idx = F_SZ_2_IDX(size);
		newAllocPtr =  allocFromFast(idx);
	}
	else if((unsortMaxSize < size) || ((newAllocPtr = allocFromUnsorted(size)) == EMPTY)){ //allocate from top chunk 
		newAllocPtr = myInternalSbrk(size); 
		setBlockSize(newAllocPtr, size, 1); 
	}
	
	return newAllocPtr + BLK_HEADER_SZ;
}


void *myrealloc(void *ptr, size_t size){
	void *p = NULL;
    if (size != 0)
    {
        p = myalloc(size);
        if (ptr != EMPTY)
            memcpy(p, ptr, size);
    }
    return p;
}


inline void insertFastBins(size_t index, BLK* startBlock){
	if(fastBins[index] != EMPTY) {
		startBlock->next = fastBins[index]; 
	}
	else 
		startBlock->next = EMPTY; 
	fastBins[index] = startBlock; 
}


inline void insertUnsortedBins(size_t size, BLK* startBlock){
	if(unsortMaxSize < size) 
		unsortMaxSize = size; 
	if(unsortedBins != EMPTY){ //not first Insert; 
		startBlock->next = unsortedBins; 
		unsortedBins->prev = startBlock; 
	}
	else {//fisrt insert 
		unsortedBinsEnd = startBlock; 
		startBlock->next = EMPTY; 
	}
	unsortedBins = startBlock; 
	unsortedBins->prev = EMPTY; 
	return; 
}


void myfree(void *ptr){
	if(ptr == NULL){
		return; 
	}
	void* prevSizePtr = ptr - BLK_HEADER_SZ;
	size_t prevSize = *(size_t*)prevSizePtr; 
	size_t size = *(size_t*)(prevSizePtr + BLK_SZ_IDX); 
	*(size_t*)(prevSizePtr + size) = size&0xfffffffffffffffe; 
	if(size < F_BIN_START_SZ ) 
		error("in myfree(), too small chunk, this size is not possible!"); 
    else if(size < S_BIN_START_SZ) {//fast bins 
		insertFastBins(F_SZ_2_IDX(size), prevSizePtr); 
	}
	else{ //unsorted bin
		insertUnsortedBins(size, prevSizePtr); 
	}
	return; 
}

