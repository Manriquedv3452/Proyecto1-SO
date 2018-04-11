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
#include <time.h>

#include "ready_queue.c"

#define PORT 9001
#define CLIENTS_LIMIT 20

char* itoa(long n);
char* concat(char* buffer, char c);
void* job_scheduler();
void* cpu_scheduler();
void* queue_time();
void* manage_terminal();
void clean_buffer(void);

clock_t begin;
clock_t end;
pthread_t t_cpu_scheduler;
pthread_t t_job_scheduler;
pthread_t t_manage_terminal;
pthread_t t_queue_time;
sem_t terminal_semaphore;
int alive = 1;
int algorithm_type;
int cpu_idle = 0;
int execution_time = 0;
int PID = 0;
int quantum;
int* turn_around_processes;
int* waiting_time_processes;
double average_waiting_time = 0.0;

int main()
{
    waiting_time_processes = calloc(1, sizeof(int));
    turn_around_processes = calloc(1, sizeof(int));
    printf("\n1. First Come First Serve\n");
    printf("2. Short Job First\n");
    printf("3. High Priority First\n");
    printf("4. Round Robin\n");
    printf("0. Salir\n");
    printf("\nIngrese el tipo de algoritmo de planificacion a utilizar: ");
    scanf("%d", &algorithm_type);
    clean_buffer();
    
    if (algorithm_type == 4)
    {
        printf("\nHa seleccionado el Round Robin, ingrese el quantum a utilizar: ");
        scanf("%d", &quantum);
	clean_buffer();
	if (quantum == 0)
	{
		printf("\n\nQuantum No valido\n\n");
		printf("\n###################### Fin de Ejecucion del Server ######################\n\n");
    		return 0;
	}
    }

    if (algorithm_type != 0)
    {
        printf("\nEn cualquier usted puede presionar: \n");
        printf("\n1 para mostrar la ready queue.\n");
        printf("\n0 para detener el server y ver las estadisticas\n\n");

        sem_init(&terminal_semaphore, 0, 1);
        pthread_create(&t_job_scheduler, NULL, (void*)job_scheduler, (void *)algorithm_type);
        pthread_create(&t_cpu_scheduler, NULL, (void*)cpu_scheduler, NULL);
        pthread_create(&t_queue_time, NULL, (void*)queue_time, NULL);
        pthread_create(&t_manage_terminal, NULL, (void*)manage_terminal, NULL);
        
        pthread_join(t_job_scheduler, NULL);
        pthread_join(t_cpu_scheduler, NULL);
        pthread_join(t_queue_time, NULL);
        pthread_join(t_manage_terminal,NULL);
        sem_destroy(&terminal_semaphore); 
    }
    
    else
    {
        printf("\n\nOpcion No Valida\n\n");
    }

    printf("\n###################### Fin de Ejecucion del Server ######################\n\n");
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
    int algorithm_type = (int)args;

    //inicio y configuracion del server_socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
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

    while(alive)
    {
        int client_socket = accept(server_socket, NULL, NULL);

        if(client_socket != -1){
            //leer info del socket cliente
            //read(client_socket, info_from_client, 20);
            recv(client_socket, info_from_client, 20, 0);

            char *burst    = calloc(1, sizeof(char));
            char *priority = calloc(1, sizeof(char));
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
            close(client_socket);

            /*
                agregar el PCB con la nueva info en la cola 
                de ready segun el algoritmo seleccionado
            */
            insert_by_algorithm(PID++, atoi(burst), atoi(priority), algorithm_type, 0, 0);
            waiting_time_processes = realloc(waiting_time_processes, PID*sizeof(int));
            turn_around_processes = realloc(turn_around_processes, PID*sizeof(int));
        }
        else
        {
            break;
        }
        
    }
    exit(0);
    pthread_exit(0);
}

