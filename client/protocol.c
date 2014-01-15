#include "protocol.h"

void start_connection(int sockfd, struct addrinfo *p) {
	/**
	 * Inicializa uma conexao com o servidor, enviado uma mensagem OI
	 * e esperando receber uma mensagem REG de registro realizado com sucesso
	 */
	Message msg;
	Message recmsg;
	char name[100];
	int namelen;
	
	printf("Please type your nickname: "); // pede ao usuario que forneca um nome
	fgets (name, 100, stdin);
	
	// abaixo constroe uma mensagem OI a ser enviada ao servidor
	namelen = strlen(name);
	strncpy(MY_NAME, name, namelen-1);
	msg.type = OI;
	strcpy(msg.text, "");
	strcpy(msg.name, MY_NAME);
	
	psend(msg, sockfd, p); // envia a mensagem OI
	printf("Waiting for registration...\n");
	
	// recebe resposta REG do servidor, para o registro
	recmsg = receive(sockfd, "Could not start connection to the server!", 1);
	
	if (recmsg.type != REG || recmsg.id != SERVER_ID) {
		// se nao recebeu resposta REG, o programa acaba
		printf("   Connection not estabilished...\n\n");
		exit(1);
	}
	
	MY_ID = atoi(recmsg.text); // define um id para o cliente fornecido pelo servidor
	
	printf("\n Registration completed successfully! \n");
	printf("\n   ** To send private messages please type: %sID%s before the message): ", PRIVATE_CALL_BEGIN, PRIVATE_CALL_END);
}

void run_chat(int sockfd, struct addrinfo *p) {
	/**
	 * Executa o chat, solicitando ao usuario que forneca mensagens de texto.
	 * As mensagens sao enviadas ao servidor como mensagens MSG
	 */
	Message msg;
	char usermsg[200];
	int privid;

	while (1) {
		// constroe mensagem MSG
		msg.id = MY_ID;
		msg.type = MSG;
		msg.to = 0;
		strcpy(msg.name, MY_NAME);
		
		printf("\n * #%d: %s: ", MY_ID, MY_NAME);
		
		fgets(usermsg, 200, stdin); // usuario precisa digitar algum texto
		
		strcpy(msg.text, usermsg);
		
		privid = is_private_message(msg); // verifica se e' uma mensagem privada a ser enviada a um unico usuario especifico
		if (privid) {
			// se for privada especifica o id do destino a ser enviado na propriedade to
			msg.to = privid;
			strcpy(msg.text, get_private_message(msg)); // adquire o texto da mensagem privada e o copia
			printf("  ...sending in private\n");
		}
		ISOK = 0;
		psend(msg, sockfd, p); // envia a mensagem ao destino especico ou a todos se o .to = 0
	}
}

void *receive_messages(void *threadarg) {
	/**
	 * Void que roda em paralelo para receber mensagens do servidor
	 */
	int sockfd;
	int ok;
	struct addrinfo *p;
   	struct thread_data_params *data;
   	Message recmsg;
   	
   	// extrai os parametros da estrutura da thread
   	data = (struct thread_data_params *) threadarg;
   	p = data->p;
    sockfd = data->sockfd;
	
	while (1) {
		recmsg = receive(sockfd, "Error ocurred while trying to receive messages!", 0);
		ok = is_ok(recmsg);
		ISOK = ok; // seta a variavel para 1 ou 0 dependendo se a mensagem recebida foi OK
	}
	pthread_exit(NULL);
}

int is_ok(Message msg) {
	/**
	 * Funcao que verifica se a mensagem e' OK e imprime na tela do exibidor as mensagens
	 */
	int r = 0;
	switch (msg.type) {
		case MSG:
			printf("\n   #%d: %s said: %s", msg.id, msg.name, msg.text);
			break; 
		case OK:
			r = 1;
			break;
		case ERRO:
			printf("\n   #%d: %s ERROR: %s", msg.id, msg.name, msg.text);
			break;			
	}
	return r;
}

