#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> 
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include <stdio.h>

#include <dirent.h>

#define HOST_NAME_MAX 1000
#define PORT 80
#define BACKLOG 10
#define BUFFER_SIZE 1024
#define PARAMETER_SIZE 100
#define MAXLINE 30

int main(int argc, char *argv[]){
	
	//socket address
	struct sockaddr_in serveradd;
	struct sockaddr_in newadd;
    int res;
    int readSize;
	int yes = 1;
    int serversock, clientsock;
 
    char client_buf[BUFFER_SIZE];
    char server_reply[BUFFER_SIZE];
    
    
    // Create TCP socket
    serversock = socket(AF_INET, SOCK_STREAM, 0);
    if (serversock == -1) {
        printf("Creating socket failed\n");
        exit(1);
    }
    printf("Socket successfully created\n");
    
	if (setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, & yes, sizeof(int)) == -1){
    perror("Server-setsockopt()");
    exit(1);
	}
	
    // Prepare the sockaddr_in structure
    serveradd.sin_family = AF_INET;
    serveradd.sin_addr.s_addr = INADDR_ANY;
    serveradd.sin_port = htons(PORT);
    
	

	printf("Server Settings: Port %d \n",PORT);
	
    // Bind addr to socket
    if(bind(serversock, (struct sockaddr *) &serveradd, sizeof(serveradd)) < 0){
        perror("Bind failed\n");
		
        exit(EXIT_FAILURE);
    }
    printf("Bind was successfully completed\n");
    
    // Listen
    if (listen(serversock, BACKLOG) < 0)
    {
        perror("listening failed");
        exit(EXIT_FAILURE);
    }
    printf("Waiting for incoming connections...\n");
	//keeps track of active clients
	fd_set active_fd_set, read_fd_set;
	FD_ZERO(&read_fd_set);
	FD_ZERO(&active_fd_set);
	
	FD_SET(serversock, &active_fd_set);
	
	int fdmax = serversock;
	int newfd;
	int addrlen;
  
  printf("Server-setsockopt() is OK...\n");
	 size_t size;
	while(1){
		read_fd_set = active_fd_set;
		  //clear the socket set
		memset(client_buf, '\0', sizeof(client_buf));
		memset(server_reply, '\0', sizeof(client_buf));
		
	    if (select (fdmax+1, &read_fd_set, NULL, NULL, NULL) < 0){
          perror ("select failure");
          exit (EXIT_FAILURE);
        }
		
		for (int i = 0; i <= fdmax; i++){
	  if (FD_ISSET(i, & read_fd_set)){
		/* we got one... */
		if (i == serversock){
		  /* handle new connections */
		  addrlen = sizeof(serveradd);
		  if ((newfd = accept(serversock, (struct sockaddr * ) & serveradd, & addrlen)) == -1){
			perror("Server-accept() error lol!");
		  }else{
			printf("Server-accept() is OK...\n");
			FD_SET(newfd, & active_fd_set); /* add to master set */
			if (newfd > fdmax){
			  /* keep track of the maximum */
			  fdmax = newfd;
			}
			printf("%s: New connection from %s on socket %d\n", argv[0], inet_ntoa(serveradd.sin_addr), newfd);
		  }
		}else{
			int test,reee;
			
			if(recv(i, client_buf, sizeof(client_buf), 0) > 0){
				//determine actions based on client input
				int init_size = strlen(client_buf);
				char delim[] = " ";
				char *ptr = strtok(client_buf, delim);
				int arraysize =0;
				char array[PARAMETER_SIZE][PARAMETER_SIZE];
				while(ptr != NULL){				
					printf("'%s'\n", ptr);
					strcpy(array[arraysize], ptr);
					arraysize +=1;
					ptr = strtok(NULL, delim);
				}
				
				
				if(strcmp(array[0],"get") == 0){
					//check that the directory exists
					DIR *dir_ptr;
					struct dirent *dirent;
					printf("Checking dir: %s \n",array[1]);
					if ((dir_ptr=opendir(array[1]))==NULL) {
						//if the directory doesn't exists
						fprintf(stderr,"ls:can't open %s\n",array[1]);
						strcpy(server_reply,"errordir");
						if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
							perror("Sending Failed");
							exit(EXIT_FAILURE);
						}						
					}else{
						printf("Checking file in dir: %s \n",array[2]);
						//if directory does exist						
						FILE *fptr1;
						char together[PARAMETER_SIZE];
						memset(together,'\0',sizeof(together));
						strcat(together,array[1]);
						strcat(together,"/");
						strcat(together,array[2]);
						printf("Checking directory: %s \n",together);
						fptr1 = fopen(together,"r"); 
						if(fptr1 == NULL){
							printf("errorfile reee \n");
							strcpy(server_reply,"errorfile");
							if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Sending Failed");
								exit(EXIT_FAILURE);
							}
						}else{
							printf("sucess file found and running \n");
							strcpy(server_reply,"success");
							if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Sending Failed");
								exit(EXIT_FAILURE);
							}
							
							char buffer[BUFFER_SIZE];
							int test = 0;
							int status = 99;
							while(fgets(buffer, sizeof(buffer), fptr1)) {
								//buffer[strlen(buffer)-1] = NULL;
								strcpy(server_reply,buffer);
								if((status = send(i, server_reply, BUFFER_SIZE, 0)) < 0){
									perror("Sending Failed");
									exit(EXIT_FAILURE);
								}
								memset(server_reply, '\0', sizeof(client_buf));
								printf("Line %d , status %d: %s --- \n",test,status,buffer);
								test += 1;
							}
							strcpy(server_reply,"finished");
							if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Sending Failed");
								exit(EXIT_FAILURE);
							}
						}
						
					}
					
					
				}
				
				if(strcmp(array[0],"put") == 0){
					//check for -f
					bool flag = false;
					for(int i = 2; i < arraysize; i++){
						if(strcmp(array[i],"-f") == 0 ){
							flag = true;
						}
					}
					
					//we have to check the directory
					DIR *dir_ptr;
					struct dirent *dirent;
					printf("Checking dir: %s \n",array[1]);
					printf("flag = true = 1: %d \n",flag);
					if ((dir_ptr=opendir(array[1]))==NULL || flag) {
						//create directory if it doesn't exist
						if((dir_ptr=opendir(array[1]))==NULL){
							
							mkdir(array[1],0777);
							
						}
						//directory doesn't exist or flag is true for overwrite
						strcpy(server_reply,"imrdy");
						printf("Recieved:  \n");
						printf("about to send \n");
						
						if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
							perror("Sending Failed");
							exit(EXIT_FAILURE);
						}
						//we start recieving loop
						FILE *fptr1;
						char thefile[PARAMETER_SIZE];
						char thefilelocation[PARAMETER_SIZE];
						printf("start loop \n");
						bool flag = true;
						while(flag){
							//we need to do it file by file.
							
							memset(client_buf, '\0', BUFFER_SIZE);
							if(recv(i, client_buf, sizeof(client_buf), 0) < 0){
								perror("Recieving Error");
								exit(EXIT_FAILURE);
							}
							
							//method is that we will check the filename constantly with the client packets
							
							memset(thefile, '\0', PARAMETER_SIZE);
							memset(thefilelocation, '\0', PARAMETER_SIZE);
							
							for(int i = 2; i < arraysize; i++){
								printf("array -%s : clientbuf- %s \n",array[i],client_buf);
								if(strcmp(array[i],client_buf) == 0){
									
									strcpy(thefile,array[i]);
								}
							}
							printf("finished thefile: %s \n",thefile);
							printf("theval:%s \n",client_buf);
							//if a filename is detected then create a file
							if(strcmp(thefile,"") != 0){
								printf("creating file \n");
								strcat(thefilelocation,array[1]);
								strcat(thefilelocation,"/");
								strcat(thefilelocation,thefile);
								printf("filelocation: %s \n",thefilelocation);
								if(fptr1 != NULL){
									fclose(fptr1);
								}
								fptr1 = fopen(thefilelocation,"w");
							}else if(strcmp(client_buf,"finished") == 0){
								printf("finished program \n");
								fclose(fptr1);
								flag = false;
							}else{
								printf("writing to file \n");
								//add to current selected file
								fprintf(fptr1,"%s",client_buf);
							}
							
							
							
						}
						printf("out of loop \n");
						
					}else{
						//directory does exist and flag isn't true
						strcpy(server_reply,"errordir");
						if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
							perror("Sending Failed");
							exit(EXIT_FAILURE);
						}
						
					}
					
					
					
				}
				
				if(strcmp(array[0],"run") == 0){
					
					//we have to check where should we output so we check the arguments to see if there is -f localfile
					char outfilename[BUFFER_SIZE];
					char thearguments[BUFFER_SIZE];
					memset(thearguments, '\0', BUFFER_SIZE);
					//we store the -f localfile part and the command line arguments if there are any
					for(int i=2; i < arraysize;i++){
						printf("The comparison: %s \n",array[i]);
						if(strcmp(array[i],"-f") == 0){
							printf("confirmed -f \n");
							strcpy(outfilename,array[i+1]);
							i++;
						}else{
							//all of this areas are command line arguments							
							strcat(thearguments, array[i]);
							strcat(thearguments, " ");
							
						}
					}
					printf("gets past here \n");
					//first we have to make sure there isnt a executable file here first
					//this is if we need to compile.
					bool isthereexe = false;
					char thecfiles[BUFFER_SIZE];
					char thesearchdirectory[BUFFER_SIZE];
					char indexdirectory[BUFFER_SIZE];
					char tempstring[BUFFER_SIZE];
					char tempstring2[BUFFER_SIZE];
					//strcat(thesearchdirectory,"./");
					strcat(thesearchdirectory,array[1]);
					int length = 0;
					int longestmodified = 0;
					DIR *directory;
					struct dirent *dir;
					printf("direca success \n");
					//search directory
					directory = opendir(thesearchdirectory);
					printf("direc success %s\n",thesearchdirectory);
					if (directory){
						while ((dir = readdir(directory)) != NULL) {
								
							
							memset(tempstring, '\0', sizeof(tempstring));
							
							if(strlen(dir->d_name) != NULL){
								length = strlen(dir->d_name);
							}
							
							//make substring of when reading files
							//substring that reads .c and .exe files
							char *wordpointer = dir->d_name;						
							memcpy( tempstring, &wordpointer[length-4], 4 );
							tempstring[4] = '\0';
							
							memcpy( tempstring2, &wordpointer[length-2], 2 );
							tempstring2[2] = '\0';
							

							
							
							
							//verfies it an executable file
							if(strcmp(tempstring,".exe") == 0){
								printf("found .exe file\n");
								isthereexe = true;
							}
							//is a source file... here we need to keep track of the lastest modified date file for later analysis
							if(strcmp(tempstring2,".c") == 0 ){
								
								
								printf("found .c file \n");
								struct stat buffer;
								strcpy(indexdirectory,thesearchdirectory);
								strcat(indexdirectory,"/");
								strcat(indexdirectory,dir->d_name);
								
								strcat(thecfiles,indexdirectory);
								strcat(thecfiles," ");
								
								printf("dire: %s\n",indexdirectory);
								stat(indexdirectory, &buffer);
								if(longestmodified < buffer.st_mtime){
									longestmodified = buffer.st_mtime;
								}
								printf("the file %d\n",buffer.st_mtime);
								
								
								
								
								//memset(thetimebuff, '\0', sizeof(client_buf));
							}
						}
					}
					//we need to check that the folder creation date is older than the last modified date.
							
					//the folder modified date
					struct stat abuffer;
					char theactualoutput[BUFFER_SIZE];
					char terminal[BUFFER_SIZE];
					char templocation[BUFFER_SIZE];
					memset(templocation, '\0', sizeof(templocation));
					
					
					strcpy(templocation,"./");
					strcat(templocation,array[1]);
					FILE *output;
					stat(templocation, &abuffer);
					printf("thelocation: %s",templocation);
				
					int foldermodified;					
					
				
					printf("the value: %d \n",abuffer.st_mtime);
					printf("the value file: %d \n",longestmodified);
					//printf("the file %d\n",buffer.st_mtime);
					//printf("2.5??\n");
					//the folder must be younger than the last modfied c file or there isn't an .exe
					if(foldermodified < longestmodified || !isthereexe){
						//we now compile
						
						memset(terminal, '\0', sizeof(terminal));
						strcat(terminal,"cc ");
						strcat(terminal,thecfiles);						
						strcat(terminal,"-o ");
						strcat(terminal,array[1]);
						strcat(terminal,"/output.exe");
						printf("the terminal: %s\n",terminal);						
						popen(terminal,"r");
						printf("Successfully Complied program[s]\n");
					}
					
					//if there isnt any c files or no exe files than return error
					if(!isthereexe && strcmp(thecfiles,"")== 0){
						strcpy(server_reply,"error");
						if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
							perror("Sending Failed");
							exit(EXIT_FAILURE);
						}
					}else{
						//lets run the exe file with args
						memset(terminal, '\0', sizeof(terminal));
						//memset(output, '\0', sizeof(output));
						strcat(terminal,array[1]);
						strcat(terminal,"/output.exe ");
						strcat(terminal," ");
						strcat(terminal,thearguments);
						printf("the terminal: %s \n",terminal);
						output = popen(terminal,"r");
						printf("succesfully run executable \n");
						printf("about to go through stdout of terminal \n");
						//test whether to print to file or to console
						if(strcmp(outfilename,"") == 0){
							strcpy(server_reply,"successconsole");
							if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Sending Failed");
								exit(EXIT_FAILURE);
							}
						}else{
							strcpy(server_reply,"successfile");
							if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Sending Failed");
								exit(EXIT_FAILURE);
							}
							//send filename
							strcpy(server_reply,outfilename);
							if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Sending Failed");
								exit(EXIT_FAILURE);
							}
							
						}
						//where we send the output of the .exe
						if(output){
							while(fgets(theactualoutput, sizeof(theactualoutput), output)) {
								printf("start: %s :end\n", theactualoutput);
								strcpy(server_reply,theactualoutput);
								if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
									perror("Sending Failed");
									exit(EXIT_FAILURE);
								}
							}
							printf("thelaststart: %s :end\n", theactualoutput);
							printf("zzzzzzzzzz\n");
							strcpy(server_reply,"finished");
							if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Sending Failed");
								exit(EXIT_FAILURE);
							}
							printf("adeawdawdw\n");
						}
						
						
						memset(terminal, '\0', sizeof(terminal));
						//printf("the output %s \n",output);
						
					}

					
					//no output file name means no file 
					if(strcmp(outfilename,"") == 0){
						
					}else{
						
					}
					
				}
				
				if(strcmp(array[0],"list") == 0){
					//check if there is a directory to list within
					char thesearchdirectory[BUFFER_SIZE];
					char indexdirectory[BUFFER_SIZE];
					thesearchdirectory[0] = '.';
					bool longlist = false;
					//iterate of request to find optional filename and -f arg
					//store results
					for(int i = 1; i < arraysize; i++){
						printf("reee:%s \n",array[i]);
						if(strcmp(array[i],"-l") == 0){
							longlist = true;
							printf("made true \n");
						}else{
							strcpy(thesearchdirectory,array[i]);
						}
					}
					//the listing directory part
					DIR *d;
					struct dirent *dir;
					d = opendir(thesearchdirectory);
					if (d){
						while ((dir = readdir(d)) != NULL) {
							memset(server_reply, '\0', sizeof(client_buf));
							strcpy(server_reply,dir->d_name);
							//if the user requests -l on command
							if(longlist == true && strcmp(server_reply,".") != 0 && strcmp(server_reply,"..") != 0){
								
								char temp[BUFFER_SIZE];
								char tempa[BUFFER_SIZE];
								
								struct stat buffer;
								strcpy(indexdirectory,thesearchdirectory);
								strcat(indexdirectory,"/");
								strcat(indexdirectory,dir->d_name);
								//list
								stat(indexdirectory, &buffer);
								
								//date created part
								char *thetimebuff = ctime(&buffer.st_birthtime);
								thetimebuff[strlen(thetimebuff)-1] = '\0';
								
								//the file size part
								sprintf(temp, "%d", buffer.st_size);
								memset(tempa,'\0',sizeof(tempa));
								//permission part of -l
								strcat(tempa,(buffer.st_mode & S_IRUSR) ? "r" : "-");
								strcat(tempa,(buffer.st_mode & S_IWUSR) ? "w" : "-");
								strcat(tempa,(buffer.st_mode & S_IXUSR) ? "x" : "-");
								strcat(tempa,(buffer.st_mode & S_IRUSR) ? "r" : "-");
								strcat(tempa,(buffer.st_mode & S_IWGRP) ? "w" : "-");
								strcat(tempa,(buffer.st_mode & S_IXGRP) ? "x" : "-");
								strcat(tempa,(buffer.st_mode & S_IROTH) ? "r" : "-");
								strcat(tempa,(buffer.st_mode & S_IWOTH) ? "w" : "-");
								strcat(tempa,(buffer.st_mode & S_IXOTH) ? "x" : "-");
								
								//sprintf(tempb, "%d", );
								
								//printf("the permission %d",buffer.st_mode);
								
								strcat(server_reply, " [Size:");
								strcat(server_reply, temp);
								strcat(server_reply, "bytes] [Date Created:");
								strcat(server_reply, thetimebuff);
								strcat(server_reply, "] [Permissions:");
								strcat(server_reply, tempa);
								strcat(server_reply, "]");
								
								memset(thetimebuff, '\0', sizeof(client_buf));
								memset(temp, '\0', sizeof(temp));
								memset(tempa, '\0', sizeof(tempa));
							}
							//send current line to client
							printf("sending:%s\n", server_reply);
							if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Sending Failed");
								exit(EXIT_FAILURE);
							}
							
							
						}
						//close dir
						closedir(d);
					}else{
						//send error if dir doesn't exist
						memset(server_reply, '\0', sizeof(client_buf));
						strcpy(server_reply,"Error Directory Not found");
						if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Sending Failed");
								exit(EXIT_FAILURE);
						}
					}
					//finishing up
					strcpy(server_reply,"finished");
					if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
						perror("Sending Failed");
						exit(EXIT_FAILURE);
					}
					
				}
				
				if(strcmp(array[0],"sys") == 0){
					char hostname[HOST_NAME_MAX];
					//get hostname
					gethostname(hostname, HOST_NAME_MAX);
					strcat(server_reply, "Host name: ");
					strcat(server_reply, hostname);
					strcat(server_reply, ", OS: ");
					//get os name
					#ifdef _WIN32
						strcat(server_reply,"Windows 32-bit");
						#elif _WIN64
						strcat(server_reply,"Windows 64-bit");
						#elif __APPLE__ || __MACH__
						strcat(server_reply,"Mac OSX");
						#elif __linux__
						strcat(server_reply,"Linux");
						#elif __unix || __unix__
						strcat(server_reply,"Unix");
						#else
						strcat(server_reply,"Other");
						#endif
					char tempstring[BUFFER_SIZE];
					memset(tempstring, '\0',sizeof(tempstring));
					//get amount of cores
					sprintf(tempstring,"%d",get_nprocs_conf());
					
					strcat(server_reply,", CPU cores: ");
					strcat(server_reply,tempstring);
					//send info to client
					if(send(i, server_reply, BUFFER_SIZE, 0) < 0){
						perror("Sending Failed");
						exit(EXIT_FAILURE);
					}
					
				}
				
				
				
			}else{
				printf("Client Disconnected \n");
				close(i);
				//remove client from active list
				FD_CLR(i, & active_fd_set);
			}
		}
	   }
	 }
		
	}
	
}