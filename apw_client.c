#include <arpa/inet.h>
#include <errno.h>
#include <net/ethernet.h>
#include <pcap.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/select.h>

#define MAXLEN    1024
#define ETHER_LEN 14
#define EVENT_SIZE (sizeof (struct inotify_event))

struct arp_header {
  uint16_t htype;
  uint16_t ptype;
  uint8_t hlen;
  uint8_t plen;
  uint16_t opcode;
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];
};

void handle_arp_traffic(u_char *ptrnull, const struct pcap_pkthdr *pkt_info, const u_char *packet) {
  struct ether_header *ethernet      = (struct ether_header *) packet;
  struct arp_header   *arp           = (struct arp_header *) (packet + ETHER_LEN);

  printf("Packet Found!\n");

  // Validate Ethernet Values
  printf("Ethernet Type: %04hx\n", ethernet->ether_type);
  printf("Ethernet Destination: %02X:%02X:%02X:%02X:%02X:%02X\n",
    ethernet->ether_dhost[0], ethernet->ether_dhost[1], ethernet->ether_dhost[2],
    ethernet->ether_dhost[3], ethernet->ether_dhost[4], ethernet->ether_dhost[5]
  );
  printf("Ethernet Sender:      %02X:%02X:%02X:%02X:%02X:%02X\n",
    ethernet->ether_shost[0], ethernet->ether_shost[1], ethernet->ether_shost[2],
    ethernet->ether_shost[3], ethernet->ether_shost[4], ethernet->ether_shost[5]
  );

  // Validate ARP Values
  printf("ARP Opcode: %d\n", ntohs(arp->opcode));
  printf("ARP Sender MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
    arp->sender_mac[0], arp->sender_mac[1], arp->sender_mac[2],
    arp->sender_mac[3], arp->sender_mac[4], arp->sender_mac[5]
  );
  printf("ARP Target MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
    arp->target_mac[0], arp->target_mac[1], arp->target_mac[2],
    arp->target_mac[3], arp->target_mac[4], arp->target_mac[5]
  );
}

void read_config(char *string, char *token) {
  char *tmp;
  strcpy(string, token);
  tmp = strstr(string, "\n");
  *tmp = '\0';
}

void monitor_arp_table() {
  int fd, wd, ret, len, i;
  char buf[MAXLEN];
  fd_set rfds;
  static struct inotify_event *event;

  if ((fd = inotify_init()) < 0) {
    perror("inotify_init");
    exit(1);
  }

  // /proc/net/arp
  if ((wd = inotify_add_watch (fd, "/home/brandondg/Documents/BTECH_T4/COMP8045/ARPPoisonWall/something", (uint32_t)IN_MODIFY)) < 0) {
    perror("inotify_add_watch");
  }

  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);

  while (1) {
    ret = select(fd + 1, &rfds, NULL, NULL, NULL);
	  len = read(fd, buf, MAXLEN);

    i = 0;
		if (len < 0) {
      if (errno == EINTR) {
			  perror ("Select Read");
      } else {
        perror ("Select Read");
      }
		} else if (!len) {
			printf ("Buffer too small!\n");
			exit (1);
		}

    while (i < len) {
      event = (struct inotify_event *) &buf[i];
  	  i += EVENT_SIZE + event->len;
    }

    printf("%s\n", event->name);

    if (ret < 0) {
      perror ("select");
    } else if (!ret) {
      printf ("timed out\n");
    } else if (FD_ISSET (fd, &rfds)) {
      /*
      if (event->mask & IN_CREATE) {
        if ((strcmp("arp", event->name)) == 0) {
        }
      } */
		}
  }
}

int main() {
  char *r_type, *a_email, *thold, *router, *server, *token;
  char fbuf[MAXLEN], error[PCAP_ERRBUF_SIZE];
  struct bpf_program fp;
  bpf_u_int32 netp;
  FILE* ccfp;
  pcap_if_t *interfaces;
  pcap_t *nic_descr;

  // Open the configuration file. Exit if the file does not open.
  if ((ccfp = fopen("apw_client_configuration.conf", "r")) == 0) {
    fprintf(stderr, "Client Configuration File.\n");
    exit(1);
  }

  // Parse the configuration file and assign pointers appropriately.
  while (fgets(fbuf, MAXLEN, ccfp)) {
    token = strtok(fbuf, " ");
    if (strcmp(token, "ReactionType") == 0) {
      token = strtok(NULL, " ");
      r_type = malloc(sizeof(char) * (strlen(token) + 1));
      read_config(r_type, token);
    } else if (strcmp(token, "Email") == 0) {
      token = strtok(NULL, " ");
      a_email = malloc(sizeof(char) * (strlen(token) + 1));
      read_config(a_email, token);
    } else if (strcmp(token, "Threshold") == 0) {
      token = strtok(NULL, " ");
      thold = malloc(sizeof(char) * (strlen(token) + 1));
      read_config(thold, token);
    } else if (strcmp(token, "Router") == 0) {
      token = strtok(NULL, " ");
      router = malloc(sizeof(char) * (strlen(token) + 1));
      read_config(router, token);
    } else if (strcmp(token, "Server") == 0) {
      token = strtok(NULL, " ");
      server = malloc(sizeof(char) * (strlen(token) + 1));
      read_config(server, token);
    }
  }

  // Find network interface to listen for traffic.
  if (pcap_findalldevs(&interfaces, error) == -1) {
    perror("pcap_findalldevs");
    exit(1);
  }

  // Verify network interface
  printf("Interface Card: %s\n", interfaces->name);
  // Verify configuration
  printf("Reaction Type(s): %s\n", r_type);
  printf("Admin Email(s): %s\n", a_email);
  printf("Threshold: %s\n", thold);
  printf("Router: %s\n", router);
  printf("Server: %s\n", server);

  //monitor_arp_table();

  //nic_descr = pcap_open_live(nic_dev, BUFSIZ, 1, -1, error);
  if ((nic_descr = pcap_open_live(interfaces->name, BUFSIZ, 1, -1, error)) == NULL) {
		printf("pcap_open_live(): %s\n", error);
		exit(1);
	}

  // Compile the filter expression
  if (pcap_compile(nic_descr, &fp, "arp", 0, netp) == -1) {
    fprintf(stderr,"Error calling pcap_compile\n");
    exit(1);
  }

  // Load the filter into the capture device
  if (pcap_setfilter(nic_descr, &fp) == -1) {
    fprintf(stderr,"Error setting filter\n");
    exit(1);
  }

  // Start the capture session
  pcap_loop(nic_descr, 0, handle_arp_traffic, NULL);

  pcap_freealldevs(interfaces);

  return 0;
}
