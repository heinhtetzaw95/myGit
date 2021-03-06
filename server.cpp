#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define max_size 512

using namespace std;

struct children{

	int connection;
	int receive;
	char buffer[max_size], reply[max_size], name[max_size], message[max_size];
	bool active, running;
	int partnerNum;
};
		
		//make the thread that will recieve the client input and sends back AKT
void *ReadWrite(void* input){
	struct children *child;
	child = (children*) input;

	send (child->connection, "You: ", max_size, 0);
			while (true){

				recv(child->connection, child->buffer, max_size, 0);

				if (true){

					cout << "<Client> " << child->name << ": " << child->buffer << endl;	

						//check if the client wants to leave
					if (strcmp(child->buffer, "--leave") == 0) {

						cout << "\n" <<child->name << " has left!\n" << endl;
						send (child->connection, "exit==true", max_size, 0);
						strcpy(child->buffer, "");
						child->active = false;
						child->running = false;

						return 0;
					}

						//send AKT and next line request
					else {
						send (child->connection, "--recieved--", max_size, 0);
						send (child->connection, "\nYou: ", max_size, 0);
					}
							//send the incoming message from another client **not working**

				/*	if (strcmp(child->buffer, "--leave") != 0){
						strcpy(child->message, child->buffer);
					}

					if (sizeof(child->message) > 0){
						send (child->connection, child->message, max_size, 0);
						strcpy(child->message, "");
					}*/

				}

			}			

			child->running = false;
			pthread_exit(NULL);
}


