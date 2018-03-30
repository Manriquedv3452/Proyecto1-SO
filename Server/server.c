#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

#include "ready_queue.c"

#define PORT 9001
#define CLIENTS_LIMIT 20

char* itoa(long n);
char* concat(char* buffer, char c);
void* interact_with_client(void *args);

int PID = 0;

int main()
{
    struct sockaddr_in server_address;
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int client_socket;
    pthread_t client_threads[CLIENTS_LIMIT];

    server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = PORT;

    if(bind(server_socket, (struct sockaddr*) &server_address,
            sizeof(server_address)) >= 0)
    {
        printf("\nConexion establecida exitosamente!\n");

        //hasta 5 conexiones de cliente en cola
        listen(server_socket, 5);

        while(1)
        {   
            client_socket = accept(server_socket, NULL, NULL);

            printf("\nConexion con cliente #: %d\n", client_socket);
                
            pthread_create(&client_threads, NULL, interact_with_client, 
                            (void *)client_socket);
        }

        close(server_socket);
        exit(0);
    }

    else
    {
        printf("\nError al crear socket de Server\n");
    }
    return 0;
}

/*
    Funcion itoa obtenida de la respuesta dada por mmdemirbas
    https://stackoverflow.com/questions/190229/where-is-the-itoa-function-in-linux
*/
char* itoa(long n)
{
    int len = n==0 ? 1 : floor(log10l(labs(n)))+1;
    if (n<0) len++; // room for negative sign '-'

    char *buf = calloc(sizeof(char), len+1); // +1 for null
    snprintf(buf, len+1, "%ld", n);
    return   buf;
}

/*
    el uso del realloc se debe a que no me permite solamente hacer
    un append al char*, da errores 
*/
char *concat(char *buffer, char c)
{
    char *new_buffer;
    int i;

    //el + 2 se refiere al nuevo char a agregar + el null que va al final
    new_buffer = realloc(buffer, sizeof(buffer) + 2);

    //avanzo al final del buffer para poner el nuevo char y el nuevo null
    for (i = 0; new_buffer[i] != '\0'; i++);
    new_buffer[i++] = c;
    new_buffer[i] = '\0';    
    return new_buffer;
}

void* interact_with_client(void *args)
{
    int client_socket = (int)args;
    char info_from_client[20];
    int flag = 0;
    int i = 0;
    char *burst    = malloc(5);
    char *priority = malloc(5);

    //leer info del socket cliente
    read(client_socket, info_from_client, 20);

    printf("\nCliente ha enviado: %s\n", info_from_client);
    
    while(info_from_client[i] != '\0')
    {
        char current_char = info_from_client[i];

        if (current_char == '/') flag = 1;

        else
        {
            if(flag == 0) concat(burst, current_char);
            if(flag == 1) concat(priority, current_char);
        }

        i++;
    }
    
    //enviar respuesta al cliente: el PID
    send(client_socket, itoa(PID), sizeof(PID), 0);

    //agregar el PCB con la nueva info en la cola de ready
    append(PID++, atoi(burst),atoi(priority));

    //mostrar la cola de ready
    display();
    close(client_socket);
}