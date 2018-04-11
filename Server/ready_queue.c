#include "server_structs.c"

void insert_by_algorithm(int id, int burst, int priority, int algorithm_type, int TAT, int WT);
void display();
void append(int pid, int burst, int priority, int TAT, int WT); //FCFS
void short_job_first(int pid, int burst, int priority, int TAT, int WT); //short_job_first
void high_priority_first(int pid, int burst, int priority, int TAT, int WT); //high_priority_first
struct PCB* remove_head();

struct PCB* head = NULL;

void insert_by_algorithm(int id, int burst, int priority, int algorithm_type, int TAT, int WT)
{
	switch(algorithm_type)
	{
		case 1:
		case 4:
			append(id, burst, priority, TAT, WT);
			break;
		case 2:
			short_job_first(id, burst, priority, TAT, WT);
			break;
		case 3:
			high_priority_first(id, burst, priority, TAT, WT);
			break;
	}
}

void display()
{
	printf("\n######################### Ready Queue #########################\n");
	struct PCB *temp = head;

	while(temp != NULL)
	{
		printf("\nPID: %d Burst: %d Prioridad: %d WT: %d TAT:%d\n", 
                temp->pid, temp->burst, temp->priority, temp->waiting_time, temp->turn_around_time);

		temp = temp->next;
	}

	printf("\n###############################################################\n");
}

void append(int id, int burst, int priority, int TAT, int WT)
{
	struct PCB *current = head;
    struct PCB *temp = head;

	current = calloc(1, sizeof(struct PCB));
	current->pid = id;
	current->burst = burst;
	current->priority = priority;
	current->waiting_time = WT;
	current->turn_around_time = TAT;
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

void short_job_first(int id, int burst, int priority, int TAT, int WT)
{
	struct PCB *current = head;
    struct PCB *temp = head;

	current = calloc(1, sizeof(struct PCB));
	current->pid = id;
	current->burst = burst;
	current->priority = priority;
	current->waiting_time = WT;
	current->turn_around_time = TAT;
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

void high_priority_first(int id, int burst, int priority, int TAT, int WT)
{
	struct PCB *current;
    struct PCB *temp = head;

	current = calloc(1, sizeof(struct PCB));
	current->pid = id;
	current->burst = burst;
	current->priority = priority;
	current->waiting_time = WT;
	current->turn_around_time = TAT;
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

void waiting_time(int time)
{
	struct PCB *temp = head;

	while(temp != NULL)
	{
		temp->waiting_time += time;
		temp = temp->next;
	}
}