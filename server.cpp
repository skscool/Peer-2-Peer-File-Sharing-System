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
#include <fstream>
#define PORT 8888

using namespace std;

struct single_user{
    string password;
    bool is_login;
    set<int> group_id;
    single_user(string pass){
        password = pass;
        is_login = false;
    }
};



struct single_file{
	string file_path;
	string owner_id;
	int owner_port;
	int file_size;
	bool is_being_shared;
	single_file(string f, string o, int p, int s){
		file_path = f;
		owner_id = o;
		owner_port = p;
		file_size = s;
		is_being_shared = true;
	}
};


struct single_group{
    set<string> pending_id;
    set<string> member_id;
    string admin_id;
    map<string, single_file*> file_info;
    single_group(string admin){
        admin_id = admin;
        member_id.insert(admin);
    }
};



//golbal vars
map<string, single_user*> user_info;
map<int, single_group*> group_info;


//the thread function
void *connection_handler(void *);
 

/*
Tracker program
1. keep listening on port 8888 (hardcoded) for incoming commands from peer-clients. (NOTE : A peer-server never contacts the tracker)
2. cater each connection request from distinct peer-clients on seperate threads. ACCEPT FN CALL -> NEW THREAD
3. execute input commands from client in a infinite loop.
*/
 
//initialize a few dummy users = 1 and 2, a group 1, and 2 uploaded files in group 1 named d and copy.cpp with seeder port as 6666.
//must login first to access these dummy data
void init(){
	user_info["1"] = new single_user("1");
	user_info["1"]->group_id.insert(1);
	group_info[1] = new single_group("1");
	(group_info[1]->file_info)["sandbox.cpp"] = new single_file("/home/satyam/sandbox.cpp", "1", 6666, 21267);
	(group_info[1]->file_info)["copy.cpp"] = new single_file("copy.cpp", "1", 6666, 10890);
	
	user_info["2"] = new single_user("2");
	user_info["1"]->group_id.insert(1);
	(group_info[1]->member_id).insert("2");
} 
 
 
int main(int argc , char *argv[])
{

	init();		//insert some dummy data for testing
	
	
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    int opt =1;
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    
    if (socket_desc == -1){
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
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
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
    //int len = args.size();
    //cout<<len<<endl;
    string status = "success";
    
    //bad number of argument to command
    // if(len < 2){
    //     status = "Error : 2 arguments expected! (usage -> create_user id pass)";
    //     return status;
    // }
    
    string user_id = args[0], password = args[1];
    
    //if user id already present i.e not unique
    auto user_itr = user_info.find(user_id);
    if(user_itr != user_info.end()){
        status = "Error : user_id already exists!";
        return status;
    }
    
    //success
    user_info[user_id] = new single_user(password);  //create user entry with pass in user_info
    return status;
}


string login(vector<string> args){
    //int len = args.size();
    string status = "success";
    
    //bad number of argument to command
    // if(len < 2){
    //     status = "Error : 2 arguments expected! (usage -> login id pass)";
    //     return status;
    // }
    
    string user_id = args[0], password = args[1];
    
    //if no such user id
    auto user_itr = user_info.find(user_id);
    if(user_itr == user_info.end()){
        status = "Error : no such user_id!";
        return status;
    }
    
    //if password incorrect
    if(password != user_info[user_id]->password){
        status = "Error : incorrect password!";
        return status;
    }
    
    //if user mentioned as part of login command is logged in
    if(user_info[user_id]->is_login == true){
        status = "Error : user already logged in, logout first!";
        return status;
    }

    //if client in client_id metadata is logged in
    // string client_user_id = args[2];
    // if(user_info[client_user_id]->is_login == true){
    //     status = "Error : to login other account, logout from user_id " + client_user_id;
    //     return status;
    // }
    
    //success
    user_info[user_id]->is_login = true;
    return status;
}


string logout(vector<string> args){     //for now it just sets the is_login flag = false
    //int len = args.size();
    string status = "success";
    
    // if(len == 0){
    //     status = "Error : not logged in!";
    //     return status;
    // }
    string client_user_id = args[0];

    user_info[client_user_id]->is_login = false;
    return status;
}

//create an entry in map with group_id as key and all group properties as a value
string create_group(vector<string> args){
    string status = "success";
    //int len = args.size();
    
    // if(len < 2){
    //     status = "Error : not logged in! / Bad argument\n (usage : 1.login <user_id> <password> 2.create_group <group id>)";
    //     return status;
    // }
    
    int group_id = stoi(args[0]);
    
    if(group_info.find(group_id) != group_info.end()){
        status = "Error : group with this id already exist!";
        return status;
    }
    
    string user_id = args[1];
    auto user_itr = user_info.find(user_id);
    
    // if(user_itr == user_info.end() || user_itr->second->is_login == false){
    //     status = "Error : Bad arguments / user_id metadata is malicious!";
    //     return status;
    // }
    
    //if(user_itr->second->group_id > 0){
    //    status = "can't participate in multiple groups! (TIP: leave group_id " + to_string(user_itr->second->group_id) +")";
    //   return status;
    //}
    
    //if(user_itr->second->group_id < 0){
    //    status = "request pending with another group! (TIP: leave group_id " + to_string(-(user_itr->second->group_id)) +")";
    //    return status;
    //}
        
    cout<<"group id : "<<group_id<<endl;
    group_info[group_id] = new single_group(user_id);
    (user_itr->second->group_id).insert(group_id);
    return status;
}


string list_groups(vector<string> args){
    //int len = args.size();
    string status = "Groups : ";
    
    // if(len == 0){
    //     status = "Error : not logged in";
    //     return status;
    // }
    
    string user_id = args[0];
    // if(user_info.find(user_id) == user_info.end() || user_info.find(user_id)->second->is_login == false){
    //     status = "Error : Bad arguments / user_id metadata is malicious!";
    //     return status;
    // }
    
    for(auto eachGroup : group_info){
        status += to_string(eachGroup.first) + ", ";
    }
    
    if(status.size() == 9){
        status = "No groups found!";
        return status;
    }
    
    return status;
}


string list_requests(vector<string> args){
    //int len = args.size();
    string status = "Join Requests : ";
    
    // if(len < 2){
    //     status = "Error : not logged in!";
    //     return status;
    // }
    
    int group_id = stoi(args[0]);
    auto grp_itr = group_info.find(group_id);
    
    if(grp_itr == group_info.end()){
        status = "Error : no such group_id!";
        return status;
    }   
    
    set<string> pending_reqs = grp_itr->second->pending_id;
    for(auto eachReq : pending_reqs){
        status += eachReq + ", ";
    }

    if(status.size() == 16){
        status = "No pending requests!";
        return status;
    }
    
    return status;
}


/*
group_id =
0   -> no group joined or requested
-ve -> requested but not accepted
+ve -> succes join

joining process -> 
join : 1) set user_info->group_id as -ve of group_id where it's request is pending
       2) update pending_id in group_info 
*/
string join_group(vector<string> args){
    //int len = args.size();
    string status = "success : request pending with admin!";
    
    // if(len < 2){
    //     status = "Error : not logged in!";
    //     return status;
    // }
    
    int group_id = stoi(args[0]);
    auto group_itr = group_info.find(group_id);
    
    if(group_itr == group_info.end()){
        status = "Error : no such group_id";
        return status;
    }
    
    string user_id = args[1];
    auto user_itr = user_info.find(user_id);
    
    set<int> group_status = user_itr->second->group_id;
    auto group_status_itr = group_status.find(group_id);
    
	if(group_status_itr != group_status.end()){
        status = "Error : user_id " + user_id + " is already part of group_id " + to_string(group_id);
        return status;
    }
    
	set<string> pending_status = group_itr->second->pending_id;
	auto pending_itr = pending_status.find(user_id);
    
	if(pending_itr != pending_status.end()){
        status = "Error : join request for user_id " + user_id + " is already pending with group_id " + to_string(group_id);
        return status;
    }
    
    (group_itr->second->pending_id).insert(user_id);
}

/*
acception precess ->
1. remove user_id entry from the respective group_info pending_id list
2. add entry in member_id list of the group
3. set user_info group id
*/
string accept_request(vector<string> args){
    //int len = args.size();
    string status = "success";
    
    // if(len < 3){
    //     status = "Error : not logged in";
    //     return status;
    // }
    
    int group_id = stoi(args[0]);
    auto group_itr = group_info.find(group_id);
    
    if(group_itr == group_info.end()){
        status = "Error : no such group_id";
        return status;
    }
    
    string user_id_accept = args[1];
    auto user_itr = user_info.find(user_id_accept);
    
    if(user_itr == user_info.end()){
        status = "Error : no such user_id";
        return status;
    }
    
    set<int> group_status = user_itr->second->group_id;
    auto group_status_itr = group_status.find(group_id);
    
	if(group_status_itr != group_status.end()){
        status = "Error : user_id " + user_id_accept + " is already part of group_id " + to_string(group_id);
        return status;
    }
    
    //if(group_status < 0){
    //    if(group_status != (-group_id)){
    //        status = "Error : join request for user_id " + user_id_accept + " is already pending with group_id " + to_string(-group_status);    
    //       return status;
    //    }
    //    // else{
    //    //  status = "Error : user is already member of this group";
    //    // }
        
    //}
    
    set<string> pending_reqs = group_itr->second->pending_id;
    auto pending_itr = pending_reqs.find(user_id_accept);
    auto pending_itr_orig = (group_itr->second->pending_id).find(user_id_accept);   //itr to original set
    
    if(pending_itr == pending_reqs.end()){
        status = "Error : no such pending request!";
        return status;
    }
    
    string client_user_id = args[2];
    if(group_itr->second->admin_id != client_user_id){
        status = "Error : you are not admin of this group!";
        return status;
    }
    
    (group_itr->second->pending_id).erase(pending_itr_orig);
    (group_itr->second->member_id).insert(user_id_accept);
    (user_itr->second->group_id).insert(group_id);
    return status;
}


string leave_group(vector<string> args){
	string status = "success : member left group!";
	
	int group_id = stoi(args[0]);
	string client_user_id = args[1];
	
	auto group_itr = group_info.find(group_id);
	
	if(group_itr == group_info.end()){
		status = "Error : group_id does not exist";
		return status;
	}
	auto user_itr = user_info.find(client_user_id);
	set<int> client_group_status = user_itr->second->group_id;
	
	if(client_group_status.empty()){
		status = "Error : user does not belong to any group!";
		return status;
	}
	
	auto client_group_status_itr = client_group_status.find(group_id);
	if(client_group_status_itr != client_group_status.end()){							//---------user belong to group
		if(group_itr->second->admin_id == client_user_id){				//if user is admin
			(user_itr->second->group_id).erase(group_id);				//user disassociates itself in user_info
			(group_itr->second->member_id).erase(client_user_id);		//membership is cancelled in group_info
			
			auto member_itr = (group_itr->second->member_id).begin();
			if(member_itr == (group_itr->second->member_id).end()){		//if admin is the last person in group to leave, delete the group
				group_info.erase(group_itr);
				status = "success : user was admin and only member of group! group deleted";
			}else{
				group_itr->second->admin_id = *member_itr;				//else group member with smallest id is made Admin
				status = "success :user was admin, new admin of group_id : " + to_string(group_id) + " is -> " + *member_itr;
			}
			
		}else{															//else user is just a member
			(user_itr->second->group_id).erase(group_id);
			(group_itr->second->member_id).erase(client_user_id);
		}
		return status;
	}
	
	auto pending_status = (group_itr->second->pending_id);
	auto pending_status_itr = pending_status.find(client_user_id);
	if(pending_status_itr != pending_status.end()){							//-----delete pending request if exists
		(group_itr->second->pending_id).erase(client_user_id);
		return status;
	}
	
	status = "Error : user does not belong to this group!";
	return status;	
}

/*
extract info from command string and create a single entry in group_info->file_info
all files are distinct in a group.
*/
string upload_file(vector<string> args){
	string status = "success";
	
	string file_path = args[0];
	int group_id = stoi(args[1]);
	int peer_server_port = stoi(args[2]);
	int file_size = stoi(args[3]);
	string peer_user_id = args[4];
	cout<<"Server-log : file_path "<<file_path<<" group_id " <<group_id<< " peer_server_port "<<peer_server_port<< " file_size "<<file_size<< " peer_user_id "<< peer_user_id;
	
	char temp[1500];
	strcpy(temp, file_path.c_str());
	char *temp2;
	string file_name = string(strtok(temp, "/"));
	while(temp2 = strtok(NULL, "/")){
		file_name = string(temp2);
	}
	cout<<" file_name "<<file_name<<endl;
	
	auto group_itr = group_info.find(group_id);
	if(group_itr == group_info.end()){
		status = "Error : no such group_id";
		return status;
	}
	
	auto member_status = group_itr->second->member_id;
	auto member_itr = member_status.find(peer_user_id);
	if(member_itr == member_status.end()){
		status = "Error : user is not member of this group!";
		return status;
	}
	
	auto file_status = group_itr->second->file_info;
	auto file_itr = file_status.find(file_name);
	/*if(file_itr != file_status.end()){
		status = "Error : file already present in this group!";
		return status;
	}*/
	
	(group_itr->second->file_info)[file_name] = new single_file(file_path, peer_user_id, peer_server_port, file_size);
}


string list_files(vector<string> args){
    //int len = args.size();
    string status = "Files Available : ";
    
    // if(len < 2){
    //     status = "Error : not logged in!";
    //     return status;
    // }
    
    int group_id = stoi(args[0]);
    auto grp_itr = group_info.find(group_id);
    
    if(grp_itr == group_info.end()){
        status = "Error : no such group_id!";
        return status;
    }   
    
    map<string, single_file*> file_info = grp_itr->second->file_info;
    for(auto eachFile : file_info){
        status += eachFile.first + ", ";
    }

    if(status.size() == 18){
        status = "No files available!";
        return status;
    }
    
    return status;
}

//form the status string to be sent to the peer server waiting for file metadata PORT, FILE_SIZE, FILE_NAME, FILE_PATH
string download_file(vector<string> args){
	string status;
	
	int group_id = stoi(args[0]);
	string file_name = args[1];
	string destination_path = args[2];	//not of use here, it is path where client wants to save it's downloaded file
	string file_path;
	string client_user_id = args[3];
	
	auto group_itr = group_info.find(group_id);
	if(group_itr == group_info.end()){
		status = "Error : no such group_id!";
		return status;
	}
	
	auto member_status = group_itr->second->member_id;
	auto member_itr = member_status.find(client_user_id);
	if(member_itr == member_status.end()){
		status = "Error : user is not member of the group_id " + to_string(group_id);
		return status;
	}
	
	auto file_status = group_itr->second->file_info;
	auto file_itr = file_status.find(file_name);
	if(file_itr == file_status.end()){
		status = "Error : file is not shared in this group!";
		return status;
	}
	
	auto owner_itr = user_info.find(file_itr->second->owner_id);
	if(owner_itr->second->is_login == false){
		status = "Error : owner of the file is not logged in!";
		return status;
	}
	
	
	//send owner port that has file, send file_size and file_name and file_path
	status = to_string(file_itr->second->owner_port) + " " + to_string(file_itr->second->file_size) + " " +file_name + " " + file_itr->second->file_path;
}


//extract command, then arguments (in vector) from client message. call fn to exucute those.
string executeCmd(char *msgFromClient){
    
    /*input is plain string of tokens by client. in this fn we do 
    1.get 1st arg i.e command name 
    2.get all the args pushed in vector 
    3.pass vector to respective fn of command
    */
    
    string cmd(strtok(msgFromClient, " "));     //to execute this command
    
    vector<string> args;                        //get list of arguments to the command
    char *temp;
    while(temp = strtok(NULL, " ")){
        args.push_back(string(temp));
    }   
    
    string status = "unknown!";                 //status returned by respective command to be sent back to client directly
    
    if(cmd == "create_user"){
        status = create_user(args);
    }
    else if(cmd == "login"){
        status = login(args);
    }
    else if(cmd == "logout"){
        status = logout(args);
    }
    else if(cmd == "create_group"){
        status = create_group(args);
    }
    else if(cmd == "list_groups"){
        status = list_groups(args);
    }
    else if(cmd == "list_requests"){
        status = list_requests(args);
    }
    else if(cmd == "join_group"){
        status = join_group(args);
    }
    else if(cmd == "accept_request"){
        status = accept_request(args);
    }
    else if(cmd == "leave_group"){
    	status = leave_group(args);
    }
    else if(cmd == "upload_file"){
    	status = upload_file(args);
    }
    else if(cmd == "list_files"){
    	status = list_files(args);
    }
    else if(cmd == "download_file"){
    	status = download_file(args);
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
        char temp_msg[2000] = {0};              /*************** always perform strtok on a copy of original msg***************/
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
        cout<< "Server-log : Message Sent to Client!\n" << endl;    
    }    
    
    puts("Server-log : Client disconnected");
             
    return 0;
} 
