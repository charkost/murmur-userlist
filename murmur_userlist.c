/*
* Murmur's ICE getUsers request + response parsing in order to print userlist.
* Works only with a Murmur that supports ICE's version >=3.4 and runs at localhost.
* Charalampos Kostas <root@charkost.gr>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MURMUR_PORT 6502
#define ICE_ISA_REPLY_PACKET_SIZE 26
#define VALIDATE_CONNECTION_PACKET_SIZE 14
#define READ_BUFFER_SIZE 5000

int main(void)
{
	const unsigned char ice_isA_packet[] =
	{
		0x49, 0x63, 0x65, 0x50, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x01, 0x00,
		0x00, 0x00, 0x04, 0x4d, 0x65, 0x74, 0x61, 0x00, 0x00, 0x07, 0x69, 0x63, 0x65, 0x5f, 0x69, 0x73,
		0x41, 0x01, 0x00, 0x15, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0e, 0x3a, 0x3a, 0x4d, 0x75, 0x72, 0x6d,
		0x75, 0x72, 0x3a, 0x3a, 0x4d, 0x65, 0x74, 0x61
	};
	const unsigned char getUsers_packet[] =
	{
		0x49, 0x63, 0x65, 0x50, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x07, 0x00,
		0x00, 0x00, 0x01, 0x31, 0x01, 0x73, 0x00, 0x08, 0x67, 0x65, 0x74, 0x55, 0x73, 0x65, 0x72, 0x73,
		0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00
	};
	unsigned char read_buffer[READ_BUFFER_SIZE];
	struct sockaddr_in serv_addr;
	int adder_sockfd = 0;
	unsigned char *username = read_buffer;

	if ((adder_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "\nError: Could not create socket\n");
		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(MURMUR_PORT);
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(adder_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "\nError: Failed to connect to Murmur server\n");
		return 1;
	}
	if (read(adder_sockfd, read_buffer, READ_BUFFER_SIZE) != VALIDATE_CONNECTION_PACKET_SIZE) {
		fprintf(stderr, "\nError: Failed to receive validate_packet\n");
		return 1;
	}
	if (write(adder_sockfd, ice_isA_packet, sizeof(ice_isA_packet)) < 0) {
		fprintf(stderr, "\nError: Failed to send ice_isA_packet\n");
		return 1;
	}
	if (read(adder_sockfd, read_buffer, READ_BUFFER_SIZE) != ICE_ISA_REPLY_PACKET_SIZE) {
		fprintf(stderr, "\nError: Failed to receive ice_isA_packet success reply\n");
		return 1;
	}

	if (write(adder_sockfd, getUsers_packet, sizeof(getUsers_packet)) < 0) {
		fprintf(stderr, "\nError: Failed to send getUsers_packet\n");
		return 1;
	}

	if (read(adder_sockfd, read_buffer, READ_BUFFER_SIZE) < 0) {
		fprintf(stderr, "\nError: Failed to receive getUsers_packet reply\n");
		return 1;
	}

	/* read_buffer[25] = number of users */
	printf("%u Online Client%s%s", read_buffer[25], read_buffer[25] == 1 ? "" : "s", read_buffer[25] == 0 ? "" : ": ");
	
	while(read_buffer[25]-- > 0x0) {

		while(!((username[0] == 0xff) && (username[1] == 0xff) && (username[2] == 0xff) && (username[3] == 0xff)))
			username++;

		username += 0x10;
		username[(unsigned)*(username - 1)] = '\0';
		printf(read_buffer[25] != 0 ? "%s, " : "%s", username);
	}

	printf("\n");
	close(adder_sockfd);
	return 0;
}
