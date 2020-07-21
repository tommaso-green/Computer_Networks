#include <sys/types.h>          /* See NOTES */
#include <signal.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>

struct hostent *he;
struct sockaddr_in local, remote, server;
char request[2000], response[2000], request2[2000], response2[2000];
char *method, *path, *version, *host, *scheme, *resource, *port;
struct headers {
    char *n;
    char *v;
} h[30];

int main() {
    FILE *f;
    char command[100];
    int i, s, t, s2, s3, n, len, c, yes = 1, j, k, pid;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("Socket Failed\n");
        return 1;
    }
    local.sin_family = AF_INET;
    local.sin_port = htons(8019);
    local.sin_addr.s_addr = 0;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes,
               sizeof(int)); // This line allows port reuse thus avoiding address already in use exception
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
        f = NULL; // <<<<< BUG
        remote.sin_family = AF_INET;
        len = sizeof(struct sockaddr_in);
        s2 = accept(s, (struct sockaddr *) &remote, &len);
        if (fork()) continue; //<< MULTI PROCESS HANDLING: in the parent program, it will keep looping every time it creates a child
        if (s2 == -1) {      // the child will go on with the rest of the code, since fork() outputs 0.
            perror("Accept Failed\n");
            return 1;
        }
        // <---- ADDED HEADER PARSER
        j = 0;
        k = 0;
        h[k].n = request;
        while (read(s2, request + j, 1)) {
            if ((request[j] == '\n') && (request[j - 1] == '\r')) {
                request[j - 1] = 0; // if CLRF is found, put string terminator
                if (h[k].n[0] == 0) break; //if final CLRF is found
                h[++k].n = request + j + 1; //increase k, and make new header name point to request+j+1
            }
            if (request[j] == ':' && (h[k].v == 0) && (k != 0)) {
                request[j] = 0; //string terminator
                h[k].v = request + j + 1; //set value of header
            }
            j++;
        }
        printf("%s", request);
        method = request;
        for (i = 0; (i < 2000) && (request[i] != ' '); i++); //forward until you get to a space, i.e GET <-
        request[i] = 0;
        path = request + i + 1;
        for (; (i < 2000) && (request[i] != ' '); i++); //forward until you get to a space, i.e /prova.html <-
        request[i] = 0;
        version = request + i + 1;
        // <---- Terminator already put by parser following line useless
        // for(   ;(i<2000) && (request[i]!=0);i++); request[i]=0;
        printf("Method = %s, path = %s , version = %s\n", method, path, version);
        if (!strcmp("GET", method)) { // if it is a GET
            //  http://www.google.com/path
            /*
                The gethostbyname() function returns a structure of type hostent for
                the given host name.
                The hostent structure is defined in <netdb.h> as follows:

                           struct hostent {
                               char  *h_name;           official name of host
                            char **h_aliases;           alias list
                            int    h_addrtype;          host address type
                            int    h_length;            length of address
                            char **h_addr_list;         list of addresses
                          }
                  ---->> #define h_addr h_addr_list[0] for backward compatibility
            */
            scheme = path;
            for (i = 0; path[i] != ':'; i++);
            path[i] = 0;
            host = path + i + 3; // http://www.google.com, with +3 we can jump to www.google.com
            for (i = i + 3; path[i] != '/'; i++);
            path[i] = 0;
            resource = path + i + 1; // path
            printf("Scheme=%s, host=%s, resource = %s\n", scheme, host, resource);
            he = gethostbyname(host);
            if (he == NULL) {
                printf("Gethostbyname Failed\n");
                return 1;
            }

            /*
             *  Ok, so now we want to forward the client's request to the server, using the server address which we found
             *  in the HTTP request from the client. We have to define a new socket, s3.
             *
             */

            printf("Server address = %u.%u.%u.%u\n", (unsigned char) he->h_addr[0], (unsigned char) he->h_addr[1],
                   (unsigned char) he->h_addr[2], (unsigned char) he->h_addr[3]);
            s3 = socket(AF_INET, SOCK_STREAM, 0); //protocol=0 means it's chosen automatically
            // this socket s3 will be used for the connection proxy-server
            if (s3 == -1) {
                perror("Socket to server failed");
                return 1;
            }
            server.sin_family = AF_INET;
            server.sin_port = htons(80);
            server.sin_addr.s_addr = *(unsigned int *) he->h_addr;
            t = connect(s3, (struct sockaddr *) &server, sizeof(struct sockaddr_in));
            if (t == -1) {
                perror("Connect to server failed");
                return 1;
            }

            // Now we forward the client's request to the server using the host we previously found
            sprintf(request2, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n\r\n", resource, host);
            write(s3, request2, strlen(request2));
            while ((t = read(s3, response2, 2000))) //as you get the required resource from server
                write(s2, response2, t); //stream it back to client
            shutdown(s3, SHUT_RDWR);
            close(s3);
        } else if (!strcmp("CONNECT", method)) { // if it is a connect, this program will act as a TUNNEL
            host = path; // we want to extract info from CONNECT server.example.com:80 HTTP/1.1
            for (i = 0; path[i] != ':'; i++); //until you found the :
            path[i] = 0;
            port = path + i + 1;
            printf("host:%s, port:%s\n", host, port);
            printf("Connect skipped ...\n");
            he = gethostbyname(host);
            if (he == NULL) {
                printf("Gethostbyname Failed\n");
                return 1;
            }
            printf("Connecting to address = %u.%u.%u.%u\n", (unsigned char) he->h_addr[0],
                   (unsigned char) he->h_addr[1], (unsigned char) he->h_addr[2], (unsigned char) he->h_addr[3]);
            s3 = socket(AF_INET, SOCK_STREAM, 0);

            if (s3 == -1) {
                perror("Socket to server failed");
                return 1;
            }
            server.sin_family = AF_INET;
            server.sin_port = htons((unsigned short) atoi(port));
            server.sin_addr.s_addr = *(unsigned int *) he->h_addr;
            t = connect(s3, (struct sockaddr *) &server, sizeof(struct sockaddr_in)); //Connecting to server
            if (t == -1) {
                perror("Connect to server failed");
                exit(0);
            }
            // ===============> Bug : missing the HTTP response for successful connection
            sprintf(response, "HTTP/1.1 200 Established\r\n\r\n");
            write(s2, response, strlen(response)); // telling the client that the proxy connected to server
            // <==============
            if (!(pid = fork())) { //Child
                while ((t = read(s2, request2, 2000))) { //Reading from socket attached to client
                    write(s3, request2, t); //Sending request to server
                    printf("CL >>>(%d)%s \n", t, host); //Just checking
                }
                exit(0);
            } else { //Parent
                while ((t = read(s3, response2, 2000))) { //If this is the parent, read response from server
                    write(s2, response2, t); //and send it back to client
                    printf("CL <<<(%d)%s \n", t, host);
                }
                kill(pid, 15); //kill parent?
                shutdown(s3, SHUT_RDWR); //close socket directed to server
                close(s3);
            }
        }
        shutdown(s2, SHUT_RDWR); //close socket directed to client
        close(s2);
        exit(0);
    }
}
