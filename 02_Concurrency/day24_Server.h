#ifndef SERVER_H
#define SERVER_H

#include<string>
#include<netinet/in.h> // for sockaddr_in
#include<mutex>
#include "day24_ThreadPool.h"

class Server{
private:
        int port;
        int server_fd;
        std::mutex print_mtx;
        std::mutex file_mtx;

        std::string get_mime_type(std::string extension);
        void log(const std::string& message);
        void handle_client(int client_fd);
	ThreadPool threadPool;
public:
        Server(int port);
        void run();
};

#endif
