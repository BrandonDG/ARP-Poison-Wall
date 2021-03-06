#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
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
#include <time.h>

// Define constants. BUFLEN is only used for testing with dummy client,
// remove later.
#define MAXLEN    1024
#define ETHER_LEN 14
#define EVENT_SIZE (sizeof (struct inotify_event))
#define BUFLEN 80

// Configuration globals.
size_t monitor_count = 0;
size_t threshold     = 0;
size_t config_port   = 0;
size_t server_port   = 0;
char   *router_ip;
char   *router_mac;
char   *server_ip;
char   *pw;

char key[] = "hW)V,>I>)Bh(T9";

// arp_header struct to parse arp packets.
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

// Callback stuct used in pcap_loop, may be removed in favour
// of more configuration globals.
struct callback_struct {
  char *routerip;
  char *routermac;
  int sd;
};

void xor_message(char *input, char *output) {
	for(size_t i = 0; i < strlen(input); i++) {
		output[i] = input[i] ^ key[i % (sizeof(key)/sizeof(char))];
	}
}

// Thread method to decrement monitor_count.
void* decrement_monitor_count() {
  while(1) {
    sleep(3);
    if (monitor_count > 0) {
      --monitor_count;
    }
  }
  return NULL;
}

// Create permanent ARP entry with given parameters.
void replace_config_value(char *buffer, char *find, char *replace1, char *replace2, char *filename) {
  memset(buffer, 0x0, sizeof(buffer));
  strcat(buffer, "sed -i '/");
  strcat(buffer, find);
  strcat(buffer, "/c\\");
  strcat(buffer, replace1);
  strcat(buffer, replace2);
  strcat(buffer, "' ");
  strcat(buffer, filename);
  system(buffer);
}

// Thread method to accomodate configuration changes.
void* configuration_interface() {
  int sd, new_sd, n, bytes_to_read, t_count;
  char buf[MAXLEN], dbuf[MAXLEN], *bp, *token;
  struct sockaddr_in client_interface, server_interface;
  socklen_t server_i_len;

  char th[20], ip[20], mac[20], rpw[20], npw[30], cc_buffer[50];

  // Create socket.
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "Can't create a socket");
		return NULL;
	}

  // Bind an address to the socket
	bzero((char *)&client_interface, sizeof(struct sockaddr_in));
	client_interface.sin_family = AF_INET;
	client_interface.sin_port = htons(config_port);
	client_interface.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sd, (struct sockaddr *)&client_interface, sizeof(client_interface)) == -1) {
    fprintf(stderr, "Can't bind name to socket\n");
		return NULL;
	}

  // Queue up to 1 connect requests.
	listen(sd, 5);

  // Accept connection and handle configuration change.
  while(1) {
    t_count = 0;
    server_i_len = sizeof(server_interface);
    if ((new_sd = accept(sd, (struct sockaddr *)&server_interface, &server_i_len)) == -1) {
      fprintf(stderr, "Can't accept client\n");
      return NULL;
    }

    if (strcmp(server_ip, inet_ntoa(server_interface.sin_addr)) == 0) {
      printf("%s\n", "Accepting connection from server");
      bp = buf;
  		bytes_to_read = MAXLEN;
  		while ((n = recv(new_sd, bp, bytes_to_read, MSG_WAITALL)) < MAXLEN) {
  			bp += n;
  			bytes_to_read -= n;
  		}
      #ifdef DEBUG
      printf("configuration_interface: Configuration Thread Received = %s\n", buf);
      #endif

      xor_message(buf, dbuf);
      printf("configuration_interface: Configuration Thread Received = %s\n", dbuf);

      token = strtok(dbuf, "\"");
      while ( token != NULL ) {
        if (t_count == 3) {
          strcpy(rpw, token);
        } else if (t_count == 9) {
          strcpy(mac, token);
        } else if (t_count == 13) {
          strcpy(ip, token);
        } else if (t_count == 17) {
          strcpy(th, token);
        } else if (t_count == 21) {
          strcpy(npw, token);
        }
        token = strtok(NULL, "\"");
        ++t_count;
      }

      printf("%s\n %s\n %s\n %s\n %s\n", rpw, mac, ip, th, npw);

      if (strcmp(rpw, pw) == 0) {
        /*
        printf("Communication Accepted\n");
        if (strcmp(mac, "Empty") != 0) {
          router_mac = mac;
          printf("Set MAC value: %s\n", router_mac);
          replace_config_value(cc_buffer, "RouterMAC", "RouterMAC ", router_mac, "apw_client_configuration.conf");
        }
        if (strcmp(ip, "Empty") != 0) {
          router_ip = ip;
          printf("Set IP value: %s\n", router_ip);
          replace_config_value(cc_buffer, "RouterIP", "RouterIP ", router_ip, "apw_client_configuration.conf");
        }
        if (strcmp(npw, "Empty") != 0) {
          pw = npw;
          printf("Set PW value: %s\n", pw);
          replace_config_value(cc_buffer, "Password", "Password ", pw, "apw_client_configuration.conf");
        }

        if (strcmp(th, "Strict") == 0) {
          printf("Set Strict\n");
          threshold = 1;
        } else if (strcmp(th, "Normal") == 0) {
          printf("Set Normal\n");
          threshold = 4;
        } else if (strcmp(th, "Lenient") == 0) {
          printf("Set Lenient\n");
          threshold = 7;
        }
        replace_config_value(cc_buffer, "Threshold", "Threshold ", th, "apw_client_configuration.conf");
        */
      }
    }

    close(new_sd);
  }
}

