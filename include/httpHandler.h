#pragma once

#include <string> 

bool sendAll(int socket, const std::string& data);               
void handleClient(int connAccept); 
