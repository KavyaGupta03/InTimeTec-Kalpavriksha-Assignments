#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define MAX_SIZE 100
#define SUCCESS 1
#define FAIL 0
#define INVALID_INPUT -1
#define SEND_TYPE 1
#define RECEIVE_TYPE 2

typedef struct Message
{
    long messageType;
    int arraySize;
    int array[MAX_SIZE];
} Message;

void executeMessageQueueSorting();
int createMessageQueue();
int readArray(Message *message);
void displayArray(Message *message);
void sortArray(Message *message);
int receiveMessage(int messageId, Message *message, long messageType);
int sendMessage(int messageId, Message *message);
void cleanMessageQueue(int messageId);

int main()
{
    executeMessageQueueSorting();
    return 0;
}

void executeMessageQueueSorting()
{
    int messageId = createMessageQueue();

    if (messageId == FAIL)
    {
        return;
    }

    Message message;

    if (readArray(&message) == INVALID_INPUT)
    {
        cleanMessageQueue(messageId);
        return;
    }

    printf("\nProcess 1: Parent process \n");
    printf("Array before sorting:\n");
    displayArray(&message);

    message.messageType = SEND_TYPE;

    if (sendMessage(messageId, &message) == FAIL)
    {
        return;
    }

    pid_t processId = fork();

    if (processId == 0)
    {
        if (receiveMessage(messageId, &message, SEND_TYPE) == FAIL)
        {
            return;
        }

        printf("\nProcess 2: child process\n");
        printf("Child process is sorting the array... \n");
        sortArray(&message);
        message.messageType = RECEIVE_TYPE;

        if (sendMessage(messageId, &message) == FAIL)
        {
            return;
        }

        exit(0);
    }
    else if (processId > 0)
    {
        wait(NULL);
        if (receiveMessage(messageId, &message, RECEIVE_TYPE) == FAIL)
        {
            return;
        }

        printf("\nProcess 1: Parent process \n");
        printf("Array after sorting:\n");
        displayArray(&message);
    }
    else
    {
        printf("fork failed. \n");
        cleanMessageQueue(messageId);
        
        return;
    }

    cleanMessageQueue(messageId);
}

int createMessageQueue()
{
    int messageId = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

    if (messageId == -1)
    {
        printf("msgget failed. \n");
        return FAIL;
    }

    return messageId;
}

int sendMessage(int messageId, Message *message)
{
    if (msgsnd(messageId, message, sizeof(Message) - sizeof(long), 0) == -1)
    {
        printf("msgsnd failed. \n");
        return FAIL;
    }

    return SUCCESS;
}

int receiveMessage(int messageId, Message *message, long messageType)
{
    if (msgrcv(messageId, message, sizeof(Message) - sizeof(long), messageType, 0) == -1)
    {
        printf("msgrcv failed. \n");
        return FAIL;
    }

    return SUCCESS;
}

int readArray(Message *message)
{
    printf("Enter number of elements: \n");
    scanf("%d", &message->arraySize);

    if (message->arraySize <= 0 || message->arraySize > MAX_SIZE)
    {
        printf("Array size must be in range 1-100. \n");
        return INVALID_INPUT;
    }

    printf("Enter %d elements: \n", message->arraySize);

    for (int currentMessageIndex = 0; currentMessageIndex < message->arraySize; currentMessageIndex++)
    {
        if (scanf("%d", &message->array[currentMessageIndex]) != 1)
        {
            printf("Array element must be an integer value. \n");
            return INVALID_INPUT;
        }
    }

    return SUCCESS;
}

void displayArray(Message *message)
{
    for (int currentMessageIndex = 0; currentMessageIndex < message->arraySize; currentMessageIndex++)
    {
        printf("%d ", message->array[currentMessageIndex]);
    }

    printf("\n");
}

void sortArray(Message *message)
{
    for (int turn = 0; turn < message->arraySize - 1; turn++)
    {
        for (int index = turn + 1; index < message->arraySize; index++)
        {
            if (message->array[turn] > message->array[index])
            {
                int temporaryStore = message->array[turn];
                message->array[turn] = message->array[index];
                message->array[index] = temporaryStore;
            }
        }
    }
}

void cleanMessageQueue(int messageId)
{
    msgctl(messageId, IPC_RMID, NULL);
}
