
#include "threadPool.h"

using namespace std; 

#include <cerrno>
#include <cstring>
#include <csignal>
#include <atomic>

volatile std::sig_atomic_t shutdownRequested = 0;

//hanlde shutdown signal
void handleSignal(int signal) {
    if (signal == SIGINT) {
        shutdownRequested = 1;
    }
}

int main() { 

    struct sigaction sa{};
    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        perror("sigaction failed");
        return 1;
    }

    std::cout << "SIGINT handler registered. Press Control+C to stop." << std::endl;

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
    while(!shutdownRequested) {
        //Acceopting connection
        sockaddr_in address{};    
        //Clieent connection descriptor
        int connAccept = acceptConnection(sockAddr, address);
        if (connAccept == -1) {
            if(errno == EINTR){  // Look at the error type and only retry if it is EINTR, which means the accept call was interrupted by a signal.
                if(shutdownRequested) {
                    cout << "Shutdown requested. Exiting accept loop." << endl;
                    break; // Exit the loop if shutdown is requested
                }
                continue;
            }else { 
                perror("Accepting connection failed.");
                break; 
            }
        }
        if(!pool.enqueueTask(connAccept)){
            close(connAccept);
        } // Enqueue the accepted connection for processing
    }; 

    close(sockAddr); 
    return 0; 
}

