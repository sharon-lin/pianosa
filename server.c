#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#include "networking.h"
#include "print_ascii.h"


//#define MAX_CLIENTS 4

void process( char * s , int sd);
void sub_server( int sd );

//int directory_num = 0;

//int total_clients = 0;

//sem_t mutex;
//int counter;

int main(){

  int sd, connection;

  sd = server_setup();


  while (1) {

    connection = server_connect( sd );

    int f = fork();
    if ( f == 0 ) {
      close(sd);
      sub_server( connection );
      exit(0);
    }
    else{
      close( connection );
    }
  }
  return 0;
}


void sub_server( int sd ) {

  char buffer[MESSAGE_BUFFER_SIZE];
  while (read( sd, buffer, sizeof(buffer) )) {

    printf("[SERVER %d] received: %s\n", getpid(), buffer );
    process( buffer ,sd);
    write( sd, buffer, sizeof(buffer));
  }

}

void get_message (char * filename, char * args){

  int c;
  FILE *file;
  char message[100];
  int i =0;

  char str[100];
  strcpy(str, filename);
  strcat(str, ".txt");


  file = fopen(str, "r");

  if (file) {
    while ((c = getc(file)) != EOF){
      message[i] = c;
      i++;
    }
    message[i] = 0;

    fclose(file);
    
    strcpy(args, message);
  }
}

void delete_file(char *filename, char *args){
  int ret;
  char str[100];
  strcpy(str, filename);
  strcat(str, ".txt");

  ret = remove(str);

  if(ret == 0)
    strcpy(args,"File deleted successfully");
  else
    strcpy(args,"Error: unable to delete the file");
}

void list_files(char * args){
  char *buffer;
  struct dirent *entry;
  DIR *dir = opendir(".");
  if (dir == NULL) {
    return;
  }
  strcpy(args, "\n");
  while ((entry = readdir(dir)) != NULL) {

    if(strlen(entry->d_name) > 4 && memcmp(entry->d_name + strlen(entry->d_name) - 4, ".txt", 4) == 0){
      strcat(args,entry->d_name);
      strcat(args,"\n ");
    }

  }
  if (strcmp(args,"\n")==0){
    strcpy(args,"There are no files yet.");
  }

  //strcat(buffer,0);

  closedir(dir);

 }

void postmessage (char * sequence, char * filename, char * args){
   int i, bytesread;
  char buffer[255];

  strcpy(buffer, sequence);
  FILE *outputfile;

  //  bzero(buffer,255);

  char str[30];
  strcpy(str, filename);
  strcat(str, ".txt");

  if ((outputfile = fopen(str, "w")) == NULL)
    strcpy(args,"Error creating file.");

  for (i = 0; i < strlen(sequence); i++){
      if (putc(sequence[i], outputfile) == EOF){
	strcpy(args,"Write error\n");
	break; // stop the for loop
      }
  }
  
  putc(0, outputfile);

  //directory_num++;

  fclose(outputfile);

}

void process( char * args , int sd) {
  char * cmd;
  char * filename;

  cmd = strtok(args, " ");
  lower_string(cmd);

  if (strcmp(cmd,"play") == 0){
    cmd = strtok(NULL," ");
    //play_sound(cmd);
  }

  else if (strcmp(cmd,"save")==0){
    filename = strtok(NULL," ");
    cmd = strtok(NULL," ");
    postmessage(cmd, filename, args);
  }

  else if (strcmp(cmd,"get")==0){
    filename = strtok(NULL," ");
    get_message(filename, args);
  }

  else if (strcmp(cmd,"list")==0){
    list_files(args);
  }

  else if(strcmp(cmd,"delete")==0){
    filename = strtok(NULL, " ");
    delete_file(filename, args);
  }

}
