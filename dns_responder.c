/*
 * DNS Responder
 * -------------
 * by gellai.com
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef _WIN32
	#include <WinSock2.h>
	#pragma comment(lib, "Ws2_32.lib")
	WSADATA wsa;
#endif

#ifdef __linux__
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <unistd.h>
#endif

/* Maximum packet length */
#define BUFLEN 1024

/* Domains will be resolved as this IP */
#define REDIRECT_TO_IP "52.44.42.61" /* <-----------------REPLACE IP ADDRESS */

/* DNS port number */
#define PORT 53

/* Exit when error occurs */
void die(char *s) 
{
	perror(s);
	exit(1);
}

/* Clear the packet buffer */
void clear_buffer(char *b) 
{
	int i;

	for(i=0; i<BUFLEN; i++)
		b[i] = 0xFF;
}

/* Check if DNS request is valid*/
int valid_request(char *r_buf) 
{
	if(r_buf[2] == 0x01)
		return 1;
	else
		return 0;
}

/* Response for DNS request */
void dns_response(int *r_socket, char *r_buf, int *size, int *r_ip, struct in_addr *dst_ip, unsigned short dst_port) 
{
	struct sockaddr_in si_response;
	int r_size = sizeof(si_response);

	r_buf[02] = 0x81;
	r_buf[03] = 0x80;
	r_buf[07] = 0x01;
	r_buf[*size+0] = 0xc0;
	r_buf[*size+1] = 0x0c;
	r_buf[*size+2] = 0x00;
	r_buf[*size+3] = 0x01;
	r_buf[*size+4] = 0x00;
	r_buf[*size+5] = 0x01;
	r_buf[*size+6] = 0x00;
	r_buf[*size+7] = 0x00;
	r_buf[*size+8] = 0x00;
	r_buf[*size+9] = 0x01;
	r_buf[*size+10] = 0x00;
	r_buf[*size+11] = 0x04;
	/* Redirecting IP address */
	r_buf[*size+12] = r_ip[0];
	r_buf[*size+13] = r_ip[1];
	r_buf[*size+14] = r_ip[2];
	r_buf[*size+15] = r_ip[3];

	memset( (char *)&si_response, 0, sizeof(si_response) );

	si_response.sin_addr = *dst_ip;
	si_response.sin_family = AF_INET;
	si_response.sin_port = htons(dst_port);

	if( sendto(*r_socket, r_buf, BUFLEN, 0, (struct sockaddr *)&si_response, r_size) == -1 )
			die("Response error");

	printf("SERVER: %d Bytes received from %s:%d \n", 
		*size, inet_ntoa(*dst_ip), dst_port);
}

int main() 
{
	struct sockaddr_in si_server, 
					   si_client;
	int c_socket, 
		c_size = sizeof(si_client),
		bytes_received,
		red_ip[4];
	char buf[BUFLEN];
	
	if(sscanf(REDIRECT_TO_IP, "%d.%d.%d.%d", &red_ip[0], &red_ip[1], &red_ip[2], &red_ip[3]) != 4)
        die("The given IP address is invalid");

	clear_buffer(buf);

#ifdef _WIN32
		if( WSAStartup(MAKEWORD(2,2), &wsa) != 0 ) {
			printf("Error code: %d\n", WSAGetLastError());
			die("Failed");
		}

		if( (c_socket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, 0)) == INVALID_SOCKET ) {
			printf("Error code: %d\n", WSAGetLastError());
			die("Could not create socket");
		}
#endif

#ifdef __linux__
		if( (c_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 ) 
			die("Socket error");
#endif

	memset( (char *)&si_server, 0, sizeof(si_server) );

	si_server.sin_family = AF_INET;
	si_server.sin_port = htons(PORT);
	si_server.sin_addr.s_addr = htonl(INADDR_ANY);

	if ( bind(c_socket, (struct sockaddr *)&si_server, sizeof(si_server)) == -1 )
		die("Bind error");

	printf("Ready and listening!\nResolving DNS queries as %s\n", REDIRECT_TO_IP);

	while(1) {
		if ( (bytes_received = recvfrom(c_socket, buf, BUFLEN, 0, (struct sockaddr *)&si_client, &c_size)) == -1 )
			die("Receiving error");
		
		if( valid_request(buf) )
			dns_response(&c_socket, buf, &bytes_received, red_ip, &si_client.sin_addr, ntohs(si_client.sin_port));

		clear_buffer(buf);
	}

	shutdown(c_socket, 2);

	return 0;
}
