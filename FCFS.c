#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_HASHMAP_SIZE 100
#define MAX_NAME_LENGTH 50
#define MAX_INPUT_LENGTH 100
#define MAX_INTEGER_LENGTH 5
#define MAX_PCB_NODES 100

typedef enum {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED,
    KILLED
} State;

typedef struct PCB {
    int pid;
    char name[MAX_NAME_LENGTH];
    int arrival;
    int burst;
    int remBurst;
    int curBurst;
    int ioStart;
    int ioDur;
    int curIo;
    int remIo;
    int completion;
    struct PCB *next;
    State state;
    int execTime;
    int ioJustStarted;
    bool killed;
} PCB;

typedef struct QNode {
    PCB *pcb;
    struct QNode *next;
} QNode;

typedef struct Queue {
    QNode *front;
    QNode *rear;
    int qSize;
} Queue;

typedef struct KillEvt {
    int pid;
    int time;
    struct KillEvt *next;
} KillEvt;

PCB *pcb_table[MAX_HASHMAP_SIZE];
KillEvt *killHead = NULL;

void initQueue(Queue *q);
void initMap(void);
void enqueue(Queue *q, PCB *p);
PCB *dequeue(Queue *q);
int removeFromQueue(Queue *q, int pid);
int hash(int pid);
void mapPut(PCB *p);
PCB *mapGet(int pid);
PCB *createPCB(char *name, int pid, int burst, int ioStart, int ioDur);
void addKill(int pid, int time);
void readProcesses(int total, Queue *ready);
void readKillEvents(int total);
bool validateLine(char line[MAX_INPUT_LENGTH]);
bool validateProcessInputs(char name[MAX_NAME_LENGTH], char inPid[MAX_INTEGER_LENGTH], char inBurst[MAX_INTEGER_LENGTH], char inIoStart[MAX_INTEGER_LENGTH], char inIoDur[MAX_INTEGER_LENGTH]);
bool validateKillInputs(char inPid[MAX_INTEGER_LENGTH], char inTime[MAX_INTEGER_LENGTH]);
bool validProcessName(char name[MAX_NAME_LENGTH]);
bool validInteger(char val[MAX_INTEGER_LENGTH]);
bool dashOrInteger(char *val);
void applyKillEvents(int curTime, Queue *ready, Queue *waiting, Queue *terminated, PCB **running);
void updateIO(Queue *waiting, Queue *ready);
void runScheduler(Queue *ready, Queue *waiting, Queue *terminated);
void printReport(Queue *terminated);
void freeQueueNodes(Queue *q);
void freeAll(Queue *ready, Queue *waiting, Queue *terminated);

int main() {
    int totalProcesses = 0;
    int totalKillEvents = 0;
    initMap();

    Queue ready;
    Queue waiting;
    Queue terminated;

    initQueue(&ready);
    initQueue(&waiting);
    initQueue(&terminated);

    printf("Enter total number of processes: ");
    scanf("%d", &totalProcesses);
    getchar();
    readProcesses(totalProcesses, &ready);

    printf("Enter total number of kill events: ");
    scanf("%d", &totalKillEvents);
    getchar();
    readKillEvents(totalKillEvents);

    runScheduler(&ready, &waiting, &terminated);
    printReport(&terminated);

    freeAll(&ready, &waiting, &terminated);
    return 0;
}

bool validateLine(char line[MAX_INPUT_LENGTH]) {
    if (line[0] == '\0') {
        printf("Input can't be empty. Please enter processName, processId, burstTime, inputOutputStartTime, inputOutputDuration. \n");
        return false;
    }
    return true;
}

