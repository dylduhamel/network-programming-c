/**
 * A simple stream socket server
 *
 * Dylan Duhamel {duhadm19@alumni.wfu.edu}
 * Feb. 28, 2024
 **/

#include <arpa/inet.h>
#include <asm-generic/socket.h>
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

#define PORT "4900" /* Port users will connect to */

#define BACKLOG 10 /* Number of pending connections in queue */

/* Signal handler function for sigaction sa_handler */
void sigchld_handler(int s)
{
  /* waitpid() might overwrite errno, so we save and restore it */
  int saved_errno = errno;

  /* Wait for child process to exit and return immedietly */
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) /* IPv4 */
  {
    /* Cast to sockaddr_in and get sin_addr */
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  else
  {
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
  }
}

int main(void)
{
  int sockfd, newfd; /* Listen on sockfd, new connection on newfd */
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; /* Address info of connection */
  socklen_t sin_size;
  struct sigaction act;
  int yes = 1;
  char s[INET6_ADDRSTRLEN]; /* Hold the IP address in after ntop is called */
  int rv;

  /* Zero out hints and add our specifics */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; /* Use IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; /* Use local machine IP */

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  /* Loop through linked list of results and bind to first applicable */
  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    /* Create a socket; continue if error */
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      perror("server: socket");
      continue;
    }
    /* Set socketopt so we can reuse port */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
      perror("setsocketopt");
      exit(1);
    }

    /* Bind to a port */
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(sockfd); /* Because we will retry the next fd in the linked list */
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo); /* p now holds valid info */

  /* Additional validation */
  if (p == NULL)
  {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1)
  {
    perror("listen");
    exit(1);
  }

  act.sa_handler = sigchld_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &act, NULL) == -1)
  {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  /* Main accept loop */
  while (1)
  {
    sin_size = sizeof(their_addr);
    if ((newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) ==
        -1)
    {
      perror("accept");
      continue; /* Block until next incoming connection */
    }

    /* Convert from binary address to printable */
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof(s));
    printf("server: got connection from %s\n", s);

    /* Create child process to send message */
    if (!fork())
    {
      close(sockfd);
      if (send(newfd, "Hello, World!", 13, 0) == -1)
      {
        perror("send");
      }
      close(newfd);
      exit(0);
    }
    close(newfd); /* Parent closes newfd, msg sent */
  }

  return 0;
}
