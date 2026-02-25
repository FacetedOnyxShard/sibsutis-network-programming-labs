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

void transform(const char *buf, char *dest, const int buflen) {
  int number = atoi(buf);
  int tnumber = number * number;
  snprintf(dest, buflen, "%d", tnumber);
}

int main(void) {
  socklen_t length;
  sockaddr_in addr;
  sockaddr_in client;
  constexpr int buflen = 256;
  char buf[buflen];
  char reply[buflen];
  signal(SIGINT, cleanup);

  sid = socket(AF_INET, SOCK_DGRAM, 0);

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  bind(sid, (sockaddr *)&addr, sizeof(addr));
  length = sizeof(addr);
  getsockname(sid, (sockaddr *)&addr, &length);

  printf("Сервер: номер порта: %d\n", ntohs(addr.sin_port));

  printf("Сервер запущен...\n\n");
  length = sizeof(client);
  while (1) {
    size_t n = recvfrom(sid, buf, buflen, 0, (sockaddr *)&client, &length);
    buf[n] = '\0';

    printf("message: %s\n", buf);
    printf("client: ip: %s\n", inet_ntoa(client.sin_addr));
    printf("client: port: %d\n", ntohs(client.sin_port));

    transform(buf, reply, buflen);
    sendto(sid, reply, buflen, 0, (sockaddr *)&client, length);

    printf("\n\n");
  }

  close(sid);
  return 0;
}
