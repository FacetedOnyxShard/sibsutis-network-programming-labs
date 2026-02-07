#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

#define BUFLEN 256

int sid;

// client ip port i
int client_program(int argc, char const *argv[]) {
  int sid;
  sockaddr_in servAddr, clientAddr;
  socklen_t servAddr_len;
  int send_message_count = 100;

  if (argc < 4) {
    printf("usage: client <ip> <port> <i>\n");
    return -1;
  }

  const char *server_ip = argv[1];
  int port = atoi(argv[2]);
  const char *message = argv[3];

  sid = socket(AF_INET, SOCK_DGRAM, 0);

  bzero((char *)&servAddr, sizeof(servAddr));

  servAddr.sin_family = AF_INET;
  servAddr.sin_port = port;

  inet_pton(AF_INET, server_ip, &servAddr.sin_addr);

  servAddr_len = sizeof(servAddr);

  sendto(sid, message, strlen(message), 0, (sockaddr *)&servAddr,
         sizeof(servAddr));
  // for (int i = 0; i < send_message_count; ++i) {

  // }

  close(sid);
  return 0;
}

int main(int argc, char const *argv[]) {
  client_program(argc, argv);
  return 0;
}
