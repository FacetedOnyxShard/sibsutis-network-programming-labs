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
  signal(SIGINT, cleanup);

  if (argc < 4) {
    printf("usage: client <ip> <port> <i>\n");
    exit(0);
  }

  const char *server_ip = argv[1];
  int port = atoi(argv[2]);
  const char *msg = argv[3];

  sid = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  inet_pton(AF_INET, server_ip, &server.sin_addr);

  length = sizeof(server);
  int m = atoi(msg);
  for (int i = 0; i < 10; ++i) {
    sendto(sid, msg, strlen(msg), 0, (sockaddr *)&server, length);

    size_t n = recvfrom(sid, reply, buflen, 0, NULL, NULL);
    reply[n] = '\0';
    printf("Ответ: %s", reply);

    printf("\n\n");

    sleep(m);
  }

  close(sid);
  return 0;
}
