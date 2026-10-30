#include "socket.h"

#include  <unistd.h>

Socket::Socket(int socketFd): fd(socketFd) {}

Socket::~Socket() { 
    if(fd != -1) {
        close(fd); 
    }
}

Socket::Socket(Socket&& other) noexcept : fd(other.fd) { 
    other.fd = -1; 
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if(this != &other) { 
        if(fd != -1) { 
            close(fd); 
        }
        fd = other.fd; 
        other.fd = -1; 
    }
    return *this; 
}

int Socket::get() const noexcept {
    return fd;
}