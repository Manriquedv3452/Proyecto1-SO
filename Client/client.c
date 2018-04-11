#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <semaphore.h>

#include "client_structs.c"

#define IP_ADDRESS "127.0.0.1"
#define PORT 9001
#define LINE_LEN 12
#define THREADS_LIMIT 100

void execution_instruction();
char* itoa(long n);
char *concat(char *buffer, char c);
void send_pcb_server(struct PCB* pcb);
void* manage_terminal(void* args);
void clean_buffer(void);

pthread_t threads[THREADS_LIMIT];
pthread_t t_manage_terminal;
sem_t terminal_semaphore;
int thread_index = 0;
int alive = 1;
//srand(time(0));

int main(int argc, char** argv)
{
    int min_burst;
    int max_burst;

    printf("\nSe ha ejecutado el cliente\n\n");
    printf("Ingrese el valor minimo del burst: ");
    scanf("%d", &min_burst);
    clean_buffer();

    printf("Ingrese el valor maximo del burst: ");
    scanf("%d", &max_burst);
    clean_buffer();

    if(argc == 2)
    {
        printf("\nSe ha ejecutado el cliente manual\n\n");
        
        FILE *input_file = fopen (argv[1], "r");        
        
        if (input_file == NULL){
            printf("El archivo %s no existe o no se puede leer\n", argv[1]);
            return 0;
        }
        
        // Procesar el archivo con "burst prioridad"       
        char line[LINE_LEN];
        while (feof(input_file) == 0)
        {
            fgets(line, LINE_LEN, input_file);
            char *burst = calloc(1, sizeof(char));
            char *priority = calloc(1, sizeof(char));
            int i = 0;
            int flag = 0;

            while(line[i] != '\0')
            {
                char current_char = line[i];

                if (current_char == '\n') break;

                if (current_char == ' ') flag = 1;

                else
                {
                    if(flag == 0) concat(burst, current_char);
                    if(flag == 1) concat(priority, current_char);
                }

                i++;
            }

            //prioridad por default
            if(priority[0] == ' ') priority[0] = 5;
            
            if(atoi(burst) >= min_burst && atoi(burst) <= max_burst)
            {
                struct PCB new_pcb = {atoi(burst), atoi(priority)};
                pthread_create(&threads[thread_index], NULL, (void*)send_pcb_server, (void*)&new_pcb);
                thread_index++;
                //sleep que se indica en la especificacion de la tarea
                sleep(rand() % (8 - 3 + 1) + 3);
            }
            else
            {
                printf("\nLos datos de burst: %d y prioridad: %d no cumplen con los datos indicados\n", atoi(burst), atoi(priority));
                printf("Min burst: %d y Max burst: %d\n", min_burst, max_burst);
                printf("Min prioridad: 1 y Max prioridad: 10\n");
            }
            
        }

        //sincronizar los procesos
        for (int i = 0; i < thread_index; i++){
            pthread_join(threads[i], NULL);
        }
    }

    else if(argc == 1)
    {
        int min_sleep;
        int max_sleep;

        printf("\nSe ha ejecutado el cliente automatico.\n\n");
        printf("Ingrese el valor minimo del sleep para creacion de procesos: ");
        scanf("%d", &min_sleep);
	clean_buffer();

        printf("Ingrese el valor maximo del sleep para creacion de procesos: ");
        scanf("%d", &max_sleep);
	clean_buffer();
        
        printf("\nPuede detener la creacion de procesos tecleando 0\n\n");
        pthread_create(&t_manage_terminal, NULL, (void*)manage_terminal, NULL);
        
        while(alive == 1)
        {
            struct PCB new_pcb = {rand() % (max_burst - min_burst + 1) + min_burst, rand() % 10 + 1};
            pthread_create(&threads[thread_index], NULL, (void*)send_pcb_server, (void*)&new_pcb);
            thread_index++;
            //sleep que se indica en la especificacion de la tarea
            sleep(rand() % (max_sleep - min_sleep + 1) + min_sleep);
        }

        pthread_join(t_manage_terminal,NULL);
    }
    else
    {
        execution_instruction();
    }

    return 0;
}

void execution_instruction()
{
    printf("\nError: Parametros no coinciden con los tipos de cliente manual o automatico\n");
    printf("Cliente Manual: ./client procesos.txt\n");
    printf("Cliente Automatico: ./client\n\n");
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

void send_pcb_server(struct PCB* pcb)
{
    struct sockaddr_in server_address;
    int client_socket;
    int connection_status;

    //asignar direccion IP y puerto al socket
    server_address.sin_family = AF_INET;
    //server_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    inet_aton(IP_ADDRESS, &server_address.sin_addr.s_addr);
    server_address.sin_port = PORT;
    
    //crear socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    connection_status = connect(client_socket, (struct sockaddr*) &server_address,
                                sizeof(server_address));

    if (connection_status >= 0)
    {
        char pcb_formatted[10];
        strcpy(pcb_formatted, itoa(pcb->burst));
        strcat(pcb_formatted, "/");
        strcat(pcb_formatted, itoa(pcb->priority));
        printf("\nProceso enviado B/P: %s\n", pcb_formatted);

        send(client_socket, pcb_formatted, sizeof(pcb_formatted), 0);

        char server_response[15];
        recv(client_socket, &server_response, sizeof(server_response), 0);
        printf("Respuesta del Server: %s\n", server_response);
        close(client_socket);
    }

    else
    {
        printf("\nError: Creacion del socket cliente ha fallado");
        printf("\nVerifique si el server se ha iniciado.\n");
    }

    pthread_exit(0);
}

void* manage_terminal(void* args)
{
    while(alive == 1)
    {
		scanf("%d", &alive);
		clean_buffer();

		if(alive == 1)				
		{
			sem_wait(&terminal_semaphore);
            sem_post(&terminal_semaphore);
            pthread_exit(0);
		}

		else 
		{
            
		}
	}
}


void clean_buffer(void)
{
    int n;
    while((n = getchar()) != EOF && n != '\n' );
}
