#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>

#define MAX_PLAYERS     16
#define MAX_MATCHES     15
#define MAX_NAME_LEN    40
#define MAX_NATION_LEN  30
#define MAX_ROUND_LEN   30
#define MAX_LINE_LEN    256

// 选手结构体
typedef struct {
    char name[MAX_NAME_LEN];
    char nation[MAX_NATION_LEN];
    int rank;
} Player;

// 比赛结构体
typedef struct {
    char round[MAX_ROUND_LEN];
    char winner[MAX_NAME_LEN];
    char loser[MAX_NAME_LEN];
    int winnerScore;
    int loserScore;
} Match;

// 全局变量
Player players[MAX_PLAYERS];
Match matches[MAX_MATCHES];
int playerCount = 0;
int matchCount = 0;

// 函数声明
int loadPlayersFromFile();
int loadMatchesFromFile();
void calculateRank();
void sortPlayers();
void printMenu();
void outputPlayers();
void outputMatches();
void queryPlayer();
void statByNation();
void savePlayersToPlayerTxt();
int findPlayerIndex(const char* name);
int displayWidth(const char* str);
void printPad(int width, int target);

int main() {
    // 修复API：使用GetModuleFileNameA窄字符版本，兼容char数组，消除C2664报错
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *lastSlash = '\0';
    SetCurrentDirectoryA(exePath);

    int choice;

    printf("========================================\n");
    printf("     2025年世乒赛数据处理系统\n");
    printf("========================================\n\n");

    printf("正在加载选手信息 players.txt ...\n");
    if (!loadPlayersFromFile()) {
        printf("错误：无法加载选手信息！请检查 players.txt\n");
        return 1;
    }
    printf("成功加载 %d 名选手信息\n\n", playerCount);

    printf("正在加载比赛信息 matches.txt ...\n");
    if (!loadMatchesFromFile()) {
        printf("错误：无法加载比赛信息！请检查 matches.txt\n");
        return 1;
    }
    printf("成功加载 %d 场比赛信息\n\n", matchCount);

    printf("正在计算选手名次...\n");
    calculateRank();
    printf("名次计算完成！\n\n");

    system("pause");

    do {
        printMenu();
        printf("请选择功能（0-4）：");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("输入有误！\n");
            system("pause");
            continue;
        }
        getchar();

        switch (choice) {
        case 1: outputPlayers(); break;
        case 2: outputMatches(); break;
        case 3: queryPlayer();   break;
        case 4: statByNation(); break;
        case 0:
            savePlayersToPlayerTxt();
            printf("运动员信息已写入 player.txt ，系统结束！\n");
            break;
        default:
            printf("输入有误！\n");
        }
        if (choice != 0) { printf("\n"); system("pause"); }
    } while (choice != 0);

    return 0;
}

// 打印填充空格，用于对齐
void printPad(int width, int target)
{
    int need = target - width;
    for (int i = 0; i < need; i++)
        putchar(' ');
}

// 计算字符串控制台显示宽度：中文2字符，英文1字符
int displayWidth(const char* str) {
    int w = 0;
    const unsigned char* p = (const unsigned char*)str;
    while (*p) {
        if (*p >= 0x20 && *p <= 0x7E) {
            w++; p++;
        }
        else {
            w += 2;
            if ((*p & 0xE0) == 0xC0) p += 2;
            else if ((*p & 0xF0) == 0xE0) p += 3;
            else if ((*p & 0xF8) == 0xF0) p += 4;
            else p++;
        }
    }
    return w;
}

// 读取players.txt
int loadPlayersFromFile() {
    FILE* fp = fopen("players.txt", "r");
    if (!fp) { printf("未找到 players.txt !\n"); return 0; }
    char line[MAX_LINE_LEN], name[MAX_NAME_LEN], nation[MAX_NATION_LEN];
    int count = 0;
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp) && count < MAX_PLAYERS) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;
        char* lastSpace = strrchr(line, ' ');
        if (lastSpace) {
            *lastSpace = '\0';
            strcpy(name, line);
            strcpy(nation, lastSpace + 1);
            strcpy(players[count].name, name);
            strcpy(players[count].nation, nation);
            players[count].rank = 0;
            count++;
        }
    }
    fclose(fp);
    playerCount = count;
    return count > 0;
}

