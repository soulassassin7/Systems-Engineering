#include "day21_Server.h"
#include <iostream>

int main() {
    std::cout << "Starting Server on Port 8080..." << std::endl;
    
    // Create the instance
    Server myServer(8080);
    
    // Start the engine
    myServer.run();

    return 0;
}
