#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int sid;

void cleanup(int sig) {
  printf("\n\nЗавершение работы...\n");
  close(sid);
  exit(0);
}

int main(int argc, char const *argv[]) {
  socklen_t length;
  sockaddr_in addr;
  sockaddr_in server;
  constexpr int buflen = 256;
  char reply[buflen];
  hostent *hp;
  signal(SIGINT, cleanup);

  if (argc < 4) {
    printf("usage: client <ip> <port> <i>\n");
    exit(0);
  }

  hp = gethostbyname(argv[1]);
  int port = atoi(argv[2]);
  const char *msg = argv[3];

  sid = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  memcpy(&server.sin_addr, hp->h_addr_list[0], hp->h_length);
  server.sin_port = htons(port);

  length = sizeof(server);
  int m = atoi(msg);

  connect(sid, (sockaddr *)&server, sizeof(server));

  for (int i = 0; i < 10; ++i) {
    send(sid, msg, strlen(msg), 0);

    size_t n = recv(sid, reply, buflen, 0);
    reply[n] = '\0';
    printf("Ответ от сервера: %s", reply);

    printf("\n\n");

    sleep(m);
  }

  close(sid);
  exit(0);
  return 0;
}
