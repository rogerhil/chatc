#ifndef protocol_h
#define protocol_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// definicao de constantes comuns ao servidor e cliente
#define SERVERPORT "3000"
#define MAXBUFLEN 206
#define MAX_CONNECTIONS 50
#define SERVERNAME "localhost"
#define TIMEOUT 5
#define SERVER_ID 99

// definicao de mensagens comuns ao servidor e cliente
#define OI 1
#define TCHAU 2
#define OK 3
#define REG 4
#define MSG 5
#define ERRO 6

typedef struct Message {
	int id;
	int to;
	int type;
	char name[100];
	char text[200];	
} Message;

typedef struct Client {
	int id;
	char name[100];
	struct sockaddr_in address;
} Client;	

typedef struct Clients {
	int num_connected;
	struct Client *item;   
} Clients;

void psend(Message msg, int sockfd, struct sockaddr_in addr);
Message receive(int sockfd, struct sockaddr_in *their_addr);
void sendok(int id, int sockfd, struct sockaddr_in addr);
void include_client(int sockfd, struct sockaddr_in addr);
void manipulate_messages(int sockfd, struct addrinfo *p);
void respond_message(int sockfd, Message msg, struct sockaddr_in addr);
void register_client(int id, char *name, int sockfd, struct sockaddr_in addr);
void send_to_all(int sockfd, Message msg);
int client_exists(int id);
void remove_client(int id);

// variaveis globais
Client *get_client(int id);
Clients myclients;

#endif
