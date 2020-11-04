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
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#define CHUNK_SIZE 10000

using namespace std;

/*
RECV - SEND
peer server is a seperate thread and runs throughout the execution of the peer, accepting requests from fellow peers in a loop.
for now it runs on the port given as command line arg to the peer client process.
accept a single msg (FILE_PATH of file to send) from peer then send a single message back (data of the FILE) at once.
*/
void * peer_server(void *PORT_peer_server){
	int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    int opt =1;
    
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1){
        printf("Peer-server-log : Could not create socket");
    }
    puts("Peer-server-log : Socket created");
    
    if(setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("Peer-server-log : socket option error");
    }
    
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(*(int *)PORT_peer_server);
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
        //print the error message
        perror("Peer-server-log : Bind failed. Error");
        return 0;
    }
    puts("Peer-server-log : Bind done");
     
    //Listen
    listen(socket_desc , 10);
     

    cout<<"\nPeer-server-log : Listening for download requests..."<<endl;
    c = sizeof(struct sockaddr_in);
     
    //pthread_t thread_id;
    
    while( 1 ){			//Accept incoming connections in a loop   
        
        if((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) == -1 ){
            perror("Peer-server-log : Couldn't accept");
            break;
        }
        puts("\nPeer-server-log : New download request!");
         
		int read_size;
		
        char msgFromClient[2000] = {0};
        read_size = recv(client_sock , msgFromClient , 2000 , 0);		//recieve file_path to send to peer client
        cout<< "Peer-server-log : Download Request received! -> "<<read_size << " Bytes"<<endl;
        cout<< "Downloader says -> " << msgFromClient << endl;
        
        //--------------------------------------------------------------------------------
        
        char filename[1024] = {0};			//file_path
        strcpy(filename, msgFromClient);
        
        char * file_buffer;     //buffer to store file contents
		long size;     //file size
		ifstream file (filename, ios::in|ios::binary|ios::ate);     //open file in binary mode, get pointer at the end of the file (ios::ate)
		size = file.tellg();     //retrieve get pointer position
		file.seekg (0, ios::beg);     //position get pointer at the begining of the file
		file_buffer = new char [size];     //initialize the buffer to size = FILE_SIZE
		file.read (file_buffer, size);     //read file to buffer
		file.close();     //close file
        
        
        //--------------------------------------------------------------------------------
        
        
        //strcpy(msgToSend, "peer-server says hi!");
        //Send some messages to the client
        write(client_sock , file_buffer , strlen(file_buffer));
        cout<< "Peer-server-log : File Sent to Client! " << size << " Bytes."<<endl;
        puts("Peer-server-log : waiting for new connections!");    
    }    
    
    puts("peer-server-log : Client disconnected");
		 
		 
		 
		 
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
     
    if (client_sock < 0){
        perror("Peer-server-log : Accept failed");
    }
     
    return 0;

}


/*
//SEND - RECV
//a peer client can ping i.e send a single msg and ger a reply to a fellow peer-server using it's port number.
//This is just to confirm the fellow peer server can be reached.
//later we can use this for download request.

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
*/

