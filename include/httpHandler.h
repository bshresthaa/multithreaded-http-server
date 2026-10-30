#pragma once

#include <string> 
#include "socket.h"

bool sendAll(int socket, const std::string& data);               
void handleClient(Socket& client); 
