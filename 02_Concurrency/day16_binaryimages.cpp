#include<iostream>
#include<sys/socket.h> // for socket()
#include<netinet/in.h> // for AF_NET
#include<unistd.h> //for close()
#include<cstring> // for strlen()
#include<fstream> // for ifstream
#include<sstream> // for stringstream

int main(){

        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if(fd == -1){
                std::cout<<"Socket failed"<<std::endl;
                return 1;
        } else{
                std::cout<<"Socket created "<<fd<<std::endl;
        } // calling socket() creates us a Kernel Socket Structure in memory
        // and returns a simple integer fd as an id to work with the socket structure


        // AFTER  closing the server with CTRL+C the port goes for cool down period
        // for 60 seconds, to avoid that we tell the OS that we know what we are doing
        // let us reuse the port immediately even if it is in timeout

        int opt = 1;
        if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))<0){
                std::cerr<<"setsockopt failed"<<std::endl;
                return 1;
        }


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

        char request[1024] = {0};
        read(client_fd,request,1024);
        std::stringstream ss(request);
        std::string method, path;
        ss >> method >> path; // automatically seperated by space
        std::cout << "DEBUG: method=" << method << " Path="<<path;

        std::string response;

        if(path =="/" || path == "/home"){
                std::ifstream file("index.html");
                if(!file){
                        std::cerr<<"Could not open the file"<<std::endl;
                }
                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string html_body = buffer.str();

                response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: " +std::to_string(html_body.length()) +"\r\n\r\n"
                           + html_body;
        }
	else if(path == "/images.png"){
		std::ifstream file("images.png", std::ios::binary);
		if(file){
			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string body = buffer.str();
			
			response = 	"HTTP/1.1 200 OK\r\n"
					"Content-Type: image/png\r\n"
					"Content-Length: "+std::to_string(body.length())+"\r\n\r\n"
					+ body;
		}
		else{
			std::string content = "Could not open image";
			response = 	"HTTP/1.1 404 Not Found\r\n"
					"Content-Length: "+std::to_string(content.length()) +"\r\n\r\n"
					+ content;
		}
	}	
        else if(path == "/data"){
                std::string json_content = "{\"id\": 1, \"status\": \"active\"}";
                response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: "+std::to_string(json_content.length()) + "\r\n\r\n"
                           + json_content;
        }

        else{
                std::string error_content = "<h1>404 Not Found</h1>";
                response = "HTTP/1.1 404 Not Found\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: "+std::to_string(error_content.length())+"\r\n\r\n"
                           + error_content;
        }

        // .c_str() and .data() return same pointer
        send(client_fd, response.c_str(), response.length(),0);
        close(client_fd);
        }
        close(fd);
        return 0;
}
