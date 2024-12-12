#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE], server_reply[BUFFER_SIZE];

    printf("Winsock başlatılıyor...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Başlatma hatası. Kod: %d\n", WSAGetLastError());
        return 1;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket oluşturulamadı. Kod: %d\n", WSAGetLastError());
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bağlantı hatası");
        return 1;
    }

    printf("Sunucuya baglanildi!\n");

    while (1) {
        printf("Komut (listele / satin al <urun_adi>): ");
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = '\0';

        if (send(sock, message, strlen(message), 0) < 0) {
            printf("Gönderim hatası\n");
            break;
        }

        int read_size = recv(sock, server_reply, BUFFER_SIZE, 0);
        if (read_size > 0) {
            server_reply[read_size] = '\0';
            printf("Sunucu: %s\n", server_reply);
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
