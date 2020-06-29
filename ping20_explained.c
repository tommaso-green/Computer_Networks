#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <net/if.h>
#include <string.h>

unsigned char myip[4] = {192, 168, 1, 82};//{88, 80, 187, 84};
unsigned char netmask[4] = {255, 255, 255, 0}; //24 bits of net address i.e 192.168.1.82/24
unsigned char mymac[6] = {0x90, 0x78, 0x41, 0x5a, 0xb0, 0x63}; //{0xf2, 0x3c, 0x91, 0xdb, 0xc2, 0x98};
unsigned char gateway[4] = {192, 168, 1, 1}; //{88, 80, 187, 1};

unsigned char targetip[4] = {216, 58, 212, 196};
unsigned char targetmac[6]; // This will hold the MAC that we want to find using ARP Request
unsigned char buffer[1500]; // MTU of 1500 bytes (run ifconfig on Linux)
int s;
struct sockaddr_ll sll;  // The sockaddr_ll structure is a device-independent physical-layer address.


int printpacket(unsigned char *b, int l) {
    int i;
    for (i = 0; i < l; i++) {
        printf("%.2x(%.3d) ", b[i], b[i]); // HEX (DEC)
        if (i % 4 == 3) printf("\n");
    }
    printf("\n ================\n");
}

struct eth_frame {
    unsigned char dst[6]; // Destination MAC address
    unsigned char src[6]; // Source MAC address
    unsigned short type; // EtherType = 0x800 if in payload there's an IPv4 datagram, 0x806 if it is an ARP Packet
    unsigned char payload[1460]; //Contains IP/ARP Packet
};

struct arp_packet {
    unsigned short hw; // Specifies the network link protocol type. For Ethernet = 1
    unsigned short proto; // Specifies the internet protocol for which the ARP request is intended. For IPv4, this has the value 0x0800
    unsigned char hlen; //Length of link layer addresses: for Ethernet it's 6
    unsigned char plen; //Length of internet layer address: 4 for IPv4
    unsigned short op; // Op=1 for ARP Request, Op=2 for ARP Reply
    unsigned char srcmac[6]; // Mac of Source
    unsigned char srcip[4]; // IP of Source
    unsigned char dstmac[6]; // Mac of destination (all 0s for ARP Request)
    unsigned char dstip[4]; // IP of destination
};

struct ip_datagram {
    unsigned char ver_ihl; //First 4 bits it's version (4 for IPv4, 6 for IPv6) and
    // last 4 bits it's Internet Header Length which can vary between 20 and 60. To get it just
    // ver_ihl & 0xFF
    unsigned char tos; //eight bits to specify precedence, delay, throughput, reliability. last two bits unused
    unsigned short len; // Entire datagram size = header_len+payload
    unsigned short id; // identifier for this datagram
    unsigned short flag_offs; //first byte = bit 0: Reserved = 0; bit 1:DF; bit2:MF. Last byte=offsets of fragmented packet
    unsigned char ttl; // Maximum number of hops before packet is discarded
    unsigned char proto; // defines the protocol used in the data portion of the IP datagram (TCP=6, ICMP=1)
    unsigned short checksum; // checksum computed on the header. At every hop, the router computes it
    // and compares it to this field.
    unsigned int src; // Source Ip Address
    unsigned int dst; // Destination address
    unsigned char payload[1480];
};

unsigned short checksum(char *buf, int size) {
    int i;
    unsigned int tot = 0; //32 bits
    unsigned short *p;
    p = (unsigned short *) buf;
    for (i = 0; i < size / 2; i++) {
        tot = tot + htons(p[i]);
        if (tot & 0x10000) tot = ((tot & 0xffff) + 1);
    }
    return (unsigned short) (0xffff - tot);
}

int forge_ip(struct ip_datagram *ip, unsigned char *dst, int payloadlen, unsigned char proto) {
    ip->ver_ihl = 0x45; // Internet Length Header (20 in this case, which is the minimum)
    ip->tos = 0; // precedence = 0, normal delay, throughput, reliability
    ip->len = htons(payloadlen + 20); // total length of IP packet
    ip->id = htons(0xABCD); // random ID
    ip->flag_offs = htons(0); // Don't fragment
    ip->ttl = 128; // Time to live
    ip->proto = proto; // For TCP=6, ICMP=1
    ip->checksum = htons(0); //set to 0 before calculation
    ip->src = *(unsigned int *) myip;
    ip->dst = *(unsigned int *) dst;
    ip->checksum = htons(checksum((unsigned char *) ip, 20)); //Computing checksum of header
    /* Calculate the checksum!!!*/
};

struct icmp_packet { //Actually this is the structure for an echo request/reply
    unsigned char type; // Macrocategory of message type
    unsigned char code; //Specification of message type
    unsigned short checksum;
    unsigned short id; //unique id for ping process
    unsigned short seq; //increasing number for each packet sent in ping
    unsigned char payload[1400];
};

