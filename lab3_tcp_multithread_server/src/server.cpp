#include <arpa/inet.h>
#include <errno.h>
#include <fstream>
#include <iostream>
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

using namespace std;

int main_socket;
FILE *file;
pthread_mutex_t st_mutex;
pthread_attr_t ta;

void cleanup(int sig) {
  printf("\n\nЗавершение работы...\n");

  close(main_socket);
  fclose(file);

  pthread_attr_destroy(&ta);
  pthread_mutex_destroy(&st_mutex);

  exit(0);
}

void transform(const char *buf, char *dest, const int buflen) {
  int number = atoi(buf);
  int tnumber = number * number;
  snprintf(dest, buflen, "%d", tnumber);
}

typedef struct {
  int csid;
  sockaddr_in client;
} worker_attributes_t;

void *buff_work(void *args) {
  worker_attributes_t *attributes = (worker_attributes_t *)args;
  int csid = attributes->csid;
  sockaddr_in client = attributes->client;
  delete attributes;

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

    pthread_mutex_lock(&st_mutex);
    fprintf(file, "message: %s\n", buf);
    fprintf(file, "client: ip: %s\n", inet_ntoa(client.sin_addr));
    fprintf(file, "client: port: %d\n", ntohs(client.sin_port));
    fprintf(file, "\n\n");

    transform(buf, reply, buflen);
    send(csid, reply, buflen, 0);
    pthread_mutex_unlock(&st_mutex);
  }
  close(csid);

  return NULL;
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

  string filename = "../data/client_requests.txt";
  file = fopen(filename.c_str(), "w");
  file = fopen(filename.c_str(), "a");

  pthread_t thread;
  pthread_attr_init(&ta);
  pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
  pthread_mutex_init(&st_mutex, NULL);

  for (;;) {
    if (file == nullptr) {
      cleanup(0);
    }

    client_sock = accept(main_socket, (sockaddr *)&client, &length);

    worker_attributes_t *attr = new worker_attributes_t;
    attr->csid = client_sock;
    attr->client = client;

    pthread_create(&thread, NULL, buff_work, (void *)attr);
  }
  return 0;
}
