#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

struct sockaddr_in local, remote;
char request[2000], response[2000];
char *method, *path, *version;

int main() {
    int s, t, s2, yes = 1;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("Socket Failed\n");
        return 1;
    }
    local.sin_family = AF_INET;
    local.sin_port = htons(8080);
    local.sin_addr.s_addr = 0;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // to avoid address already in use
    t = bind(s, (struct sockaddr *) &local, sizeof(struct sockaddr_in));
    if (t == -1) {
        perror("Bind Failed \n");
        return 1;
    }

    /*
        Now we will use the listen function:

        int listen(int sockfd, int backlog);


       listen()  marks  the  socket  referred  to  by  sockfd  as a passive socket, that is, as a socket that will be used to accept incoming connection
       requests using accept(2).

       The sockfd argument is a file descriptor that refers to a socket of type SOCK_STREAM or SOCK_SEQPACKET.

       The backlog argument defines the maximum length to which the queue of pending connections for sockfd may grow.  If a connection  request  arrives
       when  the  queue  is full, the client may receive an error with an indication of ECONNREFUSED or, if the underlying protocol supports retransmis‐
       sion, the request may be ignored so that a later reattempt at connection succeeds.


    */
    t = listen(s, 10);
    if (t == -1) {
        perror("Listen Failed \n");
        return 1;
    }

    /*
     *  Now it's time to accept incoming requests.
     *  We are going to use the accept function.
     *
     *  int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
     *
     *  The  accept()  system call is used with connection-based socket types (SOCK_STREAM, SOCK_SEQPACKET).
     *  It extracts the first connection request on
        the queue of pending connections for the listening socket, sockfd, creates a new connected socket, and returns a new file descriptor referring to
        that socket.  The newly created socket is not in the listening state.  The original socket sockfd is unaffected by this call.

        The  argument  sockfd  is  a socket that has been created with socket(2), bound to a local address with bind(2), and is listening for connections
        after a listen(2).

        The argument addr is a pointer to a sockaddr structure.  This structure is filled in with the address of the peer socket, as known to the  commu‐
        nications  layer.   The  exact format of the address returned addr is determined by the socket's address family (see socket(2) and the respective
        protocol man pages).  When addr is NULL, nothing is filled in; in this case, addrlen is not used, and should also be NULL.

        The addrlen argument is a value-result argument: the caller must initialize it to contain the size (in bytes) of  the  structure  pointed  to  by
        addr; on return it will contain the actual size of the peer address.

        The returned address is truncated if the buffer provided is too small; in this case, addrlen will return a value greater than was supplied to the
        call.

        If no pending connections are present on the queue, and the socket is not marked as nonblocking, accept() blocks the caller until a connection is
        present.   If  the  socket  is  marked  nonblocking  and no pending connections are present on the queue, accept() fails with the error EAGAIN or
        EWOULDBLOCK.
     *
     *
     *
     */
    int len, n, i, c;
    FILE *f;
    while (1) {
        remote.sin_family = AF_INET;
        len = sizeof(struct sockaddr_in);
        s2 = accept(s, (struct sockaddr *) &remote, &len);
        if (s2 == -1) {
            perror("Accept Failed\n");
            return 1;
        }
        n = read(s2, request, 1999);
        request[n] = 0;
        printf("%s", request);
        method = request;
        for (i = 0; (i < 2000) && (request[i] != ' '); i++);
        request[i] = 0;
        path = request + i + 1;
        for (; (i < 2000) && (request[i] != ' '); i++);
        request[i] = 0;
        version = request + i + 1;
        for (; (i < 2000) && (request[i] != '\r'); i++);
        request[i] = 0;
        printf("Method = %s , Path = %s , Version = %s \n", method, path, version);
        if (strcmp("GET", method) == 0) // if it is not a GET
            sprintf(response, "HTTP/1.1 501");
        else if ((f = fopen(path + 1, "r")) == NULL) {  //to jump the '/'
            sprintf(response, "HTTP/1.1 404 Not Found\r\nConnection:Close\r\n\r\n");
        } else
            sprintf(response, "HTTP/1.1 200 Not Found\r\nConnection:Close\r\n");
        write(s2, response, strlen(response)); //HTTP Header
        if (f != NULL) {
            while ((c = fgetc(f)) != EOF)
                write(s2, &c, 1);
        }
        shutdown(s2, SHUT_RDWR);
        close(s2);
    }

}
