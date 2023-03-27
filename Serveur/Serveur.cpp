#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Importations initiales

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

// Namespaces

using namespace std;
namespace fs = filesystem;

// Méthode permettant de diviser un string avec un séparateur
vector<string> splitString(string msgReceived, const char separator) {

	vector<string> splittedMsg;
	splittedMsg.clear();

	stringstream streamData(msgReceived);

	string val;

	while (getline(streamData, val, separator))
		splittedMsg.push_back(val);

	return splittedMsg;
}

// Main
int main() {

	// String pré-enregistrés

	const string servMsg = "<SERVER> ";
	const string errorMsg = "<ERROR> ";

	// Initialisation de WinShock

	WSADATA wsaData;

	int wsaStartup = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (wsaStartup != 0) {
		cout << errorMsg << "Impossible d'initialiser WinSock!" << endl;
		WSACleanup();
		return 0;
	}

	// Initialisation du Socket

	SOCKET listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listeningSocket == INVALID_SOCKET) {
		cout << errorMsg << "Impossible de creer le socket!" << endl;
		WSACleanup();
		return 0;
	}

	// Lier un ip adresse et un port au socket

	sockaddr_in saServer;

	saServer.sin_family = AF_INET;			// Adressage de la famille
	saServer.sin_addr.s_addr = INADDR_ANY;	// Adresse
	saServer.sin_port = htons(27015);		// Port

	bind(listeningSocket, (SOCKADDR*)&saServer, sizeof(saServer));

	// Mettre WinSock en mode écoute (Un thread, mais peut supporter plusieurs clients)

	listen(listeningSocket, SOMAXCONN);

	fd_set master;
	FD_ZERO(&master);

	FD_SET(listeningSocket, &master); // Arbre des sockets

	while (true) {

		fd_set copyMaster = master; // copyMaster permet de conserver un original du master (Important)

		int socketCount = select(0, &copyMaster, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++) {

			SOCKET sock = copyMaster.fd_array[i];

			if (sock == listeningSocket) {

				// Création du socket client

				sockaddr_in client;
				SOCKET clientSocket;
				int clientSize = sizeof(client);

				// Donne accès au client à se connecter
				clientSocket = accept(listeningSocket, (sockaddr*)&client, &clientSize);

				// Ajoute la nouvelle connexion dans la liste des clients connectés
				FD_SET(clientSocket, &master);

				// Envoie d'un message de bienvenu dans la console du client

				string welcome = servMsg + "Bienvenue dans le serveur!";
				send(clientSocket, welcome.c_str(), (int)welcome.size() + 1, 0);

				// Affichage de la connexion du client dans la console du serveur

				char host[NI_MAXHOST];
				char service[NI_MAXSERV];

				ZeroMemory(host, NI_MAXHOST);
				ZeroMemory(service, NI_MAXSERV);

				if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
					cout << servMsg << host << " est connecte au port #" << service << endl;
				else {
					inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
					cout << servMsg << host << " est connecte au port #" << ntohs(client.sin_port) << endl;
				}
			}

			// Nous souhaitons communiquer avec un socketClient déjà existant

			else {

				// Variables

				char buf[4096];
				int bytesReceived = 0;
				string msgReceived = "";
				vector<string> splittedMsg(0);

				string menu = "";
				int count = 0;
				const string path = "../Data/";

				int pathIndex = 0;
				string filePath = "";
				long fileSize = 0;
				vector<string> fileNameVec(0);
				string fileName;

				// On recoit des instructions du client sous la forme d'un string

				ZeroMemory(buf, 4096);
				bytesReceived = recv(sock, buf, 4096, 0);

				if (bytesReceived > 0) {

					msgReceived = string(buf, 0, bytesReceived);
					splittedMsg = splitString(msgReceived, ' ');

					if (splittedMsg[0] == "<CONNECTION>") { // Le client souhaite se connecter

						if (splittedMsg[1] == "admin" && splittedMsg[2] == "1234")
							cout << servMsg << "La connexion du client est etablie!" << endl;

						else {
							cout << errorMsg << "La connexion du client a echoue. Le client sera deconnecte!" << endl;
							closesocket(sock);
							FD_CLR(sock, &master);
						}
					}

					else if (splittedMsg[0] == "<FILE>") { // Le client veut obtenir un fichier ou quitter

						if (stoi(splittedMsg[1]) != 0) { // Le client veut un fichier

							// Trouver le fichier

							pathIndex = stoi(splittedMsg[1]) - 1; // Correction de la position

							count = 0;
							for (const auto& entry : fs::directory_iterator(path)) {
								if (count == pathIndex)
									filePath = entry.path().generic_string();
								count++;
							}

							// Lire le fichier

							ifstream file(filePath, ios::binary);

							if (file.is_open()) {

								// Envoyer la taille (octets) du fichier

								file.seekg(0, ios::end);
								fileSize = file.tellg();
								send(sock, (char*)&fileSize, sizeof(long), 0);

								// Envoyer le nom du fichier

								fileNameVec = splitString(filePath, '/');
								fileName = fileNameVec.back();
								send(sock, fileName.c_str(), 4096, 0);

								// Envoyer le fichier partie par partie

								file.seekg(0, ios::beg);

								do {
									// Lecture du fichier et envoie

									ZeroMemory(buf, 4096);
									file.read(buf, 4096);

									if (file.gcount() > 0)
										send(sock, buf, file.gcount(), 0);

								} while (file.gcount() > 0);

								file.close();

								// Réception d'un message de confirmation, puis affichage

								ZeroMemory(buf, 4096);
								bytesReceived = recv(sock, buf, 4096, 0);

								if (bytesReceived > 0) {
									msgReceived = string(buf, 0, bytesReceived);
									cout << msgReceived << endl;
								}

								else {
									cout << errorMsg << "Le message recu est null. Le client sera deconnecte!" << endl;
									closesocket(sock);
									FD_CLR(sock, &master);
								}
							}

							else { // Le client souhaite quitter
								cout << errorMsg << "Le fichier est introuvable. Le client sera deconnecte!" << endl;
								closesocket(sock);
								FD_CLR(sock, &master);
							}
						}

						else {
							cout << servMsg << "Le client souhaite se deconnecter... Fait!" << endl;
							closesocket(sock);
							FD_CLR(sock, &master);
						}
					}
				}

				else {
					cout << errorMsg << "Le message recu est null. Le client sera deconnecte!" << endl;
					closesocket(sock);
					FD_CLR(sock, &master);
				}

				// Envoyer le menu des fichiers au client (Il se produit à tous les tours)

				menu.clear();
				menu = servMsg + "Voici les fichiers du serveur:\n[0] Quitter\n";

				count = 1;
				for (const auto& entry : fs::directory_iterator(path)) {
					menu = menu + "[" + to_string(count) + "] " + entry.path().generic_string() + "\n";
					count++;
				}

				send(sock, menu.c_str(), (int)menu.size() + 1, 0);
			}
		}
	}

	// Ménage de WinSock
	WSACleanup();
}