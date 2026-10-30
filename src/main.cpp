
#include "threadPool.h"
#include "serverHeader.h"

#include <cerrno>
#include <csignal>
#include <utility>


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

    Socket listeningSocket(sockAddr);

    //binding the port
    int port = 8080;
    if (bindPort(port, listeningSocket.get()) == -1) {
        return 1;
    } 

    //Open listener
    int queueSize = 5; // Maximum number of pending connections
    if (openListener(listeningSocket.get(), queueSize, port) == -1 ) {
        return 1;
    }

    std::cout<<"Waiting for incoming connections..."<<std::endl;

    ThreadPool pool(5); 
    while(!shutdownRequested) {
        //Acceopting connection
        sockaddr_in address{};    
        //Clieent connection descriptor
        int connAccept = acceptConnection(listeningSocket.get(), address);
        if (connAccept == -1) {
            if(errno == EINTR){  // Look at the error type and only retry if it is EINTR, which means the accept call was interrupted by a signal.
                if(shutdownRequested) {
                    std::cout << "Shutdown requested. Exiting accept loop." << std::endl;
                    break; // Exit the loop if shutdown is requested
                }
                continue;
            }else { 
                perror("Accepting connection failed.");
                break; 
            }
        }
        Socket client(connAccept);
        if(!pool.enqueueTask(std::move(client))) {
            std::cerr << "Failed to enqueue client socket." << std::endl;
        } // Enqueue the accepted connection for processing
    }; 

    return 0; 
}

