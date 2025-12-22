#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define MAX_BUFFER_SIZE 1024
#define WITHDRAW_MONEY 1
#define DEPOSIT_MONEY 2
#define SHOW_AMOUNT 3
#define QUIT 4
#define MIN_CHOICE 1
#define MAX_CHOICE 4
#define SUCCESS 1
#define FAIL 0

void showAtmMenu();
int takeUserChoiceInput();
bool validateChoice(int choice);
float takeUserAmountInput();
int prepareRequest(int userChoice, char *message);
int createSocketFileDescriptor();
void receiveResponse(int socketFileDescriptor);
void sendUserRequest(int socketFileDescriptor, char *message);
int connectServer(int socketFileDescriptor);
void startClientSession(int socketFileDescriptor);

int main()
{
    int clientFileDescriptor = createSocketFileDescriptor();

    if (clientFileDescriptor < 0)
    {
        return 0;
    }

    if (connectServer(clientFileDescriptor) == FAIL)
    {
        close(clientFileDescriptor);
        return 0;
    }

    startClientSession(clientFileDescriptor);

    close(clientFileDescriptor);
    return 0;
}

void startClientSession(int socketFileDescriptor)
{
    char message[MAX_BUFFER_SIZE];

    while (1)
    {
        showAtmMenu();
        int userChoice = takeUserChoiceInput();
        memset(message, 0, MAX_BUFFER_SIZE);

        if (!prepareRequest(userChoice, message))
        {
            sendUserRequest(socketFileDescriptor, message);
            printf("Session ended.\n");

            break;
        }

        sendUserRequest(socketFileDescriptor, message);
        receiveResponse(socketFileDescriptor);
    }
}

void showAtmMenu()
{
    printf("\n-----------------------------\n");
    printf("ATM OPTIONS\n");
    printf("-----------------------------\n");
    printf("1.Withdraw\n");
    printf("2.Deposit\n");
    printf("3.Display Balance\n");
    printf("4.Exit\n");
    printf("-----------------------------\n");
}

bool validateChoice(int choice)
{
    if (choice < MIN_CHOICE || choice > MAX_CHOICE)
    {
        return false;
    }

    return true;
}

int takeUserChoiceInput()
{
    int choice;

    while (1)
    {
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input. Choice must be an integer value in range 1-4. \n");

            while (getchar() != '\n')
                ;
            continue;
        }

        if (validateChoice(choice) == false)
        {
            printf("Invalid input. Choice must be an integer value in range 1-4. \n");
            continue;
        }

        while (getchar() != '\n')
            ;
        return choice;
    }
}

float takeUserAmountInput()
{
    float amount;

    while (1)
    {
        printf("Enter amount: ");

        if (scanf("%f", &amount) != 1 || amount <= 0)
        {
            printf("Invalid amount. Amount must be greater than 0. \n");

            while (getchar() != '\n')
                ;
            continue;
        }

        while (getchar() != '\n')
            ;
        return amount;
    }
}

int prepareRequest(int userChoice, char *message)
{
    float amount = 0;

    if (userChoice == WITHDRAW_MONEY || userChoice == DEPOSIT_MONEY)
    {
        amount = takeUserAmountInput();
    }

    sprintf(message, "%d %.2f", userChoice, amount);

    if (userChoice == QUIT)
    {
        return 0;
    }

    return SUCCESS;
}

int createSocketFileDescriptor()
{
    int sockFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (sockFileDescriptor < 0)
    {
        printf("Socket creation failed.\n");
        return FAIL;
    }

    return sockFileDescriptor;
}

int connectServer(int socketFileDescriptor)
{
    struct sockaddr_in serverInformation;

    serverInformation.sin_family = AF_INET;
    serverInformation.sin_port = htons(SERVER_PORT);
    serverInformation.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(socketFileDescriptor, (struct sockaddr *)&serverInformation, sizeof(serverInformation)) < 0)
    {
        printf("Unable to connect to server.\n");
        return FAIL;
    }

    printf("Connected to ATM server.\n");
    return SUCCESS;
}

void sendUserRequest(int socketFileDescriptor, char *message)
{
    send(socketFileDescriptor, message, strlen(message), 0);
}

void receiveResponse(int socketFileDescriptor)
{
    char response[MAX_BUFFER_SIZE];
    memset(response, 0, MAX_BUFFER_SIZE);

    int bytes = recv(socketFileDescriptor, response, MAX_BUFFER_SIZE, 0);

    if (bytes == 0)
    {
        printf("Server closed the connection.\n");
        return;
    }
    else if (bytes < 0)
    {
        perror("recv failed");
        return;
    }

    printf("\nServer says: %s\n", response);
}