int forge_icmp(struct icmp_packet *icmp, int payloadsize) {
    int i;
    icmp->type = 8; // Echo Request
    icmp->code = 0; // Code associated to Echo Request or Reply
    icmp->checksum = htons(0); //checksum first set to 0
    icmp->id = htons(0x1234); // can be used to identify packet
    icmp->seq = htons(1); // seq=1 since it's the first and only packet we send
    for (i = 0; i < payloadsize; i++)icmp->payload[i] = i & 0xFF; //random payload, not needed for echo messages
    icmp->checksum = htons(checksum((unsigned char *) icmp, 8 + payloadsize)); //computing header checksum
}


int arp_resolve(unsigned char *destip, unsigned char *destmac) {
    int len, n, i;
    unsigned char pkt[1500];
    struct eth_frame *eth;
    struct arp_packet *arp;

    eth = (struct eth_frame *) pkt;
    arp = (struct arp_packet *) eth->payload; // setting arp_packet as eth payload
    for (i = 0; i < 6; i++) eth->dst[i] = 0xff; // Broadcast message, "special" MAC address ff:ff:ff:ff:ff:ff
    for (i = 0; i < 6; i++) eth->src[i] = mymac[i];
    eth->type = htons(0x0806); //This specifies that this is an ARP request
    arp->hw = htons(1);  // Protocol: ethernet
    arp->proto = htons(0x0800); //Network Protocol: IPv4
    arp->hlen = 6; //Length of hardware (i.e. Ethernet) addresses: 6
    arp->plen = 4; //Length of network (i.e. IPv4) addresses: 4
    arp->op = htons(1); // This is and arp request
    for (i = 0; i < 6; i++) arp->srcmac[i] = mymac[i];
    for (i = 0; i < 4; i++) arp->srcip[i] = myip[i];
    for (i = 0; i < 6; i++) arp->dstmac[i] = 0; // Mac address of target: this is what we want to find!
    for (i = 0; i < 4; i++) arp->dstip[i] = destip[i];
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = if_nametoindex(
            "wifi0"); //interface index of the interface, which is found using the if_.. function
    len = sizeof(sll);
    n = sendto(s, pkt, 14 + sizeof(struct arp_packet), 0, (struct sockaddr *) &sll, len); //14 is ethernet header size
    if (n == -1) {
        perror("Recvfrom failed");
        return 0;
    }
    while (1) {
        n = recvfrom(s, pkt, 1500, 0, (struct sockaddr *) &sll, &len);
        if (n == -1) {
            perror("Recvfrom failed");
            return 0;
        }
        if (eth->type == htons(0x0806)) //it is ARP
            if (arp->op == htons(2)) // it is a reply (op = 2)
                if (!memcmp(destip, arp->srcip, 4)) { //if it comes from our target
                    memcpy(destmac, arp->srcmac, 6); // copy into destmac the retrieved MAC
                    printpacket(pkt, 14 + sizeof(struct arp_packet));
                    return 0;
                }
    }
}

unsigned char packet[1500];

int main() {
    int i, n, len;
    unsigned char dstmac[6];

    struct eth_frame *eth;
    struct ip_datagram *ip;
    struct icmp_packet *icmp;

    s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (s == -1) {
        perror("socket failed");
        return 1;
    }

    /**** HOST ROUTING ****/
    if (((*(unsigned int *) &myip) & (*(unsigned int *) &netmask)) ==
        (*(unsigned int *) &targetip) & (*(unsigned int *) &netmask))
        arp_resolve(targetip, dstmac); // if host and target in the same network (same net bytes)
        //send it to target ip
    else
        arp_resolve(gateway, dstmac); // otherwise send it to gateway router!

    /********/

    printf("destmac: ");
    printpacket(dstmac, 6); //Printing destination mac

    // Now let's prepare our Echo Request!
    eth = (struct eth_frame *) packet;
    ip = (struct ip_datagram *) eth->payload;
    icmp = (struct icmp_packet *) ip->payload;

    for (i = 0; i < 6; i++) eth->dst[i] = dstmac[i];
    for (i = 0; i < 6; i++) eth->src[i] = mymac[i];
    eth->type = htons(0x0800); // Ethertype = IP
    forge_icmp(icmp, 20);
    forge_ip(ip, targetip, 20 + 8, 1); //payload of ip = icmp header (8bytes) + icmp payload
    printpacket(packet, 14 + 20 + 8 + 20); // 14 ETH HEADER, 20 IP HEADER, 8 ICMP HEADER +20 ICMP PAYLOAD

    for (i = 0; i < sizeof(sll); i++)
        ((char *) &sll)[i] = 0; // emptying sll

    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = if_nametoindex("wifi0");
    len = sizeof(sll);
    n = sendto(s, packet, 14 + 20 + 8 + 20, 0, (struct sockaddr *) &sll, len); // Send Echo Request
    if (n == -1) {
        perror("Recvfrom failed");
        return 0;
    }

    while (1) { //until I haven't received my Echo Reply
        len = sizeof(sll);
        n = recvfrom(s, packet, 1500, 0, (struct sockaddr *) &sll, &len);
        if (n == -1) {
            perror("Recvfrom failed");
            return 0;
        }
        if (eth->type == htons(0x0800)) //it is IP
            if (ip->proto == 1) // it is ICMP
                if (icmp->type == 0) { //if it is an Echo Reply
                    printpacket(packet, n);
                    break;
                }
    }

    return 0;

}
