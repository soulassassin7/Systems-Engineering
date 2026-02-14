#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

int main(){
	int server_fd = socket(AF_INET,SOCK_STREAM,0);
	if(server_fd == -1){
		std::cout<<"Socket failed"<<std::endl;
	}
	
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(8080);
	address.sin_addr.s_addr = INADDR_ANY;

	int opt = 1; // Add this to allow restarting immediately
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if(bind(server_fd,(sockaddr*)&address,sizeof(address))<0){
		std::cout<<"Binding failed"<<std::endl;
		return -1;
	}
	
	if(listen(server_fd,20)<0){
		std::cout<<"Listening failed"<<std::endl;
		return -1;
	}
	std::cout<<"Server listening on port 8080"<<std::endl;
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
	// In single threaded server if accept is blocking then the server won't be able to handle other clients
	/* 
	Blocking server_fd: Blocking server_fd freezes the server if the server has other work to do while waiting, like handling other clients.
	Blocking client_fd: Freezes the server if the current user is slow.
	*/

	fcntl(server_fd,F_SETFL,O_NONBLOCK); // this makes all the server_fd non - blocking so all types of blocking functions
	// acting on this fd will become non blocking helping us move forward and the while loop next iteration can happen
	long long count = 1;
	std::vector<int> client_fds;
	while(true){
		struct sockaddr_in client_address;
		socklen_t client_len = sizeof(client_address);

		int client_fd = accept(server_fd,(sockaddr*)&client_address,&client_len); // without non-blocking, the control freezes here
																				  // until a client connects, with nonblock 
																				  // the control moves foward and next iteration of 
																				  // while loop can continue

        if(client_fd >= 0){
            fcntl(client_fd,F_SETFL,O_NONBLOCK); // we set client_fd as non blocking as well so that reading from it will not
			// freeze the while loop
			// consider a client connects and tries to send something we read from it
			// for some reason the client is not able to send any data due to slow network or intentionally
			// that read will stop the while loop because of blocking nature of client_fd by default
			//causing the whole server to stop and we won't be able to accept any other clients until the client
			//holding has completed sending the data, this freezes our server, to avoid that we make the client_fd non-blocking

            client_fds.push_back(client_fd);
			std::cout<<"Client connected! "<<client_fd<<" the number of clients is "<<client_fds.size()<<std::endl;

        } else {
            if(errno == EWOULDBLOCK || errno == EAGAIN){
                if(count%200000 == 0){
					std::cout<<"Non-Blocking"<<std::endl;
				}
            } else {
                // Something BAD happened!
                perror("Accept Error"); // Prints the exact error string
				return 0;
            }
		}
		count += 1;
	}
	return 0;
}
