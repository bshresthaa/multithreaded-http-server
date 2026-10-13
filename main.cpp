
#include "threadPool.h"

#include <fstream>
#include <iterator>
#include <thread>

using namespace std; 

int main() { 
    //Creating a socket
    int sockAddr = serverSocket(); 

    //binding the port
    int port = 8080;
    int portBind = bindPort(port, sockAddr);  

    //Open listener
    int queueSize = 5; // Maximum number of pending connections
    int listener = openListener(sockAddr, queueSize, port);
    cout<<"Waiting for incoming connections..."<<endl;

    int i = 0; 
    ThreadPool pool(4); 
    while(true) {
        //Acceopting connection
        sockaddr_in address;    
        socklen_t addrlen = sizeof(address);

        //Clieent connection descriptor
        int connAccept = acceptConnection(sockAddr, address);
        pool.enqueueTask(connAccept); // Enqueue the accepted connection for processing
    }; 

    close(sockAddr); 
    return 0; 
}
