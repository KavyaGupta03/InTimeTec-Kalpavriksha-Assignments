#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_SIZE 100
#define SUCCESS 1
#define INVALID_INPUT -1

typedef struct Pipe
{
    int arraySize;
    int array[MAX_SIZE];
} Pipe;

void executePipeSorting();
int readArray(Pipe *pipe);
void displayArray(Pipe *pipe);
void sortArray(int array[], int arraySize);
void handleParentProcess(int writePipe[], int readPipe[], Pipe *pipe);
void handleChildProcess(int readPipe[], int writePipe[]);

int main()
{
    executePipeSorting();

    return 0;
}

void executePipeSorting()
{
    Pipe pipeData;

    if (readArray(&pipeData) == INVALID_INPUT)
    {
        return;
    }

    printf("\nProcess 1: Parent process \n");
    printf("Array before sorting: ");
    displayArray(&pipeData);

    int pipeParentToChild[2];
    int pipeChildToParent[2];

    if (pipe(pipeParentToChild) == -1 || pipe(pipeChildToParent) == -1)
    {
        printf("pipe failed. \n");
        return;
    }

    pid_t processId = fork();

    if (processId == 0)
    {
        printf("\nProcess 2: child process\n");
        handleChildProcess(pipeParentToChild, pipeChildToParent);

        exit(0);
    }
    else if (processId > 0)
    {
        handleParentProcess(pipeParentToChild, pipeChildToParent, &pipeData);
        wait(NULL);

        printf("\nProcess 1: Parent process \n");
        printf("Array after sorting: ");
        displayArray(&pipeData);
    }
    else
    {
        printf("fork failed");
        return;
    }
}

void handleParentProcess(int writePipe[], int readPipe[], Pipe *pipeData)
{
    close(writePipe[0]);
    close(readPipe[1]);

    write(writePipe[1], &pipeData->arraySize, sizeof(int));
    write(writePipe[1], &pipeData->array, sizeof(int) * pipeData->arraySize);
    close(writePipe[1]);

    read(readPipe[0], &pipeData->arraySize, sizeof(int));
    read(readPipe[0], pipeData->array, pipeData->arraySize * sizeof(int));
    close(readPipe[0]);
}

void handleChildProcess(int readPipe[], int writePipe[])
{
    Pipe data;

    close(writePipe[0]);
    close(readPipe[1]);

    read(readPipe[0], &data.arraySize, sizeof(int));
    read(readPipe[0], data.array, sizeof(int) * data.arraySize);
    close(readPipe[0]);

    printf("Child process is sorting the array... \n");
    sortArray(data.array, data.arraySize);

    write(writePipe[1], &data.arraySize, sizeof(int));
    write(writePipe[1], data.array, data.arraySize * sizeof(int));
    close(writePipe[1]);
}

int readArray(Pipe *pipeData)
{
    printf("Enter number of elements: \n");
    scanf("%d", &pipeData->arraySize);

    if (pipeData->arraySize <= 0 || pipeData->arraySize > MAX_SIZE)
    {
        printf("Array size must be in range 1-100. \n");
        return INVALID_INPUT;
    }

    printf("Enter %d elements: \n", pipeData->arraySize);

    for (int currentDataIndex = 0; currentDataIndex < pipeData->arraySize; currentDataIndex++)
    {
        if (scanf("%d", &pipeData->array[currentDataIndex]))
        {
            printf("Array element must be an integer value. \n");
            return INVALID_INPUT;
        }
    }

    return SUCCESS;
}

void displayArray(Pipe *pipeData)
{
    for (int currentDataIndex = 0; currentDataIndex < pipeData->arraySize; currentDataIndex++)
    {
        printf("%d ", pipeData->array[currentDataIndex]);
    }

    printf("\n");
}

void sortArray(int array[], int arraySize)
{
    for (int turn = 0; turn < arraySize - 1; turn++)
    {
        for (int index = turn + 1; index < arraySize; index++)
        {
            if (array[turn] > array[index])
            {
                int temporaryStore = array[turn];
                array[turn] = array[index];
                array[index] = temporaryStore;
            }
        }
    }
}
