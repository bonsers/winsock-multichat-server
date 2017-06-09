#include <iostream>
#include <WinSock2.h>
#include <thread>
#include <string>

#pragma comment(lib,"ws2_32.lib")

SOCKET CONNECTIONS[100];
int CONNECTIONCOUNT = 0;
std::thread t[100];

void clientHandlerThread(int INDEX);

int main(int argc, char *argv[]) {

	WSAData WSADATA;
	WORD DLLVERSION = MAKEWORD(2, 1);

	if (WSAStartup(DLLVERSION, &WSADATA) != 0) {
		std::cout << "SERVER::ERROR::WSAStartup failed\n";
		WSACleanup();
	}
	else {
		std::cout << "SERVER::WSAStartup successful\n";
	}

	SOCKADDR_IN ADDR;
	
	ADDR.sin_addr.s_addr = inet_addr("127.0.0.1");
	ADDR.sin_port = htons(8899);
	ADDR.sin_family = AF_INET;

	int ADDRLEN = sizeof(ADDR);
	SOCKET LISTENINGSOCKET = socket(AF_INET, SOCK_STREAM, NULL);
	bind(LISTENINGSOCKET, (SOCKADDR*)&ADDR, ADDRLEN);
	listen(LISTENINGSOCKET, SOMAXCONN);

	std::cout << "SERVER::Server listening\n";

	SOCKET NEWCLIENTCONNECTION;

	for (int i = 0; i < 100; i++) {
		NEWCLIENTCONNECTION = accept(LISTENINGSOCKET, (SOCKADDR*)&ADDR, &ADDRLEN);
		if (NEWCLIENTCONNECTION == 0) {
			std::cout << "SERVER::ERROR::Failed to accept the client's connection\n";
			WSACleanup();
		}
		else {
			std::cout << "~~ " << NEWCLIENTCONNECTION << " Client connected\n";
			std::string MESSAGE = "~~ Welcome to the chat server!";
			int MESSAGELEN = MESSAGE.size();
			send(NEWCLIENTCONNECTION, (char*)&MESSAGELEN, sizeof(int), NULL);
			send(NEWCLIENTCONNECTION, MESSAGE.c_str(), MESSAGELEN, NULL); // Sending welcome message to all clients
			CONNECTIONS[i] = NEWCLIENTCONNECTION;
			CONNECTIONCOUNT++;
			t[i] = std::thread(clientHandlerThread, i);
			t[i].detach();
		}
	}

	return 0;
}


void clientHandlerThread(int INDEX) {
	int BUFFERLENGTH;
	int RETVAL;
	while (1) {
		RETVAL = recv(CONNECTIONS[INDEX], (char*)&BUFFERLENGTH, sizeof(int), NULL);
		//std::cout << RETVAL << std::endl;
		if (RETVAL == SOCKET_ERROR) {
			std::cout << "~~ " << CONNECTIONS[INDEX] << " Client disconnected" << std::endl;
			closesocket(CONNECTIONS[INDEX]);
			break;
		}
		char* BUFFER = new char[BUFFERLENGTH + 1]; // + 1 for the null terminator
		BUFFER[BUFFERLENGTH] = '\0'; // set the last element to null terminator
		RETVAL = recv(CONNECTIONS[INDEX], BUFFER, BUFFERLENGTH, NULL);
		if (RETVAL == SOCKET_ERROR) {
			std::cout << "~~ " << CONNECTIONS[INDEX] << " Client disconnected" << std::endl;
			closesocket(CONNECTIONS[INDEX]);
			break;
		}
		std::string NEWBUFFER(BUFFER);
		int temp = CONNECTIONS[INDEX];
		NEWBUFFER = std::to_string(temp) + ": " + NEWBUFFER;
		std::cout << CONNECTIONS[INDEX] << ": " << BUFFER << std::endl;
		BUFFERLENGTH = NEWBUFFER.length();
		for (int i = 0; i < CONNECTIONCOUNT; i++) {
			if (i == INDEX)
				continue;
			send(CONNECTIONS[i], (char*)&BUFFERLENGTH, sizeof(int), NULL);
			send(CONNECTIONS[i], NEWBUFFER.c_str(), BUFFERLENGTH, NULL);
		}
		delete[] BUFFER;
	}
}