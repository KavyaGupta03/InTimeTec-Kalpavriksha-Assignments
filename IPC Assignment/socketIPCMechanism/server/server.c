#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define MAX_BUFFER_SIZE 1024
#define ACCOUNT_FILE_PATH "../resource/accountDB.txt"

#define WITHDRAW_MONEY 1
#define DEPOSIT_MONEY 2
#define BALANCE 3
#define EXIT 4
#define SUCCESS 1
#define INITIAL_ACCOUNT_BALANCE 0.00

pthread_mutex_t balanceLock = PTHREAD_MUTEX_INITIALIZER;

typedef struct ClientThreadInformation
{
    int socketFileDescriptor;
    int clientId;
} ClientThreadInformation;

float getBalance();
void updateBalance(float newBalance);
void processWithdraw(int socketFileDescriptor, float amount);
void processDeposit(int socketFileDescriptor, float amount);
void processBalance(int socketFileDescriptor);
int handleClientRequest(int socketFileDescriptor, char *buffer);
void *clientThread(void *clientThreadData);
int createServerSocket();
void startServer();

int main()
{
    printf("ATM Server started.\n");
    startServer();

    return 0;
}

float getBalance()
{
    FILE *filePointer = fopen(ACCOUNT_FILE_PATH, "r");
    float balance;

    if (filePointer == NULL)
    {
        printf("Unable to open account file. Creating new file with balance 0.00 \n");
        filePointer = fopen(ACCOUNT_FILE_PATH, "w");

        if (filePointer == NULL)
        {
            printf("Unable to open account file. \n");
            return INITIAL_ACCOUNT_BALANCE;
        }

        fprintf(filePointer, "0.00");
        fclose(filePointer);

        return INITIAL_ACCOUNT_BALANCE;
    }

    fscanf(filePointer, "%f", &balance);
    fclose(filePointer);

    return balance;
}

void updateBalance(float newBalance)
{
    FILE *file = fopen(ACCOUNT_FILE_PATH, "w");

    if (file == NULL)
    {
        printf("Unable to open account file. Unable to update new Balance. \n");
        return;
    }

    fprintf(file, "%.2f", newBalance);
    fclose(file);
}

void processWithdraw(int socketFileDescriptor, float amount)
{
    char message[MAX_BUFFER_SIZE];
    pthread_mutex_lock(&balanceLock);
    float currentBalance = getBalance();

    if (amount <= 0)
    {
        snprintf(message, MAX_BUFFER_SIZE, "FAILED: Invalid amount.");
    }
    else if (amount > currentBalance)
    {
        snprintf(message, MAX_BUFFER_SIZE, "FAILED: Not enough balance. Current Balance: %.2f", currentBalance);
    }
    else
    {
        float updatedBalance = currentBalance - amount;
        updateBalance(updatedBalance);

        snprintf(message, MAX_BUFFER_SIZE, "SUCCESS: Withdrawn %.2f. New Balance: %.2f", amount, updatedBalance);
    }

    pthread_mutex_unlock(&balanceLock);
    send(socketFileDescriptor, message, strlen(message), 0);
}

void processDeposit(int socketFileDescriptor, float amount)
{
    char message[MAX_BUFFER_SIZE];
    pthread_mutex_lock(&balanceLock);

    if (amount <= 0)
    {
        snprintf(message, MAX_BUFFER_SIZE, "FAILED: Invalid amount.");
    }
    else
    {
        float currentBalance = getBalance();
        float updatedBalance = currentBalance + amount;
        updateBalance(updatedBalance);

        snprintf(message, MAX_BUFFER_SIZE, "SUCCESS: Deposited %.2f. New Balance: %.2f", amount, updatedBalance);
    }

    pthread_mutex_unlock(&balanceLock);
    send(socketFileDescriptor, message, strlen(message), 0);
}

void processBalance(int socketFileDescriptor)
{
    char message[MAX_BUFFER_SIZE];

    pthread_mutex_lock(&balanceLock);
    float current = getBalance();
    pthread_mutex_unlock(&balanceLock);

    snprintf(message, MAX_BUFFER_SIZE, "Current Balance: %.2f", current);
    send(socketFileDescriptor, message, strlen(message), 0);
}

int handleClientRequest(int socketFileDescriptor, char *buffer)
{
    int operation;
    float amount;
    sscanf(buffer, "%d %f", &operation, &amount);

    switch (operation)
    {
    case WITHDRAW_MONEY:
        processWithdraw(socketFileDescriptor, amount);
        break;
    case DEPOSIT_MONEY:
        processDeposit(socketFileDescriptor, amount);
        break;
    case BALANCE:
        processBalance(socketFileDescriptor);
        break;
    case EXIT:
        return 0;
    default:
        send(socketFileDescriptor, "Invalid operation", 17, 0);
    }

    return SUCCESS;
}

void *clientThread(void *clientThreadData)
{
    ClientThreadInformation *clientInformation = (ClientThreadInformation *)clientThreadData;
    char buffer[MAX_BUFFER_SIZE];

    while (1)
    {
        memset(buffer, 0, MAX_BUFFER_SIZE);
        int bytes = recv(clientInformation->socketFileDescriptor, buffer, MAX_BUFFER_SIZE, 0);

        if (bytes <= 0)
        {
            break;
        }

        if (!handleClientRequest(clientInformation->socketFileDescriptor, buffer))
        {
            break;
        }
    }

    close(clientInformation->socketFileDescriptor);
    free(clientInformation);
    pthread_exit(NULL);
}

int createServerSocket()
{
    int FileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (FileDescriptor < 0)
    {
        return -1;
    }

    int enableReuse = 1;
    setsockopt(FileDescriptor, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(enableReuse));

    return FileDescriptor;
}

void startServer()
{
    int serverFileDescriptor = createServerSocket();

    if (serverFileDescriptor < 0)
    {
        return;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverFileDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        return;
    }

    if (listen(serverFileDescriptor, 5) < 0)
    {
        return;
    }

    printf("ATM server listening on port %d...\n", SERVER_PORT);

    int clientCount = 0;

    while (1)
    {
        struct sockaddr_in clientAddress;
        socklen_t length = sizeof(clientAddress);
        int clientSocket = accept(serverFileDescriptor, (struct sockaddr *)&clientAddress, &length);

        if (clientSocket < 0)
        {
            continue;
        }

        clientCount++;
        ClientThreadInformation *clientInformation = malloc(sizeof(ClientThreadInformation));
        clientInformation->socketFileDescriptor = clientSocket;
        clientInformation->clientId = clientCount;

        pthread_t threadId;
        if (pthread_create(&threadId, NULL, clientThread, clientInformation) != 0)
        {
            close(clientSocket);
            free(clientInformation);
        }
        else
        {
            pthread_detach(threadId);
        }
    }

    close(serverFileDescriptor);
}
