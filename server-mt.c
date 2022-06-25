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

void readEquipment(char buf[BUFSZ]){
    char *aux = malloc(sizeof(char)*BUFSZ);
    memset(aux, 0, BUFSZ);
    float value = (float)rand()/(float)(RAND_MAX/10);
    aux = strtok(buf, " ");
    aux = strtok(NULL, " ");
    aux = strtok(NULL, " ");
    aux = strtok(NULL, " ");
    

    //verificar se o equipamento existe
    if(Sockets[atoi(aux)-1] == 0){
        sprintf(buf,"Equipment 0%d not found\n",atoi(aux));
        puts(buf);
        sprintf(buf, "7 - - 3");

    }
    else{
        sprintf(buf,"6 - 0%d %.2f",atoi(aux),value);
    }
}

void listEquipments(char buf[BUFSZ],int IdEquip){
    memset(buf, 0, BUFSZ);
    char aux[BUFSZ];
    int NumEquips = 0;
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(Sockets[i] != 0 && (i+1) != IdEquip){
            memset(aux, 0, BUFSZ);
            sprintf(aux, "0%d ", i+1);
            strcat(buf, aux);
            NumEquips++;
        }
        }


    memset(aux, 0, BUFSZ);
    if (!NumEquips){
        sprintf(buf,"7 - - 1");
    }
    else{
    strncpy(aux, buf, strlen(buf) - 1);
    sprintf(buf,"4 - - %s",aux);
    }
}

void handleBuf(char buf[BUFSZ], int IdEquip){
    //comparamos os inputs
    char aux[BUFSZ];
    memset(aux, 0, BUFSZ);
    strcpy(aux, buf);
    strtok(aux, " ");
    if(strcmp(buf,"list equipment\n") == 0){
        listEquipments(buf,IdEquip);
    }
    else if(strcmp(aux,"request") == 0){
        readEquipment(buf);
    }

}

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
   int equipTotais = 0;
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
    sprintf(response, "3 - - %d", NumEquip);
    send(csock, response, strlen(response) + 1, 0);
    sprintf(buf,"Equipment 0%d added", NumEquip);
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
    //BroadcastNewEquipment(IdEquipamento,buf);
    
    while(1){
    memset(buf, 0, BUFSZ);
    printf("[log] waiting for data\n");	
    //Guarda em buf o que o cliente enviou
    size_t count = recv(Sockets[IdEquipamento-1], buf, BUFSZ - 1, 0);
    printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

    //Se o cliente enviar um comando de sair, fechamos o socket
    if(strcmp(buf,"close connection\n") == 0){
        printf("Equipment 0%d removed\n",IdEquipamento);
        sprintf(response, "2 %d - -", IdEquipamento);	
        send(Sockets[IdEquipamento-1], response, strlen(response) + 1, 0);
        close(Sockets[IdEquipamento-1]);
        Sockets[IdEquipamento-1] = 0;
        return;
    }

    // if(strcmp(buf,"caio\n") == 0){
    //     sprintf(buf, "7 - - 3");
    //     send(Sockets[IdEquipamento], buf, strlen(buf) + 1, 0);
    //     }
    //Caso contrário chamaos a funçõ para tratar a entrada do cliente
    handleBuf(buf,IdEquipamento);
    //puts(buf);
    count = send(Sockets[IdEquipamento-1], buf, strlen(buf) + 1, 0);
    if (count != strlen(buf) + 1) {
        logexit("send");
    }
    }
    close(cdata->csock);

    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argc, argv);
    }
    const char* IpVersion = "v4";
    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(IpVersion, argv[1], &storage)) {
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
