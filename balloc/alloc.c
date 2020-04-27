#include <stdint.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h> 

extern void debug(const char *fmt, ...);
extern void *sbrk(intptr_t increment);

typedef struct block{
        size_t prevSize;
        size_t size;

        struct block* prev;
        struct block* next;
}__attribute__((packed)) BLK;


#define tdebug //debug
/*
clock_t before; 

void tStart(){
	before = clock(); 
}

double tEnd(){
	tdebug("time : %5.2f\n", (double)(clock()-before)); 
}
*/
#define BLK_PREV_SZ_IDX 0
#define BLK_SZ_IDX 8
#define BLK_PREV_IDX 16
#define BLK_NEXT_IDX 24

#define mdebug //debug
//#define mdebug debug
#define debug //debug
#define PAGE_TRIM_SZ(size) (((size)%(PAGE_SZ)) ? (((size)&(0xfffffffffffff000)) + PAGE_SZ) : (size))

#define PAGE_SZ 0x1000
#define BLK_HEADER_SZ 0x10
#define EMPTY NULL

#define F_BIN_START_SZ 0x20
#define F_BIN_COUNT 50
#define F_BIN_INTERVAL 0x10

#define S_BIN_START_SZ (F_BIN_START_SZ + F_BIN_INTERVAL*F_BIN_COUNT)
#define F_IDX_2_SZ(idx) (F_BIN_START_SZ + (idx)*F_BIN_INTERVAL)
#define F_SZ_2_IDX(size) ((size - F_BIN_START_SZ + 1) / F_BIN_INTERVAL)
#define F_TRIM_SZ(size) (((size)%(F_BIN_INTERVAL))?(((size)&(0xfffffffffffffff0)) + F_BIN_INTERVAL): (size))

BLK* fastBins[F_BIN_COUNT] = {EMPTY,};
BLK* fastBinsEnd[F_BIN_COUNT] = {EMPTY, }; 

BLK* unsortedBins = EMPTY;
BLK* unsortedBinsEnd = EMPTY;
long long unsortMaxSize = 0;

uint8_t leftSize;
void* allocStartPtr;
void* allocBackupPtr;
uint8_t setting = 0;


void initSet(){
        allocStartPtr = sbrk(PAGE_SZ);
        allocBackupPtr = allocStartPtr + PAGE_SZ;
        leftSize = PAGE_SZ;
        return;
}


void error(const char * errmsg){
        debug("ERROR! %s\n", errmsg);
        abort();
}