bool fileExists(string file_path){
    if (FILE *file = fopen(file_path.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}

int getFileSize(char *msgToSend){
	//puts(msgToSend);
	char temp[1500];
	strcpy(temp, msgToSend);
	strtok(temp, " ");
	string file_path(strtok(NULL, " "));
	
	if(fileExists(file_path) == false){
		cout<< "Error : Can't upload, file " + file_path + " does not exist!"<< endl;
		return -1;
	}
	
	streampos begin,end;
	ifstream myfile (file_path.c_str(), ios::binary);
	begin = myfile.tellg();
	myfile.seekg (0, ios::end);
	end = myfile.tellg();
	myfile.close();
	cout << "client-log : file_size is: " << (end-begin) << " bytes.\n";
	return (end-begin);
}

/*
Send file_path to peer server and download file :
This thread receives PEER_PORT, FILE_SIZE, FILE_NAME, FILE_PATH and destination_path (SaveFileTo) as arguments. Send download request (only FILE_PATH) to that port
NOTE : the peer server sends the whole file at once using send command. Now we will need to read file in chunks.
create a file at the full path given as destinaion_path argument of the download_file command and start writing the recieved message in a loop chunk by chunk.. till there is nothing left to read

*/
void * downloader_thread(void * temp){
	char * peer_server_info = (char *) temp;
	printf("peer-server_info downloader_thread : ");/////////////////////////////////////
	puts(peer_server_info);////////////////////////////////////////////
	
	char tempInfo[1500] = {0};
	strcpy(tempInfo, peer_server_info);
	
	int PORT_peer_server = stoi(strtok(tempInfo, " "));
	int file_size = stoi(strtok(NULL, " "));
	string file_name = string(strtok(NULL, " "));
	string file_path = string(strtok(NULL, " "));
	string saveFileTo = string(strtok(NULL, " "));
	
	cout<< PORT_peer_server<< " # "<< file_size<< " # "<< file_name<< " # "<< file_path<< " # "<< saveFileTo<< endl;////////////////////////////////////////////
	
	int sock = 0, valread;
    struct sockaddr_in serv_addr; 
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 
        printf("Downloader-log : Socket creation error \n"); 
        return 0; 
    } 
   	
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT_peer_server); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0){ 
        printf("Downloader-log : Invalid address/ Address not supported \n"); 
        return 0; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ 
        printf("Downloader-log : Connection Failed \n"); 
        return 0; 
    } 

    puts("Downloader-log : Connected to Peer-Server!");
    fflush(stdin);
    
	char msgToSend[1500] = {0}, buffer[CHUNK_SIZE] = {0}; ; 
    
    strcpy(msgToSend, file_path.c_str());			
	send(sock , msgToSend , strlen(msgToSend) , 0 ); 				//-----send only FILE_PATH to the peer server as download request
	puts("Downloader-log : Download request sent to peer-server"); 
	
	string full_file_addr = file_path + "/" + file_name;
	int fd = open(saveFileTo.c_str(), O_CREAT | O_WRONLY,S_IRUSR | S_IWUSR); 	//-----create a file at the full path given as a part of command to write the downloaded data

    if(fd == -1)
        cout<< "Downloader-log : Couldn't open file" << full_file_addr<< endl;

	int bytesRead = 1, bytesFlushed;
	while(1){           									//write file in chunks from local buffer.
        bytesRead = recv(sock, buffer, CHUNK_SIZE, 0);
        printf("Downloader-log : Bytes read %d\n", bytesRead);
        
        printf("Downloader-log : Flushing data\n");
        bytesFlushed = write(fd, buffer, bytesRead);
        printf("Downloader-log : Bytes written %d\n", bytesFlushed);
        
		if(bytesRead < CHUNK_SIZE)	//if this is the last chunk to write (inlcluding 0 size chunk) then it will be less than the CHUNK_SIZE
            break;
        
        if(bytesFlushed < 0)
            perror("Downloader-log : Failed to flush data!");
    }

	close(fd);
	printf("\nDownloader-log : All File chunks successfully downloaded!\n");
	
	/*
	valread = read( sock , buffer, CHUNK_SIZE); 
	puts("Downloader-log : file downloaded -> ");
	printf("%s\n",buffer );   
	*/
}

