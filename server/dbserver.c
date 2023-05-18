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
#include <fcntl.h> 
#include <pthread.h>

#include "msg.h"

#define BUF sizeof(struct msg)
#define DB "database"

struct handler_params {
    int client_fd;
    struct sockaddr *addr;
    size_t addrlen;
    int sock_family;
    int db_fd;
    struct msg message;
};

void Usage(char *progname);
void PrintOut(int fd, struct sockaddr *addr, size_t addrlen);
void PrintReverseDNS(struct sockaddr *addr, size_t addrlen);
void PrintServerSide(int client_fd, int sock_family);
int  Listen(char *portnum, int *sock_family);
void* HandleClient(void* arg);

struct msg putStudent(struct msg message, int server_fd);
struct msg getStudent(struct msg message, int server_fd);
int openDatabase();
struct handler_params* initializeHandler(int client_fd, struct sockaddr_storage addr, size_t addrlen, int sock_family);
void launchHandler(pthread_t* handler, struct handler_params* params);
char* printClientInfo(struct sockaddr *addr, size_t addrlen);
int checkBytesReceived(ssize_t bytes_received, char* name);
void checkBytesSent(ssize_t bytes_sent, char* name);

int numRecords = 0;

int main(int argc, char **argv) {
  // Expect the port number as a command line argument.
  if (argc != 2) {
    Usage(argv[0]);
  }

  int sock_family;
  int listen_fd = Listen(argv[1], &sock_family);
  if (listen_fd <= 0) {
    // We failed to bind/listen to a socket.  Quit with failure.
    printf("Couldn't bind to any addresses.\n");
    return EXIT_FAILURE;
  }

  // Loop forever, accepting a connection from a client and doing
  // an echo trick to it.
  struct handler_params* data = malloc(sizeof(struct handler_params));
  while (1) {
    struct sockaddr_storage caddr;
    socklen_t caddr_len = sizeof(caddr);
    int client_fd = accept(listen_fd, (struct sockaddr *)(&caddr), &caddr_len);
    if (client_fd < 0) {
      if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
        continue;
      printf("Failure on accept:%s \n ", strerror(errno));
      break;
    }

    pthread_t handler;
    data = initializeHandler(client_fd, caddr, caddr_len, sock_family);
    launchHandler(&handler, data);
  }
  // Close socket
  close(listen_fd);
  free(data);
  return EXIT_SUCCESS;
}

void* HandleClient(void* arg) {
  struct handler_params *params = (struct handler_params*) arg;
  int client_fd = params->client_fd;
  struct sockaddr *addr = params->addr;
  size_t addrlen = params->addrlen;
  
  int db_fd = openDatabase();
  if (db_fd == -1) {
    close(client_fd);
    return NULL;
  }

  ssize_t bytes_sent, bytes_received;
  
  // Print out information about the client.
  char* name = printClientInfo(addr, addrlen);
  while (1) {
  struct msg message;
    bytes_received = recv(client_fd, &message, BUF, 0);
    if (checkBytesReceived(bytes_received, name) != 1) {
      close(client_fd);
      return NULL;
    }
    if (message.type == PUT) {
      printf("type: put \n");
      printf("name: %s \n", message.rd.name);
      printf("id: %d \n", message.rd.id);
      message = putStudent(message, db_fd);
      bytes_sent = write(client_fd, &message, sizeof(struct msg));
      checkBytesSent(bytes_sent, name);
    } else if (message.type == GET) {
      printf("type: get \n");
      printf("id: %d \n", message.rd.id);
      message = getStudent(message, db_fd);
      bytes_sent = write(client_fd, &message, sizeof(struct msg));
      checkBytesSent(bytes_sent, name);
    } else {
      printf("Invalid message type\n");
      printf("type: %d \n", message.type);
      printf("name: %s \n", message.rd.name);
      printf("id: %d \n", message.rd.id);
    }
  }
}

void Usage(char *progname) {
  printf("usage: %s port \n", progname);
  exit(EXIT_FAILURE);
}

