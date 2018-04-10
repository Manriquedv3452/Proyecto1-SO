struct PCB{
    int pid;
    int burst;
    int priority;
    int waiting_time;
    int turn_around_time;
    struct PCB *previous;
	struct PCB *next;
};