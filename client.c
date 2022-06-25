#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define BUFSZ 1024

int flagRemove = 0;


struct client_data {
    int csock;
};

void handleRES_REM(char IdOrigin[BUFSZ]){
	printf("Equipment 0%d removed\n",atoi(IdOrigin));
}

void handleREQ_ADD(char IdOrigin[BUFSZ]){
	char buf[BUFSZ];
	memset(buf,0,BUFSZ);
	sprintf(buf,"Equipment 0%d added",atoi(IdOrigin));
	puts(buf);
}


void handleRES_INF(char IdDest[BUFSZ],char Payload[BUFSZ]){
	char response[BUFSZ];
	memset(response, 0, BUFSZ);

	sprintf(response, "Value from %s: %s",IdDest, Payload);
	puts(response);
}

void handleRES_LIST(char buf[BUFSZ]){
	puts(buf);
}

void handleREQ_REM(char IdOrigin[BUFSZ], int s){
	char response[BUFSZ];
	memset(response, 0, BUFSZ);
	sprintf(response,"Successful removal");
	printf("%s\n",response);
	flagRemove = 1;
	close(s);
	exit(EXIT_SUCCESS);
}

void handleERROR(char IdDest[BUFSZ], char Payload[BUFSZ],int s){
   char response[BUFSZ];
   memset(response, 0, BUFSZ);

   switch (atoi(Payload)){
	case 1:
		printf("Equipment not found\n");
		break;
	case 2:
		break;
	case 3:
		printf("Target equipment not found\n");
		break;
	case 4:
		printf("Equipment limit exceded\n");
		close(s);
		flagRemove = 1;
		break;
		//talvez tem q fechar a conexao
   }

}

void handleOK(char IdDest[BUFSZ], char Payload[BUFSZ]){
   char response[BUFSZ];
   memset(response, 0, BUFSZ);

	sprintf(response, "Successful removal");
   }

void handleRES_ADD(char Payload[BUFSZ]){
   char response[BUFSZ];
   memset(response, 0, BUFSZ);
   
   sprintf(response, "New ID: 0%s\n", Payload);
   printf("%s",response);

}

void handleResponse(char buf[BUFSZ], int csock){
	//separamos os parametros da mensagem, como explicado no PDF
	char *IdMsg = malloc(sizeof(char)*BUFSZ);
	char *IdOrigin = malloc(sizeof(char)*BUFSZ);
	char *IdDest = malloc(sizeof(char)*BUFSZ);
	char *Payload = malloc(sizeof(char)*BUFSZ);

	//separando os parametros
	IdMsg = strtok(buf, " ");
	IdOrigin = strtok(NULL, " ");
	IdDest = strtok(NULL, " ");
	Payload = strtok(NULL, "");


		switch (atoi(IdMsg)){
		{	
		case 1:
			handleREQ_ADD(IdOrigin);
			break;
		case 2:
			handleREQ_REM(IdOrigin,csock);
			break;
		case 3:
			handleRES_ADD(Payload);
			break;
		case 4:
			handleRES_LIST(Payload);
			break;
		case 5:
			printf("requested information\n");
			break;
		case 6:
			handleRES_INF(IdDest,Payload);
			break;
		case 7:
			handleERROR(IdDest, Payload,csock);
			//close(csock);
			break;
		case 8:	
			handleOK(IdDest, Payload);
			break;
		case 9:
			handleRES_REM(IdOrigin);
			break;
		default:
			break;
		}

	return;
}
}

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 1024


void * client_thread(void *data) {
    struct client_data *cdata = (struct client_data *)data;

	while(1){
		char buf[BUFSZ];
		memset(buf, 0, BUFSZ);
		recv(cdata->csock, buf, BUFSZ + 1, 0);
		if(strlen(buf)!= 0){
		handleResponse(buf,cdata->csock);
		}
	}
	//close(cdata->csock);
	exit(EXIT_SUCCESS);
}







int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}

	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);
	printf("connected to %s\n", addrstr);
	recv(s, buf, BUFSZ, 0);
	handleResponse(buf, s);
	memset(buf, 0, BUFSZ);
	// if(flagRemove == 1){
	// 	return 0;
	// }
	while(1) {
	memset(buf, 0, BUFSZ);
	struct client_data *cdata = malloc(sizeof(*cdata));
	if (!cdata) {
		logexit("malloc");
	}
	cdata->csock = s;

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);

	memset(buf, 0, BUFSZ);
	fgets(buf, BUFSZ-1, stdin);
	size_t count = send(s, buf, strlen(buf)+1, 0);
	if (count != strlen(buf)+1) {
		logexit("send");
	}

	}


	exit(EXIT_SUCCESS);
}