#include "protocol.h"

void manipulate_messages(int sockfd, struct addrinfo *p) {
	/**
	 * Recebe mensagens dos clientes e executa funcionalidades de acordo com o
	 * tipo de mensagem
	 */
	Message recmsg;
	Message exitmsg;
	char errstr[100];
	struct sockaddr_in their_addr;
	
	printf("\nserver: waiting to recvfrom...\n");
	// recebe mensagens dos clientes
	recmsg = receive(sockfd, &their_addr);
	
	switch (recmsg.type) {
		case  OI:
		    // caso o tipo da mensagem seja OI, inclui o cliente no array e o registra mantendo conexao
			include_client(sockfd, their_addr);
			register_client(myclients.num_connected, recmsg.name, sockfd, their_addr);
			printf("Client \"%s\" successfully registered\n", recmsg.name);
			break;
		case TCHAU:
			// caso o tipo da mensagem seja TCHAU, envia mensagem a todos os cliente informando e remove o cliente do array
			if (client_exists(recmsg.id)) { // testa se o cliente solicitado realmente existe
				// constroe mensagem MSG a ser enviada a todos os clientes
				exitmsg.id = recmsg.id;
				exitmsg.type = MSG;
				exitmsg.to = 0;
				strcpy(exitmsg.name, "SERVER INFO");
				sprintf(exitmsg.text, " <= The %s user has gone!", recmsg.name);
				// envia a todos os clientes
				send_to_all(sockfd, exitmsg);
				// remove o cliente do array global
				remove_client(recmsg.id);
				//close()
			} else {
				// caso nao exista o cliente solicitado, o erro e' reportado na tela e atraves de uma mensagem ERRO ao rementente novamente
				sprintf(errstr, "ERROR: Trying to drop connection and remove an unexisting client, id #%d", recmsg.id);
				printf("%s\n", errstr);
				
				// constroe mensagem de error
				exitmsg.id = SERVER_ID;
				exitmsg.type = ERRO;
				exitmsg.to = recmsg.id;
				strcpy(exitmsg.name, recmsg.name);
				strcpy(exitmsg.text, errstr);
				
				psend(exitmsg, sockfd, their_addr); // envia a mensagem de ERRO ao remetente
			}
			break;
		case MSG:
			// caso o tipo da mensagem seja MSG, simplesmente responde ao destinatario
			respond_message(sockfd, recmsg, their_addr);
			break;
	}
	
}

void send_to_all(int sockfd, Message msg) {
	/**
	 * Envia uma mensagem a todos os clientes cadastrados
	 */
	int k;
	for (k = 0; k < myclients.num_connected; k++) {
		psend(msg, sockfd, myclients.item[k].address);
	}	
}

void respond_message(int sockfd, Message msg, struct sockaddr_in addr) {
	/**
	 * Responde uma mensagem MSG a um cliente especifico ou a todos os clientes
	 */
	
	
	Client *client;
	Message respmsg;
	Message errmsg;
	int k;
	
	// envia mensagem OK ao destinatario 
	sendok(msg.id, sockfd, addr);
	
	// constroe uma mensagem MSG a ser enviada
	respmsg.id = msg.id;
	respmsg.type = MSG;
	strcpy(respmsg.text, msg.text);
	strcpy(respmsg.name, msg.name);
	
	if (respmsg.to) {
		// se existe um destinatario especifico (.to), entao e' uma mensagem
		// privada e sera' enviada somente a um cliente
		client = get_client(msg.to); // adquire o cliente
		if (client->id) {
			// caso o cliente exista, entao envia a mensagem
			psend(respmsg, sockfd, client->address);
		} else {
			// caso o cliente nao exista, retorna uma mensagem ERRO ao
			// remetente informando o ocorrido
			
			// constroe a mensagem de erro
			errmsg.id = SERVER_ID;
			errmsg.type = ERRO;
			strcpy(errmsg.text, "The client doesn't exist!");
			strcpy(errmsg.name, "SERVER_INFO");
			
			psend(errmsg, sockfd, addr); // envia a mensagem de erro ao rementente
		}
	} else {
		// se nao existe destinatario específico (.to=0), entao envia a
		// mensagem MSG todos os clientes cadastrados
		for (k = 0; k < myclients.num_connected; k++) {
			if (msg.id != myclients.item[k].id) {
				// envia a mensagem MSG
				psend(respmsg, sockfd, myclients.item[k].address);
			}
		}
	}	

}

