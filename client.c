#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>

int port = 2024;

int main()
{
    int sd;
    struct sockaddr_in server;

    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror("Eroare la socket().\n");
      exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(port);

    if((connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr))) == -1)
    {
       perror("[CLIENT]Eroare la connect().\n");
       exit(EXIT_FAILURE);
    }

    while(1)
    {
       char comanda[1000] = "";
       char raspuns[1000] = "";
       bzero(comanda, sizeof(comanda));
       printf("[CLIENT]Introduceti comanda:");
       fflush(stdout);
       read(0, comanda, sizeof(comanda));
       comanda[strlen(comanda)]='\0';

       if((write(sd, &comanda, sizeof(comanda))) <=0)
       {
          perror("[CLIENT]Eroare la write() spre server.\n");
          exit(EXIT_FAILURE);
       }

       if((read(sd, &raspuns, sizeof(raspuns))) <=0)
       {
          perror("[CLIENT]Eroare la read() de la server.\n");
          exit(EXIT_FAILURE);
       }

       raspuns[strlen(raspuns)] = '\0';
       printf("[CLIENT]Rezultatul primit este: %s\n",raspuns);

    }

    close(sd);
}
