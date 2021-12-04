#include "stdio.h"
#include "stdlib.h"
#include <inttypes.h>

void printBool(int64_t c){
	if (c == 0){ 
		fprintf(stdout, "false"); 
	} else{ 
		fprintf(stdout, "true"); 
	}
	fflush(stdout);
}

void printInt(long int num){
	fprintf(stdout, "%ld", num);
	fflush(stdout);
}

void printString(const char * str){
	fprintf(stdout, "%s", str);
	fflush(stdout);
}

int64_t getBool(){
	char c;
	scanf("%c", &c);
	getchar(); // Consume trailing newline
	if (c == '0'){
		return 0;
	} else {
		return 1;
	}
}

int64_t getInt(){
	char buffer[32];
	fgets(buffer, 32, stdin);
	long int res = atol(buffer);
	return res;
}
