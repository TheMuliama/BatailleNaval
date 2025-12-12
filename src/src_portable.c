// client.c (Windows + Linux)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #define _WIN32_WINNT 0x0601
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef SOCKET socket_t;
  #define CLOSESOCK closesocket
  #define SLEEP_MS(ms) Sleep(ms)
  #include <windows.h>
#else
  #include <unistd.h>
  #include <errno.h>
  #include <arpa/inet.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  typedef int socket_t;
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR   (-1)
  #define CLOSESOCK close
  #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

#define BUFSZ 512
#define DEFAULT_PORT 30000

static void die(const char *msg) {
#ifdef _WIN32
  fprintf(stderr, "%s (WSA=%d)\n", msg, WSAGetLastError());
#else
  perror(msg);
#endif
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  const char *ip = (argc >= 2) ? argv[1] : "127.0.0.1";
  int port = (argc >= 3) ? atoi(argv[2]) : DEFAULT_PORT;

#ifdef _WIN32
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) die("WSAStartup");
#endif

  socket_t s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == INVALID_SOCKET) die("socket");

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((unsigned short)port);
  if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
    fprintf(stderr, "IP invalide: %s\n", ip);
    exit(EXIT_FAILURE);
  }

  if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) die("connect");

  printf("Client connecté à %s:%d\n", ip, port);

  char buf[BUFSZ];
  for (;;) {
    const char *msg = "Hello serveur (message du client)";
    int n = send(s, msg, (int)strlen(msg), 0);
    if (n == SOCKET_ERROR) die("send");

    int r = recv(s, buf, BUFSZ - 1, 0);
    if (r == 0) { printf("Serveur fermé.\n"); break; }
    if (r == SOCKET_ERROR) die("recv");

    buf[r] = '\0';
    printf("Réponse serveur: %s\n", buf);

    SLEEP_MS(1000);
  }

  CLOSESOCK(s);
#ifdef _WIN32
  WSACleanup();
#endif
  return 0;
}
