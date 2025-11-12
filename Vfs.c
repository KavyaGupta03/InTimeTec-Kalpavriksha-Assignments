#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024
#define NAME_SIZE 50
#define MAX_BLOCKS_PER_FILE 100

struct FreeBlock {
    int index;
    struct FreeBlock *next;
    struct FreeBlock *prev;
};

struct FileNode {
    char name[NAME_SIZE];
    int isDir;
    struct FileNode *parent;
    struct FileNode *next;
    struct FileNode *prev;
    struct FileNode *child;
    int size;
    int numBlocks;
    int blockPointers[MAX_BLOCKS_PER_FILE];
};

unsigned char virtualDisk[NUM_BLOCKS][BLOCK_SIZE];
struct FreeBlock *freeHead = NULL, *freeTail = NULL;
int freeCount = 0;

struct FileNode *root = NULL;
struct FileNode *cwd = NULL;

struct FreeBlock *createFreeBlock(int index) {
    struct FreeBlock *b = malloc(sizeof(struct FreeBlock));
    b->index = index;
    b->next = b->prev = NULL;
    return b;
}

void initFreeList() {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        struct FreeBlock *b = createFreeBlock(i);
        if (!freeHead) {
            freeHead = freeTail = b;
        } else {
            freeTail->next = b;
            b->prev = freeTail;
            freeTail = b;
        }
        freeCount++;
    }
}

struct FileNode *createFileNode(const char *name, int isDir) {
    struct FileNode *n = calloc(1, sizeof(struct FileNode));
    strncpy(n->name, name, NAME_SIZE);
    n->isDir = isDir;
    return n;
}

void initVFS() {
    initFreeList();
    root = createFileNode("/", 1);
    root->parent = NULL;
    root->next = root->prev = root;
    cwd = root;
    printf("Compact VFS - ready. Type 'exit' to quit.\n");
}

int allocateBlock() {
    if (!freeHead) return -1;
    struct FreeBlock *b = freeHead;
    int index = b->index;
    freeHead = b->next;
    if (freeHead) freeHead->prev = NULL;
    else freeTail = NULL;
    free(b);
    freeCount--;
    return index;
}

void freeBlockToTail(int index) {
    struct FreeBlock *b = createFreeBlock(index);
    if (!freeTail) {
        freeHead = freeTail = b;
    } else {
        freeTail->next = b;
        b->prev = freeTail;
        freeTail = b;
    }
    freeCount++;
}

struct FileNode *findChild(struct FileNode *parent, const char *name) {
    if (!parent->child) return NULL;
    struct FileNode *cur = parent->child;
    do {
        if (strcmp(cur->name, name) == 0) return cur;
        cur = cur->next;
    } while (cur != parent->child);
    return NULL;
}

void insertChild(struct FileNode *parent, struct FileNode *child) {
    child->parent = parent;
    if (!parent->child) {
        parent->child = child;
        child->next = child->prev = child;
    } else {
        struct FileNode *first = parent->child;
        struct FileNode *last = first->prev;
        last->next = child;
        child->prev = last;
        child->next = first;
        first->prev = child;
    }
}

