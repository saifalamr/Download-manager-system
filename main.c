#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_LENGTH 1024
#define RECEIVE_TIMEOUT 10

void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    WSADATA wsa_data;
    SOCKET client_socket;
    struct sockaddr_in server_info;
    char message_buffer[BUFFER_LENGTH];
    int received_data_len;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "Winsock initialization failed. Error Code: %d\n", WSAGetLastError());
        return EXIT_FAILURE;
    }

    // Create the socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Set up server address
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_info.sin_port = htons(SERVER_PORT);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_info, sizeof(server_info)) < 0) {
        fprintf(stderr, "Failed to connect to the server. Error Code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    printf("Successfully connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);

    // Set the socket timeout for receiving data
    struct timeval timeout;
    timeout.tv_sec = RECEIVE_TIMEOUT;
    timeout.tv_usec = 0;
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0) {
        fprintf(stderr, "Failed to set socket timeout. Error Code: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Communication loop
    while (1) {
        memset(message_buffer, 0, BUFFER_LENGTH);

        // Receive data from the server
        received_data_len = recv(client_socket, message_buffer, BUFFER_LENGTH - 1, 0);
        if (received_data_len == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAETIMEDOUT) {
                printf("Receive timeout reached. Retrying...\n");
                continue; // Retry receiving data
            } else {
                fprintf(stderr, "Receive failed. Error Code: %d\n", WSAGetLastError());
                break;
            }
        } else if (received_data_len == 0) {
            printf("Server closed the connection.\n");
            break;
        }

        // Display the message from the server
        printf("%s", message_buffer);

        // Check if no further input is required from the user
        if (strstr(message_buffer, "Invalid credentials. Try again.") ||
            strstr(message_buffer, "Account created successfully! Please login.") ||
            strstr(message_buffer, "Login successful!") ||
            strstr(message_buffer, "Money added successfully!") ||
            strstr(message_buffer, "Purchase successful!") ||
            strstr(message_buffer, "Insufficient balance.") ||
            strstr(message_buffer, "Invalid product choice.") ||
            strstr(message_buffer, "Username already exists. Please try a different username.") ||
            strstr(message_buffer, "Goodbye!")) {
            if (strstr(message_buffer, "Goodbye!")) {
                printf("\nDisconnecting...\n");
                break;
            }
            continue;
        }

        // Otherwise, prompt the user for input
        printf("Enter your message: ");
        fgets(message_buffer, BUFFER_LENGTH, stdin);
        message_buffer[strcspn(message_buffer, "\n")] = 0; // Remove newline character

        // Send the user's input to the server
        if (send(client_socket, message_buffer, strlen(message_buffer), 0) < 0) {
            fprintf(stderr, "Send failed. Error Code: %d\n", WSAGetLastError());
            break;
        }
    }

    // Clean up
    closesocket(client_socket);
    WSACleanup();
    printf("Disconnected from server.\n");
    return EXIT_SUCCESS;
}
