#ifndef SERVER_H
#define SERVER_H

#include<string>
#include<netinet/in.h> // for sockaddr_in
#include<mutex>
#include "day27_ThreadPool.h"
#include <atomic> // Added on day 27 for cleanup

// on day 27 we add the functionality that when we press ctrl + c the server
//does not directly close rather we first finish all the existing thread and then run 
// threadpool destructor, server destructor

class Server{
private:
        int port;
        int server_fd;
        std::mutex print_mtx;
        std::mutex file_mtx;

	std::atomic<bool> should_exit; // Added


        std::string get_mime_type(std::string extension);
        void log(const std::string& message);
        void handle_client(int client_fd);
	ThreadPool threadPool;
public:
        Server(int port);
	~Server();
        void run();
	void stop();
};

#endif
