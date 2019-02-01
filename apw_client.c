#include <arpa/inet.h>
#include <errno.h>
#include <net/ethernet.h>
#include <pcap.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/select.h>

#define MAXLEN    1024
#define ETHER_LEN 14
#define EVENT_SIZE (sizeof (struct inotify_event))

size_t monitor_count = 0;
size_t threshold     = 0;

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

void* decrement_monitor_count() {
  while(1) {
    sleep(3);
    if (monitor_count > 0) {
      --monitor_count;
    }
  }
  return NULL;
}

void* configuration_interface() {
  int sd, new_sd, port, n, bytes_to_read;
  char buf[MAXLEN], *bp;
  struct sockaddr_in client_interface, server_interface;
  socklen_t server_i_len;

  port = 8045;

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror ("Can't create a socket");
		return NULL;
	}

  // Bind an address to the socket
	bzero((char *)&client_interface, sizeof(struct sockaddr_in));
	client_interface.sin_family = AF_INET;
	client_interface.sin_port = htons(port);
  // TODO: Change it to only accept connections from server
	client_interface.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

  if (bind(sd, (struct sockaddr *)&client_interface, sizeof(client_interface)) == -1) {
		perror("Can't bind name to socket");
		return NULL;
	}

  // queue up to 1 connect requests
	listen(sd, 1);

  while(1) {
    server_i_len = sizeof(server_interface);
    if ((new_sd = accept(sd, (struct sockaddr *)&server_interface, &server_i_len)) == -1) {
      fprintf(stderr, "Can't accept client\n");
      return NULL;
    }

    bp = buf;
		bytes_to_read = MAXLEN;
		while ((n = recv(new_sd, bp, bytes_to_read, MSG_WAITALL)) < MAXLEN) {
			bp += n;
			bytes_to_read -= n;
		}
    printf("Configuration Thread Received: %s\n", buf);

    close(new_sd);
  }
}

void handle_arp_traffic(u_char *ptrnull, const struct pcap_pkthdr *pkt_info, const u_char *packet) {
  char str[INET_ADDRSTRLEN];
  char macStr[18];

  struct ether_header *ethernet      = (struct ether_header *) packet;
  struct arp_header   *arp           = (struct arp_header *) (packet + ETHER_LEN);

  struct callback_struct *cbs = (struct callback_struct *)(ptrnull);
  //printf("In Callback Funtion Router IP: %s\n", cbs->routerip);
  //printf("In Callback Funtion Router MAC: %s\n", cbs->routermac);

  inet_ntop(AF_INET, arp->sender_ip, str, INET_ADDRSTRLEN);
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
         arp->sender_mac[0], arp->sender_mac[1], arp->sender_mac[2],
         arp->sender_mac[3], arp->sender_mac[4], arp->sender_mac[5]);
  //printf("Parsed IP: %s\n", str);
  //printf("Parsed MAC: %s\n", macStr);

  /*
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
  ); */

  if (memcmp(cbs->routerip, str, sizeof(str)) == 0) {
    //printf("Found IP\n");
    if (memcmp(cbs->routermac, macStr, sizeof(macStr)) == 0) {
      // This is actually where we are okay. Both values match.
      //printf("Found MAC\n");
    } else {
      // This is where we know there is something bad happening.
      // sprintf(cmd,"sendmail %s < %s", to, "mail"); // prepare command.
      ++monitor_count;
      if (monitor_count > threshold) {
        printf("Issue Alert\n");
      }
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
  // TODO When removing old, unused config values, please put in new ones for config server
  char fbuf[MAXLEN], error[PCAP_ERRBUF_SIZE];
  struct bpf_program fp;
  struct callback_struct *cbs;
  bpf_u_int32 netp;
  FILE* ccfp;
  pcap_if_t *interfaces;
  pcap_t *nic_descr;
  pthread_t decrement_thread, configuration_thread;

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

  if (strcmp(thold, "Strict") == 0) {
    threshold = 1;
  } else if (strcmp(thold, "Normal") == 0) {
    threshold = 4;
  } else if (strcmp(thold, "Lenient") == 0) {
    threshold = 7;
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

  pthread_create(&decrement_thread, NULL, decrement_monitor_count, NULL);
  pthread_create(&configuration_thread, NULL, configuration_interface, NULL);

  // Start the capture session
  pcap_loop(nic_descr, 0, handle_arp_traffic, (u_char*)(cbs));

  pthread_kill(decrement_thread, SIGUSR1);
  pthread_kill(configuration_thread, SIGUSR1);
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