/*
send download command to the server -> and get the PEER_PORT, FILE_SIZE, FILE_NAME, FILE_PATH
create a seperate thread to download this file from the peer server that has this file (i.e Port returned by tracker)
wait till the download is complete
pass the control back to the input block in main
*/
void download_file(string msgToSend, string client_id, int sock){
		cout<<"check"<<endl;
	if(client_id == ""){
		cout<<"Error : user not logged in!"<<endl;
		return;
	}
	
	char tempDestPath[1500] = {0};				//store destination_path from download_file arguments. fill will be saved to this location
	strcpy(tempDestPath, msgToSend.c_str());
	char saveFileTo[1500];
	strtok(tempDestPath, " ");
	strtok(NULL, " ");
	strtok(NULL, " ");
	strcpy(saveFileTo, strtok(NULL, " "));
	cout<<"Client-log : File will be saved to : "<<saveFileTo<<endl;
	
	msgToSend = msgToSend + client_id;
	
	char msg_send[1500] ={0}, buffer[1024] ={0};
	strcpy(msg_send, msgToSend.c_str());
	send(sock , msg_send , strlen(msg_send) , 0 ); 			//send command to server
	puts("Client-log : Message sent to Server"); 
	
	int valread = read( sock , buffer, 1024); 						//recieve result from server : contains details of file to download
	cout<<"check"<<endl;
	printf("Client-log : message recieved from server -> %s\n", buffer );
	
	cout<<"check"<<endl;
	char peer_server_info[1500] = {0};
	cout<<"check"<<endl;
	strcpy(peer_server_info, buffer);
	strcat(peer_server_info, " ");
	strcat(peer_server_info, saveFileTo);
	
	printf("peer-server_info : ");/////////////////////////////////////
	puts(peer_server_info);////////////////////////////////////////////
	
	pthread_t thread_id;
	if( pthread_create( &thread_id , NULL ,  downloader_thread ,(void *) &peer_server_info) < 0){
		perror("Server-log : Could not create thread");
		return;
	}
	
	pthread_join(thread_id, NULL);
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
			else if(cmd == "upload_file"){
				if(len != 3){
					cout<< "Error : bad arguments! (usage : upload_file <file_path> <group_id>)"<<endl;
				}else{
					strcat(msgToSend, " ");
					strcat(msgToSend, (to_string(PORT_peer_server)).c_str());		//append peer-server-port to the message to be sent to tracker
					int file_size = getFileSize(msgToSend);
					if(file_size == -1){
						cout<<"Error : no such file!"<<endl;
					}else{
						strcat(msgToSend, " ");
						strcat(msgToSend, (to_string(file_size)).c_str());				//append file_size to message
						input_flag = 0;
					}
				}
			}
			else if(cmd == "list_files"){
				if(len != 2){
					cout<< "Error : bad arguments! (usage : list_files <group_id>)"<<endl;
				}else{
					input_flag = 0;
				}
			}
			else if(cmd == "download_file"){
				if(len != 4){
					cout<< "Error : bad arguments! (usage : download_file <group_id> <file_name> <destination_path>";
				}else{
					cout<<"checks "<<string(msgToSend)<<" "<<string(client_id)<<endl;
										
					download_file(string(msgToSend), string(client_id), sock);
					cout<<"checke"<<endl;
				}
			}
			/*else if(cmd == "ping"){
				if(len != 2){
					cout<<"Error : bad arguments! (usage : ping <port_number_of_peer>) [+]TIP: use to ping peers!"<<endl;
				}else{
					ping(args);
				}
				continue;		//for now ping does not need user to login, so skip rest and take next input
			}*/
			else if(cmd == "bye"){
				break;
			}
			else{
				cout<<"Error : no such command!"<<endl;
				continue;		//skip and take next input
			}
			
			if(cmd!="create_user" && cmd!="login" && strcmp(client_id, "")==0){		//dont proceed if not logged in stay in loop
				cout<<"Error : not logged in! can't execute -> " + cmd + " (TIP: login <user_id> <password>)";
				input_flag = 1;
			}
			
			
			if(cmd=="login" && strcmp(client_id, "")!=0){							//can't login if already logged in
				cout<<"Error : already logged in to user_id ->" + string(client_id);
				input_flag = 1;
			}
			
		}
		
		strcat(msgToSend, client_id);
		
		send(sock , msgToSend , strlen(msgToSend) , 0 ); 			//send command to server
		puts("Client-log : Message sent to Server"); 
		
		valread = read( sock , buffer, 1024); 						//recieve result from server
			cout<<"pakad"<<endl;
		printf("Client-log : Messssage Recieved from Server -> %s\n", buffer );
		
		
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
