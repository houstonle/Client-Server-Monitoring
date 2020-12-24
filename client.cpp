//Based on: Using class provided code (Found in Lecture Material/Code/7.Message Passing)
#include "client.h"

int main(int argc, char *argv[])
{
	//Initialization of various variables
	int sock;
	int trans = 0;
	struct sockaddr_in server;
	struct timeval tp;
	char message[1000], server_reply[1000], hostname[1024], orig_msg[1000];
	hostname[1023] = '\0';

	//Get hostname and pid to create name for client
	gethostname(hostname, 1023);
	pid_t pid = getpid();
	int length = snprintf(NULL, 0, "%d", pid);
	char* my_pid = (char*)malloc(length + 2);
	my_pid[0] = '.';
	snprintf(&(my_pid[1]), length + 1, "%d", pid);

	//Create client name
	char* clientname = (char*)malloc(strlen(my_pid) + strlen(hostname) + 1);
	strcpy(clientname, hostname);
	strcat(clientname, my_pid);

	//Create logging file and log opening statements
    ofstream myfile;
    myfile.open(clientname);
	myfile << "Using port " << argv[1];
	myfile << "\nUsing server address " << argv[2];
	myfile << "\nHost " << clientname;
	
	//Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}
	
	//Assign values based on arguments when running client
	server.sin_addr.s_addr = inet_addr(argv[2]);
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1]));

	//Connect to remote server
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}

	//Keep communicating with server, ends when it reads in EOF
	while(1) {

		//Read in message
		memset(message, 0, 1000);
		scanf("%s", message);

		//Add a delimiter (which is ' ') and client name to the original message
		strncpy(orig_msg, message, strlen(message)+1);
		message[strlen(message)] = ' ';
		strcat(message, clientname);

		//Check command. If 'T' send to server, if 'S' sleep otherwise it is EOF thus exit
		if(message[0] == 'T') {

			//Send T command to server and update transactions made
			if( send(sock, message, strlen(message), 0) < 0) {
				puts("Send failed");
				return 1;
			}
			trans++;

			//Log send
			gettimeofday(&tp, NULL);
			myfile << "\n" << tp.tv_sec << "." << setw(2) << setfill('0') << (int) (tp.tv_usec/1E4) << ": " << "Send (" << orig_msg << ")";

			//Wait for server to reply
			memset(server_reply, 0, 1000);
			if(recv(sock, server_reply, 1000, 0) < 0) {
				printf("No message has been received");
			} else {
				//Log receive
				gettimeofday(&tp, NULL);
				myfile << "\n" << tp.tv_sec << "." << setw(2) << setfill('0') << (int) (tp.tv_usec/1E4) << ": " << "Recv (" << server_reply << ")";
			}


		} else if (message[0] == 'S') {

			//Run sleep and log
			int s_val;
			sscanf(&(message[1]), "%i", &s_val);
			Sleep(s_val);
			myfile << "\nSleep " << s_val << " units";

		} else {
			//Exit if input is not 'T' or 'S'. End of file must have been reached then
			break;
		}

	}

	//Log transactions and perform clean up
	myfile << "\nSent " << trans << " transactions";
	free(my_pid);
	free(clientname);
	close(sock);
	return 0;
}
