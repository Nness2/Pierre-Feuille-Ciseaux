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
#include "game.h"

#define SCORE_FOR_WIN 3

enum { PIERRE,
      FEUILLE,
      CISEAUX,
      RIEN
      } choix;

int Game_compare (int coup1, int coup2){
  if (coup1 == PIERRE && coup2 == FEUILLE)
    return -1;
  if (coup1 == PIERRE && coup2 == CISEAUX)
    return 1;
  if (coup1 == PIERRE && coup2 == PIERRE)
    return 0;
  if (coup1 == PIERRE && coup2 == RIEN)
    return 1;

  if (coup1 == FEUILLE && coup2 == CISEAUX)
    return -1;
  if (coup1 == FEUILLE && coup2 == PIERRE)
    return 1;
  if (coup1 == FEUILLE && coup2 == FEUILLE)
    return 0;
  if (coup1 == FEUILLE && coup2 == RIEN)
    return 1;

  if (coup1 == CISEAUX && coup2 == PIERRE)
    return -1;
  if (coup1 == CISEAUX && coup2 == FEUILLE)
    return 1;
  if (coup1 == CISEAUX && coup2 == CISEAUX)
    return 0;
  if (coup1 == CISEAUX && coup2 == RIEN)
    return 1;

  if (coup1 == RIEN && coup2 == PIERRE)
    return -1;
  if (coup1 == RIEN && coup2 == FEUILLE)
    return -1;
  if (coup1 == RIEN && coup2 == CISEAUX)
    return -1;
  if (coup1 == RIEN && coup2 == RIEN)
    return 0;

  return -2;
}

int Game_attribue (char buf[]){
  if (strncmp(buf, "PIERRE", 6) == 0)
    return PIERRE;
  if (strncmp(buf, "FEUILLE", 7) == 0)
    return FEUILLE;
  if (strncmp(buf, "CISEAUX", 7) == 0)
    return CISEAUX;
  if (strncmp(buf, "RIEN", 4) == 0)
    return CISEAUX;

  return -1;
}

void Game_round (struct pollfd *fds, Player *lesPlayer){
  int tempo, util, res, i, n;
  tempo = 20000;
  int utilitaire[3] = {0,0,0};
  char buf[1024];
  struct timeb t1, t2;

    if(poll(fds,1,1) > 0)
    read(fds[0].fd,buf,1);
    if(poll(fds+1,1,1) > 0)
    read(fds[1].fd,buf,1);

  for (i = 2 ; i > 0 ; i--){
    ftime(&t1);
    if ((n = poll(fds + utilitaire[0], i, tempo)) < 0) {
      perror("poll");
      exit (1);
    }
    if (fds[0].revents & POLLIN && utilitaire[1] != 1){
      read (fds[0].fd, buf, sizeof(buf));
      lesPlayer[0].coup = Game_attribue(buf);
      utilitaire[0] = 1;
      utilitaire[1] = 1;
    }
    if (fds[1].revents & POLLIN && utilitaire[2] != 1){
      read (fds[1].fd, buf, sizeof(buf));
      lesPlayer[1].coup = Game_attribue(buf);
      utilitaire[2] = 1;
    }
    ftime(&t2);
    tempo -= (t2.time + t2.millitm) - (t1.time + t1.millitm);
  }
  // if (utilitaire[1] == 0){
  //   lesPlayer[0].coup = Game_attribue(buf);
  // }
  // if (utilitaire[2] == 0){
  //   lesPlayer[0].coup = Game_attribue(buf);
  // }

  res = Game_compare(lesPlayer[0].coup, lesPlayer[1].coup);
  if (res == 1)
    lesPlayer[0].score++;
  if (res == -1)
    lesPlayer[1].score++;
  Server_sendf (fds[0].fd, "Score/coup %d-%d/%d-%s\n", 
  lesPlayer[0].score, lesPlayer[1].score, lesPlayer[1].coup, lesPlayer[1].name);
  Server_send (STDOUT_FILENO, "Score/coup\n", 10);
  Server_sendf (fds[1].fd, "Score/coup %d-%d/%d-%s\n", 
  lesPlayer[1].score, lesPlayer[0].score, lesPlayer[0].coup, lesPlayer[0].name);
}

//Non utilisé
void Game_rejouezVous (struct pollfd *fds){
  int tempo, util, i, n;
  tempo = 10000;
  int utilitaire[3] = {0,0,0};
  char buf[1024];
  struct timeb t1, t2;


  for (i = 2 ; i > 0 ; i--){
      ftime(&t1);
      if ((n = poll(fds + utilitaire[0], i, tempo)) < 0) {
        perror("poll");
        exit (1);
      }
      if (fds[0].revents & POLLIN && utilitaire[1] != 1){
        read (fds[0].fd, buf, sizeof(buf));
        if (strncmp (buf, "Je rejoue !", 11) == 0){
          Server_send (fds[0].fd, "Tu es bien connecte.", 20);
          Server_send (STDOUT_FILENO, "Tu es bien connecte.\n", 20);
        }
        if (strncmp (buf, "Je ne rejoue pas !", 18) == 0){
          close(fds[0].fd);
          fds[0].events == 0;
          //lesPlayer[0].name = "";
        }
        utilitaire[0] = 1;
        utilitaire[1] = 1;
      }
      if (fds[1].revents & POLLIN && utilitaire[2] != 1){
        read (fds[1].fd, buf, sizeof(buf));
        if (strncmp (buf, "Je rejoue !", 11) == 0){
          Server_send (fds[1].fd, "Tu es bien connecte.", 20);
          Server_send (STDOUT_FILENO, "Tu es bien connecte.\n", 20);
        }
        if (strncmp (buf, "Je ne rejoue pas !", 18) == 0){
          close(fds[1].fd);
          fds[1].events == 0;
          //lesPlayer[1].name = "";
        }
        utilitaire[2] = 1;
      }
      ftime(&t2);
      tempo -= (t2.time + t2.millitm) - (t1.time + t1.millitm);
    } 
    if (utilitaire[1] == 0){
      close(fds[0].fd);
      fds[0].events == 0;
    }
    if (utilitaire[2] == 0){
      close(fds[1].fd);
      fds[1].events == 0;
    }

}


void Game_start (int argc, char *argv[]){
  int peer, n, i;
  char buf[1024];
  struct pollfd fds[3];
  Player lesPlayer[2];
  Srv_wait(argc, argv, fds, lesPlayer);
  while (1){
    lesPlayer[0].score = 0;
    lesPlayer[1].score = 0;

    while (lesPlayer[0].score < SCORE_FOR_WIN && lesPlayer[1].score < SCORE_FOR_WIN){
      sleep (1/4);
      Server_broadcast (fds, "C'est partie !", 14);
      Server_send (STDOUT_FILENO, "C'est partie\n", 13);
      Game_round(fds, lesPlayer);
      sleep (5);
    }
    if (lesPlayer[0].score == SCORE_FOR_WIN)
      Server_broadcastf (fds, "Le gagnant est %s\n", lesPlayer[0].name);
    
    if (lesPlayer[1].score == SCORE_FOR_WIN)
      Server_broadcastf (fds, "Le gagnant est %s\n", lesPlayer[1].name);

    //rejouez_vous(fds);
  }
}

