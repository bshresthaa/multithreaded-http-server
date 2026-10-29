
#include "serverHeader.h"

#include <iostream>
#include <queue>
#include <string>
#include <sstream>
#include <sys/socket.h>


int serverSocket() { 
    //creating the socket : 
    int serverSocket =  socket( PF_INET, SOCK_STREAM, 0 ); 
    if( serverSocket == -1 ){ 
        perror("Socket creation failed."); 
        return -1; 
    }
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
                &opt, sizeof(opt)) == -1) {
        perror("setsockopt SO_REUSEADDR failed");
        close(serverSocket);
        return -1;
    }
    std::cout<<"Socket Created"<<std::endl;
    return serverSocket;
}

int bindPort(int port, int serverSocket) { 
    struct sockaddr_in address{}; // Need to learn more about this struct.
    address.sin_family = PF_INET; 
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    if(::bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) == -1) { 
        perror("Binding failed.");
        return -1; 
    }else { 
        std::cout<<"Binding successful."<<std::endl;
        return 0;  
    }; 
}

int openListener(int serverSocket, int queueSize, int port) { 
    if (listen(serverSocket, queueSize) == -1) {
        perror("Listening failed");
        return -1;
    }

    std::cout << "Listening on port: " << port << std::endl;
    return 0;
}

int acceptConnection(int serverSocket, struct sockaddr_in& address) { 
    socklen_t addrlen = sizeof(address);
    int connAccept = accept( serverSocket, (struct sockaddr*)&address, &addrlen);
    if(connAccept   == -1) { 
        if (errno != EINTR) {
            perror("accept failed");
        }
        return -1;
    }
    std::cout<<"Connection accepted.\n"<<std::endl;
    return connAccept;
}

ssize_t readFromSocket(int connAccept, char* buffer, size_t bufferSize) { 
    ssize_t bytesRead = read(connAccept, buffer, bufferSize);
    if(bytesRead == -1) { 
        perror("Reading from socket failed.");
        return -1;
    }
    
    if (bytesRead == 0) {
        std::cout << "Client disconnected." << std::endl;
        return 0;
    }

    std::cout<<"Received: "<< bytesRead << " bytes."<< std::endl;
    return static_cast<int>(bytesRead);

}

//Might not even need this function. 
std::queue<std::string> getPath(std::string path) { 
    std::queue<std::string>slicePath; 
    std::string placeHolder; 
    // while(string::npos != path.find("/")) { 
    //     ssize_t start = path.find("/");  
    //     ssize_t end = path.find("/", start+1); 
    //     slicePath.push(path.substr(start+1, end-start-1));
    //     path.erase(start, end-start-1);
    // }; 
    // return slicePath; 
    std::stringstream pathStream(path); 
    while(std::getline(pathStream, placeHolder, '/' )) { 
        slicePath.push(placeHolder); 
    }
    return slicePath;
} 