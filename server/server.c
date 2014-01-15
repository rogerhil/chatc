#include "server.h"

int main(void) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    
	myclients.num_connected = 0; // zero cliente cadastrados no inicio
	// aloca array de clientes
	myclients.item = malloc(sizeof(struct Client)*MAX_CONNECTIONS);
    
    // inicializa socket a partir daqui
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo)) != 0) {
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

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);
	
    while (1) {
    	// loop para manipular as mensagens recebidas dos clientes
    	manipulate_messages(sockfd, p);
    }

    close(sockfd); // fecha socket

    return 0;
}