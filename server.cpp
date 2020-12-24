//Based on: https://www.binarytides.com/multiple-socket-connections-fdset-select-linux/ 
#include "server.h"

int main(int argc , char *argv[])
{
    //Initialize various variables
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[100] , max_clients = 100 , activity, i , valread , sd;
	int max_sd;
    int trans = 0;
    map<string,int> client_trans;
    double time_start = 0;
    double time_end = 0;
    struct sockaddr_in address;
    struct timeval tp;
    timeval timeout = {30,0};
    char buffer[1000];  //Data buffer of 1K
    char temp[1000];
     
    //Set of socket descriptors
    fd_set readfds;
 
    //Initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++) 
    {
        client_socket[i] = 0;
    }
     
    //Create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
 
    //Set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
 
    //Type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));
     
    //Bind the socket to port in argv
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
	
    //Try to specify maximum of int max_clients (100) pending connections for the master socket
    if (listen(master_socket, max_clients) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
     
    //Accept the incoming connection
    addrlen = sizeof(address);

    //Log port
    cout << "Using port " << argv[1];
    
    //Sever loop
	while(TRUE) {

        //Clear arrays
        memset(buffer, 0, 1000);
        memset(temp, 0, 1000);

        //Clear the socket set
        FD_ZERO(&readfds);
 
        //Add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
		
        //Add child sockets to set
        for ( i = 0 ; i < max_clients ; i++) 
        {
            //Socket descriptor
			sd = client_socket[i];
            
			//If valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);
            
            //Highest file descriptor number, need it for the select function
            if(sd > max_sd)
				max_sd = sd;
        }
 
        //Wait for an activity on one of the sockets , timeout is 30 seconds
        activity = select( max_sd + 1 , &readfds , NULL , NULL , &timeout);
   
        //Check for select error and if not checks for timeout
        if ((activity < 0) && (errno!=EINTR)){
            printf("select error");
        } else if (activity == 0){

            //Print summary
            print_summary(client_trans, time_start, time_end);
            return 0;
        }
         
        //If something happened on the master socket, then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)) 
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
                perror("accept");
                exit(EXIT_FAILURE);
            }
             
            //Add new socket to array of sockets
            for (i = 0; i < max_clients; i++) {

                //if position is empty add at the current position
				if( client_socket[i] == 0 ){
                    client_socket[i] = new_socket;
					break;
                }
            }
        }
         
        //Else its some IO operation on some other socket :)
        for (i = 0; i < max_clients; i++) 
        {
            sd = client_socket[i];
             
            if (FD_ISSET( sd , &readfds)) 
            {
                //Check if it was for closing , and also read the incoming message
                if ((valread = read( sd , buffer, 1000)) == 0){

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                } else {

                    //Split original client message from client name (which is separated by a space)
                    char *client_message;
                    char const *delim = " ";
                    client_message = strtok(buffer, delim);
                    char *client_name;
                    client_name = strtok(NULL, delim);

                    //Log receive and get start time if this is the first transaction
                    trans++;
                    gettimeofday(&tp, NULL);
			        cout << "\n" << tp.tv_sec << "." << setw(2) << setfill('0') << (int) (tp.tv_usec/1E4) << ": #" << trans <<" (" << client_message << ")"
                        << " from " << client_name;
                    cout << flush;
                    if(time_start == 0){
                        time_start = tp.tv_sec + (tp.tv_usec/1E6);
                    }
                    time_end = tp.tv_sec + (tp.tv_usec/1E6);

                    //Perform a trans operation and record it in map (named client_trans)
                    int t_val;
			        sscanf(client_message+1, "%i", &t_val);
                    Trans(t_val);

                    //Record in map
                    string str(client_name);
                    client_trans[client_name]++;

                    //Log finished transaction
                    gettimeofday(&tp, NULL);
			        cout << "\n" << tp.tv_sec << "." << setw(2) << setfill('0') << (int) (tp.tv_usec/1E4) << ": #" << trans << " (Done)"
                        << " from " << client_name;
                    cout << flush;

                    //Convert transaction int to char * to send back
                    sprintf(temp, "%i", trans);
                    memcpy(buffer+1, temp, strlen(temp)+1);
                    buffer[0] = 'D';
                    send(sd , buffer , strlen(buffer) , 0 );
                }
            }
        }
    }

    //Print summary to stdout (in event of server ending not to timeout)
    print_summary(client_trans, time_start, time_end);
    return 0;
}

void print_summary(map<string, int> client_trans, double time_start, double time_end){
    //Summary printing below
    cout << "\n\nSUMMARY\n";

    //Go through client list and add the transaction on each one to total_trans to get total transactions
    //Also outputs each client and its transactions
    int total_trans = 0;
    for(auto const& x: client_trans){
        cout << "    " << x.second << " transactions from " << x.first << "\n";
        total_trans += x.second;
    }

    //Get transactions per seconds and output then return
    double trans_per_sec = total_trans/(time_end - time_start);
    cout << setprecision(4) << trans_per_sec << " transactions/sec  (" << total_trans <<
        "/" << (time_end - time_start) << ")\n";
}