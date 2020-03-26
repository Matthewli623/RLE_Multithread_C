
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/io.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h> 
#include <sys/sysinfo.h>

typedef struct RLE  
{
	int count;
	char character;
} RLE;

typedef struct Text
{
	char *data;
  RLE list[10000];
  int used; //no of used block of the list
  int start; 
  int end;
} Text;

long fsize(char* file) //get file size
{
    FILE * f = fopen(file, "r");
    fseek(f, 0, SEEK_END);
    long len = (unsigned long)ftell(f);
    fclose(f);
    //printf(" size = %ld\n",len);
    return len;
}

void getstartendpt(Text *text,int pstart,int pend) //get starting and ending point
{
  text->start=pstart; //set starting point
  char character=text->data[pend]; //get the character at pend
  int i;
  for(i=pend;text->data[i+1]!='\0';i++) //find the end point of same char
  {
    if(character!=text->data[i+1])
    {
      break; //get ending point
    }
  }
  text->end=i; //set ending point
}


void *compressFile(void *arg) 
{
  
  Text *input = (Text *)arg;//pointer of text
  if(input->start==-1&&input->end==-1)return 0 ;//no content need to process
  int count;
  char character = '\0';
  int index=0;

  for(int x = input->start; (x<=input->end);x++)
  {
    count=1;
    character = input->data[x];
    while(input->data[x+1]==character)
    {
        x++;
        count++;
    }
    input->list[index].count=count;
    input->list[index].character=character;
    index++;
  }
  
  input->used=index;

  return 0;
}

void printresult(Text **input,int size)
{
  // input is array of results
  for(int i =0;i<size;i++)
  {
    int j=0;
    while(j<input[i]->used)//print all RLE in current result
    {
      fwrite(&(input[i]->list[j].count), 4, 1, stdout);
      printf("%c",input[i]->list[j].character);
      j++;
    }
  }

}
char* stringfromFile(char *in)//read the file to memory
{
  char *f;
  int size;
  struct stat s;
  const char *  file_name = in;
  int fd = open (file_name, O_RDONLY);
  int status = fstat (fd, & s);
  if(status == -1){
    printf("couldnt find file: %s \n",file_name);
    exit(1);
  }
  size = s.st_size;
  f =  mmap (0, size, PROT_READ, MAP_PRIVATE, fd, 0);
  return f;
}


int main(int argc, char *argv[])//argc-1=no of input
{

  if (argv[1] == NULL) { //No arg 
			char *out = "pzip: file1 [file2 ...]";
			printf("%s\n", out);
			exit(1);
	}


  if(argc>2) //merge all file to temp.in
  {
    FILE *sum = fopen("temp.in", "w");
    char c; 
    for(int i =1;i<argc;i++)
    {
      FILE *fp = fopen(argv[i], "r");
      if(fp==NULL)exit(0); 
      while ((c = fgetc(fp)) != EOF) //copy char to temp.in
      {
        fputc(c, sum);
      }
      fclose(fp);   
    }
    argv[1]="temp.in"; //chage target name 
    fclose(sum);   
  }

  int threadno=get_nprocs(); //get no of thread in PC
  int size=fsize(argv[1]); //get size of the file
  int splitsize=size/threadno; //get split size by no of thread
  char* buf = stringfromFile(argv[1]); //read the file to memory
  
  if(size<100000) //Use single thread if size is small
  {
    threadno=1;
    splitsize=size;
  }

	pthread_t p[threadno]; //create thread accroding to no of thread
  Text *text[threadno]; //create storage and information for each thread

  int pstart =0; //starting point
  int pend=splitsize-1; //end point

  for(int i =0;i<threadno;i++)  //collect starting and end point for each thread
  {
    text[i]= malloc(sizeof(Text)); //create object
    text[i]->data = buf;  //data point the buffer

    if(pstart>=(size-1))//reach the end of text 
    {
      text[i]->start=-1; //finished index
      text[i]->end=-1;  //finished index
    }
    else
    {
      getstartendpt(text[i],pstart,pend);//get the index of starting and the End
      //if(i==threadno-1)text[i]->end=size-1;
      pstart=text[i]->end+1;  //next starting point
      pend=pstart+splitsize;   //next temp ending point
      if(pend>=(size-1))pend=size-2; // prevent out of bound
    }
  }

  for(int i =0;i<threadno;i++)
  {
    pthread_create(&p[i], NULL, compressFile, text[i]); //Use each thread with compressFile function
  }

	for(int i =0;i<threadno;i++) //wait each thread to finish
  {
    pthread_join(p[i], NULL);
  }

  printresult(text,threadno); //print the final result

  return 0;
}


