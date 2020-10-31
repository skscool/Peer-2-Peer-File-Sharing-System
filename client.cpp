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
	    int input_flag = 1;
	    
	    while(input_flag){
	    	memset(&msgToSend[0], 0, sizeof(msgToSend));
			printf("\n\nEnter a Message : ");							//get command from user
			gets(msgToSend);
		
			char temp_tokenize[1500] = {0};
			strcpy(temp_tokenize, msgToSend);
			
			string cmd(strtok(temp_tokenize, " "));
			
			int len = 1;
			while(strtok(NULL, " ")){
				len++;
			}
			
			if(cmd == "create_user"){
				if(len != 3){
					cout<<"Error : bad arguments! (usage : create_user <user_id> <password>)"<<endl;
				}else{
					input_flag = 0;
				}
			}
			else if(cmd == "login"){
				if(len != 3){
					cout<<"Error : bad arguments! (usage : login <user_id> <password>)"<<endl;
				}else{
					input_flag = 0;
				}
			}
			else if(cmd == "logout"){
				if(len != 1){
					cout<<"Error : bad arguments! (usage : logout)"<<endl;
				}else{
					input_flag = 0;
				}
			}
			else if(cmd == "create_group"){
				if(len != 2){
					cout<<"Error : bad arguments! (usage : create_group <group_id>)"<<endl;
				}else{
					input_flag = 0;
				}
			}
			else if(cmd == "list_groups"){
				if(len != 1){
					cout<<"Error : bad arguments! (usage : list_groups)"<<endl;
				}else{
					input_flag = 0;
				}
			}
			else if(cmd == "list_requests"){
				if(len != 2){
					cout<<"Error : bad arguments! (usage : list_requests <group_id>)"<<endl;
				}else{
					input_flag = 0;
				}
			}
			else if(cmd == "join_group"){
				if(len != 2){
					cout<<"Error : bad arguments! (usage : join_group <group_id>)"<<endl;
				}else{
					input_flag = 0;
				}
			}
			else if(cmd == "accept_request"){
				if(len != 3){
					cout<<"Error : bad arguments! (usage : accept_request <group_id> <user_id>)"<<endl;
				}else{
					input_flag = 0;
				}
			}
			else if(cmd == "leave_group"){
				if(len != 2){
					cout<<"Error : bad arguments! (usage : leave_group <group_id>)"<<endl;
				}else{
					input_flag = 0;
				}
			}
			else if(cmd == "bye"){
				break;
			}
			else{
				cout<<"Error : no such command!"<<endl;
			}
			
			if(cmd!="create_user" && cmd!="login" && strcmp(client_id, "")==0){
				cout<<"Error : not logged in! can't execute -> " + cmd + " (TIP: login <user_id> <password>)";
				input_flag = 1;
			}
			//cout<<cmd<<" "<<string(client_id)<<" "<<strcmp(client_id, "")<<endl;
			if(cmd=="login" && strcmp(client_id, "")!=0){
				cout<<"Error : already logged in to user_id ->" + string(client_id);
				input_flag = 1;
			}
			
		}
		
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
