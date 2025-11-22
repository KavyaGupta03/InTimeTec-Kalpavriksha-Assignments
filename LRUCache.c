#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bucketSize 50
#define MAX_QUEUE_SIZE 1000

typedef struct QueueNode {
    int key;
    char *value;
    struct QueueNode *next;
    struct QueueNode *prev;
} queueNode;

typedef struct MapNode {
    int key;
    queueNode *address;
    struct MapNode *next;
} mapNode;

queueNode *front = NULL;   
queueNode *rear = NULL;    
int capacity;
int cacheSize = 0;

mapNode *map[bucketSize];

int hashFunction(int key) {
    return key % bucketSize;
}

void initMap() {
    for (int i = 0; i < bucketSize; i++)
        map[i] = NULL;
}

queueNode* createQueueNode(int key, char *value) {
    queueNode *newNode = (queueNode *)malloc(sizeof(queueNode));
    newNode->key = key;
    newNode->value = strdup(value);
    newNode->next = newNode->prev = NULL;
    return newNode;
}

void moveToFront(queueNode *node) {
    if (node == front) return;
    if (node == rear) {                  
        rear = rear->prev;
        rear->next = NULL;
    } else {                             
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    node->next = front;
    node->prev = NULL;
    front->prev = node;
    front = node;
}

void insertAtFront(int key, char *value) {
    queueNode *newNode = createQueueNode(key, value);
    if (front == NULL) {
        front = rear = newNode;
        return;
    }
    newNode->next = front;
    front->prev = newNode;
    front = newNode;
}

void removeFromQueue() {
    if (front == NULL) return;
    queueNode *temp = rear;
    if (front == rear) {   
        front = rear = NULL;
    } else {
        rear = rear->prev;
        rear->next = NULL;
    }
    free(temp->value);
    free(temp);
}

mapNode* createMapNode(int key, queueNode *addr) {
    mapNode *newNode = (mapNode*)malloc(sizeof(mapNode));
    newNode->key = key;
    newNode->address = addr;
    newNode->next = NULL;
    return newNode;
}

void insertInMap(int key, queueNode *addr) {
    int idx = hashFunction(key);
    mapNode *newNode = createMapNode(key, addr);
    newNode->next = map[idx];
    map[idx] = newNode;
}

void removeFromMap(int key) {
    int idx = hashFunction(key);
    mapNode *temp = map[idx], *prev = NULL;
    while (temp != NULL && temp->key != key) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL) return;
    if (prev == NULL)
        map[idx] = temp->next;
    else
        prev->next = temp->next;
    free(temp);
}

char* get(int key) {
    int idx = hashFunction(key);
    mapNode *head = map[idx];
    while (head != NULL) {
        if (head->key == key) {
            queueNode *node = head->address;
            moveToFront(node);
            return node->value;
        }
        head = head->next;
    }
    return "NULL";
}

void cacheEvict() {
    int key = rear->key;
    removeFromMap(key);
    removeFromQueue();
    cacheSize--;
}

void put(int key, char *value) {
    int idx = hashFunction(key);
    mapNode *head = map[idx];
    while (head != NULL) {
        if (head->key == key) {
            queueNode *node = head->address;
            free(node->value);
            node->value = strdup(value);
            moveToFront(node);
            return;
        }
        head = head->next;
    }
    if (cacheSize >= capacity)
        cacheEvict();

    insertAtFront(key, value);
    insertInMap(key, front);
    cacheSize++;
}

int main() {
    char command[200];
    initMap();
    printf("Welcome to LRU Cache!\n");
    printf("Enter commands :\n\n");
    while (1) {

        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strncmp(command, "createCache", 11) == 0) {
            int size;
            sscanf(command, "createCache %d", &size);
            if (size < 1 || size > MAX_QUEUE_SIZE) {
                printf("Invalid capacity\n");
                continue;
            }
            capacity = size;
            initMap();
            front = rear = NULL;
            cacheSize = 0;
        }else if (strncmp(command, "put", 3) == 0) {
            int key;
            char value[100];
            sscanf(command, "put %d %s", &key, value);
            put(key, value);
        }else if (strncmp(command, "get", 3) == 0) {
            int key;
            sscanf(command, "get %d", &key);
            printf("%s\n", get(key));
        }else if (strncmp(command, "exit", 4) == 0) {
            printf("Exiting...\n");
            break;
        }
    }
    return 0;
}