// 读取matches.txt
int loadMatchesFromFile() {
    FILE* fp = fopen("matches.txt", "r");
    if (!fp) { printf("未找到 matches.txt !\n"); return 0; }
    char line[MAX_LINE_LEN], round[MAX_ROUND_LEN], winner[MAX_NAME_LEN], loser[MAX_NAME_LEN];
    int wScore, lScore, count = 0;
    fgets(line, sizeof(line), fp);
    while (fgets(line, sizeof(line), fp) && count < MAX_MATCHES) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        char* colon = strrchr(line, ':');
        if (!colon || !isdigit(*(colon + 1)) || !isdigit(*(colon - 1))) continue;

        lScore = atoi(colon + 1);
        char* wp = colon - 1;
        while (wp >= line && isdigit(*wp)) wp--;
        wScore = atoi(wp + 1);
        char savedChar = *(wp + 1);
        *(wp + 1) = '\0';

        char rest[MAX_LINE_LEN];
        strcpy(rest, line);
        int foundIdx[2] = { -1, -1 };
        for (int i = 0; i < playerCount; i++) {
            char* pos = strstr(rest, players[i].name);
            if (pos) {
                int j = (foundIdx[0] == -1 ? 0 : 1);
                foundIdx[j] = i;
                if (j == 1) break;
            }
        }
        if (foundIdx[0] != -1 && foundIdx[1] != -1) {
            char* posW = strstr(rest, players[foundIdx[0]].name);
            char* posL = strstr(rest, players[foundIdx[1]].name);
            if (posW < posL) {
                strcpy(winner, players[foundIdx[0]].name);
                strcpy(loser, players[foundIdx[1]].name);
            }
            else {
                strcpy(loser, players[foundIdx[0]].name);
                strcpy(winner, players[foundIdx[1]].name);
            }
            int rLen = (posW < posL ? posW : posL) - rest;
            strncpy(matches[count].round, rest, rLen);
            matches[count].round[rLen] = '\0';
            strcpy(matches[count].winner, winner);
            strcpy(matches[count].loser, loser);
            matches[count].winnerScore = wScore;
            matches[count].loserScore = lScore;
            count++;
        }
    }
    fclose(fp);
    matchCount = count;
    return count > 0;
}

// 名次计算
void calculateRank() {
    for (int i = 0; i < playerCount; i++) {
        players[i].rank = 0;
    }
    int wins[MAX_PLAYERS] = { 0 };
    for (int i = 0; i < matchCount; i++) {
        int winnerIdx = findPlayerIndex(matches[i].winner);
        if (winnerIdx != -1) {
            wins[winnerIdx]++;
        }
    }
    Player tempPlayers[MAX_PLAYERS];
    for (int i = 0; i < playerCount; i++) {
        tempPlayers[i] = players[i];
    }
    for (int i = 0; i < playerCount - 1; i++) {
        for (int j = 0; j < playerCount - i - 1; j++) {
            if (wins[j] < wins[j + 1]) {
                Player temp = tempPlayers[j];
                tempPlayers[j] = tempPlayers[j + 1];
                tempPlayers[j + 1] = temp;
                int tempWin = wins[j];
                wins[j] = wins[j + 1];
                wins[j + 1] = tempWin;
            }
        }
    }
    int rank = 1;
    for (int i = 0; i < playerCount; i++) {
        for (int j = 0; j < playerCount; j++) {
            if (strcmp(players[j].name, tempPlayers[i].name) == 0) {
                players[j].rank = rank;
                break;
            }
        }
        if (i < playerCount - 1 && wins[i] != wins[i + 1]) {
            rank = i + 2;
        }
    }
}

// 按名次升序排序
void sortPlayers() {
    for (int i = 0; i < playerCount - 1; i++) {
        for (int j = 0; j < playerCount - i - 1; j++) {
            if (players[j].rank > players[j + 1].rank) {
                Player temp = players[j];
                players[j] = players[j + 1];
                players[j + 1] = temp;
            }
        }
    }
}

// 菜单
void printMenu() {
    system("cls");
    printf("\n========================================\n");
    printf("     2025年世乒赛数据处理系统\n");
    printf("========================================\n");
    printf("1. 显示所有选手信息\n");
    printf("2. 显示所有比赛信息\n");
    printf("3. 查询选手信息\n");
    printf("4. 按国家统计选手信息\n");
    printf("0. 退出系统\n");
    printf("========================================\n");
}

