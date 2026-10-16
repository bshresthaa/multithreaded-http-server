
#include "threadPool.h"

using namespace std; 

int main() { 
    //Creating a socket
    int sockAddr = serverSocket(); 
    if (sockAddr == -1) {
        return 1;
    }

    //binding the port
    int port = 8080;
    if (bindPort(port, sockAddr) == -1) {
        close(sockAddr);
        return 1;
    } 

    //Open listener
    int queueSize = 5; // Maximum number of pending connections
    if (openListener(sockAddr, queueSize, port) == -1) {
    close(sockAddr);
    return 1;
    }

    cout<<"Waiting for incoming connections..."<<endl;

    ThreadPool pool(5); 
    while(true) {
        //Acceopting connection
        sockaddr_in address{};    
        //Clieent connection descriptor
        int connAccept = acceptConnection(sockAddr, address);
        if (connAccept == -1) {
            continue;
        }
        pool.enqueueTask(connAccept); // Enqueue the accepted connection for processing
    }; 

    close(sockAddr); 
    return 0; 
}

