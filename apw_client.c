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

struct callback_struct {
  char *routerip;
  char *routermac;
};

void handle_arp_traffic(u_char *ptrnull, const struct pcap_pkthdr *pkt_info, const u_char *packet) {
  char str[INET_ADDRSTRLEN];
  char macStr[18];

  struct ether_header *ethernet      = (struct ether_header *) packet;
  struct arp_header   *arp           = (struct arp_header *) (packet + ETHER_LEN);

  struct callback_struct *cbs = (struct callback_struct *)(ptrnull);
  printf("In Callback Funtion Router IP: %s\n", cbs->routerip);
  printf("In Callback Funtion Router MAC: %s\n", cbs->routermac);

  inet_ntop(AF_INET, arp->sender_ip, str, INET_ADDRSTRLEN);
  printf("%s\n", str);

  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
         arp->sender_mac[0], arp->sender_mac[1], arp->sender_mac[2],
         arp->sender_mac[3], arp->sender_mac[4], arp->sender_mac[5]);
  printf("%s\n", macStr);

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

  if (memcmp(cbs->routerip, str, sizeof(str)) == 0) {
    printf("Found IP\n");
    if (memcmp(cbs->routermac, macStr, sizeof(macStr)) == 0) {
      // This is actually where we are okay. Both values match.
      printf("Found MAC\n");
    } else {
      // This is where we know there is something bad happening.
      // sprintf(cmd,"sendmail %s < %s", to, "mail"); // prepare command.
    }
  }
}

void read_config(char *string, char *token) {
  char *tmp;
  strcpy(string, token);
  tmp = strstr(string, "\n");
  *tmp = '\0';
}

void create_arp_entry(char *buffer, char *routerip, char *routermac) {
  memset(buffer, 0x0, sizeof(buffer));
  strcat(buffer, "arp -s ");
  strcat(buffer, routerip);
  strcat(buffer, " ");
  strcat(buffer, routermac);
  system(buffer);
}

void handle_arp_table(char *routerip, char *routermac) {
  FILE *fp;
  char buf[MAXLEN], arp_line[MAXLEN], *start, *end;

  // Run arp -a and pipe the results into a filepointer.
  if ((fp = popen("arp -a", "r")) == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  // Read the stdout from the command
  while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
    printf("%s", buf);
    // Find router entry, continue if not present.
    if (!(start = strstr(buf, routerip))) {
  		continue;
    }
  	if (!(end = strstr(start, " "))) {
  		continue;
    }

    // Analyze router entry.
    // If it is permanent, leave it. If it is not, make it permanent.
    memset(arp_line, 0x0, sizeof(arp_line));
 	  strncpy(arp_line, start, (end - start - 1));
    if (strcmp(arp_line, routerip) == 0) {
      printf("Found Router Line: %s\n", arp_line);
      if ((start = strstr(buf, "PERM"))) {
        printf("Perm Router Entry Found\n");
    		return;
      } else {
        printf("Non-Perm Router Entry Found\n");
        create_arp_entry(arp_line, routerip, routermac);
        return;
      }
    }
  }

  // No router entry found, make one.
  create_arp_entry(arp_line, routerip, routermac);

  // Close command pointer.
  pclose(fp);
}

int main() {
  char *r_type, *a_email, *thold, *routerip, *routermac, *server, *token;
  char fbuf[MAXLEN], error[PCAP_ERRBUF_SIZE];
  struct bpf_program fp;
  struct callback_struct *cbs;
  bpf_u_int32 netp;
  FILE* ccfp;
  pcap_if_t *interfaces;
  pcap_t *nic_descr;

  cbs = (struct callback_struct *) malloc(sizeof(struct callback_struct));

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
    } else if (strcmp(token, "RouterIP") == 0) {
      token = strtok(NULL, " ");
      routerip = malloc(sizeof(char) * (strlen(token) + 1));
      read_config(routerip, token);
    } else if (strcmp(token, "Server") == 0) {
      token = strtok(NULL, " ");
      server = malloc(sizeof(char) * (strlen(token) + 1));
      read_config(server, token);
    } else if (strcmp(token, "RouterMAC") == 0) {
      token = strtok(NULL, " ");
      routermac = malloc(sizeof(char) * (strlen(token) + 1));
      read_config(routermac, token);
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
  printf("Router IP: %s\n", routerip);
  printf("Router MAC: %s\n", routermac);
  printf("Server: %s\n", server);

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

  //handle_arp_table(routerip, routermac);
  // Setup the Callback Structure
  cbs->routerip = routerip;
  cbs->routermac = routermac;

  // Start the capture session
  pcap_loop(nic_descr, 0, handle_arp_traffic, (u_char*)(cbs));

  pcap_freealldevs(interfaces);
  free(r_type);
  free(a_email);
  free(thold);
  free(routerip);
  free(routermac);
  free(server);
  free(cbs);

  return 0;
}
