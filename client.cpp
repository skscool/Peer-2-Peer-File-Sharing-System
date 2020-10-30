#include <bits/stdc++.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#define PORT 8888

using namespace std;


int main(int argc, char const *argv[]) 
{ 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("Client-log : Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("Client-log : Invalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("Client-log : Connection Failed \n"); 
        return -1; 
    } 

    puts("Client-log : Connected to Server!");
    
	char client_id[100] = {0};
	
    //client send command recieve result in loop
    while(1){
    	char msgToSend[1500] = {0}, buffer[1024] = {0}; 
	    printf("\n\nEnter a Message : ");							//get command from user
		gets(msgToSend);
		strcat(msgToSend, client_id);
		
		send(sock , msgToSend , strlen(msgToSend) , 0 ); 			//send command to server
		puts("Client-log : Message sent to Server"); 
		
		valread = read( sock , buffer, 1024); 						//recieve result from server
		printf("Client-log : Message Recieved from Server -> %s\n", buffer );
		
		
		char temp[1500] = {0};		/*************** always perform strtok on a copy of original msg***************/
		strcpy(temp, msgToSend);
		char *temp_cmd = strtok(temp, " ");					//get the command name that was sent
		
		//if login was successful then now onwards append cliend_id to each msg sent to server
		if(strcmp(temp_cmd, "login") == 0 && strcmp(buffer, "success") == 0){
			char *temp_id = strtok(NULL, " ");
			char temp_idWithSpace[100] = {0};
			temp_idWithSpace[0] = ' ';
			strcat(temp_idWithSpace, temp_id);
			strcpy(client_id, temp_idWithSpace);
			printf("client_id set : %s\n", client_id);
		}
		
		//if logout is successful then clear options for msg sent to server
		if(strcmp(temp_cmd, "logout") == 0 && strcmp(buffer, "success") == 0){
			char *temp = "";
			strcpy(client_id, temp);
			printf("client_id cleared! %s\n", client_id);
		}
		
		
		if(strcmp(temp_cmd, "bye") == 0)							//close client on typing bye
			break;  
    }
    
    puts("\nClient-log : Disconnected!");
    return 0; 
} 
