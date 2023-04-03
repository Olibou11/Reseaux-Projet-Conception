#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Importations initiales

#include <iostream>
#include <string>
#include <ws2tcpip.h>
#include <fstream>
#include <ostream>

#pragma comment(lib, "ws2_32.lib")

// Namespace
using namespace std;

// Méthode de vérification des bytesRecv
bool bytesVerification(int bytesReceveid){

	if (bytesReceveid <= 0) {
		cout << "Une erreur de réception s'est produite" << endl;
		return false;
	}
	return true;
}

// Main
int main() {

	// String pré-enregistrés

	const string connectionMsg = "<CONNECTION> ";
	const string fileMsg = "<FILE> ";
	const string errorMsg = "<ERROR> ";
	const string clientMsg = "<CLIENT> ";

	// Adresse du serveur local
	string ipAddress = "127.0.0.1";

	// Initialisation de WinShock

	WSAData wsaData;

	int wsaStartup = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (wsaStartup != 0) {
		cout << errorMsg << "Impossible d'initialiser WinSock!" << endl;
		WSACleanup();
		return 0;
	}

	// Initialisation du Socket

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (clientSocket == INVALID_SOCKET) {
		cout << errorMsg << "Impossible de créer le socket!" << endl;
		WSACleanup();
		return 0;
	}

	// Lier un ip adresse et un port au socket

	sockaddr_in saClient;

	saClient.sin_family = AF_INET;								// Adressage de la famille
	inet_pton(AF_INET, ipAddress.c_str(), &saClient.sin_addr);	// Adresse
	saClient.sin_port = htons(27015);							// Port

	// Connection au serveur

	int connResult = connect(clientSocket, (sockaddr*)&saClient, sizeof(saClient));

	if (connResult == SOCKET_ERROR) {
		cout << errorMsg << "Impossible de se connecter au serveur! Erreur #" << WSAGetLastError() << endl;
		closesocket(clientSocket);
		WSACleanup();
		return 0;
	}


	// --- Boucle pour recevoir et envoyer des messages/données --- //


	// Variables

	char buf[4096];
	int bytesReceived = 0;
	string msgReceived = "";

	string userInput = "";

	long fileSize = 0;
	string fileName = "";
	long fileDownloaded = 0;

	const string path = "../Out/";

	// Réception du message de confirmation de connexion
	
	ZeroMemory(buf, 4096);
	bytesReceived = recv(clientSocket, buf, 4096, 0);

	if (bytesVerification(bytesReceived)) {

		cout << string(buf, 0, bytesReceived) << endl;

		// Échange commande / Txt

		while (true) {

			cout << clientMsg << "Veuillez entrer une commande :" << endl;
			cin >> userInput; // TODO : Faire une vérification > 0

			ZeroMemory(buf, 4096);
			send(clientSocket, userInput.c_str(), (int)userInput.size() + 1, 0);

			//recv()


		}
	}

	// Close

	closesocket(clientSocket);
	WSACleanup();
}