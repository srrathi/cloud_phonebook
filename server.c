#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <json-c/json.h>

#define MAX_CONTACTS 100

typedef struct
{
    char name[50];
    char number[20];
} contact_t;

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[1025];
    char recvBuff[1025];
    int num_contacts = 0;
    contact_t contacts[MAX_CONTACTS];
    json_object *jobj = NULL;

    /* Load contacts from JSON file */
    FILE *fptr = fopen("contacts.json", "r");
    if (fptr)
    {
        fseek(fptr, 0, SEEK_END);
        long file_size = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);
        char *json_str = (char *)malloc(file_size + 1);
        fread(json_str, 1, file_size, fptr);
        fclose(fptr);
        jobj = json_tokener_parse(json_str);
        free(json_str);
        if (jobj)
        {
            json_object *jcontacts = json_object_object_get(jobj, "contacts");
            if (jcontacts)
            {
                int num_jcontacts = json_object_array_length(jcontacts);
                for (int i = 0; i < num_jcontacts; i++)
                {
                    json_object *jcontact = json_object_array_get_idx(jcontacts, i);
                    json_object *jname = json_object_object_get(jcontact, "name");
                    json_object *jnumber = json_object_object_get(jcontact, "number");
                    if (jname && jnumber)
                    {
                        strcpy(contacts[num_contacts].name, json_object_get_string(jname));
                        strcpy(contacts[num_contacts].number, json_object_get_string(jnumber));
                        num_contacts++;
                    }
                }
            }
        }
    }

    /* Create socket */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    /* Configure server address */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    /* Bind socket to address */
    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    /* Listen for connections */
    listen(listenfd, 10);

    /* Accept incoming connections and handle requests */
    while (1)
    {
        connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);

        /* Read request from client */
        memset(recvBuff, '0', sizeof(recvBuff));
        read(connfd, recvBuff, sizeof(recvBuff) - 1);
        jobj = json_tokener_parse(recvBuff);
        json_object *jtype = json_object_object_get(jobj, "type");
        json_object *jname = json_object_object_get(jobj, "name");
        json_object *jnumber = json_object_object_get(jobj, "number");

        char *name = strdup(json_object_get_string(jname));
        char *number = strdup(json_object_get_string(jnumber));

        const char *jname_str = json_object_get_string(jname);
        const char *jnumber_str = json_object_get_string(jnumber);

        strcpy(sendBuff, "");

        /* Handle CRUD operations */
        if (jtype)
        {
            switch (json_object_get_string(jtype)[0])
            {
            case 'c':
                /* Create new contact */
                if (jname && jnumber)
                {
                    if (num_contacts < MAX_CONTACTS)
                    {
                        strcpy(contacts[num_contacts].name, name);
                        strcpy(contacts[num_contacts].number, number);
                        num_contacts++;
                        /* Send response to client */
                        strcpy(sendBuff, "Contact created successfully");
                        write(connfd, sendBuff, strlen(sendBuff));
                    }
                    else
                    {
                        strcpy(sendBuff, "Contact list is full");
                        write(connfd, sendBuff, strlen(sendBuff));
                    }
                    break;
                case 'r':
                    /* Read contact */
                    if (jname)
                    {
                        for (int i = 0; i < num_contacts; i++)
                        {
                            if (strcmp(contacts[i].name, json_object_get_string(jname)) == 0)
                            {
                                jobj = json_object_new_object();
                                json_object_object_add(jobj, "name", json_object_new_string(contacts[i].name));
                                json_object_object_add(jobj, "number", json_object_new_string(contacts[i].number));
                                strcpy(sendBuff, json_object_to_json_string(jobj));
                                write(connfd, sendBuff, strlen(sendBuff));
                                json_object_put(jobj);
                                break;
                            }
                        }
                    }
                    break;
                case 'u':
                    /* Update contact */
                    if (jname && jnumber)
                    {
                        for (int i = 0; i < num_contacts; i++)
                        {
                            if (strcmp(contacts[i].name, json_object_get_string(jname)) == 0)
                            {
                                strcpy(contacts[i].number, json_object_get_string(jnumber));

                                /* Update JSON file */
                                jobj = json_object_new_object();
                                json_object *jcontacts = json_object_new_array();
                                for (int j = 0; j < num_contacts; j++)
                                {
                                    json_object *jcontact = json_object_new_object();
                                    json_object_object_add(jcontact, "name", json_object_new_string(contacts[j].name));
                                    json_object_object_add(jcontact, "number", json_object_new_string(contacts[j].number));
                                    json_object_array_add(jcontacts, jcontact);
                                }
                                json_object_object_add(jobj, "contacts", jcontacts);
                                FILE *fptr = fopen("contacts.json", "w");
                                if (fptr)
                                {
                                    fprintf(fptr, "%s", json_object_to_json_string(jobj));
                                    fclose(fptr);
                                }
                                json_object_put(jobj);

                                /* Send response to client */
                                strcpy(sendBuff, "Contact updated successfully");
                                write(connfd, sendBuff, strlen(sendBuff));
                                break;
                            }
                        }
                    }
                    break;
                case 'd':
                    /* Delete contact */
                    if (jname)
                    {
                        for (int i = 0; i < num_contacts; i++)
                        {
                            if (strcmp(contacts[i].name, json_object_get_string(jname)) == 0)
                            {
                                /* Shift remaining contacts left */
                                for (int j = i; j < num_contacts - 1; j++)
                                {
                                    contacts[j] = contacts[j + 1];
                                }
                                num_contacts--;

                                /* Update JSON file */
                                jobj = json_object_new_object();
                                json_object *jcontacts = json_object_new_array();
                                for (int j = 0; j < num_contacts; j++)
                                {
                                    json_object *jcontact = json_object_new_object();
                                    json_object_object_add(jcontact, "name", json_object_new_string(contacts[j].name));
                                    json_object_object_add(jcontact, "number", json_object_new_string(contacts[j].number));
                                    json_object_array_add(jcontacts, jcontact);
                                }
                                json_object_object_add(jobj, "contacts", jcontacts);
                                FILE *fptr = fopen("contacts.json", "w");
                                if (fptr)
                                {
                                    fprintf(fptr, "%s", json_object_to_json_string(jobj));
                                    fclose(fptr);
                                }
                                json_object_put(jobj);

                                /* Send response to client */
                                strcpy(sendBuff, "Contact deleted successfully");
                                write(connfd, sendBuff, strlen(sendBuff));
                                break;
                            }
                        }
                    }
                    break;
                default:
                    /* Send response to client */
                    strcpy(sendBuff, "Invalid operation");
                    write(connfd, sendBuff, strlen(sendBuff));
                    break;
                }
                json_object_put(jname);
                json_object_put(jnumber);
                json_object_put(jobj);
            }
            close(connfd);
        }
    }
}