int main(){
	int theSocket, port;
	char hostname[256];
	struct sockaddr_in socketInfo;
	struct hostent *theHost;
	
	bzero (&theSocket, sizeof (sockaddr_in));

	cout << "Port: ";
	cin >> port;

//	port = 1500;			//for temporaty

	

		//get local address
	gethostname(hostname, 256);
	cout << hostname;

		//check the host address
	if ((theHost = gethostbyname(hostname)) == NULL){
		cout << " Warning! System hostname is not correctly configured!" << endl;
//		return -1;		//Just warning, don't need to exit the app
	}
	else cout << " (or) " << theHost << endl << "Host OK!" << endl;

		//create socket
	theSocket = socket (AF_INET, SOCK_STREAM, 0);
	
		//check socket if it's ok
	if (theSocket < 0){
		close (theSocket);
		return -1;
	}
	else cout << "Socket OK!" << endl;

		//load system info into socket structure
	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = htonl (INADDR_ANY);
	socketInfo.sin_port = htons(port);
	
		//bind socket to local address
	if ((bind(theSocket, (struct sockaddr *) &socketInfo, sizeof (socketInfo))) < 0){
		close (theSocket);
		cerr << "Binding Error!" << endl << endl;
		return -1;
	}
	else cout << "Binding OK!" << endl;

	cout << "Listening at port " << port << " for first client..." << endl;

		//wait for first 
	listen (theSocket, 1);

	children child[10];

		//register the usernames -- **this server only have these client usernames**
	strcpy(child[0].name, "Bob");
	strcpy(child[1].name, "Sam");
	strcpy(child[2].name, "Alice");
	strcpy(child[3].name, "Rick");
	strcpy(child[4].name, "Ryan");
	strcpy(child[5].name, "Hein");
	strcpy(child[6].name, "Josh");
	strcpy(child[7].name, "Clari");
	strcpy(child[8].name, "Wyane");
	strcpy(child[9].name, "Chris");

	for (int i = 0; i<10; i++){
		child[i].active = false;
		child[i].running = false;
	}

	int childCount = 0;

	pthread_t threads[10];
	int threadCount = 0;
	int rc[10] = {};

	for (;;){

		child[childCount].connection = accept (theSocket, NULL, NULL);
			if (child[childCount].connection < 0){
				//close(theSocket);
				return -1;
			}

			else {
				cout << "\nNotice: A client joined" << endl << endl;

				char tempName[512], partner[512];
				bool exists = false;
				int childNum = 0;

						//confirm connection
				send (child[childCount].connection, "\nConnection confirmed by Server!\n\n", max_size, 0);

				askAgain: while(exists == false){

					send (child[childCount].connection, "Enter your username: ", max_size, 0);

					recv(child[childCount].connection, tempName, max_size, 0);

					cout << "Client entered: " << tempName << endl;

							//check the username that client typed in
					for (int i=0; i<sizeof(child); i++){
						if (strcmp(tempName, child[i].name) == 0) {
							cout << "Username found! ID # " << i << endl;
							exists = true;
							childNum = i;
							child[childNum].connection = child[childCount].connection;
							break;
						}
						else if (i > 9) {
							cout << "\nUsername is not in Record!" << endl;
							exists = false;

							send (child[childNum].connection, "Error! Username NOT found!\n", max_size, 0);

								//if there is not such username, ask again
							goto askAgain;
						}
					}
				}

						//valid username -> check if it's a dublicate user trying to login
				if (exists) {
					cout << "\nUsername is Valid - ";

								//reject if duplicated
					if (child[childNum].active == true){
						send (child[childNum].connection, "Error! Username is already logged in!\n", max_size, 0);
						send (child[childNum].connection, "exit==true", max_size, 0);
						cout << child[childNum].name << " has been REJECTED!\n" << endl;
						
					}
								//accept if it's not duplicate
					else {
						send (child[childNum].connection, "Username ACCEPTED!\n", max_size, 0);
						cout << child[childNum].name << " has been ACCEPTED!\n" << endl;
						child[childNum].active = true;


								//check who is currently online
						int online = 0;
						for (int j=0; j<=10; j++){

							if (child[j].running == true){
								send (child[childNum].connection, " - ", max_size, 0);
								send (child[childNum].connection, child[j].name, max_size, 0);
								online++;
							}

						}

								//one person online
						if (online == 1){
							send (child[childNum].connection, " is online now.\n", max_size, 0);
						}
								//two or more people online
						else if (online > 1){
							send (child[childNum].connection, " are online now.\n", max_size, 0);
						}
								//nobody else is online
						else {
							send (child[childNum].connection, "You are only one online right now.\n", max_size, 0);
							goto passPartnerCheck;
						}

							//ask for partner name and check partner's name

						bool partnerExists = false;
						bool gotPartner = false;
						char tryAgain[512];

						enterAgain: while (partnerExists == false){

									//ask for partner's name to connect to
					
							send (child[childNum].connection, "Connect to: ", max_size, 0);
							recv(child[childNum].connection, partner, max_size, 0);

							for (int i=0; i<sizeof(child) && gotPartner == false; i++){

										//check partner
								if (strcmp(partner, child[i].name) == 0) {
									cout << "Partner found! ID # " << i << endl;

									child[childNum].partnerNum = i;
									send (child[childNum].connection, "\nPartner found ", max_size, 0);

											//check partner active or not
									if (child[child[childNum].partnerNum].running == true) {
										send (child[childNum].connection, "and is active.\n", max_size, 0);
										gotPartner = true;
										goto passPartnerCheck;
									}

									else
										send (child[childNum].connection, "but not active.\nTry another one? (Y/N): ", max_size, 0);
										recv(child[childNum].connection, tryAgain, max_size, 0);

										if (strcmp(tryAgain, "Y") == 0 || strcmp(tryAgain, "y") == 0){
											goto enterAgain;
										}

										else if (strcmp(tryAgain, "N") == 0 || strcmp(tryAgain, "n") == 0){
											child[childNum].active = false;
											child[childNum].running = false;
											send (child[childNum].connection, "\nBye\n", max_size, 0);
											send (child[childNum].connection, "exit==true", max_size, 0);
											goto nextClient;
										}

									break;
								}

										//if partner cannot be found in the list, ask again
								else if (i > 9) {
									cout << "\nPartner NOT found!" << endl;

									send (child[childNum].connection, "\nError! Partner NOT found!\nTry again? (Y/N): ", max_size, 0);
									recv(child[childNum].connection, tryAgain, max_size, 0);

										if (strcmp(tryAgain, "Y") == 0 || strcmp(tryAgain, "y") == 0){
											goto enterAgain;
										}
										else if (strcmp(tryAgain, "N") == 0 || strcmp(tryAgain, "n") == 0){
											child[childNum].active = false;
											child[childNum].running = false;
											send (child[childNum].connection, "\nBye\n", max_size, 0);
											send (child[childNum].connection, "exit==true", max_size, 0);
											goto nextClient;
										}

									break;
								}
							}
						}
					}

					exists = false;
				}

					//after checking everything, make a thread
				passPartnerCheck:
				
				if (child[childNum].running == true){
					cout << childNum << " is already running.\n" << endl;
				}

				else if(child[childNum].active == true){
					cout << "Watch Dog: " << childNum << endl;
					
					child[childNum].running = true;

							//making a thread
					rc[childNum] = pthread_create(&threads[childNum], NULL, &ReadWrite, &child[childNum]);

			//		if (child[childNum].running == false ) pthread_cancel(threads[childNum]);
							
							//creating thread fails
					if (rc[childNum]) {
						cout << "Cannot create thread, " << rc[childNum] << endl;
						child[childNum].active = false;
						child[childNum].running = false;
					}
				}
			
			
				//wait for next client to connect the server
			nextClient:

			childCount++;
			cout << "\nListening at port " << port << " for next client..." << endl;

			listen (theSocket, 1);
		}
	
	}
	return 0;

}