#include "day21_Server.h"
#include<iostream>
#include<sys/socket.h> // for socket()
#include<netinet/in.h> // for AF_NET
#include<unistd.h> //for close()
#include<cstring> // for strlen()
#include<fstream> // for ifstream
#include<sstream> // for stringstream
#include<filesystem> // for std::filesystem::exists
#include<unordered_map>
#include<thread> // to accept multiple clients
#include<mutex> // to use mutexes


namespace fs = std::filesystem;

//Constructor

Server::Server(int port): port(port),server_fd(-1){
}

std::string Server::get_mime_type(std::string extension){
        //static const makes the compiler creat this map only once, and lives throughout the end
        // this improves performance
        static const std::unordered_map<std::string,std::string> mp={
                {".html", "text/html"},
                {".css", "text/css"},
                {".js", "text/javascript"}, // Standard is usually "application/javascript" but text works
                {".png", "image/png"},
                {".jpg", "image/jpeg"}
        };

        if(mp.count(extension)) return mp.at(extension);
        // cannot return mp[extension] because operator[] is never const
        // since [] has power to modify the map it can never be const, that's why use .at();
        return "application/octet-stream"; // I don't know what this is, jus download as binary
}

void Server::log(const std::string& message){
        const std::lock_guard<std::mutex> lock(print_mtx);
        std::cout << message << std::endl;
}
void Server::handle_client(int client_fd){

        char request[4096] = {0};
        ssize_t bytes_read = read(client_fd,request,4096);
        if(bytes_read<=0){
                std::cerr<<"Client diconnected or read error"<< std::endl;
                close(client_fd);
                return;
        }

        std::string req(request,bytes_read);

        std::stringstream ss(req);
        std::string method, path;
        ss >> method >> path; // automatically seperated by space
        log("DEBUG: method=" + method + " Path="+path);
        std::string response;


        if(method == "POST" && path == "/submit"){
                size_t pos = req.find("\r\n\r\n");
                std::string body = "";
                if(pos != std::string::npos){
                        body = req.substr(pos+4);
                }

                log("RECEIVED POST DATA: "+body);
                {
                        std::lock_guard<std::mutex> lock(file_mtx);
                        std::ofstream file("guestbook.txt",std::ios::app);
                        if(!file) log("Writing failed");
                        file << body <<"\n";
                }
                std::string response_body = "<h1>Got it!</h1><p>Data: "+body+"</p>";
                response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: "+std::to_string(response_body.length())+"\r\n\r\n"
                           + response_body;

                send(client_fd,response.c_str(),response.length(),0);
                close(client_fd);
                return;
        }

        path = path.substr(1);

        if(fs::exists(path)){
                std::string mime_type = get_mime_type(fs::path(path).extension().string());
                std::ifstream file("./"+path, std::ios::binary);
                if(!file){
                        std::cerr<<"Could not open the file"<<std::endl;
                }
                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string body = buffer.str();

                response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: "+mime_type+"\r\n"
                           + "Content-Length: "+std::to_string(body.length())+"\r\n\r\n"
                           + body;
        }
        else if(path ==""){
                std::ifstream file("index.html");
                if(!file){
                        std::cerr<<"Could not find the page"<<std::endl;
                }
                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string body = buffer.str();

                response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: "+std::to_string(body.length())+"\r\n\r\n"
                           + body;
        }
         else{
                std::string content = "<h1>Could not find the page</h1>";
                response = "HTTP/1.1 404 Not Found\r\n"
                           "Content-Length: "+std::to_string(content.length())+"\r\n\r\n"
                           + content;
        }
        // .c_str() and .data() return same pointer
        send(client_fd, response.c_str(), response.length(),0);
        close(client_fd);

}


void Server::run(){
    // FIX 1: Use the class member 'server_fd', not a local 'int fd'
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1){
        log("Socket failed");
        return; // FIX 2: Return nothing (void)
    } else {
        log("Socket created");
    }

    int opt = 1;
    // FIX 1: Use 'server_fd' here
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        std::cerr << "setsockopt failed" << std::endl;
        return; // FIX 2
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port); // FIX 3: Use the class member 'port'
    address.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0){
        std::cerr << "Bind failed!" << std::endl;
        return; // FIX 2
    }
    log("Bind successful on port " + std::to_string(port));

    if(listen(server_fd, 10) < 0){
        std::cerr << "Listen failed" << std::endl;
        return; // FIX 2
    }
    log("Server is listening...");

    while(true){
        log("Waiting for a connection...");

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // FIX 1: Use 'server_fd'
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);

        if(client_fd < 0){
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        log("Client connected! FD: " + std::to_string(client_fd));

        // FIX 4: Correct Thread Syntax for Member Functions
        // We pass '&Server::handle_client' and 'this'
        std::thread t(&Server::handle_client, this, client_fd);
        t.detach();
    }
    // No need to close server_fd here as while(true) never ends, 
    // but the Destructor (if you write one) would handle it.
}
