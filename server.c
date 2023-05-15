#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_PORT 8888

typedef enum
{
    ADD_CONTACT = 1,
    GET_CONTACT,
    UPDATE_CONTACT,
    DELETE_CONTACT,
    EXIT
} OperationType;

typedef struct
{
    char name[100];
    char number[100];
} Contact;

int add_contact(Contact contact)
{
    FILE *fp = fopen("contacts.csv", "a");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return 0;
    }

    fprintf(fp, "%s,%s\n", contact.name, contact.number);
    fclose(fp);

    return 1;
}

int get_contact(char *name, Contact *contact)
{
    FILE *fp = fopen("contacts.csv", "r");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return 0;
    }

    char line[1024];
    while (fgets(line, 1024, fp))
    {
        char *token = strtok(line, ",");
        if (strcmp(token, name) == 0)
        {
            strcpy(contact->name, token);
            strcpy(contact->number, strtok(NULL, ",\n"));
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

int update_contact(Contact contact)
{
    FILE *fp = fopen("contacts.csv", "r+");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return 0;
    }

    char line[1024];
    long int offset = -1;
    while (fgets(line, 1024, fp))
    {
        offset = ftell(fp) - strlen(line);
        char *token = strtok(line, ",");
        if (strcmp(token, contact.name) == 0)
        {
            fseek(fp, offset, SEEK_SET);
            fprintf(fp, "%s,%s\n", contact.name, contact.number);
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

int delete_contact(char *name)
{
    FILE *fp = fopen("contacts.csv", "r");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return 0;
    }

    FILE *temp_fp = fopen("temp.csv", "w");
    if (temp_fp == NULL)
    {
        printf("Error opening file\n");
        fclose(fp);
        return 0;
    }

    char line[1024];
    while (fgets(line, 1024, fp))
    {
        char *token = strtok(line, ",");
        if (strcmp(token, name) != 0)
        {
            fprintf(temp_fp, "%s", line);
        }
    }

    fclose(fp);
    fclose(temp_fp);

    remove("contacts.csv");
    rename("temp.csv", "contacts.csv");

    return 1;
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // bind socket to a port and listen
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", SERVER_PORT);

    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Client connected\n");

        int option;
        Contact contact;

        // read client request
        if (read(new_socket, &option, sizeof(option)) < 0)
        {
            perror("read failed");
            close(new_socket);
            continue;
        }

        switch (option)
        {
        case ADD_CONTACT:
            if (read(new_socket, &contact, sizeof(contact)) < 0)
            {
                perror("read failed");
                close(new_socket);
                continue;
            }

            if (add_contact(contact))
            {
                printf("Contact added successfully: %s\n", contact.name);
                write(new_socket, "Contact added successfully", strlen("Contact added successfully") + 1);
            }
            else
            {
                printf("Error adding contact: %s\n", contact.name);
                write(new_socket, "Error adding contact", strlen("Error adding contact") + 1);
            }

            break;

        case GET_CONTACT:
            if (read(new_socket, &contact, sizeof(contact)) < 0)
            {
                perror("read failed");
                close(new_socket);
                continue;
            }

            if (get_contact(contact.name, &contact))
            {
                printf("Contact retrieved successfully: %s\n", contact.name);
                write(new_socket, &contact, sizeof(contact));
            }
            else
            {
                printf("Contact not found: %s\n", contact.name);
                write(new_socket, "Contact not found", strlen("Contact not found") + 1);
            }

            break;

        case UPDATE_CONTACT:
            if (read(new_socket, &contact, sizeof(contact)) < 0)
            {
                perror("read failed");
                close(new_socket);
                continue;
            }

            if (update_contact(contact))
            {
                printf("Contact updated successfully: %s\n", contact.name);
                write(new_socket, "Contact updated successfully", strlen("Contact updated successfully") + 1);
            }
            else
            {
                printf("Error updating contact: %s\n", contact.name);
                write(new_socket, "Error updating contact", strlen("Error updating contact") + 1);
            }

            break;

        case DELETE_CONTACT:
            char name[100];
            if (read(new_socket, name, sizeof(name)) < 0)
            {
                perror("read failed");
                close(new_socket);
                continue;
            }

            if (delete_contact(name))
            {
                printf("Contact deleted successfully: %s\n", name);
                write(new_socket, "Contact deleted successfully", strlen("Contact deleted successfully") + 1);
            }
            else
            {
                printf("Error deleting contact: %s\n", name);
                write(new_socket, "Error deleting contact", strlen("Error deleting contact") + 1);
            }

            break;

        case EXIT:
            printf("Client disconnected\n");
            close(new_socket);
            break;

        default:
            printf("Invalid option: %d\n", option);
            close(new_socket);
            break;
        }
    }

    return 0;
}