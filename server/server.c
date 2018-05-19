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
#include <sys/timeb.h>
#include <time.h>
#include "server.h"

int
Server_recv (int fd, char *buf, int len)
{
  int n;

  if ((n = read(fd, buf, len)) < 0) {
    perror("read");
  }

  return n;
}

void
Server_send (int fd, const char *buf, int len)
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
Server_sendf (int fd, const char *fmt, ...)
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

  Server_send(fd, buf, len);
}

void
Server_broadcast (struct pollfd *fds, const char *buf, int len)
{
  int i;

  for (i = 0; i < 2; i++) {
    Server_send(fds[i].fd, buf, len);
  }
}


void
Server_broadcastf (struct pollfd *fds, const char *fmt, ...)
{
  va_list va;
  char buf[4096];
  int len;

  va_start(va, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, va);
  va_end(va);

  if (len >= (int)sizeof(buf)) {
    fprintf(stderr, "Server_broadcastf: string too long, truncated\n");
  }

  Server_broadcast(fds, buf, len);
}

char* Srv_extract (char texte[], int debut, char crct){
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

int Srv_create (int argc, char *argv[]){
  int toclt, peer;
  struct sockaddr_in addr;
  struct pollfd fds[2];
  char buf[1024];			
  int n;

	if (argc != 2 ) {
    printf("Usage: %s [host] port\n", argv[0]);
	}

	if ((toclt = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
	 exit(1);
	}

  addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(atoi(argv[1]));

  if (bind(toclt, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
     perror("bind");
    exit(1);
  }

  if (listen(toclt, 8) < 0) {
     perror("listen");
    exit(1);
  }

  return toclt;
}

int Srv_accept (int fd){
	int peer;
	if ((peer = accept(fd, NULL, NULL)) < 0) {
    perror("accept");
    exit(1);
	}
	return peer;
}

void Srv_wait (int argc, char *argv[], struct pollfd *fds, Player* lesPlayer){
	int i, peer, n;
  char buf2[1024];
  char *buf;
	int toclt = Srv_create (argc, argv);
	for (i = 0; i < 2; i++)
	{
		peer = Srv_accept(toclt);
  	if (fds[i].events != POLLIN){
  		fds[i].fd = peer;
      fds[i].events = POLLIN;
      poll (fds + i, 1, 15000);
      if ((n = read (fds[i].fd, buf2, sizeof(buf2))) < 0)
        perror ("read");
      if ((strncmp (buf2, "Je suis ", 8)) == 0){
        Server_send (fds[i].fd, "Tu es bien connecte.", 20);
        buf = Srv_extract (buf2, 8, '\n');
        strncpy (lesPlayer[i].name, buf, sizeof(buf));
      }
    }
	}
}