void readProcesses(int total, Queue *ready) {
    int idx = 0;
    while (idx < total) {
        char line[MAX_INPUT_LENGTH];
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0';

        if (!validateLine(line)) {
            continue;
        }

        char name[MAX_NAME_LENGTH];
        char inPid[MAX_INTEGER_LENGTH];
        int pid;
        char inBurst[MAX_INTEGER_LENGTH];
        int burst;
        char inIoStart[MAX_INTEGER_LENGTH];
        int ioStart;
        char inIoDur[MAX_INTEGER_LENGTH];
        int ioDur;

        int cnt = sscanf(line, "%s %s %s %s %s", name, inPid, inBurst, inIoStart, inIoDur);
        if (cnt != 5) {
            printf("Invalid input. Process input format must be: <process_name> <pid> <burst_time> <io_start_time> <io_duration> \n");
            continue;
        }

        if (!validateProcessInputs(name, inPid, inBurst, inIoStart, inIoDur)) {
            continue;
        }

        pid = atoi(inPid);
        burst = atoi(inBurst);

        if (strcmp(inIoStart, "-") == 0) {
            ioStart = -1;
        } else {
            ioStart = atoi(inIoStart);
        }

        if (strcmp(inIoDur, "-") == 0) {
            ioDur = 0;
        } else {
            ioDur = atoi(inIoDur);
        }

        PCB *p = createPCB(name, pid, burst, ioStart, ioDur);
        if (p == NULL) {
            return;
        }

        p->state = READY;
        p->remBurst = burst;
        mapPut(p);
        enqueue(ready, p);
        idx++;
    }
}

void readKillEvents(int total) {
    int idx = 0;
    while (idx < total) {
        char line[MAX_INPUT_LENGTH];
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0';

        if (!validateLine(line)) {
            continue;
        }

        char inPid[MAX_INTEGER_LENGTH];
        int pid;
        char inTime[MAX_INTEGER_LENGTH];
        int time;

        int cnt = sscanf(line, "KILL %s %s", inPid, inTime);
        if (cnt != 2) {
            printf("Invalid input. Kill event format must be: KILL <PID> <kill_time> \n");
            continue;
        }

        if (!validateKillInputs(inPid, inTime)) {
            continue;
        }

        pid = atoi(inPid);
        time = atoi(inTime);

        addKill(pid, time);
        idx++;
    }
}

bool dashOrInteger(char *val) {
    if (strcmp(val, "-") == 0) {
        return true;
    }
    return validInteger(val);
}

bool validProcessName(char name[MAX_NAME_LENGTH]) {
    int i = 0;
    while (name[i] == ' ') {
        i++;
    }
    if (name[i] == '\0') {
        printf("Process name can't be empty. \n");
        return false;
    }
    return true;
}

bool validInteger(char val[MAX_INTEGER_LENGTH]) {
    int i = 0;
    while (val[i] != '\0') {
        if (!isdigit(val[i])) {
            return false;
        }
        i++;
    }
    return true;
}

bool validateProcessInputs(char name[MAX_NAME_LENGTH], char inPid[MAX_INTEGER_LENGTH], char inBurst[MAX_INTEGER_LENGTH], char inIoStart[MAX_INTEGER_LENGTH], char inIoDur[MAX_INTEGER_LENGTH]) {
    if (!validProcessName(name)) {
        return false;
    }
    if (!validInteger(inPid)) {
        printf("Invalid input. Process id must be an integer value. \n");
        return false;
    }
    if (!validInteger(inBurst)) {
        printf("Invalid input. Burst must be an integer value. \n");
        return false;
    }
    if (!dashOrInteger(inIoStart)) {
        printf("Invalid input. I/O start time must be integer or '-'\n");
        return false;
    }
    if (!dashOrInteger(inIoDur)) {
        printf("Invalid input. I/O duration must be integer or '-'\n");
        return false;
    }
    return true;
}

bool validateKillInputs(char inPid[MAX_INTEGER_LENGTH], char inTime[MAX_INTEGER_LENGTH]) {
    if (!validInteger(inPid)) {
        return false;
    }
    if (!validInteger(inTime)) {
        return false;
    }
    return true;
}

void initQueue(Queue *q) {
    q->front = NULL;
    q->rear = NULL;
    q->qSize = 0;
}

void initMap(void) {
    for (int i = 0; i < MAX_HASHMAP_SIZE; i++) {
        pcb_table[i] = NULL;
    }
}