void PrintOut(int fd, struct sockaddr *addr, size_t addrlen) {
  printf("Socket [%d] is bound to: \n", fd);
  if (addr->sa_family == AF_INET) {
    // Print out the IPV4 address and port

    char astring[INET_ADDRSTRLEN];
    struct sockaddr_in *in4 = (struct sockaddr_in *)(addr);
    inet_ntop(AF_INET, &(in4->sin_addr), astring, INET_ADDRSTRLEN);
    printf(" IPv4 address %s", astring);
    printf(" and port %d\n\n", ntohs(in4->sin_port));

  } else if (addr->sa_family == AF_INET6) {
    // Print out the IPV6 address and port

    char astring[INET6_ADDRSTRLEN];
    struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)(addr);
    inet_ntop(AF_INET6, &(in6->sin6_addr), astring, INET6_ADDRSTRLEN);
    printf("IPv6 address %s", astring);
    printf(" and port %d\n\n", ntohs(in6->sin6_port));

  } else {
    printf(" ???? address and port ???? \n");
  }
}

void PrintReverseDNS(struct sockaddr *addr, size_t addrlen) {
  char hostname[1024];  // ought to be big enough.
  if (getnameinfo(addr, addrlen, hostname, 1024, NULL, 0, 0) != 0) {
    sprintf(hostname, "[reverse DNS failed]");
  }
  printf("DNS name: %s \n", hostname);
}

void PrintServerSide(int client_fd, int sock_family) {
  char hname[1024];
  hname[0] = '\0';

  printf("Server side interface is ");
  if (sock_family == AF_INET) {
    // The server is using an IPv4 address.
    struct sockaddr_in srvr;
    socklen_t srvrlen = sizeof(srvr);
    char addrbuf[INET_ADDRSTRLEN];
    getsockname(client_fd, (struct sockaddr *) &srvr, &srvrlen);
    inet_ntop(AF_INET, &srvr.sin_addr, addrbuf, INET_ADDRSTRLEN);
    printf("%s", addrbuf);
    // Get the server's dns name, or return it's IP address as
    // a substitute if the dns lookup fails.
    getnameinfo((const struct sockaddr *) &srvr,
                srvrlen, hname, 1024, NULL, 0, 0);
    printf(" [%s]\n\n", hname);
  } else {
    // The server is using an IPv6 address.
    struct sockaddr_in6 srvr;
    socklen_t srvrlen = sizeof(srvr);
    char addrbuf[INET6_ADDRSTRLEN];
    getsockname(client_fd, (struct sockaddr *) &srvr, &srvrlen);
    inet_ntop(AF_INET6, &srvr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
    printf("%s", addrbuf);
    // Get the server's dns name, or return it's IP address as
    // a substitute if the dns lookup fails.
    getnameinfo((const struct sockaddr *) &srvr,
                srvrlen, hname, 1024, NULL, 0, 0);
    printf(" [%s]\n", hname);
  }
}

int Listen(char *portnum, int *sock_family) {

  // Populate the "hints" addrinfo structure for getaddrinfo().
  // ("man addrinfo")
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;       // IPv6 (also handles IPv4 clients)
  hints.ai_socktype = SOCK_STREAM;  // stream
  hints.ai_flags = AI_PASSIVE;      // use wildcard "in6addr_any" address
  hints.ai_flags |= AI_V4MAPPED;    // use v4-mapped v6 if no v6 found
  hints.ai_protocol = IPPROTO_TCP;  // tcp protocol
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  // Use argv[1] as the string representation of our portnumber to
  // pass in to getaddrinfo().  getaddrinfo() returns a list of
  // address structures via the output parameter "result".
  struct addrinfo *result;
  int res = getaddrinfo(NULL, portnum, &hints, &result);

  // Did addrinfo() fail?
  if (res != 0) {
	printf( "getaddrinfo failed: %s", gai_strerror(res));
    return -1;
  }

  // Loop through the returned address structures until we are able
  // to create a socket and bind to one.  The address structures are
  // linked in a list through the "ai_next" field of result.
  int listen_fd = -1;
  struct addrinfo *rp;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    listen_fd = socket(rp->ai_family,
                       rp->ai_socktype,
                       rp->ai_protocol);
    if (listen_fd == -1) {
      // Creating this socket failed.  So, loop to the next returned
      // result and try again.
      printf("socket() failed:%s \n ", strerror(errno));
      listen_fd = -1;
      continue;
    }

    // Configure the socket; we're setting a socket "option."  In
    // particular, we set "SO_REUSEADDR", which tells the TCP stack
    // so make the port we bind to available again as soon as we
    // exit, rather than waiting for a few tens of seconds to recycle it.
    int optval = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
               &optval, sizeof(optval));

    // Try binding the socket to the address and port number returned
    // by getaddrinfo().
    if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
      // Bind worked!  Print out the information about what
      // we bound to.
      PrintOut(listen_fd, rp->ai_addr, rp->ai_addrlen);

      // Return to the caller the address family.
      *sock_family = rp->ai_family;
      break;
    }

    // The bind failed.  Close the socket, then loop back around and
    // try the next address/port returned by getaddrinfo().
    close(listen_fd);
    listen_fd = -1;
  }

  // Free the structure returned by getaddrinfo().
  freeaddrinfo(result);

  // If we failed to bind, return failure.
  if (listen_fd == -1)
    return listen_fd;

  // Success. Tell the OS that we want this to be a listening socket.
  if (listen(listen_fd, SOMAXCONN) != 0) {
    printf("Failed to mark socket as listening:%s \n ", strerror(errno));
    close(listen_fd);
    return -1;
  }

  // Return to the client the listening file descriptor.
  return listen_fd;
}

