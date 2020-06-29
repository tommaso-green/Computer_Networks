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
    /*
     *  In this first part we will define the socket s that will accept incoming connections
     */
    FILE *f;
    char command[100];
    int i, s, t, s2, n, len, c, yes = 1;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("Socket Failed\n");
        return 1;
    }
    local.sin_family = AF_INET;
    local.sin_port = htons(8080);
    local.sin_addr.s_addr = 0;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // to avoid address already in use exception

    /*
     *  Now we want to bind the socket s to the address specified by struct local.
     *  The description of the manual is as follows:
     *
     *  When  a  socket  is  created  with  socket(2),
     *  it exists in a name space (address family) but has no address assigned to it.
     *
     *  bind() assigns the address specified by addr to the socket referred to by the file descriptor sockfd.
     *  addrlen specifies the size, in bytes, of the  address  structure pointed to by addr.
     *  Traditionally, this operation is called “assigning a name to a socket”.
     */
    t = bind(s, (struct sockaddr *) &local, sizeof(struct sockaddr_in));
    if (t == -1) {
        perror("Bind Failed \n");
        return 1;
    }

    /*
        Now we will use the listen function:

        int listen(int sockfd, int backlog);


       listen()  marks  the  socket  referred  to  by  sockfd  as a passive socket, that is, as a socket that will be used
       to accept incoming connection requests using accept(2).

       The sockfd argument is a file descriptor that refers to a socket of type SOCK_STREAM or SOCK_SEQPACKET.

       The backlog argument defines the maximum length to which the queue of pending connections for sockfd may grow.
       If a connection  request  arrives when  the  queue  is full, the client may receive an error with an indication of ECONNREFUSED or,
       if the underlying protocol supports retransmission, the request may be ignored so that a later reattempt at connection succeeds.
    */

    t = listen(s, 10);
    if (t == -1) {
        perror("Listen Failed \n");
        return 1;
    }

    /*
     *  Now it's time to accept incoming requests.
     *  We are going to use the accept function: here's the description.
     *
     *  int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
     *
     *  The  accept()  system  call  is  used  with  connection-based  socket types (SOCK_STREAM, SOCK_SEQPACKET).  It
     *  extracts the first connection request on the queue of pending connections for the  listening  socket,  sockfd,
     *  creates a new connected socket, and returns a new file descriptor referring to that socket.  The newly created
     *  socket is not in the listening state.  The original socket sockfd is unaffected by this call.
     *
     *  The argument sockfd is a socket that has been created with socket(2), bound to a local address  with  bind(2),
     *  and is listening for connections after a listen(2).
     */

    while (1) {
        f = NULL; // <<<<< BACO
        remote.sin_family = AF_INET;
        len = sizeof(struct sockaddr_in);
        s2 = accept(s, (struct sockaddr *) &remote, &len); //accepting incoming connections
        if (s2 == -1) {
            perror("Accept Failed\n");
            return 1;
        }
        if (!fork()) {
            n = read(s2, request, 1999);
            request[n] = 0;
            printf("%s", request);
            // Now let's read the status line of the incoming HTTP request
            method = request;
            for (i = 0; (i < 2000) && (request[i] != ' '); i++);
            request[i] = 0;
            path = request + i + 1;
            for (; (i < 2000) && (request[i] != ' '); i++);
            request[i] = 0;
            version = request + i + 1;
            for (; (i < 2000) && (request[i] != '\r'); i++);
            request[i] = 0;
            printf("Method = %s, path = %s , version = %s\n", method, path, version);
            if (strcmp("GET", method)) // if it is not a GET
                sprintf(response, "HTTP/1.1 501 Not Implemented\r\n\r\n"); //We haven't implemented anything else
            else { // it is a get
                if (!strncmp(path, "/cgi-bin/", 9)) { // CGI interface
                    sprintf(command, "%s > results.txt", path + 9);
                    printf("executing %s\n", command);
                    system(command);
                    if ((f = fopen("results.txt", "r")) == NULL) {
                        printf("cgi bin error\n");
                        return 1;
                    }
                    sprintf(response, "HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n");
                } else if ((f = fopen(path + 1, "r")) == NULL)
                    sprintf(response, "HTTP/1.1 404 Not Found\r\nConnection:Close\r\n\r\n");
                else sprintf(response, "HTTP/1.1 200 OK\r\nConnection:close\r\n\r\n");
            }
            write(s2, response,
                  strlen(response)); // Sending to client response (Status Line CLRF Headers (just one) CLRF CLRF)
            if (f != NULL) { // if requested resource is present, write it into the Entity Body
                while ((c = fgetc(f)) != EOF)
                    write(s2, &c, 1); // and send it byte per byte
                fclose(f);
            }
            shutdown(s2, SHUT_RDWR);
            close(s2);
            exit(0);
        }
    }

}
