#define _GNU_SOURCE		//for using MAP_ANONYMOUS

#include <unistd.h>
#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <sys/mman.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

uint8_t* Operation;			//memory mapping pointer
uint8_t* compiled_code;		//shared_memory pointer

int segment_id;

void sharedmem_init(); // shared memory attach
void sharedmem_exit();
void drecompile_init(); // memory mapping(ANONYMOUS and PRIVATE)
void drecompile_exit(); 
void* drecompile(uint8_t *func); //Assembly Optimization

int main(void)
{
	int (*func)(int a);
	int output;
    struct timespec startTime, endTime;
	double t;

	sharedmem_init();		//attach shared_memory
	drecompile_init();		//memory allocation

	//Assembly Opitmize
	func = (int (*)(int a))drecompile(Operation);

	//Check start time
    clock_gettime(CLOCK_MONOTONIC, &startTime);

	//run
	for(int i = 0; i < 1000000; i++)
		output = (*func)(1);
	
	//Check end time
    clock_gettime(CLOCK_MONOTONIC, &endTime);	
	
	//Calculate time and print
	t = (endTime.tv_sec - startTime.tv_sec) + (double)(endTime.tv_nsec - startTime.tv_nsec) / 1000000000;
	printf("total execution time: %f\n", t);
	printf("output : %d\n", output);

	drecompile_exit();	//memory deallocation
	sharedmem_exit();	//detatch shared_memory
	return 0;
}

void sharedmem_init()
{
	//get shared memory using key
	segment_id = shmget((key_t)1234, (size_t)PAGE_SIZE, 0);
	assert(segment_id != -1);	

	//attatch shared memory using segment id
	compiled_code = (uint8_t *)shmat(segment_id, NULL, 0);
	assert(compiled_code != (uint8_t *)-1);
}

void sharedmem_exit()
{
	//remove shared memory using segment id
	int shmctl_code = shmctl(segment_id, IPC_RMID, NULL);
	assert(shmctl_code != -1);
}

void drecompile_init()
{
	//Memory Mapping(RW, private and anonymous)
	Operation = mmap(0, 40, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	assert(Operation != MAP_FAILED);
}

void drecompile_exit()
{
	//Memory Unmapping
	int munmap_code = munmap(Operation, PAGE_SIZE);
	assert(munmap_code != -1);
}

void* drecompile(uint8_t* func)
{
	int i = 0;
	int protect_code;

	#ifdef dynamic
		//Dynamic option(optimize)
		uint8_t prev_op[3] = { 0 }, op_size = 0, dl = 0;
		do{
			//save dl
			if(compiled_code[i] == 0xB2)	dl = compiled_code[i + 1];

			//ADD SUB IMUL case
			if(compiled_code[i] == 0x83 || compiled_code[i] == 0x6B){
				//current inst.
				prev_op[0] = compiled_code[i++];
				prev_op[1] = compiled_code[i++];
				prev_op[2] = compiled_code[i++];

				//optimize
				for(; prev_op[0] == compiled_code[i] && compiled_code[i + 1]; i += 3){
					if(prev_op[0] == 0x83)	prev_op[2] += compiled_code[i + 2];
					else					prev_op[2] *= compiled_code[i + 2];
				}

				//insert inst.
				*func++ = prev_op[0];
				*func++ = prev_op[1];
				*func++ = prev_op[2];
			}
			//DIV case
			else if(compiled_code[i] == 0xF6){
				//current inst.
				prev_op[0] = compiled_code[i++];
				prev_op[1] = dl;

				//optimize
				for(i++; prev_op[0] == compiled_code[i]; i += 2){
					prev_op[1] *= dl;
				}

				//MOV dl prev_op[1]
				*func++ = 0xB2;
				*func++ = prev_op[1];
				//DIV dl				
				*func++ = prev_op[0];
				*func++ = 0xF2;
			}
			//Not ADD SUB IMUL DIV
			else	*func++ = compiled_code[i++];
		} while(compiled_code[i - 1] != 0xC3);
	#else
		//default option(just copy)
		do{
			*func++ = compiled_code[i];
		} while(compiled_code[i++] != 0xC3);
	#endif

	//change to RX
	protect_code = mprotect(Operation, PAGE_SIZE, PROT_READ | PROT_EXEC);
	assert(protect_code != -1);

	return (void *)Operation;
}

