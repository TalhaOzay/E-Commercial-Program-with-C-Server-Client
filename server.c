#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h> 

#pragma comment(lib, "ws2_32.lib") 

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    char urun_adi[50];
    int stok;
    float fiyat;
} Urun;

Urun urunler[100];
int urun_sayisi = 0;

// Ürünleri dosyadan oku
void urunleri_yukle(const char *dosya_adi) {
    FILE *dosya = fopen(dosya_adi, "r");
    if (!dosya) {
        perror("Dosya açılamadı");
        exit(EXIT_FAILURE);
    }

    char satir[BUFFER_SIZE];
    fgets(satir, BUFFER_SIZE, dosya); 
    while (fgets(satir, BUFFER_SIZE, dosya)) {
        sscanf(satir, "%49[^,],%d,%f", urunler[urun_sayisi].urun_adi, 
               &urunler[urun_sayisi].stok, &urunler[urun_sayisi].fiyat);
        urun_sayisi++;
    }
    fclose(dosya);
}

// Ürünleri listele
void urunleri_listele(SOCKET client_sock) {
    char mesaj[BUFFER_SIZE] = "Urun Listesi:\n";
    for (int i = 0; i < urun_sayisi; i++) {
        char urun_bilgisi[BUFFER_SIZE];
        sprintf(urun_bilgisi, "%s - Stok: %d - Fiyat: %.2f TL\n", 
                urunler[i].urun_adi, urunler[i].stok, urunler[i].fiyat);
        strcat(mesaj, urun_bilgisi);
    }
    send(client_sock, mesaj, strlen(mesaj), 0);
}

// Satın alma işlemi
void urun_satin_al(SOCKET client_sock, char *urun_adi) {
    char mesaj[BUFFER_SIZE];
    for (int i = 0; i < urun_sayisi; i++) {
        if (strcmp(urunler[i].urun_adi, urun_adi) == 0) {
            if (urunler[i].stok > 0) {
                urunler[i].stok--;
                sprintf(mesaj, "Satin alma basarili: %s\n", urun_adi);
            } else {
                sprintf(mesaj, "Urun stokta yok: %s\n", urun_adi);
            }
            send(client_sock, mesaj, strlen(mesaj), 0);
            return;
        }
    }
    sprintf(mesaj, "Urun bulunamadi: %s\n", urun_adi);
    send(client_sock, mesaj, strlen(mesaj), 0);
}

// Client işlemini yönet
DWORD WINAPI client_handler(LPVOID socket_desc) {
    SOCKET client_sock = *(SOCKET *)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;

    while ((read_size = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        if (strcmp(buffer, "listele") == 0) {
            urunleri_listele(client_sock);
        } else if (strncmp(buffer, "satin al ", 9) == 0) {
            urun_satin_al(client_sock, buffer + 9);
        } else {
            char *mesaj = "Bilinmeyen komut\n";
            send(client_sock, mesaj, strlen(mesaj), 0);
        }
    }

    closesocket(client_sock);
    free(socket_desc);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET server_sock, client_sock;
    struct sockaddr_in server, client;
    int c;
    

    urunleri_yukle("urunler.txt");

    printf("Winsock baslatiliyor...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Başlatma hatası. Kod: %d\n", WSAGetLastError());
        return 1;
    }

    // Socket oluşturma
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket oluşturulamadı. Kod: %d\n", WSAGetLastError());
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bağlama
    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind hatası. Kod: %d\n", WSAGetLastError());
        return 1;
    }

    listen(server_sock, 3);
    printf("Sunucu dinleniyor...\n");

    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(server_sock, (struct sockaddr *)&client, &c)) != INVALID_SOCKET) {
        printf("Baglanti kabul edildi\n");

        HANDLE thread;
        SOCKET *new_sock = malloc(sizeof(SOCKET));
        *new_sock = client_sock;

        thread = CreateThread(NULL, 0, client_handler, (void *)new_sock, 0, NULL);
        if (thread == NULL) {
            printf("Thread oluşturulamadı. Kod: %d\n", GetLastError());
            return 1;
        }

        CloseHandle(thread);
    }

    if (client_sock == INVALID_SOCKET) {
        printf("Bağlantı kabul edilemedi. Kod: %d\n", WSAGetLastError());
        return 1;
    }

    closesocket(server_sock);
    WSACleanup();
    return 0;
}
