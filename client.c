#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8888

typedef enum
{
    ADD_CONTACT = 1,
    GET_CONTACT,
    UPDATE_CONTACT,
    DELETE_CONTACT,
    EXIT
} OperationType;

#define MAX_MESSAGE_LENGTH 1024

void add_contact(int sock)
{
    char name[100];
    char number[20];
    char message[MAX_MESSAGE_LENGTH];
    memset(message, 0, MAX_MESSAGE_LENGTH);
    printf("Enter name: ");
    scanf("%s", name);
    printf("Enter phone number: ");
    scanf("%s", number);
    sprintf(message, "ADD %s %s", name, number);
    send(sock, message, strlen(message), 0);
    printf("Contact added successfully.\n");
}

void search_contact(int sock)
{
    char name[100];
    char message[MAX_MESSAGE_LENGTH];
    char response[MAX_MESSAGE_LENGTH];
    memset(message, 0, MAX_MESSAGE_LENGTH);
    memset(response, 0, MAX_MESSAGE_LENGTH);
    printf("Enter name: ");
    scanf("%s", name);
    sprintf(message, "SEARCH %s", name);
    send(sock, message, strlen(message), 0);
    recv(sock, response, MAX_MESSAGE_LENGTH, 0);
    if (strcmp(response, "NOT FOUND") == 0)
    {
        printf("Contact not found.\n");
    }
    else
    {
        printf("Phone number for %s is %s.\n", name, response);
    }
}

void delete_contact(int sock)
{
    char name[100];
    char message[MAX_MESSAGE_LENGTH];
    char response[MAX_MESSAGE_LENGTH];
    memset(message, 0, MAX_MESSAGE_LENGTH);
    memset(response, 0, MAX_MESSAGE_LENGTH);
    printf("Enter name: ");
    scanf("%s", name);
    sprintf(message, "DELETE %s", name);
    send(sock, message, strlen(message), 0);
    recv(sock, response, MAX_MESSAGE_LENGTH, 0);
    if (strcmp(response, "NOT FOUND") == 0)
    {
        printf("Contact not found.\n");
    }
    else
    {
        printf("Contact deleted successfully.\n");
    }
}

void update_contact(int sock)
{
    char name[100];
    char number[20];
    char message[MAX_MESSAGE_LENGTH];
    memset(message, 0, MAX_MESSAGE_LENGTH);
    printf("Enter name: ");
    scanf("%s", name);
    printf("Enter new phone number: ");
    scanf("%s", number);
    sprintf(message, "UPDATE %s %s", name, number);
    send(sock, message, strlen(message), 0);
    printf("Contact updated successfully.\n");
}

typedef struct
{
    char name[100];
    char number[100];
} Contact;

void print_menu()
{
    printf("\n");
    printf("1. Add Contact\n");
    printf("2. Get Contact\n");
    printf("3. Update Contact\n");
    printf("4. Delete Contact\n");
    printf("5. Exit\n");
    printf("Enter operation number: ");
}

int main()
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    // convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    int choice = 0;
    while (1)
    {
        print_menu();
        scanf("%d", &choice);
        getchar(); // remove newline character from input buffer

        if (choice == EXIT)
        {
            send(sock, &choice, sizeof(choice), 0);
            break;
        }

        Contact contact;
        char name[100], number[100];
        switch (choice)
        {
        case ADD_CONTACT:
            printf("Enter name: ");
            fgets(name, 100, stdin);
            name[strcspn(name, "\n")] = 0; // remove newline character
            printf("Enter number: ");
            fgets(number, 100, stdin);
            number[strcspn(number, "\n")] = 0; // remove newline character
            strcpy(contact.name, name);
            strcpy(contact.number, number);

            send(sock, &choice, sizeof(choice), 0);
            send(sock, &contact, sizeof(contact), 0);

            valread = read(sock, buffer, 1024);
            printf("%s\n", buffer);
            break;

        case GET_CONTACT:
            printf("Enter name: ");
            fgets(name, 100, stdin);
            name[strcspn(name, "\n")] = 0; // remove newline character
            strcpy(contact.name, name);

            send(sock, &choice, sizeof(choice), 0);
            send(sock, &contact, sizeof(contact), 0);

            valread = read(sock, buffer, 1024);
            printf("%s\n", buffer);
            break;

        case UPDATE_CONTACT:
            printf("Enter name: ");
            fgets(name, 100, stdin);
            name[strcspn(name, "\n")] = 0; // remove newline character
            strcpy(contact.name, name);
            printf("Enter new number: ");
            fgets(number, 100, stdin);
            number[strcspn(number, "\n")] = 0; // remove newline character
            send(sock, &choice, sizeof(choice), 0);
            send(sock, &contact, sizeof(contact), 0);

            valread = read(sock, buffer, 1024);
            if (strcmp(buffer, "Contact found") == 0)
            {
                printf("Enter new number: ");
                fgets(number, 100, stdin);
                number[strcspn(number, "\n")] = 0; // remove newline character
                strcpy(contact.number, number);

                send(sock, &contact, sizeof(contact), 0);

                valread = read(sock, buffer, 1024);
                printf("%s\n", buffer);
            }
            else
            {
                printf("%s\n", buffer);
            }
            break;

        case DELETE_CONTACT:
            printf("Enter name: ");
            fgets(name, 100, stdin);
            name[strcspn(name, "\n")] = 0; // remove newline character
            strcpy(contact.name, name);

            send(sock, &choice, sizeof(choice), 0);
            send(sock, &contact, sizeof(contact), 0);

            valread = read(sock, buffer, 1024);
            printf("%s\n", buffer);
            break;

        default:
            printf("Invalid operation\n");
            break;
        }
    }

    close(sock);
    return 0;
}
