#ifndef SERVER_H___
#define SERVER_H___


struct Player {
	char name[100];
	int score;
  int coup;
};
typedef struct Player Player;

void Srv_wait (int argc, char *argv[], struct pollfd* fds, Player* lesPlayer);

#endif