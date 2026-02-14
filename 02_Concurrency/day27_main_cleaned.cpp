#include <iostream>
#include "day27_Server.h"
#include <csignal> // required for signal()

Server* server_ptr = nullptr;

void signal_handler(int signal){
	if(signal == SIGINT && server_ptr){
		server_ptr -> stop();
	}
}

int main(){
	std::signal(SIGINT, signal_handler);
	
	Server server(8080);
	server_ptr = &server;
	server.run();
	std::cout<<"Main finished. Destructors running now..." <<std::endl;
	return 0;
}
