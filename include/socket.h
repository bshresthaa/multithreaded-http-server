#pragma once

class Socket { 

    private: 
        int fd; 

    public: 
        explicit Socket(int socket_Fd); 
        ~Socket(); 

        //Prevent copying and assignment
        Socket(const Socket&) = delete; 
        Socket& operator=(const Socket&) = delete;  //assignmene operator overloading

        //Move constructor allows transfer of ownership of the socket resource
        Socket (Socket&& other) noexcept;

        Socket& operator=(Socket&& other) noexcept;  

        int get() const noexcept;
}; 