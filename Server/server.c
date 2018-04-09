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
#include <semaphore.h>

#include "ready_queue.c"

#define PORT 9001
#define CLIENTS_LIMIT 20

char* itoa(long n);
char* concat(char* buffer, char c);
void* job_scheduler();
void* cpu_scheduler();
void* manage_terminal();

int PID = 0;

int main()
{
    pthread_t t_job_scheduler;
    pthread_t t_cpu_scheduler;
    pthread_t t_manage_terminal;
    sem_t terminal_semaphore;
    int algorithm_type;
    int quantum;

    printf("\n1. First Come First Server\n");
    printf("2. Short Job First\n");
    printf("3. High Priority First\n");
    printf("4. Round Robin\n");
    printf("0. Salir\n");
    printf("\nIngrese el tipo de algoritmo de planificacion a utilizar: ");
    scanf("%d", &algorithm_type);

    if (algorithm_type == 4)
    {
        printf("\nHa seleccionado el Round Robin, ingrese el quantum a utilizar: ");
        scanf("%d", &quantum);
    }

    printf("\nEn cualquier usted puede presionar: \n");
    printf("\n0 para mostrar la ready queue.\n");
    printf("\n1 para detener el server y ver las estadisticas\n\n");
    
    sem_init(&terminal_semaphore, 0, 1);  //Semaphore Initialized 
    pthread_create(&t_job_scheduler, NULL, (void*)job_scheduler, (void *)algorithm_type);
    pthread_create(&t_cpu_scheduler, NULL, (void*)cpu_scheduler, NULL);
    pthread_create(&t_manage_terminal, NULL, (void*)manage_terminal, NULL);
    
    pthread_join(t_job_scheduler, NULL);
    pthread_join(t_cpu_scheduler, NULL);
    pthread_join(t_manage_terminal, NULL);

    sem_destroy(&terminal_semaphore);
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

void* job_scheduler(void *args)
{
    struct sockaddr_in server_address;
    pthread_t t_clients[CLIENTS_LIMIT];
    int server_socket;
    int client_socket;
    int algorithm_type = (int)args;

    //inicio y configuracion del server_socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = PORT;

    //asignar socket al puerto PORT
    bind(server_socket, (struct sockaddr*) &server_address,
                            sizeof(server_address));
    
    char info_from_client[20];
    int flag; //determinar cuando se toma el burst y cuando la prioridad
    int i;

    /*
        Poner a la escucha al socket
        Hasta 20 conexiones de cliente en cola
    */
    listen(server_socket, 20);

    while(1)
    {
        client_socket = accept(server_socket, NULL, NULL);

        printf("\nConexion con cliente #: %d\n", client_socket);
            
        //leer info del socket cliente
        read(client_socket, info_from_client, 20);
        printf("\nCliente ha enviado: %s\n", info_from_client);

        char *burst    = malloc(5);
        char *priority = malloc(5);
        i = 0;
        flag = 0;

        //desformatear el mensaje del cliente B/P
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

        /*
            agregar el PCB con la nueva info en la cola 
            de ready segun el algoritmo seleccionado
        */
        insert_by_algorithm(PID++, atoi(burst), atoi(priority), algorithm_type);

        //free(burst);
        free(priority);

        //mostrar la cola de ready
        display();
        close(client_socket);
    }
}

void* cpu_scheduler(void* args)
{

}

void* manage_terminal(void* args)
{
    
}