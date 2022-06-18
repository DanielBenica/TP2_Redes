#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define BUFSZ 1024

int flagRemove = 0;

void handleRES_LIST(char buf[BUFSZ]){
	puts(buf);
}

void handleREQ_REM(char IdOrigin[BUFSZ], int s){
	char response[BUFSZ];
	memset(response, 0, BUFSZ);
	close(s);
	sprintf(response,"Successful removal");
	printf("%s\n",response);
	flagRemove = 1;
}

void handleERROR(char IdDest[BUFSZ], char Payload[BUFSZ],int s){
   char response[BUFSZ];
   memset(response, 0, BUFSZ);

   switch (atoi(Payload)){
	case 1:
		break;
	case 2:
		break;
	case 3:
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
	Payload = strtok(NULL, " ");



		switch (atoi(IdMsg)){
		{	
		case 1:
			/* code */
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
			/* code */
			break;
		case 6:
			/* code */
			break;
		case 7:
			handleERROR(IdDest, Payload,csock);
			//close(csock);
			break;
		case 8:	
			handleOK(IdDest, Payload);
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
	if(flagRemove == 1){
		return 0;
	}
	while(1) {
	memset(buf, 0, BUFSZ);
	printf("mensagem> ");
	fgets(buf, BUFSZ-1, stdin);
	size_t count = send(s, buf, strlen(buf)+1, 0);
	if (count != strlen(buf)+1) {
		logexit("send");
	}

	memset(buf, 0, BUFSZ);
	unsigned total = 0;
		count = recv(s, buf + total, BUFSZ - total, 0);
		if (count == 0) {
			// Connection terminated.
			break;
		}
		total += count;
	handleResponse(buf,s);
	if(flagRemove == 1){
		break;
	}
	}
	close(s);

	exit(EXIT_SUCCESS);
}