struct PCB{
    int pid;
    int burst;
    int priority;
    struct PCB *previous;
	struct PCB *next;
};