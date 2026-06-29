#include <iostream>
#include <conio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <string.h>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

struct chatpack {
	int targetClientID;
	int senderClientID;
	int payloadLength;
	char rawtext[500];
};

int myClientID;

int key[3] = { 3,5,4 };

DWORD WINAPI WorkerFunc(LPVOID lpParam) {
	
	// This secondary thread is to actively listen to the server for incoming messages

	SOCKET* serverRec = (SOCKET*)lpParam;
	chatpack serverpackrec;
	SOCKET connection = *serverRec; // dereference it so that we don't need pointers
	while (true) {
		int byteCount = recv(connection, (char*)&serverpackrec, sizeof(serverpackrec), 0);
		if (byteCount > 0) {
			
			char decrypt[500];
			strcpy_s(decrypt, serverpackrec.rawtext);
			int x = strlen(decrypt);
			char XOR_decrypt[500] = { 0 };
			for (int k = 0; k < x; k++) {
				char place_holder = decrypt[k];
				XOR_decrypt[k] = place_holder ^ key[k % 3];
			}
			XOR_decrypt[x] = '\0';
			cout << "[Client " << serverpackrec.senderClientID << " ]: " << XOR_decrypt << endl;
		}
		else if (byteCount<=0 || byteCount == SOCKET_ERROR) {
			cout << "Connection to Server Lost" << endl;
			closesocket(connection);
			break;
		}
	}

	return 0;

}


int main() {
	WSADATA OSDATA;
	int wsaCheck;
	WORD wVersionRequested = MAKEWORD(2, 2);
	wsaCheck = WSAStartup(wVersionRequested, &OSDATA);
	if (wsaCheck != 0) {
		cout << "Winsock DLL not found" << endl;
		return -1;
	}

	SOCKET clientSocket = INVALID_SOCKET;
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (clientSocket == INVALID_SOCKET) {
		cout << "Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}
	else {
		cout << "Client Socket created successfully!" << endl;
	}

	int port = 55000;
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_port = htons(port);
	InetPton(AF_INET, _T("127.0.0.1"), &clientService.sin_addr.s_addr);

	if (connect(clientSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {

		cout << "Connection to Server Failed, Try Again!" << endl;
		closesocket(clientSocket); // Cus socket created in step 2
		WSACleanup();
		return -1;

	}
	else {
		cout << "Connected to the Server successfully" << endl;
		int clientRec = recv(clientSocket, (char*)&myClientID, sizeof(myClientID), 0);
		if (clientRec > 0) {
			cout << "Your Client ID is: " << myClientID << endl;
			HANDLE hdl = CreateThread(NULL, 0, WorkerFunc, (LPVOID)&clientSocket, 0, NULL);
		}
		while (true) {
			// This loop (main) is for sending data to the server, infinitely until the connection is open
			chatpack ClientDataSend;
			char RawData[500];
			int targetID;
			cout << "Enter the client ID you want to message to (ID>0): ";
			cin >> targetID;
			cin.ignore(); // To prevent any spaces or enter
			
			while (true) {
				cout << "Enter your message(/exit to choose another client): ";
				cin.getline(RawData, 500);

				if (!strcmp(RawData, "/exit")) {
					break;
				}

				int x = strlen(RawData);

				char XOR_encrypt[500] = { 0 };
				for (int k = 0; k < x; k++) {
					char place_holder = RawData[k];
					XOR_encrypt[k] = place_holder ^ key[k % 3];
				}
				XOR_encrypt[x] = '\0';

				ClientDataSend.targetClientID = targetID;
				strcpy_s(ClientDataSend.rawtext, XOR_encrypt);
				int byteCount = send(clientSocket, (char*)&ClientDataSend, sizeof(ClientDataSend), 0);
			}
		}
		system("pause");
	}

}