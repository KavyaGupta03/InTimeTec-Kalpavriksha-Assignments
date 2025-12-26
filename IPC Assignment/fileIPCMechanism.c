#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_SIZE 100
#define SUCCESS 1
#define INVALID_INPUT -1
#define FILE_NAME "data.txt"

typedef struct FileData
{
    int arraySize;
    int array[MAX_SIZE];
} FileData;

void executeFileIPCSorting();
int readArray(FileData *data);
void displayArray(FileData *data);
void sortArray(FileData *data);
void writeToFile(FileData *data);
void readFromFile(FileData *data);

int main()
{
    executeFileIPCSorting();
    return 0;
}

void executeFileIPCSorting()
{
    FileData data;

    if (readArray(&data) == INVALID_INPUT)
    {
        return;
    }

    printf("\nProcess 1: Parent process \n");
    printf("Array before sorting: \n");
    displayArray(&data);

    writeToFile(&data);
    printf("Data is written to file: %s \n", FILE_NAME);

    pid_t processId = fork();

    if (processId == 0)
    {
        printf("\nProcess 2: child process\n");
        printf("Child process is sorting the array... \n");

        readFromFile(&data);
        sortArray(&data);
        writeToFile(&data);
        printf("Sorted data is written back to file: %s \n", FILE_NAME);

        exit(0);
    }
    else if (processId > 0)
    {
        wait(NULL);
        
        printf("\nProcess 1: Parent process \n");
        readFromFile(&data);

        printf("Array after sorting: \n");
        displayArray(&data);
    }
    else
    {
        printf("fork failed");
    }
}

int readArray(FileData *data)
{
    printf("Enter number of elements:\n");
    scanf("%d", &data->arraySize);

    if (data->arraySize <= 0 || data->arraySize > MAX_SIZE)
    {
        printf("Array size must be between 1 and 100\n");
        return INVALID_INPUT;
    }

    printf("Enter %d elements:\n", data->arraySize);
    for (int i = 0; i < data->arraySize; i++)
    {
        if (scanf("%d", &data->array[i]) != 1)
        {
            printf("Array element must be an integer value. \n");
            return INVALID_INPUT;
        }
    }

    return SUCCESS;
}

void displayArray(FileData *data)
{
    for (int i = 0; i < data->arraySize; i++)
    {
        printf("%d ", data->array[i]);
    }

    printf("\n");
}

void sortArray(FileData *data)
{
    for (int turn = 0; turn < data->arraySize - 1; turn++)
    {
        for (int index = turn + 1; index < data->arraySize; index++)
        {
            if (data->array[turn] > data->array[index])
            {
                int temp = data->array[turn];
                data->array[turn] = data->array[index];
                data->array[index] = temp;
            }
        }
    }
}

void writeToFile(FileData *data)
{
    FILE *filePointer = fopen(FILE_NAME, "w");

    if (filePointer == NULL)
    {
        printf("Failed to open file. \n");
        return;
    }

    fprintf(filePointer, "%d\n", data->arraySize);

    for (int index = 0; index < data->arraySize; index++)
    {
        fprintf(filePointer, "%d ", data->array[index]);
    }

    fclose(filePointer);
}

void readFromFile(FileData *data)
{
    FILE *filePointer = fopen(FILE_NAME, "r");

    if (filePointer == NULL)
    {
        printf("Failed to open file. \n");
        return;
    }

    fscanf(filePointer, "%d", &data->arraySize);

    for (int index = 0; index < data->arraySize; index++)
    {
        fscanf(filePointer, "%d", &data->array[index]);
    }

    fclose(filePointer);
}
