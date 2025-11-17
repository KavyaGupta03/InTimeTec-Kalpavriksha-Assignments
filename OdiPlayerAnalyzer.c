#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Players_data.h"

#define MaxNameSize 50
#define NoOfTeams 10

typedef struct PlayerNode {
    int playerId;
    char name[MaxNameSize + 1];
    int teamId;
    int role;
    int totalRuns;
    double battingAverage;
    double strikeRate;
    int wickets;
    double economyRate;
    double performanceIdx;
    struct PlayerNode* nextRole;
    struct PlayerNode* nextTeamMember;
    struct PlayerNode* nextGlobal;
} playerInfo;

typedef struct TeamNode {
    int teamId;
    char teamName[MaxNameSize + 1];
    int totalPlayers;
    double AvgBattingStrikeRate;
    double strikeRateSum;
    int strikeRateCount;
    playerInfo* bowlerHead;
    playerInfo* batsmanHead;
    playerInfo* allRounderHead;
    playerInfo* allTeamHead;
} teamInfo;

static teamInfo teamsData[NoOfTeams];
static playerInfo* globalHead = NULL;
static int flag = 0;

const char* roleName(int r) {
    if (r == 1) return "Batsman";
    if (r == 2) return "Bowler";
    return "All-Rounder";
}

double computePerformanceIdx(const playerInfo* p) {
    if (p->role == 1) return (p->battingAverage * p->strikeRate) / 100.0;
    if (p->role == 2) return (p->wickets * 2.0) + (100.0 - p->economyRate);
    return (p->battingAverage * p->strikeRate) / 100.0 + (p->wickets * 2.0);
}

teamInfo* findTeamById(int teamId) {
    if (teamId < 1 || teamId > NoOfTeams) return NULL;
    return &teamsData[teamId - 1];
}

void insertIntoRoleList(playerInfo** headRef, playerInfo* p) {
    p->nextRole = NULL;

    if (*headRef == NULL || p->performanceIdx >= (*headRef)->performanceIdx) {
        p->nextRole = *headRef;
        *headRef = p;
        return;
    }

    playerInfo* cur = *headRef;
    while (cur->nextRole && cur->nextRole->performanceIdx > p->performanceIdx) {
        cur = cur->nextRole;
    }

    p->nextRole = cur->nextRole;
    cur->nextRole = p;
}

void insertIntoTeamAllList(teamInfo* team, playerInfo* p) {
    p->nextTeamMember = team->allTeamHead;
    team->allTeamHead = p;
    team->totalPlayers++;
}

void updateTeamStrikeRate(teamInfo* team, playerInfo* p) {
    if (p->role == 1 || p->role == 3) {
        team->strikeRateSum += p->strikeRate;
        team->strikeRateCount++;
    }
    if (team->strikeRateCount > 0) {
        team->AvgBattingStrikeRate = team->strikeRateSum / team->strikeRateCount;
    } else {
        team->AvgBattingStrikeRate = 0.0;
    }
}

void addPlayerToTeam(playerInfo* p) {
    teamInfo* team = findTeamById(p->teamId);
    if (!team) {
        free(p);
        return;
    }

    p->performanceIdx = computePerformanceIdx(p);
    insertIntoTeamAllList(team, p);

    if (p->role == 1) insertIntoRoleList(&team->batsmanHead, p);
    else if (p->role == 2) insertIntoRoleList(&team->bowlerHead, p);
    else insertIntoRoleList(&team->allRounderHead, p);

    p->nextGlobal = globalHead;
    globalHead = p;

    updateTeamStrikeRate(team, p);

    if (!flag) {
        printf("Player '%s' added to Team %s (ID %d) with PerfIdx %.2f\n",
            p->name, team->teamName, team->teamId, p->performanceIdx);
    }
}

void createAndAddPlayerInteractive() {
    playerInfo* p = malloc(sizeof(playerInfo));
    if (!p) return;

    printf("Enter Player ID: ");
    if (scanf("%d", &p->playerId) != 1) { while (getchar() != '\n'); free(p); return; }
    getchar();

    printf("Enter Name: ");
    fgets(p->name, sizeof(p->name), stdin);
    p->name[strcspn(p->name, "\n")] = '\0';

    printf("Enter Team ID (1-10): ");
    if (scanf("%d", &p->teamId) != 1) { while (getchar() != '\n'); free(p); return; }

    printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &p->role) != 1) { while (getchar() != '\n'); free(p); return; }

    printf("Total Runs: ");
    scanf("%d", &p->totalRuns);

    printf("Batting Average: ");
    scanf("%lf", &p->battingAverage);

    printf("Strike Rate: ");
    scanf("%lf", &p->strikeRate);

    printf("Wickets: ");
    scanf("%d", &p->wickets);

    printf("Economy Rate: ");
    scanf("%lf", &p->economyRate);

    while (getchar() != '\n');

    p->nextRole = p->nextTeamMember = p->nextGlobal = NULL;

    addPlayerToTeam(p);
}