// Function used in pcap_loop to monitor arp traffic and behave accordingly.
void handle_arp_traffic(u_char *ptrnull, const struct pcap_pkthdr *pkt_info, const u_char *packet) {
  char str[INET_ADDRSTRLEN];
  char my_ip[INET_ADDRSTRLEN];
  char macStr[18];

  struct ether_header *ethernet      = (struct ether_header *) packet;
  struct arp_header   *arp           = (struct arp_header *) (packet + ETHER_LEN);

  struct callback_struct *cbs = (struct callback_struct *)(ptrnull);

  // Parse IP and MAC address in ARP packet to compare against configuration.
  inet_ntop(AF_INET, arp->sender_ip, str, INET_ADDRSTRLEN);
  inet_ntop(AF_INET, arp->target_ip, my_ip, INET_ADDRSTRLEN);
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
         arp->sender_mac[0], arp->sender_mac[1], arp->sender_mac[2],
         arp->sender_mac[3], arp->sender_mac[4], arp->sender_mac[5]);

  #ifdef DEBUG
  printf("-------------------------------------------------\n");
  // To validate that the callback function has the correct values
  printf("handle_arp_traffic: Router IP            =  %s\n", cbs->routerip);
  printf("handle_arp_traffic: Router MAC           =  %s\n", cbs->routermac);
  // To display the parsed values in the callback function
  printf("handle_arp_traffic: Parsed IP            =  %s\n", str);
  printf("handle_arp_traffic: Parsed MAC           =  %s\n", macStr);
  // Validate Ethernet Values
  printf("handle_arp_traffic: Ethernet Type        =  %04hx\n", ethernet->ether_type);
  printf("handle_arp_traffic: Ethernet Destination =  %02X:%02X:%02X:%02X:%02X:%02X\n",
    ethernet->ether_dhost[0], ethernet->ether_dhost[1], ethernet->ether_dhost[2],
    ethernet->ether_dhost[3], ethernet->ether_dhost[4], ethernet->ether_dhost[5]
  );
  printf("handle_arp_traffic: Ethernet Sender      =  %02X:%02X:%02X:%02X:%02X:%02X\n",
    ethernet->ether_shost[0], ethernet->ether_shost[1], ethernet->ether_shost[2],
    ethernet->ether_shost[3], ethernet->ether_shost[4], ethernet->ether_shost[5]
  );
  // Validate ARP Values
  printf("handle_arp_traffic: ARP Opcode     = %d\n", ntohs(arp->opcode));
  printf("handle_arp_traffic: ARP Sender MAC = %02X:%02X:%02X:%02X:%02X:%02X\n",
    arp->sender_mac[0], arp->sender_mac[1], arp->sender_mac[2],
    arp->sender_mac[3], arp->sender_mac[4], arp->sender_mac[5]
  );
  printf("handle_arp_traffic: ARP Target MAC =  %02X:%02X:%02X:%02X:%02X:%02X\n",
    arp->target_mac[0], arp->target_mac[1], arp->target_mac[2],
    arp->target_mac[3], arp->target_mac[4], arp->target_mac[5]
  );
  printf("-------------------------------------------------\n");
  #endif

  // Compare RouterIP and ARP packet IP.
  if (memcmp(router_ip, str, sizeof(str)) == 0) {
    #ifdef DEBUG
    printf("handle_arp_traffic: IP Entry Matched Router IP\n");
    #endif
    // Compare RouterMAC and ARP packet MAC.
    if (memcmp(router_mac, macStr, sizeof(macStr)) == 0) {
      // This is actually where we are okay. Both values match.
      #ifdef DEBUG
      printf("handle_arp_traffic: MAC Entry Matched Router MAC\n");
      #endif
    } else {
      time_t t = time(NULL);
      struct tm tm = *localtime(&t);

      // This is where we know there is something bad happening.
      char sbuf[MAXLEN], timestamp[MAXLEN], ebuf[MAXLEN];
      memset(ebuf, 0x0, MAXLEN);
      memset(sbuf, 0x0, MAXLEN);
      sprintf(timestamp, "%4d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
      sprintf(sbuf, "{\"password\": \"%s\", \"type\": \"%s\", \"payload\": {\"TimeStamp\": \"%s\", \"From\": \"%s\", \"To\": \"%s\"}}",
    	   pw, "log", timestamp, macStr, my_ip);
      printf("%s\n", sbuf);

      xor_message(sbuf, ebuf);
      send(cbs->sd, ebuf, strlen(ebuf), 0);
      printf("Issue Log\n");
      memset(ebuf, 0x0, MAXLEN);
      memset(sbuf, 0x0, MAXLEN);

      sleep(1);
      ++monitor_count;
      if (monitor_count > threshold) {
        printf("Issue Alert\n");
        sprintf(sbuf, "{\"password\": \"%s\", \"type\": \"%s\", \"payload\": {\"TimeStamp\": \"%s\", \"From\": \"%s\", \"To\": \"%s\"}}",
      	   pw, "alert", timestamp, macStr, my_ip);
        printf("%s\n", sbuf);

        xor_message(sbuf, ebuf);
        send(cbs->sd, ebuf, strlen(ebuf), 0);

      }
    }
  }
}

