#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Importations initiales      hdhdhd

#include <iostream>
#include <string>
#include <ws2tcpip.h>
#include <fstream>
#include <ostream>

#pragma comment(lib, "ws2_32.lib")

// Namespace
using namespace std;

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

	string userInputAdmin = "";
	string userInputPW = "";
	string inputConnection = "";

	string userInput = "";

	long fileSize = 0;
	string fileName = "";
	long fileDownloaded = 0;

	const string path = "../Out/";

	// Réception du Message de bienvenu et connection (On sait qu'ils arrivent qu'une fois, donc pas besoin de les mettre dans une boucle)

	ZeroMemory(buf, 4096);
	bytesReceived = recv(clientSocket, buf, 4096, 0);

	if (bytesReceived > 0) {

		cout << string(buf, 0, bytesReceived) << endl;

		// Envoie du login avec le mot de passe au serveur

		cout << connectionMsg << "Veuillez vous identifier : " << endl;
		cout << "Admin \n> ";
		getline(cin, userInputAdmin);

		cout << "Mot de passe \n> ";
		getline(cin, userInputPW);

		inputConnection = connectionMsg + userInputAdmin + " " + userInputPW;

		if (userInputAdmin.size() > 0 && userInputPW.size() > 0)
			send(clientSocket, inputConnection.c_str(), (int)inputConnection.size() + 1, 0);

		else {
			cout << errorMsg << "Vous n'avez pas rentré de réponses. Vous serez déconnecté!" << endl;
			closesocket(clientSocket);
			WSACleanup();
			return 0;
		}

		while (true) {

			// Réception du menu des fichiers

			ZeroMemory(buf, 4096);
			bytesReceived = recv(clientSocket, buf, 4096, 0);
			msgReceived = string(buf, 0, bytesReceived);

			if (bytesReceived > 0) {

				cout << msgReceived << endl;

				// Envoie de la sélection au serveur

				cout << "> ";
				getline(cin, userInput);

				userInput = fileMsg + userInput;

				if (userInput.size() > 0)
					send(clientSocket, userInput.c_str(), (int)userInput.size() + 1, 0);
				else {
					cout << errorMsg << "Vous n'avez pas rentré de réponses. Vous serez déconnecté!" << endl;
					closesocket(clientSocket);
					WSACleanup();
					return 0;
				}

				// Réception de la size et du nom du fichier

				ZeroMemory(buf, 4096);
				bytesReceived = recv(clientSocket, (char*)&fileSize, sizeof(long), 0);

				if (bytesReceived > 0) {

					cout << "Size du fichier : " << fileSize << endl;

					ZeroMemory(buf, 4096);
					bytesReceived = recv(clientSocket, buf, 4096, 0);

					if (bytesReceived > 0) {

						fileName = string(buf, 0, bytesReceived);
						cout << "Nom du fichier : " << fileName << endl;

						// Écriture du fichier dans 'Out'

						ofstream file(path, ios::binary);

						file.open(path + fileName, ios::binary | ios::trunc);

						ZeroMemory(buf, 4096);
						bytesReceived = 0;

						do {

							memset(buf, 0, 4096);
							bytesReceived = recv(clientSocket, buf, 4096, 0);

							cout << "BytesReceived : " << bytesReceived << endl;

							if (bytesReceived == 0 || bytesReceived == -1) {
								cout << errorMsg << "Le telechargement a echoue. Le client sera deconnecte!" << endl;
								closesocket(clientSocket);
								WSACleanup();
								return 0;
							}

							file.write(buf, bytesReceived);
							fileDownloaded += bytesReceived;

						} while (fileDownloaded < fileSize);

						ZeroMemory(buf, 4096);
						file.close();
						cout << "Telechargement termine!" << endl;

						// Envoi d'un message de confirmation au serveur

						string confirmation = clientMsg + "Telechargement reussit!";
						send(clientSocket, confirmation.c_str(), 4096, 0);
					}
				}
				else {
					cout << errorMsg << "Le message recu est null. Le client sera deconnecte!" << endl;
					closesocket(clientSocket);
					WSACleanup();
					return 0;
				}
			}

			else {
				cout << errorMsg << "Le message recu est null. Le client sera deconnecte!" << endl;
				closesocket(clientSocket);
				WSACleanup();
				return 0;
			}
		}
	}

	else {
		cout << errorMsg << "Le message recu est null. Le client sera deconnecte!" << endl;
		closesocket(clientSocket);
		WSACleanup();
		return 0;
	}

	// Close

	closesocket(clientSocket);
	WSACleanup();
}