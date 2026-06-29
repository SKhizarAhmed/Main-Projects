#include <iostream>
#include <conio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

struct chatpack {
	int targetClientID;
	int senderClientID;
	int payloadLength;
	char rawtext[500];
};

struct ThreadParam {
	SOCKET clientSocket; // Server needs to know the handle of the client it is talking too
	int clientID; // It also needs its id to know which client to talk too
};

SOCKET clientArray[10];

HANDLE mutex = CreateMutex(NULL, FALSE, NULL);

DWORD WINAPI ThreadFunc(LPVOID lpParam) {
	
	ThreadParam* obj = (ThreadParam*)lpParam;
	SOCKET connection = obj->clientSocket;
	int id = obj->clientID;
	delete obj; // To delete the space occupied by the obj in heap since we assigned the things to separate variables.
	chatpack packet;

	while (true) {
		// here we use acceptSocket because it is a separate socket that connects server to the client 
		int byteCount = recv(connection, (char*)&packet, sizeof(chatpack), 0);
		if (byteCount > 0) {
			//packet arrived succesfully
			
			

			packet.senderClientID = id + 1;
			cout << "Client " << id + 1 << " is forwarding a packet to Client " << packet.targetClientID << endl;
			int target = packet.targetClientID - 1;

			SOCKET targetSocket = INVALID_SOCKET;

			if (target >= 0 && target < 10) {
				WaitForSingleObject(mutex, INFINITE);
				targetSocket = clientArray[target];
				ReleaseMutex(mutex);
				int sendByteCount = send(targetSocket, (char*)&packet, sizeof(chatpack), 0);
			}

			


		}
		if (byteCount == SOCKET_ERROR || byteCount == 0) {
			
			// if program(client) closed gracefully / abruptly

			WaitForSingleObject(mutex, INFINITE);
			clientArray[id] = INVALID_SOCKET;
			ReleaseMutex(mutex);
			closesocket(connection);
			break;
		}
	}
	return 0;
}



int main() {
	
	for (int k = 0;k < 10;k++) {
		clientArray[k] = INVALID_SOCKET; // Setting all of the elements of the array to this value so that
										// the server knows every index is an empty space
	}

	WSADATA OSDATA;
	int wsaCheck;
	WORD wVersionRequested = MAKEWORD(2, 2);
	wsaCheck = WSAStartup(wVersionRequested, &OSDATA);
	if (wsaCheck != 0) {
		cout << "Winsock DLL not found" << endl;
		return -1;
	}

	SOCKET serverSocket = INVALID_SOCKET;
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (serverSocket == INVALID_SOCKET) {
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup(); 
		return -1;
	}
	else {
		cout << "Server Socket created successfully!" << endl;
	}

	sockaddr_in service;

	int port = 55000;

	service.sin_family = AF_INET;
	// InetPton(AF_INET, INADDR_ANY, &service.sin_addr.s_addr); can't use this since it converts text string into 
	// network bytes instead we use htonl which is used to convert for 32 bit integers instead of standard htons
	// for 16 bits.
	/*
	
	InetPton does the translation job: it reads the text character string,
	strips out the human-friendly dots, and parses it down into machine bytes first.
	
	htonl skips the translation step because the data is already machine bytes.
	It just applies the formatting rule directly to the raw integer.
	
	INADDR_ANY - means take data from anywhere either wifi, ethernet or local host no restriction
	represented by 0.0.0.0 or simply 0


	*/
	service.sin_addr.s_addr = htonl(INADDR_ANY);
	service.sin_port = htons(port);

	// port is like an apartment no / room no when data arrives at our hotel(computer) the apartment no. tells which
	// app requested it, so the os knows on which port to route the data, it helps to distinct data meant for specific apps.

	if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {


		cout << "bind() failed: " << WSAGetLastError() << endl;
		closesocket(serverSocket); 
		WSACleanup();
		return -1;
	}

	if (listen(serverSocket, 10) == SOCKET_ERROR) { // 10 is here in case all the 10 users want to join at once
													// or else the server would give some random errors
		cout << "listen(): Error listening on socket " << WSAGetLastError() << endl;
		closesocket(serverSocket); 
		WSACleanup();
		return -1;
	}
	
	while (true) {
		SOCKET acceptSocket;
		acceptSocket = accept(serverSocket, NULL, NULL);

		WaitForSingleObject(mutex, INFINITE);

		int a = 0;

		for (int i = 0; i < 10; i++) {
			if (clientArray[i] == INVALID_SOCKET) {
				clientArray[i] = acceptSocket;
				
				a = 1;
				int clientIDsend = i + 1;
				cout << "Client connected successfully! Assigned ID: " << i + 1 << endl;

				ThreadParam* para = new ThreadParam();
				para->clientSocket = acceptSocket;
				para->clientID = i;
				send(acceptSocket, (char*)&clientIDsend, sizeof(clientIDsend), 0);
				CreateThread(NULL, 0, ThreadFunc, (LPVOID)para, 0, NULL);
				break; // This means that now a separate client connection thread is established so now exit the loop
			}
		}

		ReleaseMutex(mutex);

		if (!a) {
			cout << "Server is Full!, client rejected!!!" << endl;
			closesocket(acceptSocket); // so that they don't want infinitely
		}
	}

}