void removeChild(struct FileNode *parent, struct FileNode *node) {
    if (!parent->child || !node) return;
    if (node->next == node) {
        parent->child = NULL;
    } else {
        if (parent->child == node) parent->child = node->next;
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
}

void mkdirCmd(const char *name) {
    if (findChild(cwd, name)) {
        printf("Name already exists in the current directory !!!\n");
        return;
    }
    struct FileNode *dir = createFileNode(name, 1);
    insertChild(cwd, dir);
    printf("Directory '%s' created successfully !!!\n", name);
}

void createCmd(const char *name) {
    if (findChild(cwd, name)) {
        printf("Name already exists in current directory !!!!\n");
        return;
    }
    struct FileNode *file = createFileNode(name, 0);
    insertChild(cwd, file);
    printf("File '%s' created successfully !!!\n", name);
}

void writeCmd(const char *name, const char *text) {
    struct FileNode *f = findChild(cwd, name);
    if (!f || f->isDir) {
        printf("File not found\n");
        return;
    }
    for (int i = 0; i < f->numBlocks; i++) {
        freeBlockToTail(f->blockPointers[i]);
    }
    f->numBlocks = 0;
    f->size = 0;
    int len = strlen(text);
    int required = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (required > freeCount) {
        printf("Disk is full !! Cannot write\n");
        return;
    }
    int pos = 0;
    for (int i = 0; i < required; i++) {
        int b = allocateBlock();
        if (b == -1) {
            printf("Disk full\n");
            return;
        }
        f->blockPointers[f->numBlocks++] = b;
        int chunk = len - pos > BLOCK_SIZE ? BLOCK_SIZE : len - pos;
        memcpy(virtualDisk[b], text + pos, chunk);
        pos += chunk;
    }
    f->size = len;
    printf("Data written successfully (size=%d bytes) !!!\n", len);
}

void readCmd(const char *name) {
    struct FileNode *f = findChild(cwd, name);
    if (!f || f->isDir) {
        printf("File not found !!!\n");
        return;
    }
    if (f->numBlocks == 0) {
        printf("(empty)\n");
        return;
    }
    int remaining = f->size;
    for (int i = 0; i < f->numBlocks && remaining > 0; i++) {
        int chunk = remaining > BLOCK_SIZE ? BLOCK_SIZE : remaining;
        fwrite(virtualDisk[f->blockPointers[i]], 1, chunk, stdout);
        remaining -= chunk;
    }
    printf("\n");
}

void deleteCmd(const char *name) {
    struct FileNode *f = findChild(cwd, name);
    if (!f) {
        printf("File not found !!!\n");
        return;
    }
    if (f->isDir) {
        printf("Use rmdir for directories.\n");
        return;
    }
    for (int i = 0; i < f->numBlocks; i++) {
        freeBlockToTail(f->blockPointers[i]);
    }
    removeChild(cwd, f);
    free(f);
    printf("File deleted successfully.\n");
}

void rmdirCmd(const char *name) {
    struct FileNode *d = findChild(cwd, name);
    if (!d) {
        printf("Directory not found.\n");
        return;
    }
    if (!d->isDir) {
        printf("Not a directory.\n");
        return;
    }
    if (d->child) {
        printf("Directory not empty. Remove files first.\n");
        return;
    }
    removeChild(cwd, d);
    free(d);
    printf("Directory removed successfully.\n");
}

void cdCmd(const char *name) {
    if (strcmp(name, "..") == 0) {
        if (cwd->parent) {
            cwd = cwd->parent;
            printf("Moved to /%s\n", cwd == root ? "" : cwd->name);
        }
        return;
    }
    struct FileNode *d = findChild(cwd, name);
    if (!d || !d->isDir) {
        printf("Directory not found.\n");
        return;
    }
    cwd = d;
    printf("Moved to /%s\n", name);
}

void lsCmd() {
    if (!cwd->child) {
        printf("(empty)\n");
        return;
    }
    struct FileNode *cur = cwd->child;
    do {
        printf("%s%s\n", cur->name, cur->isDir ? "/" : "");
        cur = cur->next;
    } while (cur != cwd->child);
}

void pwdCmd() {
    char path[1024] = "";
    struct FileNode *temp = cwd;
    while (temp && temp != root) {
        char part[NAME_SIZE + 2];
        sprintf(part, "/%s", temp->name);
        memmove(path + strlen(part), path, strlen(path) + 1);
        memcpy(path, part, strlen(part));
        temp = temp->parent;
    }
    printf("%s%s\n", "/", path);
}

void dfCmd() {
    int used = NUM_BLOCKS - freeCount;
    printf("Total Blocks: %d\n", NUM_BLOCKS);
    printf("Used Blocks: %d\n", used);
    printf("Free Blocks: %d\n", freeCount);
    printf("Disk Usage: %.2f\n", (used * 100.0) / NUM_BLOCKS);
}

void freeTree(struct FileNode *node) {
    if (!node) return;
    if (node->child) {
        struct FileNode *cur = node->child;
        do {
            struct FileNode *next = cur->next;
            freeTree(cur);
            cur = next;
        } while (cur && cur != node->child);
    }
    if (!node->isDir) {
        for (int i = 0; i < node->numBlocks; i++)
            freeBlockToTail(node->blockPointers[i]);
    }
    free(node);
}

void exitCmd() {
    freeTree(root);
    struct FreeBlock *cur = freeHead;
    while (cur) {
        struct FreeBlock *n = cur->next;
        free(cur);
        cur = n;
    }
    printf("Memory released !!! Exiting program...\n");
    exit(0);
}

void processLine(char *line) {
    char cmd[50], arg1[100], arg2[512];
    arg1[0] = arg2[0] = 0;
    sscanf(line, "%s %s %[^\n]", cmd, arg1, arg2);
    if (strcmp(cmd, "mkdir") == 0) mkdirCmd(arg1);
    else if (strcmp(cmd, "create") == 0) createCmd(arg1);
    else if (strcmp(cmd, "write") == 0) {
        char *start = strchr(line, '"');
        char *end = strrchr(line, '"');
        if (start && end && end > start) {
            char content[512];
            strncpy(content, start + 1, end - start - 1);
            content[end - start - 1] = '\0';
            writeCmd(arg1, content);
        } else {
            printf("Usage: write <file> \"text\"\n");
        }
    }
    else if (strcmp(cmd, "read") == 0) readCmd(arg1);
    else if (strcmp(cmd, "delete") == 0) deleteCmd(arg1);
    else if (strcmp(cmd, "rmdir") == 0) rmdirCmd(arg1);
    else if (strcmp(cmd, "cd") == 0) cdCmd(arg1);
    else if (strcmp(cmd, "ls") == 0) lsCmd();
    else if (strcmp(cmd, "pwd") == 0) pwdCmd();
    else if (strcmp(cmd, "df") == 0) dfCmd();
    else if (strcmp(cmd, "exit") == 0) exitCmd();
    else printf("Invalid command.\n");
}

int main() {
    char line[1024];
    initVFS();
    while (1) {
        printf("%s > ", cwd == root ? "/" : cwd->name);
        if (!fgets(line, sizeof(line), stdin)) break;
        if (strlen(line) <= 1) continue;
        processLine(line);
    }
    return 0;
}
