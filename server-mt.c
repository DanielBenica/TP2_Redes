#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define MAX_CLIENTS 15  
int Equipamentos[15];
//Vetor de sockets
int Sockets[MAX_CLIENTS] = {0};

void BroadcastNewEquipment(int IdEquipment, char buf[BUFSZ]){
    int i;    
    for(i = 0; i < MAX_CLIENTS; i++){
        if(Sockets[i] != 0){
            send(Sockets[i],buf,strlen(buf)+1,0);
        }
}
}

int addEquipment(char buf[BUFSZ],int csock){
   //Verificamos o numero total de equipamentos
   char response[BUFSZ];
   memset(response, 0, BUFSZ);
   int equipTotais = 14;
   for(int i = 0; i < MAX_CLIENTS; i++){
      if(Sockets[i] != 0){
        equipTotais ++;
      }
   }
    //Se o numero de equipamentos for 15 retornamos 0
    if(equipTotais == 15){
      sprintf(response, "7 - - 4");
      send(csock, response, strlen(response) + 1, 0);
      return 0;
    }

    //fazer o tratamento do ID aqui dentro
   int NumEquip = 0;
    //cria a novo id para o equipamento
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(Sockets[i] == 0){
            NumEquip = i+1;
            Sockets[i] = csock;
            break;
        }
    }
    // coloca o ID do equipamento no buffer e an resposta
    sprintf(response, "8 - %d 2", NumEquip);
    send(csock, response, strlen(response) + 1, 0);
    sprintf(buf,"Equipament 0%d added", NumEquip);
    return NumEquip;
}


void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

struct client_data {
    int csock;
    struct sockaddr_storage storage;
};

void * client_thread(void *data) {
    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    int IdEquipamento = 0;
    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    //printf("[log] connection from %s\n", caddrstr);

    //Buf será nosso input do cliente
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    //Response será nossa resposta ao cliente
    char response[BUFSZ];
    memset(response, 0, BUFSZ);
    //função para adicionar
    IdEquipamento = addEquipment(buf,cdata->csock);
    puts(buf);
    if(!IdEquipamento){
       close(Sockets[IdEquipamento-1]);
       return;
    }
    //Implementar a logica de broadcast aqui
    BroadcastNewEquipment(IdEquipamento,buf);
    //colocar um send com e evniar a resposta para o cliente
    //send(cdata->csock, response, strlen(response) + 1, 0);
    while(1){
    memset(buf, 0, BUFSZ);
    printf("[log] waiting for data\n");	
    //Guarda em buf o que o cliente enviou
    size_t count = recv(cdata->csock, buf, BUFSZ - 1, 0);
    printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);
   
    puts(buf);
    count = send(cdata->csock, buf, strlen(buf) + 1, 0);
    if (count != strlen(buf) + 1) {
        logexit("send");
    }
    }
    close(cdata->csock);

    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    //zeramos o vetor de equipamentos
    memset(Equipamentos, 0, sizeof(Equipamentos));

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

	struct client_data *cdata = malloc(sizeof(*cdata));
	if (!cdata) {
		logexit("malloc");
	}
	cdata->csock = csock;
	memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);
    }

    exit(EXIT_SUCCESS);
}
