/* Dylan Duhamel
* duhadm19@alumni.wfu.edu
*/

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
  struct addrinfo hints, *res;
  int sockfd;
  int status_getaddrinfo, status_bind;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; /* Use IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; /* Fill in IP of local machine for me */

  if ((status_getaddrinfo = getaddrinfo(NULL, "3490", &hints, &res)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status_getaddrinfo));
    return 1;
  }

  /* Make a socket */
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if ((status_bind = bind(sockfd, res->ai_addr, res->ai_addrlen)) == 0)
  {
    printf("Successfully bound to port\n");
  }
  else
  {
    fprintf(stderr, "bind: %s\n", gai_strerror(status_bind));
    return 2;
  }

  return 0;
}
