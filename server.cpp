/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/
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

struct single_user{
	string password;
	bool is_login;
	single_user(string pass){
		password = pass;
		is_login = false;
	}
};

//golbal vars
map<string, single_user*> user_info;


//the thread function
void *connection_handler(void *);
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    int opt =1;
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Server-log : Could not create socket");
    }
    puts("Server-log : Socket created");
    
    if(setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
    	perror("Server-log : socket option error");
    }
    
    
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("Server-log : Bind failed. Error");
        return 1;
    }
    puts("Server-log : Bind done");
     
    //Listen
    listen(socket_desc , 10);
     

    cout<<"\nServer-log : Listening for incoming connections..."<<endl;
    c = sizeof(struct sockaddr_in);
     
	pthread_t thread_id;
	
    while( 1 )
    {
		//Accept incoming connection	
		
		if((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) == -1 ){
			perror("Server-log : Couldn't accept");
			break;
		}
        puts("\nServer-log : New connection!");
         
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("Server-log : Could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
    }
     
    if (client_sock < 0)
    {
        perror("Server-log : Accept failed");
        return 1;
    }
     
    return 0;
}
 

//add user id,pass entry in user_cred if not exist. return status string accordingly
string create_user(vector<string> args){
	int len = args.size();
	//cout<<len<<endl;
	string status = "success";
	
	//bad number of argument to command
	if(len < 2){
		status = "Error : 2 arguments expected! (usage -> create_user id pass)";
		return status;
	}
	
	string id = args[0], pass = args[1];
	
	//if user id already present i.e not unique
	auto itr = user_info.find(id);
	if(itr != user_info.end()){
		status = "Error : userid already exists!";
		return status;
	}
	
	//success
	user_info[id] = new single_user(pass);	//create user entry with pass in user_info
	return status;
}


string login(vector<string> args){
	int len = args.size();
	string status = "success";
	
	//bad number of argument to command
	if(len < 2){
		status = "Error : 2 arguments expected! (usage -> login id pass)";
		return status;
	}
	
	string id = args[0], pass = args[1];
	
	//if no such user id
	auto itr = user_info.find(id);
	if(itr == user_info.end()){
		status = "Error : no such userid!";
		return status;
	}
	
	//if password incorrect
	if(pass != user_info[id]->password){
		status = "Error : incorrect password!";
		return status;
	}
	
	//if user mentioned as part of login command is logged in
	if(user_info[id]->is_login == true){
		status = "Error : already logged in, logout first!";
		return status;
	}

	//if client in client_id metadata is logged in
	if(len > 2 && user_info[args[2]]->is_login == true){
		status = "Error : to access other account, logout first!";
		return status;
	}
	
	//success
	user_info[id]->is_login = true;
	return status;
}


string logout(vector<string> args){		//for now it just sets the is_login flag = false
	int len = args.size();
	string status = "success";
	
	if(len == 0){
		status = "Error : not logged in!";
		return status;
	}
	
	user_info[args[0]]->is_login = false;
	return status;
}


//extract command, then arguments (in vector) from client message. call fn to exucute those.
string executeCmd(char *msgFromClient){
	
	/*input is plain string of tokens by client. in this fn we do 
	1.get 1st arg i.e command name 
	2.get all the args pushed in vector 
	3.pass vector to respective fn of command
	*/
	
	string cmd(strtok(msgFromClient, " "));		//to execute this command
	
	vector<string> args;						//get list of arguments to the command
	char *temp;
	while(temp = strtok(NULL, " ")){
		args.push_back(string(temp));
	}	
	
	string status = "unknown!";					//status returned by respective command to be sent back to client directly
	
	if(cmd == "create_user"){
		status = create_user(args);
	}
	else if(cmd == "login"){
		status = login(args);
	}
	else if(cmd == "logout"){
		status = logout(args);
	}
	else{
		status = "Error : no such command!";
	}
	cout<<status<<endl;
	return status;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;


	while(1){		 
		char msgToSend[2000] = {0} , msgFromClient[2000] = {0};
		//Receive a message from client
		read_size = recv(sock , msgFromClient , 2000 , 0);
		cout<< "Server-log : Command received! -> "<<read_size << " Bytes"<<endl;
		cout<< "Client says -> " << msgFromClient << endl;
		
		//If client wants to disconnect, then break loop gracefully
		char temp_msg[2000] = {0};				/*************** always perform strtok on a copy of original msg***************/
		strcpy(temp_msg, msgFromClient);
		if(strcmp(strtok(temp_msg, " "), "bye") == 0){
			//Send some messages to the client
			strcpy(msgToSend, "bye");
			write(sock , msgToSend , strlen(msgToSend));
			cout<< "Server-log : Message Sent to Client!" << endl;	
			break;
		}
		
		strcpy(msgToSend, executeCmd(msgFromClient).c_str());
		
		//Send some messages to the client
		write(sock , msgToSend , strlen(msgToSend));
		cout<< "Server-log : Message Sent to Client!" << endl;	
	}    
	
	puts("Server-log : Client disconnected");
	
	
/*	
    msgToSend = "Now type something and i shall repeat what you type \n";
    write(sock , msgToSend , strlen(msgToSend));
     
    //Receive a message from client
    while( (read_size = recv(sock , msgFromClient , 2000 , 0)) > 0 )
    {
        //end of string marker
		msgFromClient[read_size] = '\0';
		
		//Send the message back to client
        write(sock , msgFromClient , strlen(msgFromClient));
		
		//clear the message buffer
		memset(msgFromClient, 0, 2000);
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
*/
         
    return 0;
} 
