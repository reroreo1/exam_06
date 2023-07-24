#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>


#define MAX_CLIENTS 128
#define BUFFER_SIZE 200000

int main(int ac, char **av){
	if (ac!= 2){
		fprintf(stderr, "Usage: %s <port>\n", av[0]);
        exit(1);
	}

	int clSock[MAX_CLIENTS];
	int next_id = 0;
	fd_set active, ready;
	char buf[BUFFER_SIZE];
	//socket creation and server creation
	int serv = socket(AF_INET, SOCK_STREAM, 0);
	if (serv < 0) 
    {
        perror("Error creating server socket");
        exit(1);
    }

    struct sockaddr_in serverAddress = {0};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serverAddress.sin_port = htons(atoi(av[1]));

    if (bind(serv, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) 
    {
        perror("Error binding server socket");
        exit(1);
    }

    if (listen(serv, MAX_CLIENTS) < 0) 
    {
        perror("Error listening on server socket");
        exit(1);
    }
	//initialize the socket sets
	FD_ZERO(&active);
	FD_SET(serv,&active);
	int max = serv;

	while(1){
		ready = active;
		if (select(max + 1, &ready, NULL, NULL, NULL) < 0) 
        {
            perror("Error in select");
            exit(1);
        }
		for (int socketId = 0; socketId <= max;socketId++){
			if (FD_ISSET(socketId,&ready)){
				//new clients
				if (socketId == serv){
					int client = accept(serv, NULL, NULL);
                    if (client < 0) 
                    {
                        perror("Error accepting client connection");
                        exit(1);
                    }
					FD_SET(client,&active);
					max = (client > max) ? client : max;
					sprintf(buf, "server: client %d just arrived\n", next_id);
					send(client,buf,strlen(buf),0);
					clSock[next_id++] = client;
				}
				else{
					int readBytes = recv(socketId, buf, sizeof(buf) - 1, 0);
					//client disconnect or ended the connection
					if (readBytes <= 0){
						sprintf(buf, "server: client %d just left\n", socketId);
						for (int i = 0; i < next_id ; i++){
							if (clSock[i] != socketId){
								send(clSock[i], buf, strlen(buf), 0);
							}
						}
						close(socketId);
						FD_CLR(socketId,&active);
					}
					//broadcast the message to all the clients
					else{
						buf[readBytes] ='\0';
						sprintf(buf, "client %d: %s\n", socketId, buf);
                        for (int i = 0; i < next_id; i++) 
                        {
                            if (clSock[i] != socketId) 
                            {
                                send(clSock[i], buf, strlen(buf), 0);
                            }
                        }
					}
				}
			}
		}
	}
	return 0;
}