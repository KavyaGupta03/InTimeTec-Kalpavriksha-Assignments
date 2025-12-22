#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define MAX_SIZE 100
#define SUCCESS 1
#define INVALID_INPUT -1

typedef struct SharedData
{
    int arraySize;
    int array[MAX_SIZE];
} SharedData;

void executeSharedMemorySorting();
SharedData *createSharedMemory(int *sharedMemoryId);
void displayArray(SharedData *data);
void sortArray(SharedData *data);
int readArray(SharedData *data);
void handleChildProcess(SharedData *data);
void cleanSharedMemory(SharedData *data, int sharedMemoryId);

int main()
{
    executeSharedMemorySorting();
    return 0;
}

SharedData *createSharedMemory(int *sharedMemoryId)
{
    *sharedMemoryId = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);

    if (*sharedMemoryId == -1)
    {
        printf("shmget failed. \n");
        return NULL;
    }

    SharedData *data = (SharedData *)shmat(*sharedMemoryId, NULL, 0);

    if (data == (void *)-1)
    {
        printf("shmat failed. \n");
        return NULL;
    }

    return data;
}

void executeSharedMemorySorting()
{
    int sharedMemoryId;
    SharedData *data;

    data = createSharedMemory(&sharedMemoryId);

    if (data == NULL)
    {
        return;
    }

    if (readArray(data) == INVALID_INPUT)
    {
        cleanSharedMemory(data, sharedMemoryId);
        return;
    }

    printf("\nProcess 1: Parent process \n");
    printf("Array before sorting: \n");
    displayArray(data);

    pid_t processId = fork();

    if (processId == 0)
    {
        printf("\nProcess 2: child process\n");
        handleChildProcess(data);
    }
    else if (processId > 0)
    {
        wait(NULL);

        printf("\nProcess 1: Parent process \n");
        printf("Array after sorting: \n");
        displayArray(data);
    }
    else
    {
        printf("fork failed. \n");
    }

    cleanSharedMemory(data, sharedMemoryId);
}

int readArray(SharedData *data)
{
    printf("Enter number of elements: \n");
    scanf("%d", &data->arraySize);

    if (data->arraySize <= 0 || data->arraySize > MAX_SIZE)
    {
        printf("Array size must be in range 1-100. \n");
        return INVALID_INPUT;
    }

    printf("Enter %d elements: \n", data->arraySize);

    for (int currentDataIndex = 0; currentDataIndex < data->arraySize; currentDataIndex++)
    {
        if (scanf("%d", &data->array[currentDataIndex]))
        {
            printf("Array element must be an integer value. \n");
            return INVALID_INPUT;
        }
    }

    return SUCCESS;
}

void displayArray(SharedData *data)
{
    for (int currentDataIndex = 0; currentDataIndex < data->arraySize; currentDataIndex++)
    {
        printf("%d ", data->array[currentDataIndex]);
    }

    printf("\n");
}

void sortArray(SharedData *data)
{
    for (int turn = 0; turn < data->arraySize - 1; turn++)
    {
        for (int index = turn + 1; index < data->arraySize; index++)
        {
            if (data->array[turn] > data->array[index])
            {
                int temporaryStore = data->array[turn];
                data->array[turn] = data->array[index];
                data->array[index] = temporaryStore;
            }
        }
    }
}

void handleChildProcess(SharedData *data)
{
    printf("Child process is sorting the array... \n");
    sortArray(data);

    exit(0);
}

void cleanSharedMemory(SharedData *data, int sharedMemoryId)
{
    shmdt(data);
    shmctl(sharedMemoryId, IPC_RMID, NULL);
}