Client *get_client(int id) {
	/**
	 * Adquire um cliente atraves de seu id do array de clientes
	 */
	int k;
	Client cl;
	
	for (k = 0; k < myclients.num_connected; k++) {
		if (myclients.item[k].id == id) {
			return &myclients.item[k];
		}
	}
	// caso nao exista o cliente, e' informado na tela do servidor o ocorrido
	// e um cliente vazio generico com id=0 e' retornado
	printf("\n *** Client \"%d\" not found ***\n", id);
	cl.id = 0;
	return &cl;
}

void include_client(int sockfd, struct sockaddr_in addr) {
	/**
	 * Inclue um novo cliente no array de clientes
	 */
	int id;
	// incrementa o numero de clientes cadastrados
	myclients.num_connected++;
	// determina um novo id
	id = myclients.num_connected;
	myclients.item[myclients.num_connected-1].id = id;
	myclients.item[myclients.num_connected-1].address = addr; // endereco gravado
}

int client_exists(int id) {
	/**
	 * Testa se o cliente existe pelo id
	 */
	int k;
	for (k = 0; k < myclients.num_connected; k++) {
		if (myclients.item[k].id == id) {
			return id;	
		}
	}
	return 0;
}

void remove_client(int id) {
	/**
	 * Remove um cliente do array de clientes
	 */
	Client *clts;
	int k;
	int r = 0;
	
	clts = malloc(sizeof(struct Client)*MAX_CONNECTIONS);
	
	myclients.num_connected--; // decrementa o numero de clientes cadastrados
	// Realocando o array
	for (k = 0; k < myclients.num_connected; k++) {
		if (id == myclients.item[k].id) {
			r = 1;
		}
		if (r && k+r <= myclients.num_connected) {
			myclients.item[k].id = myclients.item[k+r].id;
			myclients.item[k].address = myclients.item[k+r].address;
			strcpy(myclients.item[k].name, myclients.item[k+r].name);
		}
	}
	//free(&myclients.item[myclients.num_connected]);
	
}

void sendok(int id, int sockfd, struct sockaddr_in addr) {
	/**
	 * Envia uma mensagem OK para um cliente segundo seu id
	 */
	Message okmsg;
	
	//constroe uma mensagem OK a ser enviada ao cliente
	okmsg.id = SERVER_ID;
	okmsg.type = OK;
	okmsg.to = id;
	strcpy(okmsg.text, "OK");
	
	psend(okmsg, sockfd, addr); // envia mensagem OK ao cliente
}

void register_client(int id, char *name, int sockfd, struct sockaddr_in addr) {
	/**
	 * Envia uma mensagem REG ao novo cliente informando sucesso no registro
	 */
	Message regmsg;
	Client *client;
	char strid[2];
	
	// constroe mensagem REG a ser enviada ao novo cliente
	regmsg.id = SERVER_ID;
	regmsg.type = REG;
	sprintf(strid, "%d", id);
	strcpy(regmsg.text, strid);
	strcpy(regmsg.name, name);
	printf("\n  Clients up: %d\n", myclients.num_connected);
	client = get_client(id); // adquire o cliente previamente cadastrado
	
	psend(regmsg, sockfd, addr); // envia mensagem REG ao novo cliente
}

void psend(Message msg, int sockfd, struct sockaddr_in addr) {
	/**
	 * Funcao que abstrae o sento
	 */
	int numbytes;
	size_t addr_len;
	printf("Sending back the following message to #%d: %s\n", msg.to, msg.text);
	addr_len = sizeof(addr);
    if ((numbytes = sendto(sockfd, &msg, sizeof(msg), 0,
             (struct sockaddr *)&addr, addr_len)) == -1) {
        perror("ERROR client: sendto, while trying to send message to the server");
        exit(1);
    }
}

Message receive(int sockfd, struct sockaddr_in *their_addr) {
	/**
	 * Funcao que abstrae o recv, retornado uma mensagem do tipo Message
	 */
	int numbytes = -1;
	Message msg;
	size_t addr_len;

	addr_len = sizeof(*their_addr);
	while (numbytes == -1) {
		numbytes = recvfrom(sockfd, &msg, MAXBUFLEN-1 , 0, (struct sockaddr *)&*their_addr, &addr_len);
		if (numbytes == -1) {
	        perror("recvfrom");
	        //exit(1);
		}
    }
    printf("answer (%d bytes, type: %d): %s \n", numbytes, msg.type, msg.text);
    printf("\n  Clients up: %d\n", myclients.num_connected);
    return msg;
}

