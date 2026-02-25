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
#include <sys/wait.h>
#include <unistd.h>

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

void buff_work(int csid, sockaddr_in *client) {
  constexpr int buflen = 256;
  char buf[buflen];
  char reply[buflen];
  size_t n;

  while (1) {
    n = recv(csid, buf, buflen, 0);
    if (n == 0)
      break;
    if (n > 0) {
      buf[n] = '\0';
    }

    printf("message: %s\n", buf);
    printf("client: ip: %s\n", inet_ntoa(client->sin_addr));
    printf("client: port: %d\n", ntohs(client->sin_port));

    transform(buf, reply, buflen);
    send(csid, reply, buflen, 0);

    printf("\n\n");
  }
}

void reaper(int sig) {
  int status;
  while (wait3(&status, WNOHANG, 0) >= 0) {
  }
}

int main(void) {
  socklen_t length;
  sockaddr_in addr;

  signal(SIGINT, cleanup);
  signal(SIGCHLD, reaper);

  main_socket = socket(AF_INET, SOCK_STREAM, 0);

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  bind(main_socket, (sockaddr *)&addr, sizeof(addr));
  length = sizeof(addr);
  getsockname(main_socket, (sockaddr *)&addr, &length);

  printf("Сервер: номер порта: %d\n", ntohs(addr.sin_port));

  listen(main_socket, 5);

  printf("Сервер запущен...\n\n");

  int client_sock;
  sockaddr_in client;
  length = sizeof(client);
  pid_t child;
  for (;;) {
    client_sock = accept(main_socket, (sockaddr *)&client, &length);

    child = fork();

    if (child == 0) {
      close(main_socket);

      buff_work(client_sock, &client);

      close(client_sock);
      exit(0);
    }

    close(client_sock);
  }

  return 0;
}
