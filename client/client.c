#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/select.h>
#include <termios.h>
#include "client.h"

void
Client_send (int fd, const char *buf, int len)
{
  int sent_len = 0;
  int n;

  while (sent_len < len) {
    n = write(fd, buf + sent_len, len - sent_len);
    if (n < 0) {
      break;
    }
    sent_len += n;
  }

  if (n == -1) {
    fprintf(stderr, "Server_send: partial send\n");
  }
}

void
Client_sendf (int fd, const char *fmt, ...)
{
  va_list va;
  char buf[4096];
  int len;

  va_start(va, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, va);
  va_end(va);

  if (len >= (int)sizeof(buf)) {
    fprintf(stderr, "Server_sendf: string too long, truncated\n");
  }

  Client_send(fd, buf, len);
}

char* Server_extract (char texte[], int debut, char crct){
  int i = 0; 
  int pointeur;
  char *extract = NULL;
  extract = malloc(1024*sizeof(char));
  for (pointeur = debut ; texte[pointeur]!= crct ; pointeur++){
    extract[i]=texte[pointeur];
    i++;
  }
  return extract;
}

void Clt_create (int argc, char *argv[], struct pollfd *fds){
  int tosrv, n;
  struct sockaddr_in addr;
  struct hostent *host;
  char buf[1024];

  if (argc != 3) {
      printf("Usage: %s [host] port\n", argv[0]);
    }

    if ((tosrv = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

    if ((host = gethostbyname(argv[1])) == NULL) {
      herror("gethostbyname");
      exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = *((uint32_t *) host->h_addr);
    addr.sin_port = htons(atoi(argv[2]));

    if (connect(tosrv, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }

    fds[0].fd = tosrv;
    fds[0].events = POLLIN;
}

char* intToChar (char *coup){
  char* sCoup;
  sCoup = malloc(7*sizeof(char));
  if (strncmp (coup, "0", 1)){
    strncpy (sCoup, "PIERRE ", 7);
    return sCoup;
  }
  if (strncmp (coup, "1", 1)){
    strncpy (sCoup, "FEUILLE", 7);
    return sCoup;
  }
  if (strncmp (coup, "2", 1)){
    strncpy (sCoup, "CISEAUX", 7);
    return sCoup;
  }
  if (strncmp (coup, "3", 1)){
    strncpy (sCoup, "RIEN", 4);
    return sCoup;
  }
}

int testSaisie (char buf[]){
  if (strncmp (buf, "PIERRE", 6) == 0)
    return 1;
  if (strncmp (buf, "FEUILLE", 7) == 0)
    return 1;
  if (strncmp (buf, "CISEAUX", 7) == 0)
    return 1;
  if (strncmp (buf, "RIEN", 4) == 0)
    return 1;
  return 0;
}

int Clt_timeOut (int filedesc, char buf[]){
  fd_set set;
  struct timeval timeout;
  int rv, n;
  int len = 100;

  FD_ZERO(&set); 
  FD_SET(filedesc, &set); 

  timeout.tv_sec = 10;
  timeout.tv_usec = 0;
  rv = select(filedesc + 1, &set, NULL, NULL, &timeout);
  if(rv == -1)
    perror("select"); 
  else if(rv == 0){
    printf("timeout\n"); 
    strncpy (buf, "RIEN", 4);
    return 4; 
  }
  else{
    n = read( filedesc, buf, len ); 
    return n;
  }
}

void Client (int argc, char *argv[]){
  struct pollfd fds[1];
  Clt_create (argc, argv, fds);
  int i, n, finMatch, invalide;
  char myname[100];
  char buf[1024];


  printf("Choisissez un pseudo :\n");
  if ((n = read(STDIN_FILENO, buf, sizeof(buf))) < 0) {
    perror("read");
  }
  strncpy (myname, buf, n);
  Client_sendf (fds[0].fd, "Je suis %s\n", buf);
  finMatch = 0;
  poll(fds, 1, 15000);
  if ((n = read (fds[0].fd, buf, sizeof(buf))) < 0){
    perror ("read");
  }
  if (strncmp (buf, "Tu es bien connecte.", n) == 0){
    system ("clear");
    printf("En attente d'un deuxième joueur...\nTenez-vous prêt !\n");
        
    while (finMatch != 1){
      poll (fds, 1, 600000);
      read(fds[0].fd, buf, sizeof(buf));
      if (strncmp (buf, "C'est partie !", 14) == 0){
        invalide = 0;
        while (!testSaisie(buf)){
          if (invalide == 1)
            printf("Invalide !\n");
          printf("Choisissez un coup (PIERRE, FEUILLE ou CISEAUX)\n");
          //n = read (STDIN_FILENO, buf, sizeof(buf));
          n = Clt_timeOut (STDIN_FILENO, buf);
          invalide = 1;
        }
        Client_send (fds[0].fd, buf, n);
        poll (fds, 1, 15000);
        n = read (fds[0].fd, buf, sizeof(buf));
        if (strncmp (buf, "Score/coup", 10) == 0){
          system ("clear");
          Client_sendf (STDOUT_FILENO, "Votre adversaire à joué : %s\n%s : %s\n%s : %s\nTenez-vous prêt !\n",
          intToChar( Server_extract(buf, 15,'-')),myname,Server_extract(buf, 11,'-'),Server_extract(buf,17,'\n'),Server_extract(buf,13,'/'));
        }
      } 
      if (strncmp (buf, "Le gagnant est ", 13) == 0){
        Client_sendf (STDOUT_FILENO, "Le gagnant est %s !\n", Server_extract(buf, 15, '\n'));
        // printf("Voulez-vous rejouer ? (OUI - NON)\n");
        // read (STDIN_FILENO, buf, 3);
        // if (strncmp (buf, "OUI", 3) == 0)
        //   Client_send (fds[0].fd, "Je rejoue !", 11);
        // if (strncmp (buf, "NON", 3) == 0){
        //   Client_send (fds[0].fd, "Je ne rejoue pas !", 18);
        //   return 0;
        //}
        //finMatch = 1;
        finMatch = 1;
      }
    }
  }
}

