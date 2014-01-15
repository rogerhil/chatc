#include "client.h"
#include <pthread.h>

struct timeval tv;

int main(int argc, char *argv[]) {
    struct addrinfo hints, *servinfo;
    int rv; 
    int rc;
	pthread_t threads[NUM_THREADS];
	struct thread_data_params tdp;
	
	(void) signal(SIGINT, terminate); // associa a funcao terminate a um signal (catch CTRL+C)
	
	MY_ID = 0;
	
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((rv = getaddrinfo(SERVERNAME, SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
	
    // constroe um socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }        
        break;
    }

	 tv.tv_sec = TIMEOUT; // determina um timeout para o recv
	 tv.tv_usec = 0;
	 
	 if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof tv)) {
		perror("setsockopt");
		return -1;
	 } // temporizacao



    if (p == NULL) {
        fprintf(stderr, "client: failed to bind socket\n");
        return 2;
    }
	
	start_connection(sockfd, p); // inicializa conexao com o servidor
    freeaddrinfo(servinfo);
	
	// abaixo coloca a funcao que recebe mensagens do servidor em paralelo
	tdp.sockfd = sockfd;
	tdp.p = p;
    rc = pthread_create(&threads[0], NULL, receive_messages, (void *) &tdp);
    
    run_chat(sockfd, p); // inicializa o chat
    
    close(sockfd);
    
	pthread_exit(NULL);
	
    return 0;
}