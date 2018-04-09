#include "server_structs.c"

void insert_by_algorithm(int id, int burst, int priority, int algorithm_type);
void display();
void append(int pid, int burst, int priority); //FCFS
void insert_by_burst(int pid, int burst, int priority); //SJF
void insert_by_priority(int pid, int burst, int priority); //HPF
struct PCB* remove_head();

struct PCB* head = NULL;

void insert_by_algorithm(int id, int burst, int priority, int algorithm_type)
{
	switch(algorithm_type)
	{
		case 1:
			append(id, burst, priority);
			break;
		case 2:
			insert_by_burst(id, burst, priority);
			break;
		case 3:
			insert_by_priority(id, burst, priority);
			break;
		case 4:
			append(id, burst, priority);
			break;
	}
}

void display()
{
	printf("\n######################### Ready Queue #########################\n");
	struct PCB *temp = head;

	while(temp != NULL)
	{
		printf("\nPID: %d Burst: %d Prioridad: %d\n", 
                temp->pid, temp->burst, temp->priority);

		temp = temp->next;
	}

	printf("\n###############################################################\n");
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
	struct PCB *current;
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

	else if(priority > head->priority)
	{	
		current->next = head;
		head->previous = current;
		head = current;
	}

	else
	{
		while(temp->next != NULL && 
                    temp->next->priority > priority)
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

struct PCB* remove_head()
{
	if(head == NULL)
	{
		return NULL;
	}

	else
	{
		struct PCB *temp = head;

		//Solo un elemento en la cola
		if(temp->next == NULL)
		{
			head = NULL;
		}

		else
		{
			head = head->next;
			head->previous = NULL;
		}
		return temp;
	}
}