void displayPlayersOfTeam(int teamId) {
    teamInfo* team = findTeamById(teamId);
    if (!team) {
        puts("Invalid Team ID");
        return;
    }
    printf("\nPlayers of Team %s (ID %d):\n", team->teamName, team->teamId);
    printf("--------------------------------------------------------------------------------\n");
    printf("ID\tName\tRole\tRuns\tAvg\tSR\tWkts\tER\tPerfIdx\n");
    printf("--------------------------------------------------------------------------------\n");
    playerInfo* cur = team->allTeamHead;
    while (cur) {
        printf("%d\t%s\t%s\t%d\t%.2f\t%.2f\t%d\t%.2f\t%.2f\n",
            cur->playerId, cur->name, roleName(cur->role),
            cur->totalRuns, cur->battingAverage, cur->strikeRate,
            cur->wickets, cur->economyRate, cur->performanceIdx);
        cur = cur->nextTeamMember;
    }
    printf("--------------------------------------------------------------------------------\n");
    printf("Total Players: %d\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n\n", team->AvgBattingStrikeRate);
}

int cmpTeamsByAvg(const void* a, const void* b) {
    const teamInfo* ta = *(const teamInfo**)a;
    const teamInfo* tb = *(const teamInfo**)b;
    if (tb->AvgBattingStrikeRate > ta->AvgBattingStrikeRate) return 1;
    if (tb->AvgBattingStrikeRate < ta->AvgBattingStrikeRate) return -1;
    return 0;
}

void displayTeamsByAvgSR() {
    teamInfo* arr[NoOfTeams];
    for (int i = 0; i < NoOfTeams; ++i) arr[i] = &teamsData[i];
    qsort(arr, NoOfTeams, sizeof(teamInfo*), cmpTeamsByAvg);
    printf("\nTeams sorted by Average Batting Strike Rate:\n");
    printf("ID\tTeam Name\tAvg Bat SR\tTotal Players\n");
    printf("------------------------------------------------\n");
    for (int i = 0; i < NoOfTeams; ++i) {
        printf("%d\t%s\t\t%.2f\t\t%d\n",
            arr[i]->teamId, arr[i]->teamName, arr[i]->AvgBattingStrikeRate, arr[i]->totalPlayers);
    }
    puts("");
}

void topKPlayersOfTeamByRole(int teamId, int role, int K) {
    teamInfo* team = findTeamById(teamId);
    if (!team) {
        puts("Invalid team id");
        return;
    }
    playerInfo* cur = NULL;
    if (role == 1) cur = team->batsmanHead;
    else if (role == 2) cur = team->bowlerHead;
    else cur = team->allRounderHead;
    if (!cur) {
        puts("No players of that role in the team.");
        return;
    }
    printf("\nTop %d %s(s) of Team %s:\n", K, roleName(role), team->teamName);
    printf("ID\tName\tPerfIdx\tSR\tAvg\tRuns\tWkts\n");
    printf("------------------------------------------------\n");
    int count = 0;
    while (cur && count < K) {
        printf("%d\t%s\t%.2f\t%.2f\t%.2f\t%d\t%d\n",
            cur->playerId, cur->name, cur->performanceIdx,
            cur->strikeRate, cur->battingAverage, cur->totalRuns, cur->wickets);
        cur = cur->nextRole;
        count++;
    }
    puts("");
}

int cmpPlayersByPerfIdx(const void* a, const void* b) {
    const playerInfo* pa = *(const playerInfo**)a;
    const playerInfo* pb = *(const playerInfo**)b;
    if (pb->performanceIdx > pa->performanceIdx) return 1;
    if (pb->performanceIdx < pa->performanceIdx) return -1;
    return 0;
}

void displayAllPlayersOfRoleAcrossTeams(int role) {
    playerInfo** arr = NULL;
    int cap = 0;
    int n = 0;
    for (int i = 0; i < NoOfTeams; ++i) {
        playerInfo* cur = NULL;
        if (role == 1) cur = teamsData[i].batsmanHead;
        else if (role == 2) cur = teamsData[i].bowlerHead;
        else cur = teamsData[i].allRounderHead;
        while (cur) {
            if (n >= cap) {
                cap = cap == 0 ? 16 : cap * 2;
                arr = realloc(arr, cap * sizeof(playerInfo*));
                if (!arr) { puts("Memory allocation failed"); return; }
            }
            arr[n++] = cur;
            cur = cur->nextRole;
        }
    }
    if (n == 0) {
        puts("No players of that role across teams.");
        free(arr);
        return;
    }
    qsort(arr, n, sizeof(playerInfo*), cmpPlayersByPerfIdx);
    printf("\nAll players of role %s across teams sorted by Performance Index:\n", roleName(role));
    printf("ID\tName\tTeamID\tPerfIdx\tSR\tAvg\tRuns\tWkts\n");
    printf("-----------------------------------------------------------\n");
    for (int i = 0; i < n; ++i) {
        playerInfo* p = arr[i];
        printf("%d\t%s\t%d\t%.2f\t%.2f\t%.2f\t%d\t%d\n",
            p->playerId, p->name, p->teamId, p->performanceIdx, p->strikeRate,
            p->battingAverage, p->totalRuns, p->wickets);
    }
    puts("");
    free(arr);
}

