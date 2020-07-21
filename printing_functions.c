#include <stdio.h>
#include <arpa/inet.h>


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

struct icmp_packet { //Actually this is the structure for an echo request/reply
    unsigned char type; // Macrocategory of message type
    unsigned char code; //Specification of message type
    unsigned short checksum;
    unsigned short id; //unique id for ping process
    unsigned short seq; //increasing number for each packet sent in ping
    unsigned char payload[1400];
};

void print_eth(struct eth_frame *e) {
    printf("\n\n ***** Ethernet Packet *****\n");
    printf("Mac destination: %x:%x:%x:%x:%x:%x\n", e->dst[0], e->dst[1], e->dst[2], e->dst[3], e->dst[4], e->dst[5]);
    printf("Mac soruce: %x:%x:%x:%x:%x:%x\n", e->src[0], e->src[1], e->src[2], e->src[3], e->src[4], e->src[5]);
    printf("EtherType: 0x%x\n", htons(e->type));
}

void arp_print(struct arp_packet *a) {
    printf("\n\n ***** ARP Packet *****\n");
    printf("Hardware type: %d\n", htons(a->hw));
    printf("Protocol type: %x\n", htons(a->proto));
    printf("Hardware Addr len: %d\n", a->hlen);
    printf("Protocol Addr len: %d\n", a->plen);
    printf("Operation: %d\n", htons(a->op));
    printf("HW Addr Source: %x:%x:%x:%x:%x:%x\n", a->srcmac[0], a->srcmac[1], a->srcmac[2], a->srcmac[3],
           a->srcmac[4], a->srcmac[5]);
    printf("IP Source: %d.%d.%d.%d\n", a->srcip[0], a->srcip[1], a->srcip[2], a->srcip[3]);
    printf("HW Addr Destination: %x:%x:%x:%x:%x:%x\n", a->dstmac[0], a->dstmac[1], a->dstmac[2], a->dstmac[3],
           a->dstmac[4], a->dstmac[5]);
    printf("IP Destination: %d.%d.%d.%d\n", a->dstip[0], a->dstip[1], a->dstip[2], a->dstip[3]);
}

void ip_print(struct ip_datagram *i) {
    unsigned int ihl = (i->ver_ihl & 0x0F) * 4;
    unsigned int totlen = htons(i->len);
    unsigned int opt_len = ihl - 20;

    printf("\n\n ***** IP Packet *****\n");
    printf("Version: %d\n", ((i->ver_ihl) & 0xF0) >> 4);
    printf("IHL (bytes 60max): %d\n", ihl);
    printf("TOS: %d\n", i->tos);
    printf("Lunghezza totale: %d\n", totlen);
    printf("ID: %x\n", htons(i->id));
    unsigned char flags = (unsigned char) (htons(i->flag_offs) >> 13);
    printf("Flags: %d | %d | %d \n", flags & 4, flags & 2, flags & 1);
    printf("Fragment Offset: %d\n", htons(i->flag_offs) & 0x1FFF);
    printf("TTL: %d\n", i->ttl);
    printf("Protocol: %d\n", i->proto);
    printf("Checksum: %x\n", htons(i->checksum));

    unsigned char *saddr = (unsigned char *) &i->src;
    unsigned char *daddr = (unsigned char *) &i->dst;

    printf("IP Source: %d.%d.%d.%d\n", saddr[0], saddr[1], saddr[2], saddr[3]);
    printf("IP Destination: %d.%d.%d.%d\n", daddr[0], daddr[1], daddr[2], daddr[3]);

    if (ihl > 20) {
        printf("Options: ");
        for (int j = 0; j < opt_len; j++) {
            printf("%.3d(%.2x) ", i->payload[j], i->payload[j]);
        }
        printf("\n");
    }
}

void icmp_print(struct icmp_packet *i) {
    printf("\n\n ***** ICMP Packet *****\n");
    printf("Type: %d\n", i->type);
    printf("Code: %d\n", i->code);
    printf("Checksum: 0x%x\n", htons(i->checksum));
    printf("ID: %d\n", htons(i->id));
    printf("Sequence: %d\n", htons(i->seq));
}