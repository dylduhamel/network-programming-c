/*
 * showip.c -- show IP address for a given host on the command line
 * */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
  struct addrinfo hints, *res, *p;
  int status;
  char ipstr[INET6_ADDRSTRLEN];

  if (argc != 2)
  {
    fprintf(stderr, "USAGE: showip hostname");
    return 1;
  }

  /* Make sure addrinfo hints is initialized to all zeros */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; /* AF_INET or AF_INET6 to force IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* TCP socket type */

  if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 2;
  }

  printf("IP address for %s:\n\n", argv[1]);

  /* p is a pointer to struct addrinfo and res is a linked list result of
   * getaddrinfo */
  for (p = res; p != NULL; p = p->ai_next)
  {
    void *addr;
    char *ipver;

    /* Get the pointer to the address itsself, different fields in IPv4 and IPv6
     */
    if (p->ai_family == AF_INET)
    {
      /* Cast sockaddr to sockaddr_in which is for IPv4 IP only */
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      /* Dereference value of sockaddr_in ipv4.sin_addr */
      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
    }
    else
    {
      /* Cast sockaddr to sockaddr_in6 which is for IPv6 only */
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
    }

    /* Convert the IP into a string and print it */
    /* Network to print translation */
    inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
    printf("    %s: %s\n", ipver, ipstr);
  }

  freeaddrinfo(res);

  return 0;
}
