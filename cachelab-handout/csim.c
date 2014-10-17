/*
 * author: tianyuy
 * date 2014.10.01
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "cachelab.h"
/*
 * The type of data structure that simulate a line of the cache
 */
typedef struct{
	//This stands for the valid bit for the lind
	int valid;
	//This stands for the tag of the line
	int tag;
	//This is the access_time
	int access_rank;
}Simulator_line;

/*
 * The data structrue that simulate a set of the cache
 */
typedef struct{
	Simulator_line *lineArray;
}Simulator_set;

/*
 * The data structure that simulate the cache
 */
typedef struct{
	Simulator_set *setArray;
}Simulator_cache;

/*
 * This is the method I will use, 
 * the detailed of the method I will introduce later
 */
void get_operation(int argc, char **argv);
void initialize_malloc(Simulator_cache *cache);
void readFromFile();
int getSetNumber(unsigned address, int s, int b);
int getTagNumber(unsigned address, int s, int b);
void doCacheAction(char identi,unsigned addr,int size,Simulator_cache *_cache);
void refresh_access(Simulator_cache *_cache, int setIndex, int lineIndex);
void free_malloc(Simulator_cache *_cache);
int hitFunc(char identifier, unsigned address, Simulator_cache *_cache);
int missFunc(char identifier, unsigned address, Simulator_cache *_cache);
int evictFunc(char identifier, unsigned address, Simulator_cache *_cache);

/*
 * This stands for the attribute of the cache like the :s, E and b
 */
int s=0, E=0, b=0;
/*
 * This will store the hit, miss and evict of the cache action
 */
int hit=0, miss=0, evict=0;
/*
 * This will store the file path of the trace file we are going to read
 */
char *readFile = NULL;

int setSize = 0;

int main (int argc, char **argv){

	get_operation(argc, argv);
	Simulator_cache _cache;
	initialize_malloc(&_cache);
	readFromFile(&_cache);
	printSummary(hit, miss, evict);
        return 0;
}

/*
 * This is the function that I get the operation from the input
 * the -s goes for s, the  -E goes for E, the -b goes for b and
 * the -t goes for the readFile
 * if other argument input just press errors
 *
 * argument: int argc, int argv
 */
void get_operation(int argc, char **argv){
    int opt;
    int optNumber=0;
    while((opt=getopt(argc, argv, "s:E:b:t:"))!=-1){
	switch(opt){
	    case 's':
	    	s = atoi(optarg);
		//specify the set size of the operation
		setSize = 1<<s;
		optNumber++;
		break;
	    case 'E':
		E = atoi(optarg);
		optNumber++;
		break;
	    case 'b':
		b = atoi(optarg);
		optNumber++;
		break;
	    case 't':
		readFile = optarg;
		optNumber++;
		break;
	    default:
		printf("Wrong argument. Please check your input\n");
		exit(0);
	}
		
    }
    //if the operation number is not correct just exit
    if(optNumber!=4){
	printf("Wrong argument. Please check your input\n");
	exit(0);
    }
}

/*
 * This will initialize the cache
 */
void initialize_malloc(Simulator_cache *_cache){
    //initialize the set of the cache
    _cache->setArray=(Simulator_set *)malloc((setSize)*sizeof(Simulator_set));
    int index=0;
    while(index<(setSize)){
	int totalLineS = E*sizeof(Simulator_line);
	_cache->setArray[index].lineArray=(Simulator_line *)malloc(totalLineS);
	int index2=0;
	while(index2<E){
	    _cache->setArray[index].lineArray[index2].valid=0;
	    _cache->setArray[index].lineArray[index2].tag=0;
	    _cache->setArray[index].lineArray[index2].access_rank = E+1;
	    index2++;
	}
	index++;
    }
}

/*
 * This method will read the command from the file
 * parse the command and do the action according to it
 */
void readFromFile(Simulator_cache *_cache){
    FILE * pFile;	
    pFile = fopen(readFile, "r");
    char identifier;
    unsigned address;
    int size;
    while(fscanf(pFile," %c %x,%d",&identifier,&address,&size)>0){
	if(identifier=='I'){
	    //if it is I simply ignore it
	}else{
	    doCacheAction(identifier, address, size, _cache);
	}
    }
    fclose(pFile); //remember to close file when done
}