void enqueue(Queue *q, PCB *p) {
    QNode *n = malloc(sizeof(QNode));
    if (n == NULL) {
        printf("Memory allocation failed. \n");
        return;
    }
    n->pcb = p;
    n->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = n;
        q->qSize++;
        return;
    }
    q->rear->next = n;
    q->rear = n;
    q->qSize++;
}

PCB *dequeue(Queue *q) {
    if (q->front == NULL) {
        return NULL;
    }
    QNode *tmp = q->front;
    PCB *p = tmp->pcb;
    q->front = tmp->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(tmp);
    q->qSize--;
    return p;
}

int removeFromQueue(Queue *q, int pid) {
    QNode *cur = q->front;
    QNode *prev = NULL;
    while (cur != NULL) {
        if (cur->pcb->pid == pid) {
            if (prev == NULL) {
                q->front = cur->next;
            } else {
                prev->next = cur->next;
            }
            if (cur == q->rear) {
                q->rear = prev;
            }
            free(cur);
            q->qSize--;
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

int hash(int pid) {
    int idx = pid % MAX_HASHMAP_SIZE;
    if (idx < 0) idx += MAX_HASHMAP_SIZE;
    return idx;
}

void mapPut(PCB *p) {
    int idx = hash(p->pid);
    p->next = pcb_table[idx];
    pcb_table[idx] = p;
}

PCB *mapGet(int pid) {
    int idx = hash(pid);
    PCB *cur = pcb_table[idx];
    while (cur != NULL) {
        if (cur->pid == pid) return cur;
        cur = cur->next;
    }
    return NULL;
}

PCB *createPCB(char *name, int pid, int burst, int ioStart, int ioDur) {
    PCB *p = malloc(sizeof(PCB));
    if (p == NULL) {
        printf("Memory allocation failed.\n");
        return NULL;
    }
    p->pid = pid;
    strcpy(p->name, name);
    p->arrival = 0;
    p->burst = burst;
    p->curBurst = 0;
    p->remBurst = burst;
    p->ioStart = ioStart;
    p->ioDur = ioDur;
    p->remIo = 0;
    p->curIo = 0;
    p->completion = 0;
    p->next = NULL;
    p->execTime = 0;
    p->ioJustStarted = 0;
    p->killed = false;
    return p;
}

void addKill(int pid, int time) {
    KillEvt *n = malloc(sizeof(KillEvt));
    if (n == NULL) {
        printf("Memory allocation failed. \n");
        return;
    }
    n->pid = pid;
    n->time = time;
    n->next = NULL;
    if (killHead == NULL || killHead->time > time) {
        n->next = killHead;
        killHead = n;
        return;
    } else {
        KillEvt *cur = killHead;
        while (cur->next != NULL && cur->next->time <= time) {
            cur = cur->next;
        }
        n->next = cur->next;
        cur->next = n;
    }
}

void applyKillEvents(int curTime, Queue *ready, Queue *waiting, Queue *terminated, PCB **running) {
    KillEvt *cur = killHead;
    KillEvt *prev = NULL;
    while (cur != NULL) {
        if (cur->time == curTime) {
            PCB *p = mapGet(cur->pid);
            if (p != NULL && !p->killed && p->state != TERMINATED) {
                p->state = KILLED;
                p->killed = true;
                p->completion = curTime;
                removeFromQueue(ready, p->pid);
                removeFromQueue(waiting, p->pid);
                if (*running != NULL && (*running)->pid == p->pid) {
                    *running = NULL;
                }
                enqueue(terminated, p);
            }
            KillEvt *del = cur;
            if (prev == NULL) {
                killHead = killHead->next;
                cur = killHead;
            } else {
                prev->next = cur->next;
                cur = prev->next;
            }
            free(del);
            continue;
        }
        prev = cur;
        cur = cur->next;
    }
}

void updateIO(Queue *waiting, Queue *ready) {
    QNode *cur = waiting->front;
    QNode *prev = NULL;
    while (cur != NULL) {
        PCB *p = cur->pcb;
        QNode *next = cur->next;
        if (p->ioJustStarted) {
            p->ioJustStarted = 0;
        } else {
            if (p->remIo > 0) {
                p->remIo--;
            }
        }
        if (p->remIo == 0) {
            if (!p->killed) {
                p->state = READY;
                enqueue(ready, p);
            }
            if (prev == NULL) {
                waiting->front = cur->next;
            } else {
                prev->next = cur->next;
            }
            if (cur == waiting->rear) {
                waiting->rear = prev;
            }
            free(cur);
            waiting->qSize--;
            cur = next;
            continue;
        }
        prev = cur;
        cur = cur->next;
    }
}

void runScheduler(Queue *ready, Queue *waiting, Queue *terminated) {
    int curTime = 0;
    PCB *running = NULL;
    while (ready->qSize > 0 || waiting->qSize > 0 || running != NULL) {
        applyKillEvents(curTime, ready, waiting, terminated, &running);
        if (running == NULL && ready->qSize > 0) {
            PCB *p = dequeue(ready);
            if (p != NULL && !p->killed) {
                running = p;
            } else if (p != NULL) {
                enqueue(terminated, p);
            }
        }
        if (running != NULL) {
            running->curBurst++;
            running->execTime++;
            running->remBurst--;
            if (running->curBurst == running->ioStart && running->ioDur > 0) {
                running->state = WAITING;
                running->remIo = running->ioDur;
                running->curIo = 0;
                running->ioJustStarted = 1;
                enqueue(waiting, running);
                running = NULL;
            } else if (running->remBurst <= 0) {
                running->completion = curTime + 1;
                running->state = TERMINATED;
                enqueue(terminated, running);
                running = NULL;
            }
        }
        updateIO(waiting, ready);
        curTime++;
    }
}

void printReport(Queue *terminated) {
    QNode *cur = terminated->front;
    int total = 0;
    while (cur != NULL) {
        total++;
        cur = cur->next;
    }
    if (total == 0) {
        return;
    }
    PCB *arr[total];
    cur = terminated->front;
    int idx = 0;
    while (cur != NULL) {
        arr[idx++] = cur->pcb;
        cur = cur->next;
    }
    for (int i = 0; i < total - 1; i++) {
        for (int j = 0; j < total - i - 1; j++) {
            if (arr[j]->pid > arr[j + 1]->pid) {
                PCB *tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
    printf("\n%-5s %-10s %-5s %-5s %-15s %-12s %-8s\n", "PID", "Name", "CPU", "IO", "Status", "Turnaround", "Waiting");
    int i = 0;
    while (i < total) {
        PCB *p = arr[i];
        int cpu = p->burst;
        int io = p->ioDur;
        if (p->state == KILLED) {
            printf("%-5d %-10s %-5d %-5d KILLED at %-7d %-12s %-8s\n", p->pid, p->name, cpu, io, p->completion, "-", "-");
        } else {
            int tat = p->completion - p->arrival;
            int wait = tat - cpu;
            printf("%-5d %-10s %-5d %-5d OK%-12s %-12d %-8d\n", p->pid, p->name, cpu, io, "", tat, wait);
        }
        i++;
    }
}

void freeQueueNodes(Queue *q) {
    QNode *cur = q->front;
    while (cur != NULL) {
        QNode *next = cur->next;
        free(cur);
        cur = next;
    }
    q->front = NULL;
    q->rear = NULL;
    q->qSize = 0;
}

void freeAll(Queue *ready, Queue *waiting, Queue *terminated) {
    for (int i = 0; i < MAX_HASHMAP_SIZE; i++) {
        PCB *cur = pcb_table[i];
        while (cur != NULL) {
            PCB *n = cur->next;
            free(cur);
            cur = n;
        }
        pcb_table[i] = NULL;
    }
    KillEvt *k = killHead;
    while (k != NULL) {
        KillEvt *n = k->next;
        free(k);
        k = n;
    }
    killHead = NULL;
    freeQueueNodes(ready);
    freeQueueNodes(waiting);
    freeQueueNodes(terminated);
}
