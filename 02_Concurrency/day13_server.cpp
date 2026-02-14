#include<iostream>
#include<sys/socket.h> // for socket()
#include<netinet/in.h> // for AF_NET
#include<unistd.h> //for close()
#include<cstring> // for strlen()

int main(){

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1){
		std::cout<<"Socket failed"<<std::endl;
		return 1;
	} else{
		std::cout<<"Socket created "<<fd<<std::endl;
	} // calling socket() creates us a Kernel Socket Structure in memory
	// and returns a simple integer fd as an id to work with the socket structure



	
	struct sockaddr_in address;
	address.sin_family = AF_INET; // ipv4
	address.sin_port = htons(8080); // htons() converts 8080 to network bite order (big endian)
	address.sin_addr.s_addr = INADDR_ANY; // ip address, this tells listen to 0.0.0.0 any interface

	// casting sockaddr_in* to sockaddr* because bind accepts the generic container
	if(bind(fd,(sockaddr*)&address,sizeof(address))<0){
		std::cerr<<"Bind failed!"<<std::endl;
		return 1;
	}
	// this bind call tells the OS to link a specific port with our socket with id fd
	// so any packet arriving at port 8080 should be dumped into the buffer of id fd
	std::cout<<"Bind successful on port 8080"<<std::endl;
	
	// socket is like buying new phone and binding is like getting a phone number
	// so that others can call to our phone

	// Right now if someone tries to connect to my ip at port 8080 the connection will be
	// refused because we have the phone number(port) claimed but we still haven't 
	// turned on the phone, we need to tell os that I am willing to accept incoming 
	// calls(packets), we can do this using listen()

	// the syntax is `int listen(int fd, int backlog)`, here backlog is the number of connections on hold
	// basically The backlog determines how many people can stay on "hold" (ringing) 
	// before the OS starts hanging up on new people.
	// it flips a switch in the socket structure marking it as PASSIVE
	// it can accept incoming connections but can't send outgoing ones

	if(listen(fd,10)<0){
		std::cerr<<"Listen failed"<<std::endl;
		return 1;
	}
	std::cout<<"Server is listnening on Port 8080..."<<std::endl;
	
	while(true){
	std::cout<<"Waiting for a connection..."<<std::endl;

	// preparing to store client info
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	
	// Accept() is a blocking call that stops the code execution, it stops, it sleeps, it 
	// consumes almost 0% cpu. It waits forever until a client (like browser) connects
	// to port 8080, accept() returns a new file descripter, fd 3 job is to wait for 
	// connection we don't send data on fd 3
	// fd4 is a private line created for the specific client, we send/receive data on fd 4

	int client_fd = accept(fd,(sockaddr*)&client_addr,&client_len);

	if(client_fd <0){
		std::cerr<<"Accept failed"<<std::endl;
		return 1;
	}
	
	std::cout<<"Client connected! assigned new FD: "<<client_fd<<std::endl;

	char buffer[3000] = {0}; // buffer to store the message
	// syntax ssize_t read(int fd, void *buf, size_t count);
	// or recv(int fd, void *buf, size_t count, int flags);
	// count is the nmber of bytes we want to read up to
	int valread = read(client_fd, buffer,3000); // valread stores the number of bytes we got
	std::cout<<"--- RECEIVED DATA ("<<valread<<" bytes) ---"<<std::endl;
	std::cout<<buffer<<std::endl;
	std::cout<<"------------------------------------------"<<std::endl;

	// to reply/send response we use send()
	// a valid http response looks like
	/*
	HTTP/1.1 200 OK
	Content-Type: text/plain
	Content-Length: 12

	Hello World!
	*/
	// the empty line is curcial, it tells the browser "Headers are done,Body is starting"
	//  line break in http  is written as \r\n, the double \r\n\r\n represents linespace
	const char* hello = "HTTP/1.1 200 OK\r\n"
			    "Content-Type: text/plain\r\n"
			    "Content-Length: 12\r\n\r\n"
			    "Hello World!";
	// this declaration is for readability at compile time it's converted to
	// "Header: value\r\nAnother: value\r\n..." all literals are concatenated
	
	// syntax of send, ssize_t send(int fd, const void*buf, size_t len, int flags)
	// here flags are special instructions
	// I have written two flags in notes

	send(client_fd, hello, strlen(hello),0);
	std::cout<<"--- Response Sent ---"<<std::endl;
	close(client_fd);a
	}
	close(fd);
	return 0;
}