/*
 * This will return the set index of the command
 * 0x7fffffff>>(31-s) is a mask
 * while address>>b will get the set we need
 * just be careful of the valid number
 */
int getSetNumber(unsigned address, int s, int b){
    return ((address>>b)&(0x7fffffff>>(31-s)));
}

/*
 * This will return the tag index of the command
 * It is of the same idea as set
 */
int getTagNumber(unsigned address, int s, int b){
        
    return ((address>>(s+b))&(0x7fffffff>>(31-s-b)));
}

/*
 * This will do the cache action
 * at first if it is a hit, then done for this command
 * if not, then it is a miss. 
 * then we need to check whether it is a evict
 * This will execute the different function according to it
 */
void doCacheAction(char identi,unsigned addr,int size,Simulator_cache *_cache){

    int hitResult = hitFunc(identi, addr, _cache);
    if(hitResult==0){
	int missResult = missFunc(identi, addr, _cache);
	if(missResult==0){
	    evictFunc(identi, addr, _cache);
	}
    }
}

/*
 * This will refresh the line's access_rank. 
 * Originially it will set as E+1
 * The least recent accessed one will be the highest position
 * When one has been accessed, all of the lines in the set increase by 1
 */
void refresh_access(Simulator_cache *_cache, int setIndex, int lineIndex){
    
    int i=0;
    //The one need to be updated
    Simulator_line cl=_cache->setArray[setIndex].lineArray[lineIndex];
    while(i<E){
	Simulator_line tempLine= _cache->setArray[setIndex].lineArray[i];
	//if vaid
	if(tempLine.valid==1){
	    //if less recent accessed
	    if(tempLine.access_rank<cl.access_rank)
	    	(_cache->setArray[setIndex].lineArray[i].access_rank)++;
	}
	i++;
    }
    _cache->setArray[setIndex].lineArray[lineIndex].access_rank = 1;
    
}

/*
 * This will do the hit function
 * if the line is valid, hit++
 * if the command is an modification 
 * which is a combinator of a read and write
 * the hit++ once again
 * If there is a hit return 1
 * if not return 0
 */
int hitFunc(char identifier, unsigned address, Simulator_cache *_cache){
    int setIndex = getSetNumber(address, s, b);
    int tagIndex = getTagNumber(address, s, b);
    int i=0;
    while(i<E){
	Simulator_line tempLine = _cache->setArray[setIndex].lineArray[i];
	if((tempLine.valid==1)&&(tempLine.tag == tagIndex)){
	    refresh_access(_cache, setIndex, i);
	    hit++;
	    if(identifier=='M'){
		hit++;
	    }
	    return 1;
	}
	i++;
    }
    return 0;
}

/*
 * This will do the missfunction of the cache
 * if the line is not valid, and it is not a hit, then it is a miss
 * So change the valid and update the tag and refresh the access_rank
 * If it is a modification the hit++
 * If it is a miss return 1
 * if not return 0
 */
int missFunc(char identifier, unsigned address, Simulator_cache *_cache){
    int setIndex = getSetNumber(address, s, b);
    int tagIndex = getTagNumber(address, s, b);
    miss++;
    int i=0;
    while(i<E){
	if(_cache->setArray[setIndex].lineArray[i].valid==0){
	    _cache->setArray[setIndex].lineArray[i].valid = 1;
	    _cache->setArray[setIndex].lineArray[i].tag = tagIndex;
	    refresh_access(_cache, setIndex, i);
	    if(identifier=='M'){
	        hit++;
	    }
	    return 1;
	}
	i++;
    }
    return 0;
}

/*
 * This will do the evict function of the cache
 * Go through the least recent accessed.
 * update the line
 * if it is a modification, hitt+
 * if it is an eviction return 1
 * if not return 0
 */
int evictFunc(char identifier, unsigned address, Simulator_cache *_cache){
    int setIndex = getSetNumber(address, s, b);
    int tagIndex = getTagNumber(address, s, b);
    evict++;
    int i=0;
    while(i<E){
	if(_cache->setArray[setIndex].lineArray[i].access_rank==E){
	    _cache->setArray[setIndex].lineArray[i].valid = 1;
	    _cache->setArray[setIndex].lineArray[i].tag = tagIndex;
	    refresh_access(_cache, setIndex, i);
	    if(identifier=='M'){
	  	hit++;
	    }
	    return 1;
		
			
	}
	i++;
    }
    return 0;
}


