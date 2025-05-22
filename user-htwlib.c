#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "htwlib.c"

#define MAX_STRING 100

typedef enum{
	ALLOC_BLOCK,
	FREE_BLOCK,
	PUTSTR_BLOCK,
	GET_BLOCKINFO,
	PRINT_ALLBLOCK,
	FREE_ALLBLOCK,
	STOP_PROGRAM,
	N_OPTION
}FMSoption;

int user_FMSmenu();
int user_FMSrun();
int user_FMSalloc();
int user_FMSfree();
int user_FMSfreeAll();
int user_FMSputSTR();
int user_FMSgetBlockInfo();
int user_FMSstop();
int user_FMSprintAllBlocks();

void clearInputBuffer();

int user_FMSmenu()
{
    int option = 0;
    char opttext[N_OPTION][20] = {
		"Allocate new block",
		"Free block i",
		"Put string",
		"Get block info",
		"Print all blocks",
		"Free all blocks",
		"Stop program",
    };

    printf("-------------  File Memory Simulation --------------\n");
    printf("\t Option: \n");
    for(int i=0; i < N_OPTION; i++){
        printf("\t %d. %s\n", i, opttext[i]);
    }
    printf("-----------------------------------\n");
    printf("Enter the option: ");
    scanf(" %d", &option);
    clearInputBuffer();
    printf("-----------------------------------\n");

	return option;
}

int user_FMSrun()
{
    int option;
    char input;

	createMemory();

	while(1){
		system("clear");
		system("xxd memory.txt");

		option = user_FMSmenu();
		switch(option){
			case ALLOC_BLOCK:
				user_FMSalloc();
				break;
			case FREE_BLOCK:
				user_FMSfree();
				break;
			case PUTSTR_BLOCK:
				user_FMSputSTR();
				break;
			case GET_BLOCKINFO:
				user_FMSgetBlockInfo();
				break;
			case PRINT_ALLBLOCK:
				user_FMSprintAllBlocks();
				break;
			case FREE_ALLBLOCK:
				user_FMSfreeAll();
				break;
			case STOP_PROGRAM:
				user_FMSstop();
				break;
		}

	}
	close(memfd);

	return 0;
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int user_FMSalloc()
{
    int input, block_id;
	printf("How many bytes to allocate? 8, 16, 32, 48,...\n > ");
	scanf("%d", &input);
	clearInputBuffer();

	if(input%8 !=0){
		input += 8-(input%8);
	} else if (input <= 0) input = 8;
	block_id = htw_alloc(input);
	printf("%d Bytes have been successfully allocated\n", input);
	printf("The block has id = %d\n", block_id);
	printf("	< Press ENTER to continue >\n");
	getchar();
	return 0;
}

int user_FMSfree()
{
	Metadata meta;
    int input;
	printf("Which block to free?\n > ");
	scanf("%d", &input);
	clearInputBuffer();
	meta = getMetadata(input);
	if(meta.status != -1)
	{
		htw_free(input);
		printf("Block %d has been successfully freed\n", input);
	} else {
		printf("Block %d doesn't exist\n", input);
	}

	printf("	<Press ENTER to continue >\n");
	getchar();

	return 0;
}

int user_FMSfreeAll()
{
	Metadata meta;
	int n = 0;
	for(int i = 0; i <= n_block_id; i++){
		meta = getMetadata(i);
		if(meta.status != -1) htw_free(i);
	}
	printf("%d Blocks have been successfully freed\n", n);
	printf("	<Press ENTER to continue >\n");
	getchar();
	return 0;
}

int user_FMSputSTR()
{
	Metadata meta;
	int input;
    char string[MAX_STRING];
	printf("in which block do you want to put the string in? \n > ");
	scanf("%d", &input);
	clearInputBuffer();

	meta = getMetadata(input);
	if(meta.status == -1){
		printf("The block (id=%d) doesn't exist\n", input);
		printf("	<Press ENTER to continue >\n");
		return 0;
	} else {
		printf("What should the string be?\n > ");
		fgets(string, sizeof(string), stdin);
		htw_put(input, string, strlen(string));
		printf("The string has been successfully put in the block\n");
		printf("	<Press ENTER to continue >\n");
		return 0;
	}
}

int user_FMSgetBlockInfo()
{
    int input;
	Metadata meta;
	
	printf("Which block info?\n > ");
	scanf("%d", &input);
	clearInputBuffer();
	meta = getMetadata(input);

	if(meta.status == -1){
		printf("The block (id=%d) doesn't exist\n", input);
		printf("	<Press ENTER to continue >\n");
		return 0;
	} else {
			printf("Metadata block_id = %d\n", meta.block_id);
			printf("size = %d\n", meta.size);
			printf("pos = %d\n\n", meta.pos);
		printf("	<Press ENTER to continue >\n");
		getchar();
		return 0;
	}
}

int user_FMSprintAllBlocks()
{
	printAllMetadata();
	printf("	<Press ENTER to continue >\n");
	getchar();
	return 0;
}

int user_FMSstop()
{
    int input;
    printf("Exit program? (y/n) ");
    input = getchar();
    if(input == 'y'){
		unlink(MEMORYFILENAME);
        exit(0);
    }
	return 0;
}
