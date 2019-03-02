#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_TCP_PORT		7000	// Default port
#define BUFLEN			1024  	// Buffer length

const char* pw = "password1";
char key[] = "hW)V,>I>)Bh(T9";

void create_alert_message(char *buffer) {
	sprintf(buffer, "{\"password\": \"%s\", \"type\": \"%s\", \"payload\": {\"TimeStamp\": \"%s\", \"From\": \"%s\", \"To\": \"%s\"}}",
	   pw, "alert", "2018-02-07 16:36:55", "192.168.0.10", "192.168.0.22");
}

void create_log_message(char *buffer) {
	sprintf(buffer, "{\"password\": \"%s\", \"type\": \"%s\", \"payload\": {\"TimeStamp\": \"%s\", \"From\": \"%s\", \"To\": \"%s\"}}",
	   pw, "log", "2018-02-07 16:36:55", "192.168.0.10", "192.168.0.22");
}

void xor_message(char *input, char *output) {
	for(size_t i = 0; i < strlen(input); i++) {
		output[i] = input[i] ^ key[i % (sizeof(key)/sizeof(char))];
	}
}

int main (int argc, char **argv) {
	int n, bytes_to_read;
	int sd, port;
	struct hostent	*hp;
	struct sockaddr_in server;
	char  *host, *bp, rbuf[BUFLEN], sbuf[BUFLEN], ebuf[BUFLEN], **pptr, *sptr;
	char str[16];

	switch(argc) {
		case 2:
			host =	argv[1];	// Host name
			port =	SERVER_TCP_PORT;
		break;
		case 3:
			host =	argv[1];
			port =	atoi(argv[2]);	// User specified port
		break;
		default:
			fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
			exit(1);
	}

	// Create the socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Cannot create socket");
		exit(1);
	}
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((hp = gethostbyname(host)) == NULL) {
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

	// Connecting to the server
	if (connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		exit(1);
	}
	printf("Connected:    Server Name: %s\n", hp->h_name);
	pptr = hp->h_addr_list;
	printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));

	create_log_message(sbuf);
	printf("Sending: \n%s\n", sbuf);
	// Transmit data through the socket
	xor_message(sbuf, ebuf);
	send(sd, ebuf, strlen(ebuf), 0);
	//memset(ebuf, 0x0, sizeof(ebuf));
	//memset(sbuf, 0x0, sizeof(sbuf));
	/*
	printf("Receive:\n");
	bp = rbuf;
	bytes_to_read = BUFLEN;
	// client makes repeated calls to recv until no more data is expected to arrive.
	n = 0;
	while ((n = recv (sd, bp, bytes_to_read, 0)) < BUFLEN) {
		bp += n;
		bytes_to_read -= n;
	}
	printf ("%s\n", rbuf); */

	printf("-------\n");
	sleep(3);

	create_alert_message(sbuf);
	printf("Sending: \n%s\n", sbuf);
	// Transmit data through the socket
	xor_message(sbuf, ebuf);
	send(sd, ebuf, strlen(ebuf), 0);
	memset(ebuf, 0x0, sizeof(ebuf));
	memset(sbuf, 0x0, sizeof(sbuf));
	/*
	printf("Receive:\n");
	bp = rbuf;
	bytes_to_read = BUFLEN;
	// client makes repeated calls to recv until no more data is expected to arrive.
	n = 0;
	while ((n = recv (sd, bp, bytes_to_read, 0)) < BUFLEN) {
		bp += n;
		bytes_to_read -= n;
	}
	printf ("%s\n", rbuf); */

	fflush(stdout);
	close (sd);
	return (0);
}