struct handler_params* initializeHandler(int client_fd, struct sockaddr_storage addr, size_t addrlen, int sock_family) {
  struct handler_params* params = malloc(sizeof(struct handler_params));
  params->client_fd = client_fd;
  params->addr = (struct sockaddr *)(&addr);
  params->addrlen = addrlen;
  params->sock_family = sock_family;
  return params;
}

void launchHandler(pthread_t* handler, struct handler_params* params) {
  if (pthread_create(handler, NULL, HandleClient, params) != 0) {
    printf("Failed to create thread: %s\n", strerror(errno));
    close(params->client_fd);
    free(params); // Free the dynamically allocated struct
  }
}


char* printClientInfo(struct sockaddr *addr, size_t addrlen) {
  char* name = malloc(sizeof(char) * 1024);
  if (name == NULL) {
    printf("Failed to allocate memory for name\n");
    return NULL; // Or handle the error in an appropriate way
  }
  
  int result = getnameinfo(addr, addrlen, name, 1024, NULL, 0, 0);
  if (result != 0) {
    printf("Failed to resolve client name: %s\n", gai_strerror(result));
    free(name); // Free the allocated memory
    return NULL; // Or handle the error in an appropriate way
  }
  
  printf("\nNew connection from client: %s\n", name);
  return name;
}


int checkBytesReceived(ssize_t bytesReceived, char* name) {
  if (bytesReceived == 0) {
    printf("\nClient %s disconnected\n", name);
    return 0;
  } else if (bytesReceived != sizeof(struct msg)) {
    printf("Failed to receive message from %s\n", name);
    return 0;
  } else {
    printf("Successfully received message from %s\n", name);
    return 1;
  }
}

void checkBytesSent(ssize_t bytesSent, char* name) {
  if (bytesSent != sizeof(struct msg)) {
    printf("Failed to send message to %s\n", name);
  } else {
    printf("Successfully sent message to %s\n", name);
  }
}

struct msg putStudent(struct msg message, int db_fd) {
  struct msg response;
  struct record student = message.rd; 
  response.rd = student;
  lseek(db_fd, 0, SEEK_END);
  int bytesWritten = write(db_fd, &student, sizeof(student));
  if (bytesWritten != sizeof(struct record)) {
    printf("Failed to write to database\n\n");
    response.type = FAIL;
  } else {
    printf("Successfully wrote to database\n\n");
    response.type = SUCCESS;
    numRecords++;
  }
  return response;
}

struct msg getStudent(struct msg message, int db_fd) {
  uint32_t id = message.rd.id;
  struct msg response;
  struct record parser;  

  lseek(db_fd, 0, SEEK_SET);  // Move file offset to the beginning

  while (read(db_fd, &parser, sizeof(struct record)) == sizeof(struct record)) {
    if (parser.id == id) {
      response.rd = parser;
      response.type = SUCCESS;
      printf("Successfully read from database\n\n");
      return response;
    }
  }

  response.type = FAIL;
  printf("Failed to read from database\n\n");
  return response;

}

int openDatabase() {
  int db_fd = open(DB, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
  if (db_fd == -1) {
    printf("Failed to open database file %s: %s\n", DB, strerror(errno));
    return -1;
  } else {
    return db_fd;
  }
}