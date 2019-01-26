#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLEN 1024

void read_config(char *string, char *token) {
  char *tmp;
  strcpy(string, token);
  tmp = strstr(string, "\n");
  *tmp = '\0';
}

int main() {
  char *r_type, *a_email, *thold, *router, *server, *token;
  char fbuf[MAXLEN];
  FILE* ccfp;

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

  

  // Verify configuration
  printf("Reaction Type(s): %s\n", r_type);
  printf("Admin Email(s): %s\n", a_email);
  printf("Threshold: %s\n", thold);
  printf("Router: %s\n", router);
  printf("Server: %s\n", server);
}
