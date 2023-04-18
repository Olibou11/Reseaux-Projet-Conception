#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Importations

#include <iostream>
#include <string>
#include <ws2tcpip.h>
#include <fstream>
#include <ostream>

#pragma comment(lib, "ws2_32.lib")

// Namespace
using namespace std;

// String pré-enregistrés

const string connectionMsg = "<CONNECTION> ";
const string fileMsg = "<FILE> ";
const string errorMsg = "<ERROR> ";
const string servMsg = "<SERVER> ";
const string clientMsg = "<CLIENT> ";

// Méthodes supplémentaires

bool bytesVerification(int bytesReceived, SOCKET clientSocket);
void encryption(string path);

// Main

int main() {

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
		cout << errorMsg << "Impossible de creer le socket!" << endl;
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

	string userInput = "";

	string confirmation = "OK";

	long fileSize = 0;
	long fileDownloaded = 0;

	ofstream file;
	ifstream outputFile;

	const string path = "output.txt";

	// R�ception du message de confirmation de connexion

	ZeroMemory(buf, 4096);
	bytesReceived = recv(clientSocket, buf, 4096, 0);

	if (bytesVerification(bytesReceived, clientSocket)) {

		cout << servMsg << string(buf, 0, bytesReceived) << endl;

		// échange commande / .txt

		while (true) {

			cout << clientMsg << "Veuillez entrer une commande : ";
			cin >> userInput; // TODO : Faire une v�rification > 0 et faire attention aux espaces

			ZeroMemory(buf, 4096);
			send(clientSocket, userInput.c_str(), (int)userInput.size() + 1, 0);

			// R�ception de la taille du fichier "output.txt" du serveur

			ZeroMemory(buf, 4096);
			bytesReceived = recv(clientSocket, (char*)&fileSize, sizeof(long), 0);
			
			if (bytesVerification(bytesReceived, clientSocket)) {
				
				cout << servMsg << "La taille du fichier est de " << fileSize << " bytes!" << endl;

				// Envoie d'un message de confirmation

				ZeroMemory(buf, 4096);
				send(clientSocket, confirmation.c_str(), (int)confirmation.size() + 1, 0);

				// Réception du fichier "output.txt" morceaux par morceaux

				file.open(path, ios::binary | ios::trunc);

				if (file.is_open()) {

					file.clear();

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

					cout << "Telechargement termine!" << endl;
					file.close();

					// Affichage du coutenu de "output.txt" dans la console client

					encryption(path);

					outputFile.open(path, ios::binary);

					if (outputFile.is_open())
						cout << outputFile.rdbuf() << endl;
					outputFile.close();

				}
				else
					cout << errorMsg << "Erreur dans l'ouverture du fichier" << endl;
			}
		}
	}

	// Close

	closesocket(clientSocket);
	WSACleanup();
}

bool bytesVerification(int bytesReceived, SOCKET clientSocket) {

	if (bytesReceived <= 0) {
		cout << errorMsg << "Une erreur de reception s'est produite!" << endl;
		closesocket(clientSocket);
		WSACleanup();
		return 0;
	}
	return true;
}

void encryption(string path) {

	char buf[4096];
	ZeroMemory(buf, 4096);

	string line = "";

	string t = "";

	string file = "";

	fstream fileToEncrypt(path, ios::in | ios::out);

	ofstream of(path, ios::in | ios::out);

	if (fileToEncrypt.is_open()) {

		//lecture de chaque ligne de output.txt et fait l'encryption

		while (getline(fileToEncrypt, line)) {
			for (char c : line) {
				c += -2;
				string tt = string(1, c);
				t.append(tt);
			}
			t.append("\n");
			file += t;
			t.clear();
		}

		fileToEncrypt.clear();
		fileToEncrypt.close();

		of.write(file.c_str(), file.size());
		of.close();
	}
}