void psend(Message msg, int sockfd, struct addrinfo *p) {
	/**
	 * Pequena abstracao para o sento
	 */
	int numbytes;
    if ((numbytes = sendto(sockfd, &msg, sizeof(msg), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("\n  ERROR client: sendto, while trying to send message to the server\n");
        exit(1);
    }
}

Message receive(int sockfd, char *errstr, int verbose) {
	/**
	 * Recebe uma mensagem do servidor e a retorna
	 */
	int numbytes;
	int tries = TIMEOUT_TRIES;
	Message msg;
	Message errmsg;
	size_t addr_len;
	struct sockaddr_in their_addr;
	addr_len = sizeof their_addr;
	numbytes = -1;
	while (numbytes == -1 && tries) { // se ocorrer algum erro, pode haver mais tentativas
		numbytes = recvfrom(sockfd, &msg, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
		if (numbytes == -1 && verbose) { 
			// Caso a funcao seja chamada como verbose, mostra na tela do
			// exibidor mensagens de erro e envia uma mensagem de volta
			// ao servidor notificando o erro
			
			// constroe mensagem de erro 
			errmsg.type = ERRO;
			errmsg.id = MY_ID;
			strcpy(errmsg.name, MY_NAME);
			strcpy(errmsg.text, errmsg.text);
			
			psend(errmsg, sockfd, p); // envia mensagem de erro ao servidor
			tries--;
			perror("recvfrom");
			printf("%s  Trying to receive again...\n\n", errstr);
		}
    }
    
    if (!tries) {
    	return errmsg;	
    }

    return msg;
}

int startswith(char *text, char *str) {
	/**
	 * Funcao que procura no comeco de uma string a ocorrencia de uma substring
	 */
	int len = strlen(str);
	int mylen = strlen(text);
	char *sub;
	sub = malloc(sizeof(char)*len);
	if (mylen > len) {
		strncpy(sub, text, len);
		if (strcmp(sub, str)) {
			return 1;
		}	
	}
	return 0;
}

int is_private_message(Message msg) {
	/**
	 * Verifica se a mensagem e' privada ou nao
	 */
	int len = strlen(msg.text);
	int k = 0;
	char *strid;
	char o[10];
	strid = malloc(sizeof(char)*len);
	sprintf(o, "%c", msg.text[0]);
	
	// a partir dai verifica os caracteres que identificam uma mensagem privada
	// tais como PRIVATE_CALL_BEGIN (inicio) e PRIVATE_CALL_END (fim)
	// Ex.: PRIVATE_CALL_BEGIN = #
	//      PRIVATE_CALL_END = :
	//      #id:
	//      seja um id 27, entao #27:
	
	if (len && strcmp(o, PRIVATE_CALL_BEGIN) == 0) {
		for (k = 1; k < len; k++) {
			sprintf(o, "%c", msg.text[k]);
			if (!isdigit(msg.text[k]) && strcmp(o, PRIVATE_CALL_END) != 0) {
				return 0;
			} else {
				if (strcmp(o, PRIVATE_CALL_END)) {
					strid[k-1] = msg.text[k];
				} else {
					strid[k-1] = '\0';
					break;	
				}
			}
		}
	}
	if (strlen(strid)) {
		return atoi(strid); // retorn o id inteiro
	} 
	return 0;
}

char *get_private_message(Message msg) {
	/**
	 * Adquire a mensagem real de uma mensagem privado, retirando o #id:
	 */
	int len = strlen(msg.text);
	int k = 0;
	char *strid;
	char o[10];
	char *res;
	strid = malloc(sizeof(char)*len);
	res = malloc(sizeof(char)*len);
	sprintf(o, "%c", msg.text[0]);
	// Verifica os caracteres que identificam uma mensagem privada e o retira
	if (len && strcmp(o, PRIVATE_CALL_BEGIN) == 0) {
		for (k = 1; k < len; k++) {
			sprintf(o, "%c", msg.text[k]);
			if (!isdigit(msg.text[k]) && strcmp(o, PRIVATE_CALL_END) != 0) {
				return 0;
			} else {
				if (strcmp(o, PRIVATE_CALL_END)) {
					strid[k-1] = msg.text[k];
				} else {
					strid[k-1] = '\0';
					break;	
				}
			}
		}
	}
	strncpy(res, msg.text + strlen(strid)+2, len);
	return res;
}

void terminate(int sig) {
	/**
	 * Funcao chamada atraves de um signal determinado no programa principal
	 * quando o usuario pressiona a tecla CTRL+C para sair
	 */
	Message exitmsg;
	
	// constroe uma mensagem TCHAU a ser enviada para o servidor
	exitmsg.id = MY_ID;
	exitmsg.type = TCHAU;
	exitmsg.to = 0;
	strcpy(exitmsg.name, MY_NAME);
	strcpy(exitmsg.text, "");
	
	psend(exitmsg, sockfd, p); // Envia a mensagem TCHAU para o servidor
	printf("\n\n  Successfully ended...\n");
	close(sockfd); // fecha o socket
	(void) signal(SIGINT, SIG_DFL);
	exit(0); // termina o programa
}