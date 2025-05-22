#pragma once

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////////
#define MAX_READ 2048
#define MEMORYFILENAME "memory.txt"

typedef uint32_t blockid_t;  // 32-bit block ID
typedef struct metadata{
	uint32_t size;
	uint16_t pos;
	int8_t status; // -1 if metadata doesnt exist
	uint8_t block_id;
}Metadata;

blockid_t htw_alloc(uint32_t size); // return block ID
blockid_t htw_free(blockid_t block_id);
int htw_put(blockid_t block_id, const char *buf, int size);
ssize_t htw_get(blockid_t block_id, char *buf, int max_size);
int htw_get_block_size(blockid_t block_id);

int createMemory();
Metadata getNextMetadata(int curr_block_id);
Metadata getMetadata(int block_id); // also automatically update the metadata position
void printAllMetadata();
int expandBlock(blockid_t block_id);
int moveChunk(blockid_t startblock, uint16_t pos);

int memfd;
uint16_t n_block_id = 0;
uint64_t memsize = 0;

////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
blockid_t htw_alloc(uint32_t size)
{
	Metadata info;
	memsize = memsize + size + sizeof(Metadata);
	info.size = size;
	info.block_id = n_block_id++;
	char ch = '0'+info.block_id;

	info.pos = lseek(memfd, 0, SEEK_END);
	 
	write(memfd, &info, sizeof(info));
	for(int i = 0; i<size; i++){
		//lseek(memfd, 0, SEEK_END);
		write(memfd, &ch, 1);
	}
	
	printf("%d\n", info.pos);
	lseek(memfd, 0, SEEK_SET);
	return info.block_id;	
}

blockid_t htw_free(blockid_t block_id)
{
	Metadata meta = getNextMetadata(block_id);
	char data[MAX_READ];
	ssize_t bytes_read;

	//while(memsize > MAX_READ);

	// if next block exist, read next block till EOF
	// printf("meta status = %d \n", meta.status);
	if(meta.status != -1){
		lseek(memfd, meta.pos, SEEK_SET);
		bytes_read = read(memfd, &data, MAX_READ);
		printf("next block exists, bytes_read = %ld \n", bytes_read);
		lseek(memfd, 0, SEEK_SET);
	} 
	
	// overwrite target block with read data
	meta = getMetadata(block_id); // this also update the metadata position
	lseek(memfd, meta.pos, SEEK_SET);
	write(memfd, &data, bytes_read);


	// cut excess data
	memsize = memsize - meta.size - sizeof(Metadata);
	ftruncate(memfd, memsize);
	lseek(memfd, 0, SEEK_SET);

	//printf("Memory block (id = %d) has been successfully freed\n", block_id);
	return block_id;
}

int htw_get_block_size(blockid_t block_id)
{
	Metadata meta = getMetadata(block_id);
	if(meta.status == -1) return -1;
	return meta.size;
}

ssize_t htw_get(blockid_t block_id, char *buf, int max_size)
{
	Metadata meta = getMetadata(block_id);
	ssize_t bytes_read;
	if(meta.status != -1){
		lseek(memfd, meta.pos, SEEK_SET);
		bytes_read = read(memfd, buf, max_size);
		//printf("bytes_read = %ld \n", bytes_read);
		lseek(memfd, 0, SEEK_SET);
	} else return -1; 
	return bytes_read;
}

int htw_put(blockid_t block_id, const char *buf, int size)
{
	Metadata meta = getMetadata(block_id);
	int status = 0;
	while(meta.size < size){
		expandBlock(block_id);
		meta = getMetadata(block_id);
	}

	lseek(memfd, meta.pos+sizeof(Metadata), SEEK_SET);
	status = write(memfd, buf, size);
	if(status == -1) return -1;

	lseek(memfd, 0, SEEK_SET);

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////
int createMemory()
{
	unlink(MEMORYFILENAME);
	memfd = open(MEMORYFILENAME, O_CREAT | O_RDWR , 0644);
    
	if (memfd == -1) {
		perror("Error opening file");
		return 1;
	}

	 return 0;
}

Metadata getMetadata(int block_id)
{    
	Metadata meta;
	ssize_t bytes_read, written;
	off_t pos = 0;
	
	if(block_id >= n_block_id){
		printf("error: requested block_id is out of scope\n");
		meta.status = -1; // to mark an error
		return meta;
	}

	for(int i = 0; i < n_block_id; i++){
		bytes_read = read(memfd, &meta, sizeof(Metadata));

		if(bytes_read > 0){
			meta.pos = pos;
			lseek(memfd, -bytes_read, SEEK_CUR);
			written = write(memfd, &meta, sizeof(meta));
			lseek(memfd, -written, SEEK_CUR);
		} else break;
		// metadata found
		if(meta.block_id == block_id){
			meta.status = 0;
			write(memfd, &meta, sizeof(meta));
			lseek(memfd, 0, SEEK_SET); // reset lseek
			return meta;
		}
		pos = lseek(memfd, pos+sizeof(meta)+meta.size, SEEK_SET);
		if(bytes_read == -1) printf("error: read failed \n");
	}
	
	//metadata not found
	meta.status = -1;
	lseek(memfd, 0, SEEK_SET); // reset lseek
	return meta;
}

Metadata getNextMetadata(int curr_block_id){
	Metadata meta, curr = getMetadata(curr_block_id);
	ssize_t bytes_read;
	off_t pos = 0;

	pos = lseek(memfd, curr.pos+sizeof(curr)+curr.size, SEEK_SET);
	bytes_read = read(memfd, &meta, sizeof(struct metadata));
	lseek(memfd, 0, SEEK_SET);
	if(bytes_read == 0) meta.status = -1;
	meta.pos = pos;
	return meta;
}

int expandBlock(blockid_t block_id)
{
	Metadata meta = getMetadata(block_id);
	Metadata nextmeta = getNextMetadata(block_id);
	off_t newnextpos = nextmeta.pos;
	ssize_t newsize = meta.size, written;
	char ch = '0'+ meta.block_id;

		newsize = 2*meta.size;	
		newnextpos += meta.size;
	moveChunk(nextmeta.block_id, newnextpos);
	
	lseek(memfd, nextmeta.pos, SEEK_SET);
	for(int i = 0; i < meta.size; i++){
		written = write(memfd, &ch, 1);
		//lseek(memfd, nextmeta.pos+sizeof(char)-written, SEEK_SET);
	}
	memsize += (newsize - meta.size);
	meta.size = newsize;
	lseek(memfd, meta.pos, SEEK_SET);
	write(memfd, &meta, sizeof(Metadata));

	lseek(memfd, 0, SEEK_SET);
	return 0;
}

int moveChunk(blockid_t startblock, uint16_t pos)
{
	Metadata meta = getMetadata(startblock);
	char data[MAX_READ];
	ssize_t bytes_read;

	if(meta.status != -1){
		lseek(memfd, meta.pos, SEEK_SET);
		bytes_read = read(memfd, &data, MAX_READ);
		printf("bytes_read = %ld \n", bytes_read);
		lseek(memfd, 0, SEEK_SET);
	}
	lseek(memfd, 0, SEEK_SET);
	lseek(memfd, pos, SEEK_SET);
	write(memfd, &data, bytes_read);
	lseek(memfd, 0, SEEK_SET);

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////
void printAllMetadata()
{
	Metadata meta;
	for(int i = 0; i < n_block_id; i++){
		meta = getMetadata(i);
		if(meta.status != -1) {
			printf("Metadata block_id = %d\n", meta.block_id);
			printf("size = %d\n", meta.size);
			printf("pos = %d\n\n", meta.pos);
		}

	}
}
