#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <cerrno>
#include <sys/epoll.h>



int main(){
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd == -1){
		std::cout<<"Socket creation failed"<<std::endl;
		return -1;
	}
	
	int opt = 1; // Add this to allow restarting immediately
    	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(8080);
	address.sin_addr.s_addr = INADDR_ANY;

	if(bind(server_fd,(sockaddr*)&address,sizeof(address))<0){
		std::cout<<"Binding failed"<<std::endl;
		return -1;
	}
	
	if(listen(server_fd,20)<0){
		std::cout<<"Listening failed"<<std::endl;
		return -1;
	}
	
	std::cout<<"Listening on port 8080"<<std::endl;

		/*
	* THE "SINGLE WAITER" ANALOGY (Why we use Non-Blocking I/O)
	* ---------------------------------------------------------
	* Concept: This server is Single-Threaded. It is like a Restaurant with only ONE Waiter.
	* If the Waiter stops to sleep (Block) for even 1 second, EVERY customer waits.
	*
	* 1. Why Server Socket (The Door) is Non-Blocking:
	* - The Trap: "The Phantom Knock." A client connects but disconnects immediately.
	* - Blocking: The Waiter stands at the open door staring into space, waiting for a
	* replacement guest, while customers at tables (connected clients) starve.
	* - Non-Blocking: The Waiter checks the door. If no one is there, they immediately
	* turn around to serve existing tables.
	*
	* 2. Why Client Socket (The Conversation) is Non-Blocking:
	* - The Trap: "The Slow Talker." A client sends 5 bytes of a 1000-byte message.
	* - Blocking: The Waiter stands at that table, frozen, refusing to move until the
	* client finishes the sentence. The entire restaurant freezes.
	* - Non-Blocking: The Waiter takes the 5 bytes, writes them down, serves other tables,
	* and comes back later for the rest.
 	*/
	
	fcntl(server_fd,F_SETFL, O_NONBLOCK); // making the accept at server_fd as non-blocking

	long long count = 1;
	std::vector<int> client_fds;
	
	int epoll_fd = epoll_create1(0); // 0 means default options
	// this creates an event manager used to keep track of sockets
	// this allocates space in memory to hold a list of sockets to watch
	//epoll_id is the file descriptor used to talk to the manager
	
	//epoll form
	struct epoll_event event;
	event.events = EPOLLIN; // What we are waiting for? Epoll Input, "wake me up when data comes IN
	event.data.fd =  server_fd; // which socket is this event for

	// registrering the form (event)
	if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_fd,&event) == -1){
		std::cout<<"Epoll creation failed"<<std::endl;	// EPOLL_CTL_ADD is for the action which is ADDING new socket here
	}							// to the list, we can use EPOLL_CTL_DEL to delete one later
								// we are adding server_fd socket
	
	std::cout<<"Epoll created"<<std::endl;
	
	struct epoll_event events[10];

	/* 
	Blocking server_fd: Blocking server_fd freezes the server if the server has other work to do while waiting.
	Blocking client_fd: Freezes the server if the current user is slow.
	*/

	while(true){
		int num_events = epoll_wait(epoll_fd,events,10,-1); // this line pauses the program and waits for events consuming 0% cpu
		// it only returns when something happens, -1 is to indicate wait forever until an event occurs
		for(int i = 0; i<num_events; ++i){
			if(events[i].data.fd == server_fd){
						struct sockaddr_in client_address;
						socklen_t client_len = sizeof(client_address);

						int client_fd = accept(server_fd,(sockaddr*)&client_address,&client_len); // this accept is non-blocking 
																								  // if no client connects the control
																								  // goes forward continuously
																								// as the accept immediately returns 
																								// even with no clients

						if(client_fd >= 0){
							fcntl(client_fd,F_SETFL,O_NONBLOCK);

							struct epoll_event client_event;
							client_event.events = EPOLLIN; // watch for the data from this guest
							client_event.data.fd = client_fd;
							if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_fd,&client_event)<0){ // when the control goes out of if block
								std::cout<<"Client fd registration failed"<<std::endl;   // the stack variable client_event is destroyed
							}	// the manager(epoll_fd) makes the copy of this event in the kernel memory and keeps track of the client_fd
							// even thought the event structure is destroyed
							// think of the struct declaration as piece of paper 
							std::cout<<"Client fd registered"<<std::endl;
							

							client_fds.push_back(client_fd);
							std::cout<<"Client connected! "<<client_fd<<" the number of clients is "<<client_fds.size()<<std::endl;

						} else {
							if(errno == EWOULDBLOCK || errno == EAGAIN){
									std::cout<<"Non-Blocking"<<std::endl;
							} else {
								// Something BAD happened!
								perror("Accept Error"); // Prints the exact error string
								return 0;
							}
						}
			}
		}
	}
	return 0;
}