void* cpu_scheduler(void* args)
{
    struct PCB* current_pcb;
    int quantum_time;

    while(alive)
    {
        //funcion para recorrer cola

        current_pcb = remove_head();

        if(current_pcb != NULL)
        {
            waiting_time_processes[current_pcb->pid] = current_pcb->waiting_time;
            turn_around_processes[current_pcb->pid] = current_pcb->turn_around_time;

            if(algorithm_type != 4)
            { 
                printf("\nProceso con PID: %d Burst: %d Prioridad: %d entra en ejecucion\n", 
                    current_pcb->pid, current_pcb->burst, current_pcb->priority);

                sleep(current_pcb->burst); //Simular la ejecucion del proceso

                printf("\nProceso con PID: %d Burst: %d Prioridad: %d ha terminado su ejecucion\n", 
                        current_pcb->pid, current_pcb->burst, current_pcb->priority);

                turn_around_processes[current_pcb->pid] += current_pcb->burst;
                turn_around_processes[current_pcb->pid] += 
                                        waiting_time_processes[current_pcb->pid];

            }
            else
            {
                printf("\nProceso con PID: %d Burst: %d Prioridad: %d entran en ejecucion. Quantum de %d\n", 
                    current_pcb->pid, current_pcb->burst, current_pcb->priority, quantum);
                
                quantum_time = 0;

                while(current_pcb->burst > 0 && quantum_time < quantum)
                {
                    current_pcb->burst--;
                    quantum_time++;
                    sleep(1);
                }

                current_pcb->turn_around_time += quantum_time;
                turn_around_processes[current_pcb->pid] += quantum_time;

                if(current_pcb->burst > 0)
                {
                    //el proceso no ha terminado, reingresa a la cola
                    append(current_pcb->pid, current_pcb->burst, current_pcb->priority, 
                        current_pcb->turn_around_time, current_pcb->waiting_time);
                }
                else
                {
                    current_pcb->turn_around_time += current_pcb->waiting_time;
                    turn_around_processes[current_pcb->pid] = current_pcb->turn_around_time;
                    printf("\nProceso con PID: %d Burst: %d Prioridad: %d ha terminado su ejecucion\n", 
                            current_pcb->pid, current_pcb->burst, current_pcb->priority);

                }
            }

        }
        else
        {
            sleep(1);
            cpu_idle++;
        }
    }
    
    pthread_exit(0);
}

void* manage_terminal(void* args)
{
    while(alive == 1)
    {
		scanf("%d", &alive);
		clean_buffer();

		if (alive != 1 && alive != 0)
			alive = 1;

		//Mostrar ready queue
		if(alive == 1)				
		{
			sem_wait(&terminal_semaphore);
			display(); //queue
			sem_post(&terminal_semaphore);
		}

		//Se termina el server y se muestra el log
		else if(alive == 0)
		{
            sem_wait(&terminal_semaphore);
            printf("\n###################### RESULTADOS FINALES ######################\n");
            printf("\nCANTIDAD DE PROCESOS: \t%d\n", PID);
            printf("\nCPU IDLE TIME: \t%d\n", cpu_idle);

			//mostra el log, falta los WT, TAT, etc
            printf("\nProceso \t Waiting Time \tTurn Around Time\n");
            for(int i=0; i<PID; i++)
            {
                printf("P%d\t\t %d\t\t%d\n", i, waiting_time_processes[i], turn_around_processes[i]);
                average_waiting_time += waiting_time_processes[i];
            }

            printf("\nPROMEDIO DE WAITING TIME: \t%.2f\n", average_waiting_time/(double)(PID));
            printf("\n################################################################\n");
            pthread_cancel(t_job_scheduler);
            sem_post(&terminal_semaphore);
            pthread_exit(0);
		}
		else 
		{
            
		}
	}
}

void* queue_time(void* args)
{
    while(alive)
    {
        waiting_time(1);
        sleep(1);
    }

    pthread_exit(0);
}

void clean_buffer(void)
{
    int n;
    while((n = getchar()) != EOF && n != '\n' );
}
