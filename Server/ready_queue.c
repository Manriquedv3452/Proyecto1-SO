#include "server_structs.c"

void display();
void append(int pid, int burst, int priority); //FCFS
void insert_by_burst(int pid, int burst, int priority); //SJF
void insert_by_priority(int pid, int burst, int priority); //HPF
//struct PCB* remove(int pid);

struct PCB* head = NULL;

void display()
{
	struct PCB *temp = head;
	printf("\nMostrando Cola de Ready\n");

	while(temp != NULL)
	{
		printf("PID: %d Burst: %d Prioridad: %d\n", 
                temp->pid, temp->burst, temp->priority);

		temp = temp->next;
	}

}

void append(int id, int burst, int priority)
{
	struct PCB *current = head;
    struct PCB *temp = head;

	current = malloc(sizeof(struct PCB));
	current = malloc(sizeof(struct PCB));
	current->priority = priority;
	current->burst = burst;
	current->pid = id;
	current->next = NULL;
	current->previous = NULL;

	if (head == NULL)
	{
		head = current;
	}
	else
	{
		while(temp->next != NULL)
			temp = temp->next;

		current->previous = temp;
		temp->next = current;
	}
}

void insert_by_burst(int id, int burst, int priority)
{
	struct PCB *current = head;
    struct PCB *temp = head;

	current = malloc(sizeof(struct PCB));
	current->pid = id;
	current->burst = burst;
	current->priority = priority;
	current->previous = NULL;
	current->next = NULL;

	if(head == NULL)
	{
		head = current;
	}
	else if(burst < temp->burst)
	{	
		current->next = head;
		head->previous = current;
		head = current;
	}
	else
	{
		while(temp->next != NULL && temp->next->burst < burst)
        {
            temp = temp->next;
        }

		current->next = temp->next;

		if(temp->next != NULL)
		{
			current->next->previous = current;
		}

		temp->next = current;
		current->previous = temp;
	}
}

void insert_by_priority(int id, int burst, int priority)
{
	struct PCB *current = head;
    struct PCB *temp = head;

	current = malloc(sizeof(struct PCB));
	current->pid = id;
	current->burst = burst;
	current->priority = priority;
	current->previous = NULL;
	current->next = NULL;

	//if linked list is empty
	if(head == NULL)
	{
		head = current;
	}

	else if(priority < temp->priority)
	{	
		current->next = head;
		head->previous = current;
		head = current;
	}

	else
	{
		while(temp->next != NULL && 
                    temp->next->priority < priority)
        {
			temp = temp->next;
        }

		current->next = temp->next;

		if(temp->next != NULL)
		{
			current->next->previous = current;
		}

		temp->next = current;
		current->previous = temp;
	}
}