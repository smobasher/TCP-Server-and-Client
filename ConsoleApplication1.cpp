/*Salma Mobasher
8120214
Due April 14, 2021
Most of code taken off site provided by prof:
https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code
also
https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm

BTW it doubles the input on putty because of TCP
*/

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream> 
#include <string> 
#include "sqlite3.h"
using namespace std; 
std::string str; 

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015" //port to put into putty

	static int callback(void *data, int argc, char **argv, char **azColName) {
		int i;
		fprintf(stderr, "%s: ", (const char*)data);

		for (i = 0; i < argc; i++) {
			printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		}

		printf("\n");
		return 0;
	}

int __cdecl main(int argc, char* argv[])
{
	WSADATA wsaData;
	int iResult;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	std::string sql;
	const char* data = "Callback function called";

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(ListenSocket);

	//////////////
	do {
	//////////////////////////////////////////////////////////////////////

	/* Open database */
		rc = sqlite3_open("assignment44.db", &db);

		if (rc) {
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			return(0);
		}
		else {
			fprintf(stderr, "Opened database successfully\n");
		}

		/* Create SQL statement */
		sql = "SELECT * from STUDENT";

		/* Execute SQL statement */
		rc = sqlite3_exec(db, sql.c_str(), callback, (void*)data, &zErrMsg);

		if (rc != SQLITE_OK) {
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
		else {
			fprintf(stdout, "Operation done successfully\n");
		}
		
		///////////////////////////////////////////////////////////////////
		memset(recvbuf, '\0', recvbuflen);
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			int check = false;
			string temp;
			if (check == false)
			{
				// Echo the buffer back to the sender
				iSendResult = send(ClientSocket, recvbuf, iResult, 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}
				check = true;
			}

			int len = strlen(recvbuf);
			for (int i = 0; i < len; i++)
			{
				if (recvbuf[i] != '&')
				{
					temp += recvbuf[i];
				}
				else
				{
					cout << temp << endl;
					////////////////////////////below delimiting the input strings/////////////////////////////////////////////
					std::string delimiter = "!"; //for student number
					std::string stopper = "@"; //for first name
					std::string del2 = "#"; //for last name
					std::string donezo = "$"; //for number of classes
					std::string sNum = temp.substr(0, temp.find(delimiter)); //finds first delimiter
					if ((temp.find(stopper)> temp.find(delimiter))&&(temp.find(del2)> temp.find(stopper))&&(temp.find(donezo)> temp.find(del2))) //makes sure they are in order
					{
				
					std::string fName = temp.substr(((temp.find(delimiter))+1), ((temp.find(stopper)-1)-(temp.find(delimiter)))); //first name between the 2 delimiters...-1, +1 to not include the delimiters
					
					std::string lName = temp.substr((temp.find(stopper))+1, ((temp.find(del2)-1)-(temp.find(stopper)))); //last name between the 2 delimiters
					
					std::string numClass = temp.substr((temp.find(del2))+1, ((temp.find(donezo)-1)- temp.find(del2))); //number of classes between the 2 delimiters
					
					/*was printing them to test them 
					cout << sNum << "\n" << endl;
					cout << fName << "\n" << endl;
					cout << lName << "\n" << endl;
					cout << numClass << "\n" << endl;*/
					
					sql = "INSERT INTO STUDENT (studentID,firstName,lastName,numberOfCourses) VALUES("; //inserting into database
					sql.append(sNum); //student number
					sql.append(", \"");
					sql.append(fName);//firstname
					sql.append("\",\"");
					sql.append(lName); //last name
					sql.append("\",");
					sql.append(numClass);//number ofclasses
					sql.append(");");


					rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
					if (rc != SQLITE_OK) {
						fprintf(stderr, "SQL error: %s\n", zErrMsg);
						sqlite3_free(zErrMsg);
					}
					else {
						fprintf(stdout, "Records created successfully\n");
					}
					}
					/////////////////////above added from last time//////////////////////////////////////////////////////////
				
				}
			}
		}

	} while (iResult > 0);
	///////////
	////////////////////////////////////////////////Here is the changed code/////////////////////////////////////////////////////////
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			
			if (recvbuf[0] == '&')  //checks for the delimiter!!
			{
			
				cout << str << endl; //prints out the string!!!
				str.clear();
			}
			else
			{
				str += recvbuf[0]; //adds to the string!!
			}
			
			///////////////////////////////////////End of changed code//////////////////////////////////////////////////////////////

			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}

		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}