#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <dirent.h> 

#define PORT 80
#define BUFFER_SIZE 1024
#define PARAMETER_SIZE 100

int main(int argc, char * argv[]){
	
	//server address
	struct sockaddr_in server;  // server address
	//client socket
	int clientsock;
	
	//client buffers
	char client_buffer[BUFFER_SIZE];
	char client_buffer_temp[BUFFER_SIZE];
	char server_reply[BUFFER_SIZE];	
	
	//making sure there is a command line argurment for IP
	if(argc < 2){
		printf("Argument required: IP is missing");
		exit(EXIT_FAILURE);
	}
	
	
	
	//defining connection settings
	server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
	
	printf("Connection Settings: IP %s, Port %d \n",argv[1],PORT);
	
	// create socket and exits if error
    if((clientsock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Could not create socket");
		exit(EXIT_FAILURE);
	}
	printf("Socket Success \n");
	
	
	//connecting to server and exits if error	
	if(connect(clientsock, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("Connection Failed");
		exit(EXIT_FAILURE);
	}
	printf("Connection Established \n");
	 
	 //set the socket to non blocking
     fcntl(clientsock, F_SETFL, SOCK_NONBLOCK); 

	//infinite feedback loop to represent shell
	
	// printf("> ");
	while(1){
		
		
		
       
		//reading in stdin
		
		
		//sets for selection
		
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
		
		//tells user it ready for input command
		printf("INPUT COMMAND BELOW \n");
        int selection = select(1, &readfds, NULL, NULL, NULL);
		//reset buffer both from server and client so it doesn't mess up next stdout and server response
		memset(client_buffer, '\0', sizeof(client_buffer));
        memset(server_reply, '\0', sizeof(server_reply));
		
		
		
		if(selection == 0){
			printf("reee \n");
			if(recv(clientsock, server_reply, BUFFER_SIZE-1, 0) < 0){
				perror("Recieving  Failed");
				exit(EXIT_FAILURE);
			}
			if(strcmp(client_buffer,"") != 0){
				 printf(" Response: %s \n", server_reply);
				 
			}
		}else{
			
			//scanf("%s",client_buffer);
			
			gets(client_buffer);
			
			//start timer for response
			struct timeval  tv1, tv2;
			gettimeofday(&tv1, NULL);
			
			//we split up the client input here into array. easier for reading
			strcpy(client_buffer_temp,client_buffer);
			int init_size = strlen(client_buffer_temp);
			char delim[] = " ";
			char *ptr = strtok(client_buffer_temp, delim);
			int arraysize = 0; 
			char array[PARAMETER_SIZE][PARAMETER_SIZE];
			while(ptr != NULL){				
				//printf("'%s'\n", ptr);
				strcpy(array[arraysize], ptr);
				arraysize +=1;
				ptr = strtok(NULL, delim);
			}
			
			

			//this is where we compare requests
			if(strcmp(array[0],"quit") == 0){
				//close the connection and the .exe
				printf("Sucessfully quit connection \n");
				close(clientsock);
				exit(EXIT_SUCCESS);
			}
			if(strcmp(array[0],"list") == 0){
				//send request to server
				if(send(clientsock, client_buffer, BUFFER_SIZE, 0) < 0){
					perror("Sending Failed");
					exit(EXIT_FAILURE);
				}
				//the response is going to be a loop until 'finished' is sent
				while(1){
					memset(server_reply, '\0', sizeof(server_reply));
					if(recv(clientsock, server_reply, BUFFER_SIZE, 0) < 0){
						perror("Recieving  Failed");
						exit(EXIT_FAILURE);
					}
					//if we recieve finished than there is nothing more to list and we break from loop
					if(strcmp(server_reply,"finished") == 0){
						break;
					}
					//print the list
					printf("%s\n",server_reply);
				}
				//print delay
				gettimeofday(&tv2, NULL);
				 printf ("Response time = %f seconds\n",(double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
			}
			if(strcmp(array[0],"put") == 0){
					//check source files exist
					printf("Running Put command");
					bool found = true;
					for(int i = 2; i < arraysize;i++){
						if(strcmp(array[i],"-f") != 0){
							FILE *fptr1;
							fptr1 = fopen(array[i],"r");
							if(fptr1 == NULL){
								found = false;
								printf("Error: %s not found \n",array[i]);
							}
						}
					}
					//if the file exists than we go to the server
					if(found){
						printf("sending to server %s \n",client_buffer);
						//now that we have verfied that the sources file exist we want to verfiy the directory exists
						if(send(clientsock, client_buffer, BUFFER_SIZE, 0) < 0){
							perror("Sending Failed");
							exit(EXIT_FAILURE);
						}
						//reiceve response on whether it failed
						if(recv(clientsock, server_reply, BUFFER_SIZE, 0) < 0){
							perror("Recieving  Failed");
							exit(EXIT_FAILURE);
						}
						if(strcmp(server_reply,"errordir") == 0){
							printf("Error: Directory exists -f needed to overwrite\n");
						}else{
							//The server is ready to recieve and place the files
							
							//we first send the filename of the sourcefile we are going to send
							char current[PARAMETER_SIZE];
							for(int i = 2; i < arraysize; i++){
								//make sure that -f isn't a filename
								if(strcmp(array[i],"-f") != 0){								
									memset(current, '\0', sizeof(current));
									strcpy(current,array[i]);
									//send file name
									if(send(clientsock, current, BUFFER_SIZE, 0) < 0){
										perror("sending Failed");
										exit(EXIT_FAILURE);
									}
									//send file. we opening it for reading then send line by line
									char buffer[BUFFER_SIZE];
									
									FILE *fptr1;
									fptr1 = fopen(current,"r"); 
									while(fgets(buffer, sizeof(buffer), fptr1)) {
										memset(server_reply, '\0', sizeof(server_reply));
										strcpy(server_reply,buffer);
										if(send(clientsock, server_reply, BUFFER_SIZE, 0) < 0){
											perror("sending Failed");
											exit(EXIT_FAILURE);
										}
									}
									
								}
							}
							
							//send finished
							strcpy(client_buffer,"finished");
							if(send(clientsock, client_buffer, BUFFER_SIZE, 0) < 0){
								perror("sending Failed");
								exit(EXIT_FAILURE);
							}
							printf("Succesfully Sent sourcefile(s) to server \n");
							
							
							
						}
						
						
					}
					
					
					
					
					//print delay
					gettimeofday(&tv2, NULL);
				 printf ("Response time = %f seconds\n",(double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
			}
			if(strcmp(array[0],"run") == 0){
				//we want to send out request to the server 
				if(send(clientsock, client_buffer, BUFFER_SIZE, 0) < 0){
					perror("sending Failed");
					exit(EXIT_FAILURE);
				}
				//we will recieve errors or the output of the executed file
				if(recv(clientsock, server_reply, BUFFER_SIZE, 0) < 0){
					perror("Recieving  Failed");
					exit(EXIT_FAILURE);
				}
				//prints error if server returns error
				if(strcmp(server_reply,"failure") == 0){
					printf("There was an error. Either no .c files or no .exe to run\n");
				}else{
					//success we print out the results by default
					if(strcmp(server_reply,"successfile") == 0){
						//recieve filename
						memset(server_reply, '/0', BUFFER_SIZE);
						if(recv(clientsock, server_reply, BUFFER_SIZE, 0) < 0){
							perror("Recieving  Failed");
							exit(EXIT_FAILURE);
						}
						//printf("Output was written to %s\n",server_reply);
						char tempfile[BUFFER_SIZE];
						FILE *outputfile;
						memset(tempfile, '\0', BUFFER_SIZE);
						//make a text file with the specified filename and print to file
						strcat(tempfile,server_reply);
						strcat(tempfile,".txt");						
						outputfile = fopen(tempfile, "w");
						printf("Output written to file: %s \n",tempfile);
						while(1){
							memset(server_reply, '/0', BUFFER_SIZE);
							if(recv(clientsock, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Recieving  Failed");
								exit(EXIT_FAILURE);
							}
							if(strcmp(server_reply,"finished") == 0){
								break;
							}
							fprintf(outputfile,"%s",server_reply);
							//print output
							//printf("writing: %s",server_reply);
						}
						fclose(outputfile);
						
					}else{
						//this is we just want the output instead of printing to a file
						printf("the executable output\n");
						while(1){
							memset(server_reply, '/0', BUFFER_SIZE);
							if(recv(clientsock, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Recieving  Failed");
								exit(EXIT_FAILURE);
							}
							if(strcmp(server_reply,"finished") == 0){
								break;
							}
							//print output
							printf("%s\n",server_reply);
						}
						
					}
				}
				//print delay
				gettimeofday(&tv2, NULL);
				 printf ("Response time = %f seconds\n",(double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
			}
			if(strcmp(array[0],"get") == 0){
				//we want to first verify that we can get the file 
				//send request to server for clarification
				printf("about to send \n");
				printf("oof: %s \n",server_reply);
				if(send(clientsock, client_buffer, BUFFER_SIZE, 0) < 0){
					perror("sending Failed");
					exit(EXIT_FAILURE);
				}
				if(recv(clientsock, server_reply, BUFFER_SIZE, 0) < 0){
					perror("Recieving  Failed");
					exit(EXIT_FAILURE);
				}
				printf("oof: %s \n",server_reply);
				//determine whether we can get the files
				//first error for incorrect directory
				if(strcmp(server_reply,"errordir") == 0){
					printf("Server response: Error Directory not found\n");
				}else if(strcmp(server_reply,"errorfile") == 0){
					//second error for incorrect file
					printf("Server response: Error file not found\n");
				}else{
					//success for file and server send file
					printf("File: \n");
					int lines = 0;
					while(1){
						//print the lines out and keep track of the recieveing it n amount of times
						memset(server_reply, '/0', BUFFER_SIZE);
						if(lines < 40){						
							if(recv(clientsock, server_reply, BUFFER_SIZE, 0) < 0){
								perror("Recieving  Failed");
								exit(EXIT_FAILURE);
							}
							lines += 1;
							printf("Line %d :%s \n",lines,server_reply);
						}else{
							//once 40 is hit requires user to type something to reset buffer
							if(getchar()){
								lines = 0;
							}
						}
						//if we recieve this there is nothing else to send and we break from while loop
						if(strcmp(server_reply,"finished") == 0){
							break;
						}
						
						
						
					}
					printf("End of Output :| \n");
					
				}
				
				
				//print delay
				gettimeofday(&tv2, NULL);
				 printf ("Response time = %f seconds\n",(double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
				
			}
			if(strcmp(array[0],"sys") == 0){
				//send request
				if(send(clientsock, client_buffer, BUFFER_SIZE, 0) < 0){
					perror("sending Failed");
					exit(EXIT_FAILURE);
				}
				//recieve results
				if(recv(clientsock, server_reply, BUFFER_SIZE, 0) < 0){
					perror("Recieving  Failed");
					exit(EXIT_FAILURE);
				}
				//print results
				printf("%s \n",server_reply);
				gettimeofday(&tv2, NULL);
				 printf ("Response time = %f seconds\n",(double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
				
			}
			
		}
		
		
	};
	
	
	
}