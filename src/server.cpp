#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

#define BUFLEN 256

volatile sig_atomic_t server_running = 1;
int sid;

void close_server_sid(int sig) {
  if (sig == SIGINT) {
    server_running = 0;
    close(sid);
  }
}

int transform(int n) { return n * n; }

void print_available_ips() {
  cout << "Available network interfaces:\n";

  ifaddrs *ifaddr;
  getifaddrs(&ifaddr);

  for (ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL) { // пропускает интерфейсы без адреса
      continue;
    }

    if (ifa->ifa_addr->sa_family != AF_INET) {
      continue;
    }

    sockaddr_in *addr = (sockaddr_in *)ifa->ifa_addr;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

    cout << "\t" << ifa->ifa_name << ": " << ip << '\n';
  }
}

int server_program() {
  int msgLength;
  sockaddr_in servAddr, clientAddr;
  socklen_t length;
  char buf[BUFLEN];

  sid = socket(AF_INET, SOCK_DGRAM, 0);

  bzero((char *)&servAddr, sizeof(servAddr));

  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = 0;

  bind(sid, (sockaddr *)&servAddr, sizeof(servAddr));

  length = sizeof(servAddr);
  getsockname(sid, (sockaddr *)&servAddr, &length);

  printf("SERVER: ip: %s\n", inet_ntoa(servAddr.sin_addr));
  printf("SERVER: port number: %d\n", ntohs(servAddr.sin_port));
  print_available_ips();
  fflush(stdout);

  // обработка завершения на ^C
  signal(SIGINT, close_server_sid);

  while (server_running) {
    length = sizeof(clientAddr);
    bzero(&buf, sizeof(buf));

    recvfrom(sid, buf, sizeof(buf), 0, (sockaddr *)&clientAddr, &length);

    printf("SERVER: client ip: %s\n", inet_ntoa(clientAddr.sin_addr));
    printf("SERVER: client port: %d\n", ntohs(clientAddr.sin_port));
    printf("SERVER: message: %d\n", ntohs(servAddr.sin_port));
    fflush(stdout);
  }

  close(sid);
  return 0;
}

int main(int argc, char const *argv[]) {
  server_program();
  return 0;
}
