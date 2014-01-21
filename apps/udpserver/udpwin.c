/* udpserv.c */
/* A simple UDP server that sends the current date and time to the client */
/* Last modified: September 20, 2005 */
/* http://www.gomorgan89.com */
/* Link with library file wsock32.lib */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>
#include <time.h>
#include "udpcam.h"

#define BUFFER_SIZE 4096

void usage(void);


int main(int argc, char **argv)
{
	WSADATA w;							/* Used to open windows connection */
	unsigned short port_number;			/* Port number to use */
	int a1, a2, a3, a4;					/* Components of address in xxx.xxx.xxx.xxx form */
	SOCKET sd;							/* Socket descriptor of server */
	struct sockaddr_in server;			/* Information about the server */
	struct sockaddr_in client;			/* Information about the client */
	char buffer[BUFFER_SIZE];			/* Where to store received data */
	struct hostent *hp;					/* Information about this computer */
	char host_name[256];				/* Name of the server */
    short* data = (short*) buffer;

	/* Interpret command line */
	if (argc == 2)
	{
		/* Use local address */
		if (sscanf(argv[1], "%u", &port_number) != 1)
		{
			usage();
		}
	}
	else if (argc == 3)
	{
		/* Copy address */
		if (sscanf(argv[1], "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4)
		{
			usage();
		}
		if (sscanf(argv[2], "%u", &port_number) != 1)
		{
			usage();
		}
	}
	else
	{
		usage();
	}

	/* Open windows connection */
	if (WSAStartup(0x0101, &w) != 0)
	{
		fprintf(stderr, "Could not open Windows connection.\n");
		exit(0);
	}

	/* Open a datagram socket */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == INVALID_SOCKET)
	{
		fprintf(stderr, "Could not create socket.\n");
		WSACleanup();
		exit(0);
	}

	/* Clear out server struct */
	memset((void *)&server, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	server.sin_family = AF_INET;
	server.sin_port = htons(port_number);

	/* Set address automatically if desired */
	if (argc == 2)
	{
		/* Get host name of this computer */
		gethostname(host_name, sizeof(host_name));
		hp = gethostbyname(host_name);

		/* Check for NULL pointer */
		if (hp == NULL)
		{
			fprintf(stderr, "Could not get host name.\n");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}
		
		/* Assign the address */
		server.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
		server.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
		server.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
		server.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
	}
	/* Otherwise assign it manually */
	else
	{
		server.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)a1;
		server.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)a2;
		server.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)a3;
		server.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)a4;
	}

	/* Bind address to socket */
	if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "Could not bind name to socket.\n");
		closesocket(sd);
		WSACleanup();
		exit(0);
	}

	/* Print out server information */
	printf("Server running on %u.%u.%u.%u\n", (unsigned char)server.sin_addr.S_un.S_un_b.s_b1,
											  (unsigned char)server.sin_addr.S_un.S_un_b.s_b2,
											  (unsigned char)server.sin_addr.S_un.S_un_b.s_b3,
											  (unsigned char)server.sin_addr.S_un.S_un_b.s_b4);
	printf("Press CTRL + C to quit\n");

	/* Loop and get data from clients */
	while (1)
	{
		int client_length = (int)sizeof(struct sockaddr_in);
        short result, count;

		/* Receive bytes from client */
		count = recvfrom(sd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client, &client_length);
		if (count < sizeof(short))
		{
			fprintf(stderr, "Could not receive datagram.\n");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}

		/* Check for time request */
        result = CamCommand(ntohs(data[0]), count - sizeof(short), data + 1);
#ifdef _DEBUG
        fprintf(stderr, "=%d\n", result);
#endif
        data[0] = htons(result);
        if (result >= CAM_OK)
        {
            count = result + sizeof(short);
        }
        else
        {
            count = sizeof(short)*2;
        }

		/* Send data back */
		if (sendto(sd, (char *) data, count, 0, (struct sockaddr *)&client, client_length) != count)
		{
			fprintf(stderr, "Error sending datagram.\n");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}
	}
	closesocket(sd);
	WSACleanup();

	return 0;
}

void usage(void)
{
	fprintf(stderr, "timeserv [server_address] port\n");
	exit(0);
}