// 【修复对齐】标准化输出选手名次表
void outputPlayers() {
    sortPlayers();
    printf("\n=====2025年世乒赛16强选手信息=====\n");
    printf("名次    姓名                    国籍/地区\n");
    printf("------------------------------------------------------------\n");
    const int NAME_FIX = 26;
    const int NATION_FIX = 16;
    for (int i = 0; i < playerCount; i++) {
        printf("%-4d", players[i].rank);
        printf("%s", players[i].name);
        printPad(displayWidth(players[i].name), NAME_FIX);
        printf("%s\n", players[i].nation);
    }

    // 写入output.txt
    FILE* fp = fopen("output.txt", "w");
    if (fp != NULL) {
        fprintf(fp, "=====2025年世乒赛16强选手名次表=====\n");
        fprintf(fp, "名次    姓名                    国籍/地区\n");
        fprintf(fp, "------------------------------------------------------------\n");
        for (int i = 0; i < playerCount; i++) {
            fprintf(fp, "%-4d%s", players[i].rank, players[i].name);
            int w = displayWidth(players[i].name);
            for (int k = 0; k < NAME_FIX - w; k++) fputc(' ', fp);
            fprintf(fp, "%s\n", players[i].nation);
        }
        fclose(fp);
        printf("\n选手名次信息已写入 output.txt\n");
    }
}

// 输出全部比赛
void outputMatches() {
    printf("\n========== 比赛信息 ==========\n");
    printf("轮次\t胜者\t\t败者\t\t比分\n");
    printf("----------------------------\n");
    for (int i = 0; i < matchCount; i++) {
        printf("%s\t%s\t%s\t%d:%d\n",
            matches[i].round,
            matches[i].winner,
            matches[i].loser,
            matches[i].winnerScore,
            matches[i].loserScore);
    }
}

// 选手查询
void queryPlayer() {
    char name[MAX_NAME_LEN];
    printf("请输入要查询的选手姓名：");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    int idx = findPlayerIndex(name);
    if (idx == -1) {
        printf("未找到该选手！\n");
        return;
    }

    printf("\n========== 选手信息 ==========\n");
    printf("姓名：%s\n", players[idx].name);
    printf("国籍：%s\n", players[idx].nation);
    printf("排名：%d\n", players[idx].rank);

    printf("\n比赛记录：\n");
    int hasMatch = 0;
    for (int i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].winner, name) == 0 || strcmp(matches[i].loser, name) == 0) {
            hasMatch = 1;
            printf("%s: %s vs %s %d:%d %s\n",
                matches[i].round,
                matches[i].winner,
                matches[i].loser,
                matches[i].winnerScore,
                matches[i].loserScore,
                strcmp(matches[i].winner, name) == 0 ? "(胜)" : "(负)");
        }
    }
    if (!hasMatch) {
        printf("该选手暂无比赛记录\n");
    }
}

// 国籍统计
void statByNation() {
    char nations[MAX_PLAYERS][MAX_NATION_LEN];
    int nationCount[MAX_PLAYERS] = { 0 };
    int uniqueNations = 0;

    for (int i = 0; i < playerCount; i++) {
        int found = 0;
        for (int j = 0; j < uniqueNations; j++) {
            if (strcmp(nations[j], players[i].nation) == 0) {
                nationCount[j]++;
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(nations[uniqueNations], players[i].nation);
            nationCount[uniqueNations] = 1;
            uniqueNations++;
        }
    }

    printf("\n========== 各国选手统计 ==========\n");
    printf("国家\t\t人数\n");
    printf("----------------------------\n");
    for (int i = 0; i < uniqueNations; i++) {
        printf("%s\t\t%d\n", nations[i], nationCount[i]);
    }
}

// 保存到player.txt
void savePlayersToPlayerTxt() {
    FILE* fp = fopen("player.txt", "w");
    if (!fp) {
        printf("无法创建 player.txt 文件！\n");
        return;
    }
    sortPlayers();
    fprintf(fp, "排名\t姓名\t国籍\n");
    for (int i = 0; i < playerCount; i++) {
        fprintf(fp, "%d\t%s\t%s\n", players[i].rank, players[i].name, players[i].nation);
    }
    fclose(fp);
}

// 根据姓名查找下标
int findPlayerIndex(const char* name) {
    for (int i = 0; i < playerCount; i++) {
        if (strcmp(players[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}