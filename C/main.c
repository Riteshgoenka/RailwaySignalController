/* 
 * Copyright (C) 2012-2014 Chris McClelland
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <makestuff.h>
#include <libfpgalink.h>
#include <libbuffer.h>
#include <liberror.h>
#include <libdump.h>
#include <time.h>
#include <argtable2.h>
#include <readline/readline.h>
#include <readline/history.h>
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif
typedef unsigned char uint8;

int TE[16][16][8],TO[16][16][8];
int NS[16][16][8];
char k[]="11001100110011001100110011000001";
//0-or 1-and 2-xor -boolean function
char fb(char c1,char c2,int c){
   if(c==1){
      if(c1=='0'&&c2=='0')return '0';
      else return '1';
   }
   else if(c==2){
      if(c1=='1'&&c2=='1')return '1';
      else return '0';
   }
   else{
      if(c1==c2)return '0';
      else return '1';
   }
}

//appending 8T's
char* append(char a[]){
   char* T=(char*)malloc(33);
   for(int i=0;i<32;i++){
      T[i]=a[i%4];
   }
   T[32]='\0';
   return (char*)T;
}

//sum of two 4 bit numbers
char* sum(char a[],char b[]){
   char *T=(char*)malloc(5);
   char c='0';
   for(int i=3;i>=0;i--){
      T[i]=fb(fb(a[i],b[i],3),c,3);
      c=fb(fb(fb(a[i],b[i],2),fb(a[i],c,2),1),fb(c,b[i],2),1);
   }
   T[4]='\0';
   return (char*)T;
}

//decrypt function
char* decrypt(char c[],char k[]){
   int n0=0;
   for(int i=0;i<32;i++){
      if(k[i]=='0')n0++;
   }
   char* T=malloc(5*sizeof(char));
   for(int i=0;i<4;i++){
      char temp = k[i];
      for(int j=1;j<8;j++){
         temp = fb(temp,k[i+4*j],3);
      }
      T[i]=temp;
   }
   T[4]='\0';
   char* p=(char*)malloc(5); 
   p=sum("1111",T);
   p[4]='\0';
   for(int i=0;i<4;i++){
    T[i]=p[i];
   }
   char* p1=(char*)malloc(33);
   char* temp=(char*)malloc(33);
   for(int i=0;i<32;i++){
    temp[i]=c[i];
   }
   temp[32]='\0';
   for(int i=0;i<n0;i++){
      p1=append(T);
      p1[32]='\0';
      for(int j=0;j<32;j++){
         temp[j]=fb(temp[j],p1[j],3);
      }
      p=sum("1111",T);
      for(int i=0;i<4;i++){
        T[i]=p[i];
      }
   }
   return(char*)temp;
}

char* encrypt(char c[],char k[]){
   int n0=0;
   for(int i=0;i<32;i++){
      if(k[i]=='1')n0++;
   }
   char* T=(char*)malloc(5);
   for(int i=0;i<4;i++){
      char temp = k[i];
      for(int j=1;j<8;j++){
         temp = fb(temp,k[i+4*j],3);
      }
      T[i]=temp;
   }
   T[4]='\0';
   char* p=(char*)malloc(5); 
   p[4]='\0';
   char* p1=(char*)malloc(33);
   char* temp=(char*)malloc(33);
   for(int i=0;i<32;i++){
    temp[i]=c[i];
   }
   temp[32]='\0';
   for(int i=0;i<n0;i++){
      p1=append(T);
      p1[32]='\0';
      for(int j=0;j<32;j++){
         temp[j]=fb(temp[j],p1[j],3);
      }
      p=sum("0001",T);
      for(int i=0;i<4;i++){
        T[i]=p[i];
      }
   }
   return(char*)temp;
}

//convert into b bits
char* cnt_bit(int a,int b){
   char* p=(char*)malloc(b+1);
   char* p1=(char*)malloc(b+1);
   for(int i=0;i<b;i++){
      p[i]=(a%2 == 0)? '0' :'1';
      a=a/2;
   }
   p[b]='\0';
   for(int i=b-1;i>=0;i--){
      p1[i]=p[b-1-i];
   }
   p1[b]='\0';
   return (char*)p1;
}

//get n-th in a line 
char* getfield(char* line, int num)
{
    char* tok;
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

// converting a base-string into number
int cnt_base(char p[],int base){
    int i=0,n=0;
    while(i<strlen(p)){
        if(p[i]=='1')n=base*n+1;
        else n=base*n;
        i++;
    }
    return n;
}

//csv file read fn
void f(){
   FILE* stream = fopen("/home/sathvik/Documents/logic/20140524/makestuff/apps/flcli/track_data.csv", "r");
   if(stream == NULL){
    printf("no such file\n");
   }
   int x,y,dir,ok,next;
   while (fscanf(stream, "%d,%d,%d,%d,%d", &x, &y, &dir, &ok, &next) != EOF)
   {
        TE[x][y][dir]=1;
        TO[x][y][dir]=ok;
        NS[x][y][dir]=next;

   }
   fclose(stream);
}

char* hex_char(char p[]){
  char* hex=malloc(9);
  char* s=malloc(5);
  for(int i=0;i<8;i++){
    for(int j=0;j<4;j++){
      s[j]=p[4*i+j];
    }
    s[4]='\0';
    int n=cnt_base(s,2);
    if(n==0)hex[i]='0';
    else if(n==1)hex[i]='1';
    else if(n==2)hex[i]='2';
    else if(n==3)hex[i]='3';
    else if(n==4)hex[i]='4';
    else if(n==5)hex[i]='5';
    else if(n==6)hex[i]='6';
    else if(n==7)hex[i]='7';
    else if(n==8)hex[i]='8';
    else if(n==9)hex[i]='9';
    else if(n==10)hex[i]='A';
    else if(n==11)hex[i]='B';
    else if(n==12)hex[i]='C';
    else if(n==13)hex[i]='D';
    else if(n==14)hex[i]='E';
    else hex[i]='F';
  }
  hex[8]='\0';
  return (char*)hex;
} 

void enc_ack(uint8 a1,uint8 a2,uint8 a3,uint8 a4,uint8 *ha){
   char* AM=(char*)malloc(33);
   char* l0=(char*)malloc(9);
   char* l1=(char*)malloc(9);
   char* l2=(char*)malloc(9);
   char* l3=(char*)malloc(9);
   l0=cnt_bit(a1,8);l1=cnt_bit(a2,8);l2=cnt_bit(a3,8);l3=cnt_bit(a4,8);
   l0[8]='\0';l1[8]='\0';l2[8]='\0';l3[8]='\0';
   for(int i=0;i<8;i++){
    AM[i]=l0[i];
   }
   for(int i=0;i<8;i++){
    AM[8+i]=l1[i];
   }
   for(int i=0;i<8;i++){
    AM[16+i]=l2[i];
   }
   for(int i=0;i<8;i++){
    AM[24+i]=l3[i];
   }
   AM[32]='\0';
   char* EA=encrypt(AM,k);
   char* u=(char*)malloc(9);
   for(int i=0;i<4;i++){
    for(int j=0;j<8;j++){
      u[j]=EA[8*i+j];
    }
    u[8]='\0';
    ha[i]=cnt_base(u,2);
   }
}

void dec_ack(uint8 a1,uint8 a2,uint8 a3,uint8 a4,uint8 *ha){
   char* AM=(char*)malloc(33);
   char* l0=(char*)malloc(9);
   char* l1=(char*)malloc(9);
   char* l2=(char*)malloc(9);
   char* l3=(char*)malloc(9);
   l0=cnt_bit(a1,8);l1=cnt_bit(a2,8);l2=cnt_bit(a3,8);l3=cnt_bit(a4,8);
   l0[8]='\0';l1[8]='\0';l2[8]='\0';l3[8]='\0';
   for(int i=0;i<8;i++){
    AM[i]=l0[i];
   }
   for(int i=0;i<8;i++){
    AM[8+i]=l1[i];
   }
   for(int i=0;i<8;i++){
    AM[16+i]=l2[i];
   }
   for(int i=0;i<8;i++){
    AM[24+i]=l3[i];
   }
   AM[32]='\0';
   char* EA=decrypt(AM,k);
   char* u=(char*)malloc(9);
   for(int i=0;i<4;i++){
    for(int j=0;j<8;j++){
      u[j]=EA[8*i+j];
    }
    u[8]='\0';
    ha[i]=cnt_base(u,2);
   }
}

void finalencrypt(uint8 a1,uint8 a2,uint8 a3,uint8 a4,uint8 *ha,uint8 *sa){
   char* AM=(char*)malloc(33);
   char* l0=(char*)malloc(9);
   char* l1=(char*)malloc(9);
   char* l2=(char*)malloc(9);
   char* l3=(char*)malloc(9);
   l0=cnt_bit(a1,8);l1=cnt_bit(a2,8);l2=cnt_bit(a3,8);l3=cnt_bit(a4,8);
   l0[8]='\0';l1[8]='\0';l2[8]='\0';l3[8]='\0';
   for(int i=0;i<8;i++){
    AM[i]=l0[i];
   }
   for(int i=0;i<8;i++){
    AM[8+i]=l1[i];
   }
   for(int i=0;i<8;i++){
    AM[16+i]=l2[i];
   }
   for(int i=0;i<8;i++){
    AM[24+i]=l3[i];
   }
   AM[32]='\0';
   char* dcrypt=(char*)malloc(33); 
   dcrypt=decrypt(AM,k);

   char* x=(char*)malloc(5);
   char* y=(char*)malloc(5);
   x[0]=dcrypt[24];x[1]=dcrypt[25];x[2]=dcrypt[26];x[3]=dcrypt[27];
   y[0]=dcrypt[28];y[1]=dcrypt[29];y[2]=dcrypt[30];y[3]=dcrypt[31];
   x[4]='\0';y[4]='\0';
   int xn=cnt_base(x,2),yn=cnt_base(y,2);

   char* s=(char*)malloc(65);
   char* ns=(char*)malloc(4);
   char* dir=(char*)malloc(4);
   char* to=(char*)malloc(2);
   char* te=(char*)malloc(2);
   ns[3]='\0';dir[3]='\0';to[1]='\0';te[1]='\0';
   for(int i=0;i<8;i++){
      ns=cnt_bit(NS[xn][yn][i],3);
      dir=cnt_bit(i,3);
      to=cnt_bit(TO[xn][yn][i],1);
      te=cnt_bit(TE[xn][yn][i],1);
      s[8*i+5]=ns[0];s[8*i+6]=ns[1];s[8*i+7]=ns[2];s[8*i]=dir[0];s[8*i+1]=dir[1];s[8*i+2]=dir[2];s[8*i+4]=to[0];s[8*i+3]=te[0];
      //printf("%c%c%c-%c%c%c-%c-%c\n",s[8*i],s[8*i+1],s[8*i+2],s[8*i+3],s[8*i+4],s[8*i+5],s[8*i+6],s[8*i+7]);
   }
   s[64]='\0';
   char* s1=(char*)malloc(33);
   char* s2=(char*)malloc(33);
   for(int i=0;i<32;i++){
    s1[i]=s[i];
   }
   s1[32]='\0';
   for(int i=32;i<64;i++){
      s2[i-32]=s[i];
   }
   s2[32]='\0';
   char* fh=encrypt(s1,k);
   char* sh=encrypt(s2,k);
   fh[32]='\0';sh[32]='\0';
   char* u=(char*)malloc(9);
   for(int i=0;i<4;i++){
    for(int j=0;j<8;j++){
      u[j]=fh[8*i+j];
    }
    u[8]='\0';
    ha[i]=cnt_base(u,2);
    //printf("%s\n",u);
   }
   for(int i=0;i<4;i++){
    for(int j=0;j<8;j++){
      u[j]=sh[8*i+j];
    }
    u[8]='\0';
    sa[i]=cnt_base(u,2);
    //printf("%s\n",u);
   }
}

bool sigIsRaised(void);
void sigRegisterHandler(void);
static struct Buffer ridersdata = {0,};


static const char *ptr;
static bool enableBenchmarking = false;

static bool isHexDigit(char ch) {
	return
		(ch >= '0' && ch <= '9') ||
		(ch >= 'a' && ch <= 'f') ||
		(ch >= 'A' && ch <= 'F');
}

static uint16 calcChecksum(const uint8 *data, size_t length) {
	uint16 cksum = 0x0000;
	while ( length-- ) {
		cksum = (uint16)(cksum + *data++);
	}
	return cksum;
}

static bool getHexNibble(char hexDigit, uint8 *nibble) {
	if ( hexDigit >= '0' && hexDigit <= '9' ) {
		*nibble = (uint8)(hexDigit - '0');
		return false;
	} else if ( hexDigit >= 'a' && hexDigit <= 'f' ) {
		*nibble = (uint8)(hexDigit - 'a' + 10);
		return false;
	} else if ( hexDigit >= 'A' && hexDigit <= 'F' ) {
		*nibble = (uint8)(hexDigit - 'A' + 10);
		return false;
	} else {
		return true;
	}
}

static int getHexByte(uint8 *byte) {
	uint8 upperNibble;
	uint8 lowerNibble;
	if ( !getHexNibble(ptr[0], &upperNibble) && !getHexNibble(ptr[1], &lowerNibble) ) {
		*byte = (uint8)((upperNibble << 4) | lowerNibble);
		byte += 2;
		return 0;
	} else {
		return 1;
	}
}

static const char *const errMessages[] = {
	NULL,
	NULL,
	"Unparseable hex number",
	"Channel out of range",
	"Conduit out of range",
	"Illegal character",
	"Unterminated string",
	"No memory",
	"Empty string",
	"Odd number of digits",
	"Cannot load file",
	"Cannot save file",
	"Bad arguments"
};

typedef enum {
	FLP_SUCCESS,
	FLP_LIBERR,
	FLP_BAD_HEX,
	FLP_CHAN_RANGE,
	FLP_CONDUIT_RANGE,
	FLP_ILL_CHAR,
	FLP_UNTERM_STRING,
	FLP_NO_MEMORY,
	FLP_EMPTY_STRING,
	FLP_ODD_DIGITS,
	FLP_CANNOT_LOAD,
	FLP_CANNOT_SAVE,
	FLP_ARGS
} ReturnCode;

static ReturnCode doRead(
	struct FLContext *handle, uint8 chan, uint32 length, FILE *destFile, uint16 *checksum,
	const char **error)
{
	ReturnCode retVal = FLP_SUCCESS;
	uint32 bytesWritten;
	FLStatus fStatus;
	uint32 chunkSize;
	const uint8 *recvData;
	uint32 actualLength;
	const uint8 *ptr;
	uint16 csVal = 0x0000;
	#define READ_MAX 65536

	// Read first chunk
	chunkSize = length >= READ_MAX ? READ_MAX : length;
	fStatus = flReadChannelAsyncSubmit(handle, chan, chunkSize, NULL, error);
	CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doRead()");
	length = length - chunkSize;

	while ( length ) {
		// Read chunk N
		chunkSize = length >= READ_MAX ? READ_MAX : length;
		fStatus = flReadChannelAsyncSubmit(handle, chan, chunkSize, NULL, error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doRead()");
		length = length - chunkSize;
		
		// Await chunk N-1
		fStatus = flReadChannelAsyncAwait(handle, &recvData, &actualLength, &actualLength, error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doRead()");

		// Write chunk N-1 to file
		bytesWritten = (uint32)fwrite(recvData, 1, actualLength, destFile);
		CHECK_STATUS(bytesWritten != actualLength, FLP_CANNOT_SAVE, cleanup, "doRead()");

		// Checksum chunk N-1
		chunkSize = actualLength;
		ptr = recvData;
		while ( chunkSize-- ) {
			csVal = (uint16)(csVal + *ptr++);
		}
	}

	// Await last chunk
	fStatus = flReadChannelAsyncAwait(handle, &recvData, &actualLength, &actualLength, error);
	CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doRead()");
	
	// Write last chunk to file
	bytesWritten = (uint32)fwrite(recvData, 1, actualLength, destFile);
	CHECK_STATUS(bytesWritten != actualLength, FLP_CANNOT_SAVE, cleanup, "doRead()");

	// Checksum last chunk
	chunkSize = actualLength;
	ptr = recvData;
	while ( chunkSize-- ) {
		csVal = (uint16)(csVal + *ptr++);
	}
	
	// Return checksum to caller
	*checksum = csVal;
cleanup:
	return retVal;
}

static ReturnCode doWrite(
	struct FLContext *handle, uint8 chan, FILE *srcFile, size_t *length, uint16 *checksum,
	const char **error)
{
	ReturnCode retVal = FLP_SUCCESS;
	size_t bytesRead, i;
	FLStatus fStatus;
	const uint8 *ptr;
	uint16 csVal = 0x0000;
	size_t lenVal = 0;
	#define WRITE_MAX (65536 - 5)
	uint8 buffer[WRITE_MAX];

	do {
		// Read Nth chunk
		bytesRead = fread(buffer, 1, WRITE_MAX, srcFile);
		if ( bytesRead ) {
			// Update running total
			lenVal = lenVal + bytesRead;

			// Submit Nth chunk
			fStatus = flWriteChannelAsync(handle, chan, bytesRead, buffer, error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doWrite()");

			// Checksum Nth chunk
			i = bytesRead;
			ptr = buffer;
			while ( i-- ) {
				csVal = (uint16)(csVal + *ptr++);
			}
		}
	} while ( bytesRead == WRITE_MAX );

	// Wait for writes to be received. This is optional, but it's only fair if we're benchmarking to
	// actually wait for the work to be completed.
	fStatus = flAwaitAsyncWrites(handle, error);
	CHECK_STATUS(fStatus, FLP_LIBERR, cleanup, "doWrite()");

	// Return checksum & length to caller
	*checksum = csVal;
	*length = lenVal;
cleanup:
	return retVal;
}

static int parseLine(struct FLContext *handle, const char *line, const char **error) {
	ReturnCode retVal = FLP_SUCCESS, status;
	FLStatus fStatus;
	struct Buffer dataFromFPGA = {0,};
	BufferStatus bStatus;
	uint8 *data = NULL;
	char *fileName = NULL;
	FILE *file = NULL;
	double totalTime, speed;
	#ifdef WIN32
		LARGE_INTEGER tvStart, tvEnd, freq;
		DWORD_PTR mask = 1;
		SetThreadAffinityMask(GetCurrentThread(), mask);
		QueryPerformanceFrequency(&freq);
	#else
		struct timeval tvStart, tvEnd;
		long long startTime, endTime;
	#endif
	bStatus = bufInitialise(&dataFromFPGA, 1024, 0x00, error);
	CHECK_STATUS(bStatus, FLP_LIBERR, cleanup);
	ptr = line;
	do {
		while ( *ptr == ';' ) {
			ptr++;
		}
		switch ( *ptr ) {
		case 'r':{
			uint32 chan;
			uint32 length = 1;
			char *end;
			ptr++;
			
			// Get the channel to be read:
			errno = 0;
			chan = (uint32)strtoul(ptr, &end, 16);
			CHECK_STATUS(errno, FLP_BAD_HEX, cleanup);

			// Ensure that it's 0-127
			CHECK_STATUS(chan > 127, FLP_CHAN_RANGE, cleanup);
			ptr = end;

			// Only three valid chars at this point:
			CHECK_STATUS(*ptr != '\0' && *ptr != ';' && *ptr != ' ', FLP_ILL_CHAR, cleanup);

			if ( *ptr == ' ' ) {
				ptr++;

				// Get the read count:
				errno = 0;
				length = (uint32)strtoul(ptr, &end, 16);
				CHECK_STATUS(errno, FLP_BAD_HEX, cleanup);
				ptr = end;
				
				// Only three valid chars at this point:
				CHECK_STATUS(*ptr != '\0' && *ptr != ';' && *ptr != ' ', FLP_ILL_CHAR, cleanup);
				if ( *ptr == ' ' ) {
					const char *p;
					const char quoteChar = *++ptr;
					CHECK_STATUS(
						(quoteChar != '"' && quoteChar != '\''),
						FLP_ILL_CHAR, cleanup);
					
					// Get the file to write bytes to:
					ptr++;
					p = ptr;
					while ( *p != quoteChar && *p != '\0' ) {
						p++;
					}
					CHECK_STATUS(*p == '\0', FLP_UNTERM_STRING, cleanup);
					fileName = malloc((size_t)(p - ptr + 1));
					CHECK_STATUS(!fileName, FLP_NO_MEMORY, cleanup);
					CHECK_STATUS(p - ptr == 0, FLP_EMPTY_STRING, cleanup);
					strncpy(fileName, ptr, (size_t)(p - ptr));
					fileName[p - ptr] = '\0';
					ptr = p + 1;
				}
			}
			if ( fileName ) {
				uint16 checksum = 0x0000;

				// Open file for writing
				file = fopen(fileName, "wb");
				CHECK_STATUS(!file, FLP_CANNOT_SAVE, cleanup);
				free(fileName);
				fileName = NULL;

				#ifdef WIN32
					QueryPerformanceCounter(&tvStart);
					status = doRead(handle, (uint8)chan, length, file, &checksum, error);
					QueryPerformanceCounter(&tvEnd);
					totalTime = (double)(tvEnd.QuadPart - tvStart.QuadPart);
					totalTime /= freq.QuadPart;
					speed = (double)length / (1024*1024*totalTime);
				#else
					gettimeofday(&tvStart, NULL);
					status = doRead(handle, (uint8)chan, length, file, &checksum, error);
					gettimeofday(&tvEnd, NULL);
					startTime = tvStart.tv_sec;
					startTime *= 1000000;
					startTime += tvStart.tv_usec;
					endTime = tvEnd.tv_sec;
					endTime *= 1000000;
					endTime += tvEnd.tv_usec;
					totalTime = (double)(endTime - startTime);
					totalTime /= 1000000;  // convert from uS to S.
					speed = (double)length / (1024*1024*totalTime);
				#endif
				if ( enableBenchmarking ) {
					printf(
						"Read %d bytes (checksum 0x%04X) from channel %d at %f MiB/s\n",
						length, checksum, chan, speed);
				}
				CHECK_STATUS(status, status, cleanup);

				// Close the file
				fclose(file);
				file = NULL;
			} 
			else {
				size_t oldLength = dataFromFPGA.length;
				bStatus = bufAppendConst(&dataFromFPGA, 0x00, length, error);
				CHECK_STATUS(bStatus, FLP_LIBERR, cleanup);
				#ifdef WIN32
					QueryPerformanceCounter(&tvStart);
					fStatus = flReadChannel(handle, (uint8)chan, length, dataFromFPGA.data + oldLength, error);
					QueryPerformanceCounter(&tvEnd);
					totalTime = (double)(tvEnd.QuadPart - tvStart.QuadPart);
					totalTime /= freq.QuadPart;
					speed = (double)length / (1024*1024*totalTime);
				#else
					gettimeofday(&tvStart, NULL);
					fStatus = flReadChannel(handle, (uint8)chan, length, dataFromFPGA.data + oldLength, error);
					gettimeofday(&tvEnd, NULL);
					startTime = tvStart.tv_sec;
					startTime *= 1000000;
					startTime += tvStart.tv_usec;
					endTime = tvEnd.tv_sec;
					endTime *= 1000000;
					endTime += tvEnd.tv_usec;
					totalTime = (double)(endTime - startTime);
					totalTime /= 1000000;  // convert from uS to S.
					speed = (double)length / (1024*1024*totalTime);
				#endif
				if ( enableBenchmarking ) {
					printf(
						"Read %d bytes (checksum 0x%04X) from channel %d at %f MiB/s\n",
						length, calcChecksum(dataFromFPGA.data + oldLength, length), chan, speed);
				}
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			}
			ridersdata = dataFromFPGA;
			break;
		}
		case 'w':{
			unsigned long int chan;
			size_t length = 1, i;
			char *end, ch;
			const char *p;
			ptr++;
			
			// Get the channel to be written:
			errno = 0;
			chan = strtoul(ptr, &end, 16);
			CHECK_STATUS(errno, FLP_BAD_HEX, cleanup);

			// Ensure that it's 0-127
			CHECK_STATUS(chan > 127, FLP_CHAN_RANGE, cleanup);
			ptr = end;

			// There must be a space now:
			CHECK_STATUS(*ptr != ' ', FLP_ILL_CHAR, cleanup);

			// Now either a quote or a hex digit
		   ch = *++ptr;
			if ( ch == '"' || ch == '\'' ) {
				uint16 checksum = 0x0000;

				// Get the file to read bytes from:
				ptr++;
				p = ptr;
				while ( *p != ch && *p != '\0' ) {
					p++;
				}
				CHECK_STATUS(*p == '\0', FLP_UNTERM_STRING, cleanup);
				fileName = malloc((size_t)(p - ptr + 1));
				CHECK_STATUS(!fileName, FLP_NO_MEMORY, cleanup);
				CHECK_STATUS(p - ptr == 0, FLP_EMPTY_STRING, cleanup);
				strncpy(fileName, ptr, (size_t)(p - ptr));
				fileName[p - ptr] = '\0';
				ptr = p + 1;  // skip over closing quote

				// Open file for reading
				file = fopen(fileName, "rb");
				CHECK_STATUS(!file, FLP_CANNOT_LOAD, cleanup);
				free(fileName);
				fileName = NULL;
				
				#ifdef WIN32
					QueryPerformanceCounter(&tvStart);
					status = doWrite(handle, (uint8)chan, file, &length, &checksum, error);
					QueryPerformanceCounter(&tvEnd);
					totalTime = (double)(tvEnd.QuadPart - tvStart.QuadPart);
					totalTime /= freq.QuadPart;
					speed = (double)length / (1024*1024*totalTime);
				#else
					gettimeofday(&tvStart, NULL);
					status = doWrite(handle, (uint8)chan, file, &length, &checksum, error);
					gettimeofday(&tvEnd, NULL);
					startTime = tvStart.tv_sec;
					startTime *= 1000000;
					startTime += tvStart.tv_usec;
					endTime = tvEnd.tv_sec;
					endTime *= 1000000;
					endTime += tvEnd.tv_usec;
					totalTime = (double)(endTime - startTime);
					totalTime /= 1000000;  // convert from uS to S.
					speed = (double)length / (1024*1024*totalTime);
				#endif
				if ( enableBenchmarking ) {
					printf(
						"Wrote "PFSZD" bytes (checksum 0x%04X) to channel %lu at %f MiB/s\n",
						length, checksum, chan, speed);
				}
				CHECK_STATUS(status, status, cleanup);

				// Close the file
				fclose(file);
				file = NULL;
			} 
			else if ( isHexDigit(ch) ) {
				// Read a sequence of hex bytes to write
				uint8 *dataPtr;
				p = ptr + 1;
				while ( isHexDigit(*p) ) {
					p++;
				}
				CHECK_STATUS((p - ptr) & 1, FLP_ODD_DIGITS, cleanup);
				length = (size_t)(p - ptr) / 2;
				data = malloc(length);
				dataPtr = data;
				for ( i = 0; i < length; i++ ) {
					getHexByte(dataPtr++);
					ptr += 2;
				}
				#ifdef WIN32
					QueryPerformanceCounter(&tvStart);
					fStatus = flWriteChannel(handle, (uint8)chan, length, data, error);
					QueryPerformanceCounter(&tvEnd);
					totalTime = (double)(tvEnd.QuadPart - tvStart.QuadPart);
					totalTime /= freq.QuadPart;
					speed = (double)length / (1024*1024*totalTime);
				#else
					gettimeofday(&tvStart, NULL);
					fStatus = flWriteChannel(handle, (uint8)chan, length, data, error);
					gettimeofday(&tvEnd, NULL);
					startTime = tvStart.tv_sec;
					startTime *= 1000000;
					startTime += tvStart.tv_usec;
					endTime = tvEnd.tv_sec;
					endTime *= 1000000;
					endTime += tvEnd.tv_usec;
					totalTime = (double)(endTime - startTime);
					totalTime /= 1000000;  // convert from uS to S.
					speed = (double)length / (1024*1024*totalTime);
				#endif
				if ( enableBenchmarking ) {
					printf(
						"Wrote "PFSZD" bytes (checksum 0x%04X) to channel %lu at %f MiB/s\n",
						length, calcChecksum(data, length), chan, speed);
				}
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				free(data);
				data = NULL;
			} else {
				FAIL(FLP_ILL_CHAR, cleanup);
			}
			break;
		}
		case '+':{
			uint32 conduit;
			char *end;
			ptr++;

			// Get the conduit
			errno = 0;
			conduit = (uint32)strtoul(ptr, &end, 16);
			CHECK_STATUS(errno, FLP_BAD_HEX, cleanup);

			// Ensure that it's 0-127
			CHECK_STATUS(conduit > 255, FLP_CONDUIT_RANGE, cleanup);
			ptr = end;

			// Only two valid chars at this point:
			CHECK_STATUS(*ptr != '\0' && *ptr != ';', FLP_ILL_CHAR, cleanup);

			fStatus = flSelectConduit(handle, (uint8)conduit, error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			break;
		}
		default:
			FAIL(FLP_ILL_CHAR, cleanup);
		}
	} while ( *ptr == ';' );
	CHECK_STATUS(*ptr != '\0', FLP_ILL_CHAR, cleanup);

	dump(0x00000000, dataFromFPGA.data, dataFromFPGA.length);

cleanup:
	bufDestroy(&dataFromFPGA);
	if ( file ) {
		fclose(file);
	}
	free(fileName);
	free(data);
	if ( retVal > FLP_LIBERR ) {
		const int column = (int)(ptr - line);
		int i;
		fprintf(stderr, "%s at column %d\n  %s\n  ", errMessages[retVal], column, line);
		for ( i = 0; i < column; i++ ) {
			fprintf(stderr, " ");
		}
		fprintf(stderr, "^\n");
	}
	return retVal;
}

static const char *nibbles[] = {
	"0000",  // '0'
	"0001",  // '1'
	"0010",  // '2'
	"0011",  // '3'
	"0100",  // '4'
	"0101",  // '5'
	"0110",  // '6'
	"0111",  // '7'
	"1000",  // '8'
	"1001",  // '9'

	"XXXX",  // ':'
	"XXXX",  // ';'
	"XXXX",  // '<'
	"XXXX",  // '='
	"XXXX",  // '>'
	"XXXX",  // '?'
	"XXXX",  // '@'

	"1010",  // 'A'
	"1011",  // 'B'
	"1100",  // 'C'
	"1101",  // 'D'
	"1110",  // 'E'
	"1111"   // 'F'
};

int main(int argc, char *argv[]) {
	ReturnCode retVal = FLP_SUCCESS, pStatus;
	struct arg_str *ivpOpt = arg_str0("i", "ivp", "<VID:PID>", "            vendor ID and product ID (e.g 04B4:8613)");
	struct arg_str *vpOpt = arg_str1("v", "vp", "<VID:PID[:DID]>", "       VID, PID and opt. dev ID (e.g 1D50:602B:0001)");
	struct arg_str *fwOpt = arg_str0("f", "fw", "<firmware.hex>", "        firmware to RAM-load (or use std fw)");
	struct arg_str *portOpt = arg_str0("d", "ports", "<bitCfg[,bitCfg]*>", " read/write digital ports (e.g B13+,C1-,B2?)");
	struct arg_str *queryOpt = arg_str0("q", "query", "<jtagBits>", "         query the JTAG chain");
	struct arg_str *progOpt = arg_str0("p", "program", "<config>", "         program a device");
	struct arg_uint *conOpt = arg_uint0("c", "conduit", "<conduit>", "        which comm conduit to choose (default 0x01)");
	struct arg_str *actOpt = arg_str0("a", "action", "<actionString>", "    a series of CommFPGA actions");
	struct arg_lit *shellOpt  = arg_lit0("s", "shell", "                    start up an interactive CommFPGA session");
	struct arg_lit *ridersOpt  = arg_lit0("m", "riders", "                    start up an interactive CommFPGA session");
	struct arg_lit *benOpt  = arg_lit0("b", "benchmark", "                enable benchmarking & checksumming");
	struct arg_lit *rstOpt  = arg_lit0("r", "reset", "                    reset the bulk endpoints");
	struct arg_str *dumpOpt = arg_str0("l", "dumploop", "<ch:file.bin>", "   write data from channel ch to file");
	struct arg_lit *helpOpt  = arg_lit0("h", "help", "                     print this help and exit");
	struct arg_str *eepromOpt  = arg_str0(NULL, "eeprom", "<std|fw.hex|fw.iic>", "   write firmware to FX2's EEPROM (!!)");
	struct arg_str *backupOpt  = arg_str0(NULL, "backup", "<kbitSize:fw.iic>", "     backup FX2's EEPROM (e.g 128:fw.iic)\n");
	struct arg_end *endOpt   = arg_end(20);
	void *argTable[] = {
		ivpOpt, vpOpt, fwOpt, portOpt, queryOpt, progOpt, conOpt, actOpt,
		shellOpt, ridersOpt, benOpt, rstOpt, dumpOpt, helpOpt, eepromOpt, backupOpt, endOpt
	};
	const char *progName = "flcli";
	int numErrors;
	struct FLContext *handle = NULL;
	FLStatus fStatus;
	const char *error = NULL;
	const char *ivp = NULL;
	const char *vp = NULL;
	bool isNeroCapable, isCommCapable;
	uint32 numDevices, scanChain[16], i;
	const char *line = NULL;
	uint8 conduit = 0x01;

	if ( arg_nullcheck(argTable) != 0 ) {
		fprintf(stderr, "%s: insufficient memory\n", progName);
		FAIL(1, cleanup);
	}

	numErrors = arg_parse(argc, argv, argTable);

	if ( helpOpt->count > 0 ) {
		printf("FPGALink Command-Line Interface Copyright (C) 2012-2014 Chris McClelland\n\nUsage: %s", progName);
		arg_print_syntax(stdout, argTable, "\n");
		printf("\nInteract with an FPGALink device.\n\n");
		arg_print_glossary(stdout, argTable,"  %-10s %s\n");
		FAIL(FLP_SUCCESS, cleanup);
	}

	if ( numErrors > 0 ) {
		arg_print_errors(stdout, endOpt, progName);
		fprintf(stderr, "Try '%s --help' for more information.\n", progName);
		FAIL(FLP_ARGS, cleanup);
	}

	fStatus = flInitialise(0, &error);
	CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);

	vp = vpOpt->sval[0];

	printf("Attempting to open connection to FPGALink device %s...\n", vp);
	fStatus = flOpen(vp, &handle, NULL);
	if ( fStatus ) {
		if ( ivpOpt->count ) {
			int count = 60;
			uint8 flag;
			ivp = ivpOpt->sval[0];
			printf("Loading firmware into %s...\n", ivp);
			if ( fwOpt->count ) {
				fStatus = flLoadCustomFirmware(ivp, fwOpt->sval[0], &error);
			} else {
				fStatus = flLoadStandardFirmware(ivp, vp, &error);
			}
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			
			printf("Awaiting renumeration");
			flSleep(1000);
			do {
				printf(".");
				fflush(stdout);
				fStatus = flIsDeviceAvailable(vp, &flag, &error);
				CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
				flSleep(250);
				count--;
			} while ( !flag && count );
			printf("\n");
			if ( !flag ) {
				fprintf(stderr, "FPGALink device did not renumerate properly as %s\n", vp);
				FAIL(FLP_LIBERR, cleanup);
			}

			printf("Attempting to open connection to FPGLink device %s again...\n", vp);
			fStatus = flOpen(vp, &handle, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		} else {
			fprintf(stderr, "Could not open FPGALink device at %s and no initial VID:PID was supplied\n", vp);
			FAIL(FLP_ARGS, cleanup);
		}
	}

	printf(
		"Connected to FPGALink device %s (firmwareID: 0x%04X, firmwareVersion: 0x%08X)\n",
		vp, flGetFirmwareID(handle), flGetFirmwareVersion(handle)
	);

	if ( eepromOpt->count ) {
		if ( !strcmp("std", eepromOpt->sval[0]) ) {
			printf("Writing the standard FPGALink firmware to the FX2's EEPROM...\n");
			fStatus = flFlashStandardFirmware(handle, vp, &error);
		} else {
			printf("Writing custom FPGALink firmware from %s to the FX2's EEPROM...\n", eepromOpt->sval[0]);
			fStatus = flFlashCustomFirmware(handle, eepromOpt->sval[0], &error);
		}
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
	}

	if ( backupOpt->count ) {
		const char *fileName;
		const uint32 kbitSize = strtoul(backupOpt->sval[0], (char**)&fileName, 0);
		if ( *fileName != ':' ) {
			fprintf(stderr, "%s: invalid argument to option --backup=<kbitSize:fw.iic>\n", progName);
			FAIL(FLP_ARGS, cleanup);
		}
		fileName++;
		printf("Saving a backup of %d kbit from the FX2's EEPROM to %s...\n", kbitSize, fileName);
		fStatus = flSaveFirmware(handle, kbitSize, fileName, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
	}

	if ( rstOpt->count ) {
		// Reset the bulk endpoints (only needed in some virtualised environments)
		fStatus = flResetToggle(handle, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
	}

	if ( conOpt->count ) {
		conduit = (uint8)conOpt->ival[0];
	}

	isNeroCapable = flIsNeroCapable(handle);
	isCommCapable = flIsCommCapable(handle, conduit);

	if ( portOpt->count ) {
		uint32 readState;
		char hex[9];
		const uint8 *p = (const uint8 *)hex;
		printf("Configuring ports...\n");
		fStatus = flMultiBitPortAccess(handle, portOpt->sval[0], &readState, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		sprintf(hex, "%08X", readState);
		printf("Readback:   28   24   20   16    12    8    4    0\n          %s", nibbles[*p++ - '0']);
		printf(" %s", nibbles[*p++ - '0']);
		printf(" %s", nibbles[*p++ - '0']);
		printf(" %s", nibbles[*p++ - '0']);
		printf("  %s", nibbles[*p++ - '0']);
		printf(" %s", nibbles[*p++ - '0']);
		printf(" %s", nibbles[*p++ - '0']);
		printf(" %s\n", nibbles[*p++ - '0']);
		flSleep(100);
	}

	if ( queryOpt->count ) {
		if ( isNeroCapable ) {
			fStatus = flSelectConduit(handle, 0x00, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = jtagScanChain(handle, queryOpt->sval[0], &numDevices, scanChain, 16, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			if ( numDevices ) {
				printf("The FPGALink device at %s scanned its JTAG chain, yielding:\n", vp);
				for ( i = 0; i < numDevices; i++ ) {
					printf("  0x%08X\n", scanChain[i]);
				}
			} else {
				printf("The FPGALink device at %s scanned its JTAG chain but did not find any attached devices\n", vp);
			}
		} else {
			fprintf(stderr, "JTAG chain scan requested but FPGALink device at %s does not support NeroProg\n", vp);
			FAIL(FLP_ARGS, cleanup);
		}
	}

	if ( progOpt->count ) {
		printf("Programming device...\n");
		if ( isNeroCapable ) {
			fStatus = flSelectConduit(handle, 0x00, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flProgram(handle, progOpt->sval[0], NULL, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		} else {
			fprintf(stderr, "Program operation requested but device at %s does not support NeroProg\n", vp);
			FAIL(FLP_ARGS, cleanup);
		}
	}

	if ( benOpt->count ) {
		enableBenchmarking = true;
	}
	
	if ( actOpt->count ) {
		printf("Executing CommFPGA actions on FPGALink device %s...\n", vp);
		if ( isCommCapable ) {
			uint8 isRunning;
			fStatus = flSelectConduit(handle, conduit, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flIsFPGARunning(handle, &isRunning, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			if ( isRunning ) {
				pStatus = parseLine(handle, actOpt->sval[0], &error);
				CHECK_STATUS(pStatus, pStatus, cleanup);
			} else {
				fprintf(stderr, "The FPGALink device at %s is not ready to talk - did you forget --program?\n", vp);
				FAIL(FLP_ARGS, cleanup);
			}
		} else {
			fprintf(stderr, "Action requested but device at %s does not support CommFPGA\n", vp);
			FAIL(FLP_ARGS, cleanup);
		}
	}

	if ( dumpOpt->count ) {
		const char *fileName;
		unsigned long chan = strtoul(dumpOpt->sval[0], (char**)&fileName, 10);
		FILE *file = NULL;
		const uint8 *recvData;
		uint32 actualLength;
		if ( *fileName != ':' ) {
			fprintf(stderr, "%s: invalid argument to option -l|--dumploop=<ch:file.bin>\n", progName);
			FAIL(FLP_ARGS, cleanup);
		}
		fileName++;
		printf("Copying from channel %lu to %s", chan, fileName);
		file = fopen(fileName, "wb");
		CHECK_STATUS(!file, FLP_CANNOT_SAVE, cleanup);
		sigRegisterHandler();
		fStatus = flSelectConduit(handle, conduit, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		fStatus = flReadChannelAsyncSubmit(handle, (uint8)chan, 22528, NULL, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		do {
			fStatus = flReadChannelAsyncSubmit(handle, (uint8)chan, 22528, NULL, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flReadChannelAsyncAwait(handle, &recvData, &actualLength, &actualLength, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fwrite(recvData, 1, actualLength, file);
			printf(".");
		} while ( !sigIsRaised() );
		printf("\nCaught SIGINT, quitting...\n");
		fStatus = flReadChannelAsyncAwait(handle, &recvData, &actualLength, &actualLength, &error);
		CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
		fwrite(recvData, 1, actualLength, file);
		fclose(file);
	}

	if ( shellOpt->count ) {
		printf("\nEntering CommFPGA command-line mode:\n");
		if ( isCommCapable ) {
		   uint8 isRunning;
			fStatus = flSelectConduit(handle, conduit, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			fStatus = flIsFPGARunning(handle, &isRunning, &error);
			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
			if ( isRunning ) {
				do {
					do {
						line = readline("> ");
					} while ( line && !line[0] );
					if ( line && line[0] && line[0] != 'q' ) {
						add_history(line);
						pStatus = parseLine(handle, line, &error);
						CHECK_STATUS(pStatus, pStatus, cleanup);
						free((void*)line);
					}
				} while ( line && line[0] != 'q' );
			} else {
				fprintf(stderr, "The FPGALink device at %s is not ready to talk - did you forget --xsvf?\n", vp);
				FAIL(FLP_ARGS, cleanup);
			}
		} else {
			fprintf(stderr, "Shell requested but device at %s does not support CommFPGA\n", vp);
			FAIL(FLP_ARGS, cleanup);
		}
	}

	if ( ridersOpt->count ) {
		printf("\nEntering CommFPGA command-line mode:\n");
		if ( isCommCapable ) {
		   	uint8 isRunning;
  			fStatus = flSelectConduit(handle, conduit, &error);
  			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
  			fStatus = flIsFPGARunning(handle, &isRunning, &error);
  			CHECK_STATUS(fStatus, FLP_LIBERR, cleanup);
  			uint8 i, change_file;
  			uint8 ack1_enc[4],ack2_enc[4];
  			uint8 coordinates[4],ack1_in[4],ack1[4]={255,255,0,0},ack2[4]={0,0,255,255};
  			uint8 first_half[4],second_half[4],fpga_data_rec[4],fpga_data[4],board_coordinates[4];
  			enc_ack(ack1[0],ack1[1],ack1[2],ack1[3],ack1_enc);
  			enc_ack(ack2[0],ack2[1],ack2[2],ack2[3],ack2_enc);
  			if (isRunning) {
  			__label__ H1, H2;
  			int msec, timeout, timeout1 = 25000;
  			clock_t begin;
        bool read_coordinates=0;

  		H1:
        change_file = 0;
  			f();
  		H2:
        if(read_coordinates == 0)
          i=0;
  			while(true){
  				printf("\nchannel-%u\n",2*i);
          if(read_coordinates == 0){
  				  flReadChannel(handle, 2*i, 4, &coordinates, error);
            dec_ack(coordinates[0], coordinates[0], coordinates[0], coordinates[0], board_coordinates);
  				  printf("\ndone reading encrypted coordinates\n");
  				}
          else
            read_coordinates=0;

  				flWriteChannel(handle, 2*i+1, 4, &coordinates, error);
  				printf("\ndone writing encrypted coordinates\n");

  				flReadChannel(handle, 2*i,4, &ack1_in, error);
  				printf("\nreading first ack1\n");

  				if ( ack1_in[0]==ack1_enc[0] && ack1_in[1]==ack1_enc[1] && ack1_in[2]==ack1_enc[2] && ack1_in[3]==ack1_enc[3] ) {
  					printf("\nreceived correct first ack1\n");
  					break;
  				}
  				else{
  					sleep(5);
  					flReadChannel(handle,2*i,4, &ack1_in, error);
  					printf("\nreading ack1 after 5 secs\n");
  					if(ack1_in[0]==ack1_enc[0] && ack1_in[1]==ack1_enc[1] && ack1_in[2]==ack1_enc[2] && ack1_in[3]==ack1_enc[3]){
  						printf("\nreceived correct ack1 after 5 sec\n");
  						break;
  					}
  					else 
  						i=(i+1)%64;
  				}
  			}

        printf("\ncoordinates of the controller are %u , %u \n",board_coordinates[0]/16, board_coordinates[0]%16);

  			flWriteChannel(handle, 2*i+1, 4, &ack2_enc, error);
  			printf("\n wrote ack2\n");
  			
  			finalencrypt(coordinates[0], coordinates[0], coordinates[0], coordinates[0], first_half, second_half);
  			printf("\nfirst half of the sent data is %u , %u , %u , %u \n", first_half[0], first_half[1], first_half[2], first_half[3]);
  			printf("\nsecond half of the sent data is %u , %u , %u , %u \n", second_half[0], second_half[1], second_half[2], second_half[3]);

  			flWriteChannel(handle, 2*i+1, 4, &first_half, error);
  			printf("\n wrote first_half\n");

  			msec = 0, timeout = 256000;
  			begin = clock();
  			do{
  				flReadChannel(handle,2*i,4, &ack1_in, error);
  				if(ack1_in[0]==ack1_enc[0] && ack1_in[1]==ack1_enc[1] && ack1_in[2]==ack1_enc[2] && ack1_in[3]==ack1_enc[3]){
  					printf("\n reading first_half ack1\n");
  					break;
  				}
  				clock_t diff = clock() - begin;
  				msec = diff*1000/CLOCKS_PER_SEC;
  			}while(msec < timeout);

  			if(msec >= timeout)
  				goto H2;

  			flWriteChannel(handle,2*i+1,4, &second_half, error);
  			printf("\n wrote second_half\n");

  			msec = 0;
  			begin = clock();
  			do{
  				flReadChannel(handle,2*i,4, &ack1_in, error);
  		
  				if(ack1_in[0]==ack1_enc[0] && ack1_in[1]==ack1_enc[1] && ack1_in[2]==ack1_enc[2] && ack1_in[3]==ack1_enc[3]){
  					printf("\n reading second_half ack1\n");
  					break;
  				}
  				clock_t diff = clock() - begin;
  				msec = diff*1000/CLOCKS_PER_SEC;
  			}while(msec < timeout);

  			if(msec >= timeout)
  				goto H2;
  		
  			flWriteChannel(handle, 2*i+1, 4, &ack2_enc, error);
  			printf("\n wrote finalack2\n");
  			
  			sleep(24);

        msec = 0;
        begin = clock();
        do{
          flReadChannel(handle,2*i,4, &fpga_data_rec, error);
          dec_ack(fpga_data_rec[0], fpga_data_rec[1], fpga_data_rec[2], fpga_data_rec[3], fpga_data);
          if(fpga_data[1]==0 && fpga_data[2]==255 && fpga_data[3]==0){
            board_coordinates[0] = fpga_data[0];
            board_coordinates[1] = fpga_data[1];
            board_coordinates[2] = fpga_data[2];
            board_coordinates[3] = fpga_data[3];
            read_coordinates = 1;
            goto H2;            
          }
          else{
            change_file = 1;
            printf("\ndecrypted fpga data is %u , %u , %u , %u \n", fpga_data[0], fpga_data[1], fpga_data[2], fpga_data[3]);
            break;
          }
          clock_t diff = clock() - begin;
          msec = diff*1000/CLOCKS_PER_SEC;
        }while(msec < timeout1);
        
        if(change_file == 1){
          uint8 x1 = board_coordinates[0]/16;
          uint8 y1 = board_coordinates[0]%16;
          uint8 dir1 = fpga_data[0]/32;
          uint8 to1 = (fpga_data[0]%16)/8;
          uint8 ns1 = fpga_data[0]%8;
          TO[x1][y1][dir1] = to1;
          NS[x1][y1][dir1] = ns1;
          FILE* stream2 = fopen("/home/sathvik/Documents/logic/20140524/makestuff/apps/flcli/track_data.csv", "w");

          for(uint8 xi=0; xi<16; xi++){
            for(uint8 yi=0; yi<16; yi++){
              for(uint8 diri=0; diri<8; diri++){
                if(TE[xi][yi][diri]==1)
                  fprintf(stream2, "%u, %u, %u, %u, %u \n", xi, yi, diri, TO[xi][yi][diri], NS[xi][yi][diri]);
              }
            }
          }

          fclose(stream2);
        }
        sleep(30);

  			goto H1;
			}
			else 
			{
				fprintf(stderr, "The FPGALink device at %s is not ready to talk - did you forget --xsvf?\n", vp);
				FAIL(FLP_ARGS, cleanup);
			}
		} 
		else 
		{
			fprintf(stderr, "Shell requested but device at %s does not support CommFPGA\n", vp);
			FAIL(FLP_ARGS, cleanup);
		}
	}

cleanup:
	free((void*)line);
	flClose(handle);
	if ( error ) {
		fprintf(stderr, "%s\n", error);
		flFreeError(error);
	}
	return retVal;
}
