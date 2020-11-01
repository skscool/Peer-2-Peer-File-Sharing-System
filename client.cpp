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

using namespace std;

/*
RECV - SEND
peer server is a seperate thread and runs throughout the execution of the peer, accepting requests from fellow peers in a loop.
for now it runs on the port given as command line arg to the peer client process.
accept a single msg from peer then send a single message back.
*/
void * peer_server(void *PORT_peer_server){
	int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    int opt =1;
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1){
        printf("peer-server-log : Could not create socket");
    }
    puts("peer-server-log : Socket created");
    
    if(setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("peer-server-log : socket option error");
    }
    
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(*(int *)PORT_peer_server);
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("peer-server-log : Bind failed. Error");
        return 0;
    }
    puts("peer-server-log : Bind done");
     
    //Listen
    listen(socket_desc , 10);
     

    cout<<"\npeer-server-log : Listening for incoming connections..."<<endl;
    c = sizeof(struct sockaddr_in);
     
    //pthread_t thread_id;
    
    while( 1 )
    {
        //Accept incoming connection    
        
        if((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) == -1 ){
            perror("peer-server-log : Couldn't accept");
            break;
        }
        puts("\npeer-server-log : New connection!");
         
        //if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        //{
        //    perror("Server-log : Could not create thread");
        //    return 1;
        //}
         
		int read_size;
		
        char msgToSend[2000] = {0} , msgFromClient[2000] = {0};
        //Receive a message from client
        read_size = recv(client_sock , msgFromClient , 2000 , 0);
        cout<< "peer-server-log : Command received! -> "<<read_size << " Bytes"<<endl;
        cout<< "peer says -> " << msgFromClient << endl;
        
        strcpy(msgToSend, "peer-server says hi!");
        
        //Send some messages to the client
        write(client_sock , msgToSend , strlen(msgToSend));
        cout<< "peer-server-log : Message Sent to Client!\n" << endl;
        puts("peer-server waiting for new connection!");    
    }    
    
    puts("peer-server-log : Client disconnected");
		 
		 
		 
		 
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
     
    if (client_sock < 0){
        perror("peer-server-log : Accept failed");
    }
     
    return 0;

}

/*
SEND - RECV
a peer client can ping i.e send a single msg and ger a reply to a fellow peer-server using it's port number.
This is just to confirm the fellow peer server can be reached.
later we can use this for download request.
*/
void ping(vector<string> args){
	int PORT = stoi(args[0]);
	int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("ping-log : Socket creation error \n"); 
        return; 
    } 
   	
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("ping-log : Invalid address/ Address not supported \n"); 
        return; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("ping-log : Connection Failed \n"); 
        return; 
    } 

    puts("ping-log : Connected to Server!");
    fflush(stdin);
    //while(1){
    	char msgToSend[1500] = {0}, buffer[1024] = {0}; ; 
	    printf("\n\nEnter a Message : ");
		fflush(stdin);
		gets(msgToSend);
		//puts(msgToSend);
					
		send(sock , msgToSend , strlen(msgToSend) , 0 ); 
		puts("ping-log : Message sent to Server"); 
		valread = read( sock , buffer, 1024); 
		puts("ping-log : Message Recieved from Server -> ");
		printf("%s",buffer );   
		
		//if(strcmp(msgToSend, "bye") == 0)
			//break;  
    //}
    
    puts("\nping-log : Disconnected!");
}

/*
1. run a thread for peer-server (run forever) on the port given (mainly to listen to fellow client for download req.)
2. connect to the tracker (stay connected forever) running at port 8888 (hardcoded).
3. take command from user, and send to tracker for processing (infinite loop)
4. if user wants can connect to a fellow peer server for a single message passing.
*/
int main(int argc, char const *argv[]) 
{ 
	if(argc < 2){				//port number is a must for it's server
		cout<< "Error : bad arguments! (usage: ./cc <port_number_of_peer_server>)"<< endl;
		return -1;
	}
	
	int PORT_peer_server = stoi(argv[1]);
	cout<<"Peer server port : "<<PORT_peer_server<<endl;
	pthread_t thread_id;
	if( pthread_create( &thread_id , NULL ,  peer_server ,(void *) &PORT_peer_server) < 0){
		perror("Server-log : Could not create thread");
		return 1;
	}
	
	
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("Client-log : Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(8888); 
       
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
			
			vector<string> args;                        //get list of arguments to the command
			char *temp;			
			
			int len = 1;
			while(temp = strtok(NULL, " ")){			//store args in vector and find length of input command as well
				args.push_back(string(temp));
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
			else if(cmd == "ping"){
				if(len != 2){
					cout<<"Error : bad arguments! (usage : ping <port_number_of_peer>) [+]TIP: use to ping peers!"<<endl;
				}else{
					ping(args);
				}
				continue;		//for now ping does not need user to login, so skip rest and take next input
			}
			else{
				cout<<"Error : no such command!"<<endl;
				continue;		//skip and take next input
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
