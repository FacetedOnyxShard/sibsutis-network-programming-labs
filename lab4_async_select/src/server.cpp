#include <algorithm>
#include <arpa/inet.h>
#include <cstddef>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

int main_socket;

void cleanup(int sig) {
  printf("\n\nЗавершение работы...\n");

  close(main_socket);

  exit(0);
}

void transform(const char *buf, char *dest, const int buflen) {
  int number = atoi(buf);
  int tnumber = number * number;
  snprintf(dest, buflen, "%d", tnumber);
}

int handler(int client_sock, sockaddr_in *client) {
  constexpr int buflen = 256;
  char buf[buflen];
  char reply[buflen];
  size_t n;

  n = recv(client_sock, buf, buflen, 0);
  if (n == 0) {
    return n;
  }
  if (n > 0) {
    buf[n] = '\0';
  }

  printf("message: %s\n", buf);
  printf("client: ip: %s\n", inet_ntoa(client->sin_addr));
  printf("client: port: %d\n", ntohs(client->sin_port));
  printf("\n\n");

  transform(buf, reply, buflen);
  send(client_sock, reply, buflen, 0);

  return n;
}

int main(void) {
  socklen_t length;
  sockaddr_in addr;

  signal(SIGINT, cleanup);

  main_socket = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(main_socket, (sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Связывание сервера неудачно");
    exit(1);
  }

  length = sizeof(addr);
  getsockname(main_socket, (sockaddr *)&addr, &length);

  printf("Сервер: номер порта: %d\n", ntohs(addr.sin_port));

  listen(main_socket, 5);

  printf("Сервер запущен...\n\n");

  int client_sock;
  sockaddr_in client;
  length = sizeof(client);
  pid_t child;

  fd_set rfds, afds;
  int nfds = FD_SETSIZE;

  FD_ZERO(&afds);
  FD_ZERO(&rfds);
  FD_SET(main_socket, &afds);
  int maxfd = main_socket;
  sockaddr_in clients[FD_SETSIZE];
  bzero((void *)clients, sizeof(sockaddr_in) * FD_SETSIZE);

  for (;;) {
    memcpy(&rfds, &afds, sizeof(rfds));

    if (select(maxfd + 1, &rfds, NULL, NULL, NULL) < 0) {
      perror("select()");
    }

    if (FD_ISSET(main_socket, &rfds)) {
      client_sock = accept(main_socket, (sockaddr *)&client, &length);
      memcpy(clients + client_sock, &client, sizeof(client));
      FD_SET(client_sock, &afds);
      maxfd = max(maxfd, client_sock);
    }

    for (int fd = 0; fd <= maxfd && fd < nfds; ++fd) {
      if (fd != main_socket && FD_ISSET(fd, &rfds)) {
        if (handler(fd, &clients[fd]) == 0) {
          close(fd);
          FD_CLR(fd, &afds);

          while (maxfd > main_socket && !FD_ISSET(maxfd, &afds)) {
            maxfd--;
          }
        }
      }
    }
  }
  return 0;
}
