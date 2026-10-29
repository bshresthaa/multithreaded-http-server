#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstddef>

int serverSocket(); 

int bindPort(int port, int serverSocket); 

int openListener(int serverSocket, int queueSize, int port); 

int acceptConnection(int serverSocket, sockaddr_in& address); 

ssize_t readFromSocket(int connAccept, char* buffer, std::size_t bufferSize); 