void init() {
    globalHead = NULL;
    for (int i = 0; i < NoOfTeams; ++i) {
        teamInfo* team = &teamsData[i];
        team->teamId = i + 1;
        strncpy(team->teamName, teams[i], MaxNameSize);
        team->teamName[MaxNameSize] = '\0';
        team->totalPlayers = 0;
        team->AvgBattingStrikeRate = 0.0;
        team->strikeRateSum = 0.0;
        team->strikeRateCount = 0;
        team->bowlerHead = team->batsmanHead = team->allRounderHead = team->allTeamHead = NULL;
    }
    flag = 1;
    for (int i = 0; i < playerCount; ++i) {
        const Player* src = &players[i];
        playerInfo* p = malloc(sizeof(playerInfo));
        if (!p) { 
            flag = 0; puts("Memory allocation failed"); 
            return; 
        }
        p->playerId = src->id;
        strncpy(p->name, src->name, MaxNameSize);
        p->name[MaxNameSize] = '\0';
        int teamIndex = -1;
        for (int t = 0; t < NoOfTeams; ++t) {
            if (strcmp(src->team, teams[t]) == 0) { teamIndex = t + 1; break; }
        }
        if (teamIndex == -1) { 
            free(p); 
            continue; 
        }
        p->teamId = teamIndex;
        if (strcmp(src->role, "Batsman") == 0) p->role = 1;
        else if (strcmp(src->role, "Bowler") == 0) p->role = 2;
        else p->role = 3;
        p->totalRuns = src->totalRuns;
        p->battingAverage = src->battingAverage;
        p->strikeRate = src->strikeRate;
        p->wickets = src->wickets;
        p->economyRate = src->economyRate;
        p->nextRole = p->nextTeamMember = p->nextGlobal = NULL;
        addPlayerToTeam(p);
    }
    flag = 0;
}

int main() {
    init();
    int choice = 0;
    int flag2=1;
    while (flag2) {
        printf("==================================================================\n");
        printf("ICC ODI PLAYER PERFORMANCE ANALYZER\n");
        printf("==================================================================\n");
        printf("1. Add Player to the team\n");
        printf("2. Display players of a specific team\n");
        printf("3. Display Teams by Average batting Strike Rate\n");
        printf("4. Display Top K players of a Specific Team by Role\n");
        printf("5. Display all Players of specific role Across All Teams by performance index\n");
        printf("6. Exit\n");
        printf("==================================================================\n");
        printf("Enter Your choice : ");
        if (scanf("%d", &choice) != 1) { 
            while (getchar() != '\n'); 
            continue; 
        }
        switch (choice) {
            case 1:
                while (getchar() != '\n');
                createAndAddPlayerInteractive();
                break;
            case 2: {
                int tid;
                printf("Enter Team ID: ");
                if (scanf("%d", &tid) != 1) { while (getchar() != '\n'); break; }
                displayPlayersOfTeam(tid);
                break;
            }
            case 3:
                displayTeamsByAvgSR();
                break;
            case 4: {
                int tid, role, k;
                printf("Team ID: ");
                if (scanf("%d", &tid) != 1) { 
                    while (getchar() != '\n'); 
                    break; 
                }
                printf("Role (1-Batsman,2-Bowler,3-All-rounder): ");
                if (scanf("%d", &role) != 1) {
                    while (getchar() != '\n'); 
                    break; 
                }
                printf("K: ");
                if (scanf("%d", &k) != 1) { 
                while (getchar() != '\n'); 
                break; 
                }
                topKPlayersOfTeamByRole(tid, role, k);
                break;
            }
            case 5: {
                int role;
                printf("Role (1-Batsman,2-Bowler,3-All-rounder): ");
                if (scanf("%d", &role) != 1) { 
                    while (getchar() != '\n'); 
                    break; 
                }
                displayAllPlayersOfRoleAcrossTeams(role);
                break;
            }
            case 6:
                flag2=0;
                break;
            default:
                printf("Invalid choice\n");
                break;
        }
    }
    return 0;
}
