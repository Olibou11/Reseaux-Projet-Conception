﻿#ifndef WIN32_LEAN_AND_MEAN
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

// M�thode de v�rification des bytesRecv
bool bytesVerification(int bytesReceveid) {

	if (bytesReceveid <= 0) {
		cout << "Une erreur de r�ception s'est produite" << endl;
		return false;
	}
	return true;
}

// Main
int main() {

	// String pr�-enregistr�s

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
		cout << errorMsg << "Impossible de cr�er le socket!" << endl;
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


	// --- Boucle pour recevoir et envoyer des messages/donn�es --- //


	// Variables

	char buf[4096];
	int bytesReceived = 0;
	string msgReceived = "";

	string userInput = "";

	string confirmation = "OK";

	long fileSize = 0;
	string fileName = "";
	long fileDownloaded = 0;

	const string path = "output.txt";

	// R�ception du message de confirmation de connexion

	ZeroMemory(buf, 4096);
	bytesReceived = recv(clientSocket, buf, 4096, 0);

	if (bytesVerification(bytesReceived)) {

		cout << string(buf, 0, bytesReceived) << endl;

		// �change commande / Txt

		while (true) {

			cout << clientMsg << "Veuillez entrer une commande :" << endl;
			cin >> userInput; // TODO : Faire une v�rification > 0 et faire attention aux espaces

			ZeroMemory(buf, 4096);
			send(clientSocket, userInput.c_str(), (int)userInput.size() + 1, 0);

			// R�ception de la taille du fichier "output.txt" du serveur

			ZeroMemory(buf, 4096);
			bytesReceived = recv(clientSocket, (char*)&fileSize, sizeof(long), 0);
			cout << "La taille du fichier est de " << fileSize << endl;
			if (bytesVerification(bytesReceived)) {} // TODO : mieux implémenter / ajouter entre les accolades

			// Envoie d'un message de confirmation

			ZeroMemory(buf, 4096);
			send(clientSocket, confirmation.c_str(), (int)confirmation.size() + 1, 0);

			// - 

			if (bytesVerification(bytesReceived)) {

				// R�ception du fichier "output.txt" morceau par morceau

				ofstream file(path, ios::binary | ios::trunc);
				

				if (file.is_open()) {

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

					//decryption du fichier output.txt du client-----------------------------------------
					fstream fileToDecrypt(path, ios::binary | ios::trunc);
					ZeroMemory(buf, 4096);
					//lecture de chaque ligne de output.txt et fait la decryption
					while (fileToDecrypt.getline(buf, 4096))
					{
						for (int i = 0; i < strlen(buf); i++)
							buf[i] = buf[i] - 2; //la clef pout l'encryption est de 2, donc j'enleve 2 a la valeur ASCII

						//ecriture du buf decrypter dans le fichier output.txt
						fileToDecrypt.write(buf, strlen(buf));
					}
					fileToDecrypt.close();
					//--------------------------------------------------------------------------------------

					cout << "Telechargement termine!" << endl;

				}

				else
					cout << "Erreur dans l'ouverture du fichier" << endl;

			}
		}
	}

	// Close

	closesocket(clientSocket);
	WSACleanup();
}