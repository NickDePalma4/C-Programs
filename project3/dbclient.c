#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "msg.h"

#define BUF sizeof(struct msg)

void Usage(char *progname);

int LookupName(char *name,
                unsigned short port,
                struct sockaddr_storage *ret_addr,
                size_t *ret_addrlen);

int Connect(const struct sockaddr_storage *addr,
             const size_t addrlen,
             int *ret_fd);

void put(struct msg message, int socket_fd);
void get(struct msg message, int socket_fd);

int 
main(int argc, char **argv) {
  if (argc != 3) {
    Usage(argv[0]);
  }

  unsigned short port = 0;
  if (sscanf(argv[2], "%hu", &port) != 1) {
    Usage(argv[0]);
  }

  // Get an appropriate sockaddr structure.
  struct sockaddr_storage addr;
  size_t addrlen;
  if (!LookupName(argv[1], port, &addr, &addrlen)) {
    Usage(argv[0]);
  }

  // Connect to the remote host.
  int socket_fd;
  if (!Connect(&addr, addrlen, &socket_fd)) {
    Usage(argv[0]);
  }

  // Write something to the remote host.
  int flag = 1;
  while (flag) {
  struct msg message;
    printf("Enter 1 to put, 2 to get, 0 to quit: ");
	  scanf("%hhu", &message.type);
    getchar(); // remove the newline character
	  switch (message.type){
		  case PUT: 
			  put(message, socket_fd);
			  break;

		  case GET:
			  get(message, socket_fd);
			  break;

		  default:
			  flag = 0;
	  }
  }

  // Clean up.
  close(socket_fd);
  return EXIT_SUCCESS;
}

void Usage(char *progname) {
  printf("usage: %s  hostname port \n", progname);
  exit(EXIT_FAILURE);
}

int LookupName(char *name,
                unsigned short port,
                struct sockaddr_storage *ret_addr,
                size_t *ret_addrlen) {
  struct addrinfo hints, *results;
  int retval;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // Do the lookup by invoking getaddrinfo().
  if ((retval = getaddrinfo(name, NULL, &hints, &results)) != 0) {
    printf( "getaddrinfo failed: %s", gai_strerror(retval));
    return 0;
  }

  // Set the port in the first result.
  if (results->ai_family == AF_INET) {
    struct sockaddr_in *v4addr =
            (struct sockaddr_in *) (results->ai_addr);
    v4addr->sin_port = htons(port);
  } else if (results->ai_family == AF_INET6) {
    struct sockaddr_in6 *v6addr =
            (struct sockaddr_in6 *)(results->ai_addr);
    v6addr->sin6_port = htons(port);
  } else {
    printf("getaddrinfo failed to provide an IPv4 or IPv6 address \n");
    freeaddrinfo(results);
    return 0;
  }

  // Return the first result.
  assert(results != NULL);
  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);
  *ret_addrlen = results->ai_addrlen;

  // Clean up.
  freeaddrinfo(results);
  return 1;
}

int Connect(const struct sockaddr_storage *addr,
             const size_t addrlen,
             int *ret_fd) {
  // Create the socket.
  int socket_fd = socket(addr->ss_family, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("socket() failed: %s", strerror(errno));
    return 0;
  }

  // Connect the socket to the remote host.
  int res = connect(socket_fd,
                    (const struct sockaddr *)(addr),
                    addrlen);
  if (res == -1) {
    printf("connect() failed: %s", strerror(errno));
    return 0;
  }

  *ret_fd = socket_fd;
  return 1;
}

void put(struct msg message, int socket_fd) {
  struct msg sender = message;
  sender.type = PUT;
  struct msg receiver;

  printf("Enter the name: ");
  fgets(sender.rd.name, sizeof(sender.rd.name), stdin);
  int32_t length = strlen(sender.rd.name);
  if (length > 0 && sender.rd.name[length - 1] == '\n') {
    sender.rd.name[length - 1] = '\0';  // Remove the newline character
  }

  printf("Enter the id: ");
  scanf("%u", &sender.rd.id);
  getchar();  // remove the newline character

  int res;
  ssize_t total_sent = 0;
  while (total_sent < sizeof(sender)) {
    res = write(socket_fd, ((char *)&sender) + total_sent, sizeof(sender) - total_sent);
    if (res == -1) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      printf("socket write failure \n");
      close(socket_fd);
    }
    total_sent += res;
  }

  ssize_t total_received = 0;
  while (total_received < sizeof(receiver)) {
    res = read(socket_fd, ((char *)&receiver) + total_received, sizeof(receiver) - total_received);
    if (res == 0) {
      printf("socket closed prematurely \n");
      close(socket_fd);
    }
    if (res == -1) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      printf("socket read failure \n");
      close(socket_fd);
    }
    total_received += res;
  }

  if (receiver.type == FAIL) {
    printf("put failed \n");
  } else if (receiver.type == SUCCESS) {
    printf("put success \n");
  }
}

void get(struct msg message, int socket_fd) {
  struct msg sender = message;
  sender.type = GET;
  struct msg receiver;

  printf("Enter the id: ");
  scanf("%u", &sender.rd.id);
  getchar();  // remove the newline character

  int res;
  ssize_t total_sent = 0;
  while (total_sent < sizeof(sender)) {
    res = write(socket_fd, ((char *)&sender) + total_sent, sizeof(sender) - total_sent);
    if (res == -1) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      printf("socket write failure \n");
      close(socket_fd);
    }
    total_sent += res;
  }

  ssize_t total_received = 0;
  while (total_received < sizeof(receiver)) {
    res = read(socket_fd, ((char *)&receiver) + total_received, sizeof(receiver) - total_received);
    if (res == 0) {
      printf("socket closed prematurely \n");
      close(socket_fd);
    }
    if (res == -1) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      printf("socket read failure \n");
      close(socket_fd);
    }
    total_received += res;
  }

  if (receiver.type == FAIL) {
    printf("get failed \n");
  } else if (receiver.type == SUCCESS) {
    printf("name: %s \n", receiver.rd.name);
    printf("id: %u\n", receiver.rd.id);
  }
}