// Read config token to buffer.
void read_config(char *string, char *token) {
  char *tmp;
  strcpy(string, token);
  tmp = strstr(string, "\n");
  *tmp = '\0';
}

// Create permanent ARP entry with given parameters.
void create_arp_entry(char *buffer, char *routerip, char *routermac) {
  memset(buffer, 0x0, sizeof(buffer));
  strcat(buffer, "arp -s ");
  strcat(buffer, routerip);
  strcat(buffer, " ");
  strcat(buffer, routermac);
  system(buffer);
}

// Analyze ARP table and create entries if needed.
void handle_arp_table(char *routerip, char *routermac) {
  FILE *fp;
  char buf[MAXLEN], arp_line[MAXLEN], *start, *end;

  // Run arp -a and pipe the results into a filepointer.
  if ((fp = popen("arp -a", "r")) == NULL) {
    fprintf(stderr, "Failed to run command\n" );
    exit(1);
  }

  // Read the stdout from the command
  while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
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
      #ifdef DEBUG
      printf("handle_arp_table: Router Line = %s\n", arp_line);
      #endif
      if ((start = strstr(buf, "PERM"))) {
        #ifdef DEBUG
        printf("handle_arp_table: Perm Router Entry Found\n");
        #endif
    		return;
      } else {
        #ifdef DEBUG
        printf("handle_arp_table: Non-Perm Router Entry Found\n");
        #endif
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

// Main.
int main() {
  char *cport, *thold, *routerip, *routermac, *server, *token, *s_port;
  char *password;
  char fbuf[MAXLEN], error[PCAP_ERRBUF_SIZE];
  struct bpf_program fp;
  struct callback_struct *cbs;
  bpf_u_int32 netp;
  FILE* ccfp;
  pcap_if_t *interfaces;
  pcap_t *nic_descr;
  pthread_t decrement_thread, configuration_thread;
  int sd;
  struct hostent *hp;
  struct sockaddr_in apw_server;

  cport = thold = routerip = routermac = server = s_port = NULL;
  cbs = (struct callback_struct *) malloc(sizeof(struct callback_struct));

  // Open the configuration file. Exit if the file does not open.
  if ((ccfp = fopen("apw_client_configuration.conf", "r")) == 0) {
    fprintf(stderr, "Client Configuration File.\n");
    exit(1);
  }

  // Parse the configuration file and assign pointers appropriately.
  while (fgets(fbuf, MAXLEN, ccfp)) {
    token = strtok(fbuf, " ");
    if (strcmp(token, "ConfigServerPort") == 0) {
      token = strtok(NULL, " ");
      cport = malloc(sizeof(char) * (strlen(token) + 1));
      read_config(cport, token);
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
    } else if (strcmp(token, "ServerPort") == 0) {
      token = strtok(NULL, " ");
      s_port = malloc(sizeof(char) * (strlen(token) + 1));
      read_config(s_port, token);
    } else if (strcmp(token, "Password") == 0) {
      token = strtok(NULL, " ");
      password = malloc(sizeof(char) * (strlen(token) + 1));
      read_config(password, token);
    }
  }

  // Ensure that each configuration option has been found.
  if ((cport == NULL) || (thold == NULL) || (routerip == NULL) || (routermac == NULL) ||
             (server == NULL) || (s_port == NULL)) {
    fprintf(stderr, "Configuration Incomplete.\n");
    exit(1);
  }

  // Point global config value (routerip) to read value.
  router_ip = routerip;
  // Point global config value (routermac) to read value.
  router_mac = routermac;
  // Point global config value (server) to read value.
  server_ip = server;
  // Point global config value (password) to read value.
  pw = password;

  // Set threshold based on configuration.
  if (strcmp(thold, "Strict") == 0) {
    threshold = 1;
  } else if (strcmp(thold, "Normal") == 0) {
    threshold = 4;
  } else if (strcmp(thold, "Lenient") == 0) {
    threshold = 7;
  }

  // Parse configuration port based on configuration.
  config_port = strtoul(cport, NULL, 0);
  if (errno == EINVAL || errno == ERANGE) {
    fprintf(stderr, "strtoul config_port conversion.\n");
    exit(1);
  }

  // Parse configuration port based on configuration.
  server_port = strtoul(s_port, NULL, 0);
  if (errno == EINVAL || errno == ERANGE) {
    fprintf(stderr, "strtoul server_port conversion.\n");
    exit(1);
  }

  // Find network interface to listen for traffic.
  if (pcap_findalldevs(&interfaces, error) == -1) {
    fprintf(stderr, "pcap_findalldevs.\n");
    exit(1);
  }

  // Verify network interface
  printf("Interface Card: %s\n", interfaces->name);
  // Verify configuration
  printf("Threshold: %s\n", thold);
  printf("Password: %s\n", pw);
  printf("Router IP: %s\n", router_ip);
  printf("Router MAC: %s\n", router_mac);
  printf("Server: %s\n", server);
  printf("Server Port: %lu\n", server_port);
  printf("Configuration Port: %lu\n", config_port);

  // Open interface to listen on.
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

  // Establish server here
  bzero((char *)&apw_server, sizeof(struct sockaddr_in));
  apw_server.sin_family = AF_INET;
  apw_server.sin_port = htons(server_port);
  if ((hp = gethostbyname("localhost")) == NULL) {
    fprintf(stderr, "Unknown server address\n");
    exit(1);
  }
  bcopy(hp->h_addr, (char *)&apw_server.sin_addr, hp->h_length);

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Cannot create socket");
    exit(1);
  }

  if (connect(sd, (struct sockaddr *)&apw_server, sizeof(apw_server)) == -1) {
    fprintf(stderr, "Can't connect to server\n");
    perror("connect");
    exit(1);
  }

  // Setup the Callback Structure
  cbs->routerip = routerip;
  cbs->routermac = routermac;
  cbs->sd = sd;

  #ifdef DEBUG
  printf("Debug Defined\n");
  #endif

  // Start threads.
  //pthread_create(&decrement_thread, NULL, decrement_monitor_count, NULL);
  pthread_create(&configuration_thread, NULL, configuration_interface, NULL);

  // Handle the ARP table.
  //handle_arp_table(routerip, routermac);

  while (1) {
    
  }

  // Start the capture session
  //pcap_loop(nic_descr, 0, handle_arp_traffic, (u_char*)(cbs));

  // Stop threads and free memory.
  //pthread_kill(decrement_thread, SIGUSR1);
  pthread_kill(configuration_thread, SIGUSR1);
  pcap_freealldevs(interfaces);
  free(cport);
  free(thold);
  free(routerip);
  free(router_ip);
  free(routermac);
  free(router_mac);
  free(server);
  free(server_ip);
  free(password);
  free(pw);
  free(cbs);
  close(sd);

  return 0;
}
