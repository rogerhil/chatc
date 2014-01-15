#ifndef protocol_h
#define protocol_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>

// definicao de constantes comuns ao servidor e cliente
#define SERVERPORT "3000"
#define MAXBUFLEN 206
#define MAX_CONNECTIONS 50
#define SERVERNAME "localhost"
#define TIMEOUT 20
#define TIMEOUT_TRIES 1
#define SERVER_ID 99
#define NUM_THREADS 1

// definicao de mensagens comuns ao servidor e cliente
#define OI 1
#define TCHAU 2
#define OK 3
#define REG 4
#define MSG 5
#define ERRO 6

// constantes que determinam uma mensagem privada
#define PRIVATE_CALL_BEGIN "#"
#define PRIVATE_CALL_END ":"

typedef struct Message {
	int id;
	int to;
	int type;
	char name[100];
	char text[200];	
} Message;

typedef struct Client {
	int id;
	struct sockaddr_storage address;
} Client;

typedef struct  Clients {
	int num_connected;
	char name[100];
	struct client *item;   
} Clients;

struct thread_data_params {
	int sockfd;
	struct addrinfo *p;
};

void start_connection(int sockfd, struct addrinfo *p);
void psend(Message msg, int sockfd, struct addrinfo *p);
Message receive(int sockfd, char *errstr, int verbose);
void run_chat(int sockfd, struct addrinfo *p);
int is_private_message(Message msg);
char *get_private_message(Message msg);
void *receive_messages(void *threadarg);
void terminate(int sig);
int is_ok(Message msg);

// variaveis globais
int MY_ID;
char MY_NAME[100];
int sockfd;
struct addrinfo *p;
int ISOK;

#endif