void dumpChunk(BLK* ptr){
        debug("[+] dumpChunk at 0x%x -----------------\n", ptr);
        debug("  prevSize :\t 0x%x\n", ptr->prevSize);
        debug("  size :    \t 0x%x\n", ptr->size);
        debug("  prev :    \t 0x%p\n", ptr->prev);
        debug("  next :    \t 0x%p\n", ptr->next);
        debug("---------------------------------------\n");
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


void dumpALL(){
        for(int i = 0; i < F_BIN_COUNT; i++)
                dumpFast(i);
        dumpUnsorted();
}


void* myInternalSbrk(size_t size){
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


void setBlockSize(BLK* block, size_t size, int inUsed){
        block->size = size;
        *(size_t*)(((void*)block) + size) = size|inUsed;
     	debug("block set succes\n"); 
     	return;
}


void unlink(BLK* block){
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


void* allocFromUnsorted(size_t size){
	debug("trying alloc from unsorted\n"); 
        BLK* here = unsortedBins;

        BLK* newAllocPtr;
        size_t totalSize;
        BLK* leftPtr;
        size_t leftSize;
        int count = 0;
        while(here != EMPTY){
                if((here->size) >= size){
                        totalSize = here->size;
                        newAllocPtr = (void*)here + totalSize -size;
                        leftPtr = (void*)here;
                        leftSize = totalSize - size;
                        break;
                }
                here = here->next;
        	count++; 
		if(count > 1500) {
			here = EMPTY; 
			break; 
		}
	}
	if(here ==  EMPTY) {
                debug("  => nothing is in unsorted \n");
                return EMPTY;
        }
        else if(leftSize < F_BIN_START_SZ){
                debug("  => find one! but left size is so small, so just give all\n");
                unlink(leftPtr);
                *(size_t*)((void*)leftPtr + totalSize) = size|0x1;
                return leftPtr;
        }
        else if(leftSize < S_BIN_START_SZ){ //split to fast bin
		debug("  => find one! and tryiny to split -> fastbins\n");
               // dumpChunk(leftPtr); 
		//	dumpChunk(newAllocPtr); 
		
		setBlockSize(newAllocPtr, size, 1);
                setBlockSize(leftPtr, leftSize, 0);
                dumpChunk(leftPtr); 
		dumpChunk(newAllocPtr); 
		unlink(leftPtr);
                insertFastBins(F_SZ_2_IDX(leftSize), leftPtr);
                return newAllocPtr;
        }
        else{//just split
                debug("  => just split -> unsorted\n");
      		setBlockSize(newAllocPtr, size, 1);
                setBlockSize(leftPtr, leftSize, 0);
		newAllocPtr->next = leftPtr->next; 
                dumpChunk(leftPtr); 
		dumpChunk(newAllocPtr); 
		newAllocPtr->prev = leftPtr; 
		leftPtr->next = newAllocPtr; 
		if(newAllocPtr->next != EMPTY) newAllocPtr->next->prev = newAllocPtr; 
		if(leftPtr == unsortedBinsEnd) unsortedBinsEnd = newAllocPtr; 	
                unlink(newAllocPtr);
                return newAllocPtr;
        }
        return EMPTY;
}


void* allocFromFast(size_t index){
        BLK* newAllocPtr = fastBins[index];
        fastBins[index] = fastBins[index]->next;
        int size = F_IDX_2_SZ(index);
        setBlockSize(newAllocPtr, size, 1);
        mdebug ("  alloc fastBins[%d] \t: 0x%p\n", index, newAllocPtr);
	return (void*)newAllocPtr;
}


void *myalloc(size_t size)
{
        debug("= [myalloc start] size = 0x%x ==============================\n", size);
        //tStart(); 
	size += BLK_HEADER_SZ;
        void* newAllocPtr;

        if(size < F_BIN_START_SZ)
                size = F_BIN_START_SZ;
        size = F_TRIM_SZ(size);

        if(size < S_BIN_START_SZ && fastBins[F_SZ_2_IDX(size)]!= EMPTY) { //allocate fast bin! 
                size_t idx = F_SZ_2_IDX(size);
		debug("alloc from fast\n"); 
                newAllocPtr =  allocFromFast(idx);
        }
        else if((newAllocPtr = allocFromUnsorted(size)) == EMPTY){ //allocate from top chunk 
                newAllocPtr = myInternalSbrk(size);
		debug("alloc from topchunk\n"); 
                setBlockSize(newAllocPtr, size, 1);
        }
	//tEnd(); 
        debug ("========================================================\n\n");
        return newAllocPtr + BLK_HEADER_SZ;
}


void *myrealloc(void *ptr, size_t size)
{       debug("= [myrealloc started] ===================================\n");
        void *p = NULL;
    if (size != 0)
    {
        p = myalloc(size);
        if (ptr != EMPTY)
            memcpy(p, ptr, size);
    }
    //dumpALL();
        debug("========================================================\n\n");
    return p;
}


void insertFastBins(size_t index, BLK* startBlock){ 
	//abort();
	mdebug("  insert into fastBins[%d]_size(0x%x)\n", index, F_IDX_2_SZ(index));
        if(fastBins[index] != EMPTY) {
                startBlock->next = fastBins[index];
        }
        else{
                startBlock->next = EMPTY;
		fastBinsEnd[index] = startBlock; 
	}
	fastBins[index] = startBlock;
}


void insertUnsortedBins(size_t size, BLK* startBlock){
        mdebug("  insert into UnsortedBins size(0x%x) ptr(0x%p)\n", size, startBlock);
        if(unsortedBins != EMPTY){ //not first Insert;
                if((startBlock->prevSize & 0x1) == 0 && startBlock->prevSize >= S_BIN_START_SZ){ //trying merging.. 
                        debug("merging at \n");
                        BLK* prevBlock = (void*)startBlock - startBlock->prevSize;
                        setBlockSize(prevBlock, prevBlock->size + size, 0);
                        return;
                }
                startBlock->next = unsortedBins;
                unsortedBins->prev = startBlock;
        }
        else {//fisrt insert
                unsortedBinsEnd = startBlock;
                startBlock->next = EMPTY;
        }
	debug("inserted success\n"); 
        unsortedBins = startBlock;
        unsortedBins->prev = EMPTY;
        return;
}


void myfree(void *ptr){
        debug("= [myfree started] in 0x%x===============================\n", ptr);
        //tStart(); 
	if(ptr == NULL){
                debug("nullptr just return it!\n\n");
                return;
        }
        void* startBlock = ptr - BLK_HEADER_SZ;
        size_t prevSize = *(size_t*)startBlock;
        size_t size = *(size_t*)(startBlock + BLK_SZ_IDX);
        *(size_t*)(startBlock + size) = size;
        
	if(size < F_BIN_START_SZ )
                error("in myfree(), too small chunk, this size is not possible!");
        else if(size < S_BIN_START_SZ) {//fast bins
 		debug("insert into fastBins\n"); 
 		insertFastBins(F_SZ_2_IDX(size), startBlock);
        }
        else{ //unsorted bin
		debug("insert into unsortedBins\n"); 
                insertUnsortedBins(size, startBlock);
        }
	//tEnd(); 
        debug("======================================================\n\n");
        return;
}
