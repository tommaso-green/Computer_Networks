#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

struct sockaddr_in server;

struct headers {
    char* n;
    char* v;
} h[30];


int main() {
    int s, t, size, i, j, k;
    char request[100], response[1000000];
    unsigned char ipaddr[4] = {216, 58, 211, 163}; //unsigned is important to reach 255 as max value
    int bodylength = 0;

    /*   ****** Sockaddr structures *******
       The sockaddr structure is the generic structure to hold address information

         struct sockaddr{
                     sa_family_t sa_family; // Address family (e.g. AF_INET)
                  char sa_data[14];  // "Blob" of bits which depend on the specific type of address family
                };

               In our case we want to use IPv4 addresses, therefore we refer to struct sockaddr_in

                       struct sockaddr_in{
                         sa_family_t sin_family;  // Internet Protocol (AF_INET)
                         in_port_t sin_port; // Address port (16 bits)
                         struct in_addr sin_addr; // IPv4 address
                         char sin_zero[8]; // Not used
                       };

                       struct in_addr{
                         uint32_t s_addr; //Internet address 32 bits
                       };


     -----> We can think of sockaddr_in as a "specific" case of sockaddr. This is the reason why
                  in the below connect function we need to cast to sockaddr: the function will look inside the
                    sa_family field to understand the type of address it has to deal with


        ***** Sockets *****

           The  int socket(int domain, int type, int protocol) function is defined in sys/socket.h
           The  domain indicates the protocol family: AF_INET for IPv4.
           The  type specifies the connection semantics: SOCK_STREAM provides  sequenced,
                    reliable,  two-way,  connection-based byte streams.
           The  protocol indicates a particular protocol to be used with the socket.  Normally only a single protocol
           exists to support a  particular  socket type  within  a  given  protocol family,
           in which case protocol can be specified as 0.

           RETURN VALUE
      On success, a file descriptor for the new socket is returned.
            On error, -1 is returned, and errno is set appropriately.



    */
    s = socket(AF_INET, SOCK_STREAM, 0);
    // perror prints an error message to stderr, based on the error state stored in errno
    // (which is set by the socket function)
    if (s == -1) {
        printf("Errno = %d\n", errno);
        perror("Socket Failed");
        return 1;
    }

    // here we need to initialise our sockaddr_in structure. Note that the htons() functions
    // writes port 80 in network byte order (Little Endian --> Big Endian) to sin_port field
    server.sin_family = AF_INET;
    server.sin_port = htons(80); //Ex: Write a function to revert endianness
    server.sin_addr.s_addr = *(uint32_t *) ipaddr; // WRONG : server.sin_addr.s_addr = (uint32_t )*ipaddr


    //Let's connect: we need to specify the socket, cast the sockaddr_in pointer (as mentioned above),
    // and also specify the size, since &server is now sockaddr and connect can't retrieve its size
    // If the connection or binding succeeds, zero is returned.
    // On error, -1 is returned, and errno is set appropriately
    t = connect(s, (struct sockaddr *) &server, sizeof(server));
    if (t == -1) {
        perror("Connect Failed");
        return 1;
    }

    // Let's go now into application layer. We now want to write our HTTP request inside the request buffer.
    // To do so we use sprintf (which stands for string printf)
    // The request follows the guidelines of RFC 1945 (https://tools.ietf.org/html/rfc1945#section-8.1)
    /*	An HTTP request is composed as follows:

            • Request Line: here we specify our method, which is the action that we want to do
                                     In our case GET / HTTP/1.0 CRLF where GET is the method name,
                                       "/" is the URL, HTTP/1.0 is the protocol and CRLF indicates the end of the
                                       request line.
           • Headers: there are three types of Headers: General, Request and Entity Headers.
                                In our case we just wrote Connection: keep-alive
                                Each line has to end with CRLF
                                There is also a final CRLF that separates the header section from the entity body.
           • Entity Body: optional field. In our request the entity body is empty.
           • CRLF: this final \r\n indicates the end of our HTTP request.

           For further details:
           • Slide 6 of http://www.ce.uniroma2.it/~lopresti/Didattica/RetiWeb/RetiWeb1213/HTTP_completo4.pdf
           • RFC 1945, Section 5 https://tools.ietf.org/html/rfc1945#section-5

   */
    sprintf(request,
            "GET / HTTP/1.0\r\nConnection: keep-alive\r\n\r\n"); // request[0]='G', request[1]='E',... request[7]=0 (or '\0')
    for (size = 0; request[size]; size++); // Very nerdy way to get strlen(request) and store it in size .... :-\
	 																	 // Not even the great Mike could achieve such levels of nerdiness...

    /*
            After having stored our HTTP request in the request variable, we need to send
            away our request: we're going to use the write function.

            int write(int fd, const void *buf, size_t count);

            write() writes up to count bytes from the buffer starting at buf to the file referred to by the file descriptor fd.
            In our case the socket s acts as file descriptor.

            RETURN VALUE
      On success, the number of bytes written is returned (zero indicates nothing was written).
            On error, -1 is returned, and errno is set appropriately.
    */
    t = write(s, request, size);
    if (t == -1) {
        perror("Write failed");
        return 1;
    }


    j = 0; //for the response
    k = 0; //for the struct

    /*
          We are now going to read the HTTP response using the read() function.
          read(int fd, void *buf, size_t count) will read from the socket just one byte
          per while loop and store it in a location pointed by response+j.
          This function is going to return the number of bytes read: therefore when nothing is
          left to read it will cause the while loop to cease.

          Our objective is to read the headers and store them inside the struct headers h (which will be an array of
          headers) defined in the static area of the program. h will contain all the headers, with their name in the
          h[k].n field and value in the h[k].v field.

          The first header h[0] will contain the status line and value equal to 0.

          In general an HTTP Response has the following structure:

          •Status Line: it's the line that consists of the protocol version followed by a numeric status code
         and its associated textual phrase, with each element separated by SP
         characters. No CR or LF is allowed except in the final CRLF sequence.
          This is its structure:
         Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
          • Headers: there are three types of Headers: General, Request and Entity Headers.
              Each instance of them is composed of name and value separated by a ":" ( <header name>:<value> ).
              Each line has to end with CRLF
              There is also a final CRLF that separates the header section from the entity body.
          • Entity Body: optional field.


      */
    h[k].n = response;    //h[0].n = Status Line, h[0].v = 0 (default value)
    while (read(s, response + j, 1)) {

        if ((response[j] == '\n') &&
            (response[j - 1] == '\r')) { //if I find a CRLF, it's time to change the header considered
            response[j - 1] = 0; // Let's put a string terminator at the end of response
            if (h[k].n[0] == 0)
                break; // if I reached the last CRLF. This is the sequence: /r/n/r/n--> 0/n/r/n---> 0/n0/n
            h[++k].n = response + j +
                       1; //let's increase k and make h[k] to point to the new location where data will be written
        }

        if (response[j] == ':' &&
            (h[k].v == 0)) { //If I'm reading a : I'm about to read the header value, example Connection: close
            response[j] = 0; // String terminator
            h[k].v = response + j + 1; // As above
        }
        j++;
    }


    printf("Status line: %s\n", h[0].n);

    /*
     Now let's ready the entity body (if there is any).
	 To know its length we have to inspect the "Content-Length"
	 field.

	 For academic reasons, let's also print all the headers.

	 Then we'll read and print the entity body.
    */

    for (i = 1; h[i].n[0]; i++) {

        if (!strcmp(h[i].n, "Content-Length")) { bodylength = atoi(h[i].v); } //atoi converts string to integer

        printf("Name = %s ---> Value = %s\n", h[i].n, h[i].v); //Let's print all the headers like crazy

    }

    if (bodylength) // if we have a non-zero content-length
        for (size = 0;
             (t = read(s, response + size, bodylength - size)) > 0; size = size + t); //size saves # bytes read so far
    else
        for (size = 0; (t = read(s, response + size, 1000000 - size)) > 0; size = size + t);

    if (t == -1) { // t is -1 if an error occurred
        perror("Read failed");
        return 1;
    }
    for (i = 0; i < size; i++)
        printf("%c", response[i]); //printing entity body
}
