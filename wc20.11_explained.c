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
    char *n;
    char *v;
} h[30];

int main() {
    int s, t, size, i, j, k, l;
    unsigned char c;
    int L;
    char request[100], response[10000], entity[1000000];
    unsigned char ipaddr[4] = {216, 58, 211, 163};
    int bodylength = 0;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        printf("Errno = %d\n", errno);
        perror("Socket Failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(80); //Ex: Write a function to revert endianness
    server.sin_addr.s_addr = *(uint32_t *) ipaddr;
    // WRONG : server.sin_addr.s_addr = (uint32_t )*ipaddr

    t = connect(s, (struct sockaddr *) &server, sizeof(server));
    if (t == -1) {
        perror("Connect Failed");
        return 1;
    }
    sprintf(request,
            "GET /  HTTP/1.1\r\nHost:www.google.it\r\n\r\n"); // request[0]='G', request[1]='E',... request[7]=0 (or '\0')
    for (size = 0; request[size]; size++);    //Specifying Host is mandatory in HTTP 1.1
    t = write(s, request, size);
    if (t == -1) {
        perror("Write failed");
        return 1;
    }

    j = 0;
    k = 0;
    h[k].n = response;
    while (read(s, response + j, 1)) {
        if ((response[j] == '\n') && (response[j - 1] == '\r')) {
            response[j - 1] = 0;
            if (h[k].n[0] == 0) break;
            h[++k].n = response + j + 1;
        }
        if (response[j] == ':' && (h[k].v == 0)) {
            response[j] = 0;
            h[k].v = response + j + 1;
        }
        j++;
    }

    /*
     *  So, up to now, everything was pretty much the same if compared to HTTP 1.0 . The main difference
     *  will be the possibility of having a chunked transmission of the content. This is a peculiar feature of HTTP
     *  1.1 that enables the data to be divided into chunks, where each chunk will have its size and content.
     *  The chunked body is structured as follows:
     *   Chunked-Body   = *chunk
                        last-chunk
                        trailer
                        CRLF
    *    where each chunk is built up in this way:
     *    chunk          = chunk-size [ chunk-extension ] CRLF
                         chunk-data CRLF
          chunk-size     = 1*HEX
          last-chunk     = 1*("0") [ chunk-extension ] CRLF

        References:
        RFC 2616, Section 3.6.1 https://tools.ietf.org/html/rfc2616#section-3.6.1
        Slides of Lo Presti http://www.ce.uniroma2.it/~lopresti/Didattica/RetiWeb/RetiWeb1213/HTTP_completo4.pdf
     */

    printf("Status line: %s\n", h[0].n);
    for (i = 1; h[i].n[0]; i++) {
        if (!strcmp(h[i].n, "Content-Length")) { bodylength = atoi(h[i].v); }
        if ((!strcmp(h[i].n, "Transfer-Encoding")) && (!strcmp(h[i].v, " chunked")))
            bodylength = -1; // this is a flag to remember later on that the content is chunked

        printf("Name = %s ---> Value = %s\n", h[i].n, h[i].v);
    }
    if (bodylength > 0) { // we have content-length
        for (size = 0; (t = read(s, entity + size, bodylength - size)) > 0; size = size + t);
        if (t == -1) {
            perror("Chunk Body Read Failed");
            return 1;
        }
    } else if (bodylength == 0) { // Connection Close
        for (size = 0; (t = read(s, entity + size, 1000000 - size)) > 0; size = size + t);
        if (t == -1) {
            perror("Chunk Body Read Failed");
            return 1;
        }
    } else if (bodylength == -1) { //Chunked
        printf("Chunked!");
        L = 0;
        /*
         *  The crazy fact is that the chunk size is a HEX Number. So, to avoid being a sane person who understands that
         *  C language is not the proper one for string parsing, let's build a parser that for a given cipher
         *  of the size c (in ASCII) converts it into the corresponding DEC cipher.Note that this could have been done
         *  easily using sscanf (thanks Elia).
         *
         */
        while (1) {
            l = 0;
            while (t = read(s, &c, 1)) { //Let's read one char at a time
                if (t == 0) break;
                if (t == -1) {
                    perror("Chunk size read failed\n");
                    return 1;
                }
                if (c == '\n') break; //LF
                if (c == '\r') continue; // CR
                printf("length digit:%c\n", c); //let's print the ASCII char
                switch (c) { //let's convert the char c into the corresponding DEC number
                    case '0' ... '9':
                        c = c - '0';  //the idea is simple: we have to subtract the base ASCII to get the code
                        break;
                    case 'A' ... 'F':
                        c = c + 10 -
                            'A'; //here we also have to add the "+10" because if c is a letter then its minimum value is 10
                        break; // brief example, let c = 'D' . The ASCII Code for 'D' is 44 and for 'A' is 41. Then the difference of this values
                        // plus 10 is 13 which is the right conversion for 'D'.
                    case 'a' ... 'f':  //same as above, but characters could be also lowercase
                        c = c + 10 - 'a';
                        break;
                    default:
                        printf("Ill-formed chunk\n");
                        return 1;
                }
                l = l * 16 + c; //here we use the powers of 16 to build up the final integer
            }
            printf("New Chunk: Length =%d\n", l); // printing final integer
            if (l == 0) break;
            for (size = 0;
                 (t = read(s, entity + L + size, l - size)) > 0; size += t); //size keeps track of number of bytes read
            // and l-size represent the number of bytes left to read
            if (t == -1) {
                perror("Chunk Body Read Failed");
                return 1;
            }
            printf("size = %d\n", size);
            read(s, &c, 1);
            printf("CR = %d\n", c);  // CR after chunk body
            read(s, &c, 1);
            printf("LF = %d\n", c); // LF after chunk body
            L = L + l; // L is the cumulative number of chunk size read so far
        }
        size = L;
    }
    for (i = 0; i < size; i++)
        printf("%c", entity[i]); // Let's print all the chunks
}
