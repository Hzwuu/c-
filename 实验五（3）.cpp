#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PLAYERS 16
#define MAX_MATCHES 15
#define MAX_NAME_LEN 40
#define MAX_NATION_LEN 30
#define MAX_ROUND_LEN 30
#define MAX_LINE_LEN 256

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

int main() {
    int choice;
    int loadSuccess;

    printf("========================================\n");
    printf("     2025年世乒赛数据处理系统\n");
    printf("========================================\n\n");

    // 1. 绝对路径读取Debug下的players.txt
    printf("正在加载选手信息 players.txt ...\n");
    loadSuccess = loadPlayersFromFile();
    if (!loadSuccess) {
        printf("错误：无法加载选手信息，程序退出！\n");
        printf("请确保 players.txt 文件存在且格式正确。\n");
        return 1;
    }
    printf("成功加载 %d 名选手信息\n\n", playerCount);

    // 2. 绝对路径读取Debug下的matches.txt
    printf("正在加载比赛信息 matches.txt ...\n");
    loadSuccess = loadMatchesFromFile();
    if (!loadSuccess) {
        printf("错误：无法加载比赛信息，程序退出！\n");
        printf("请确保 matches.txt 文件存在且格式正确。\n");
        return 1;
    }
    printf("成功加载 %d 场比赛信息\n\n", matchCount);

    // 3. 计算选手名次（修复3跳5断层问题）
    printf("正在计算选手名次...\n");
    calculateRank();
    printf("名次计算完成！\n\n");

    system("pause");

    // 菜单循环
    do {
        printMenu();
        printf("请选择功能（0-4）：");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("输入有误，请重新输入！\n");
            system("pause");
            continue;
        }
        getchar();

        switch (choice) {
        case 1:
            outputPlayers();
            break;
        case 2:
            outputMatches();
            break;
        case 3:
            queryPlayer();
            break;
        case 4:
            statByNation();
            break;
        case 0:
            savePlayersToPlayerTxt();
            printf("运动员信息已写入文件player.txt，系统运行结束！\n");
            break;
        default:
            printf("输入有误，请重新输入！\n");
            break;
        }

        if (choice != 0) {
            printf("\n");
            system("pause");
        }

    } while (choice != 0);

    return 0;
}

// ==================== 读取选手：固定绝对路径 E:\create.c\create.c\x64\Debug\players.txt ====================
int loadPlayersFromFile() {
    FILE* fp;
    char line[MAX_LINE_LEN];
    char name[MAX_NAME_LEN];
    char nation[MAX_NATION_LEN];
    int count = 0;

    // 写死你的Debug文件夹完整绝对路径
    fp = fopen("E:\\create.c\\create.c\\x64\\Debug\\players.txt", "r");
    if (fp == NULL) {
        printf("  未找到 players.txt 文件！\n");
        return 0;
    }

    fgets(line, sizeof(line), fp); // 跳过表头

    while (fgets(line, sizeof(line), fp) != NULL && count < MAX_PLAYERS) {
        line[strcspn(line, "\n")] = 0;
        line[strcspn(line, "\r")] = 0;
        if (strlen(line) == 0) continue;

        // 兼容带空格姓名（如雨果·卡尔德拉诺）
        char* lastSpace = strrchr(line, ' ');
        if (lastSpace != NULL) {
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

// ==================== 读取比赛：固定绝对路径 E:\create.c\create.c\x64\Debug\matches.txt ====================
int loadMatchesFromFile() {
    FILE* fp;
    char line[MAX_LINE_LEN];
    char round[MAX_ROUND_LEN];
    char winner[MAX_NAME_LEN];
    char loser[MAX_NAME_LEN];
    int wScore, lScore;
    int count = 0;

    // 写死你的Debug文件夹完整绝对路径
    fp = fopen("E:\\create.c\\create.c\\x64\\Debug\\matches.txt", "r");
    if (fp == NULL) {
        printf("  未找到 matches.txt 文件！\n");
        return 0;
    }

    fgets(line, sizeof(line), fp); // 跳过表头

    while (fgets(line, sizeof(line), fp) != NULL && count < MAX_MATCHES) {
        line[strcspn(line, "\n")] = 0;
        line[strcspn(line, "\r")] = 0;
        if (strlen(line) == 0) continue;

        if (sscanf(line, "%s %s %s %d:%d", round, winner, loser, &wScore, &lScore) == 5) {
            strcpy(matches[count].round, round);
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

// ==================== 名次计算（无断层1~16连续） ====================
void calculateRank() {
    int i;
    int currentRank = 1;
    int hIdx = 0, qIdx = 0, rIdx = 0;
    int halfFinalLosers[2] = { 0 };
    int quarterFinalLosers[4] = { 0 };
    int round16Losers[8] = { 0 };

    for (i = 0; i < playerCount; i++) players[i].rank = 0;

    // 决赛：1、2名
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "决赛") == 0) {
            int idx = findPlayerIndex(matches[i].winner);
            if (idx != -1) players[idx].rank = currentRank++;
            idx = findPlayerIndex(matches[i].loser);
            if (idx != -1) players[idx].rank = currentRank++;
            break;
        }
    }

    // 半决赛负者：3、4名
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "半决赛") == 0) {
            int idx = findPlayerIndex(matches[i].loser);
            if (idx != -1 && hIdx < 2) halfFinalLosers[hIdx++] = idx;
        }
    }
    for (i = 0; i < hIdx; i++) players[halfFinalLosers[i]].rank = currentRank++;

    // 1/4决赛负者：5~8名
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "1/4决赛") == 0) {
            int idx = findPlayerIndex(matches[i].loser);
            if (idx != -1 && qIdx < 4) quarterFinalLosers[qIdx++] = idx;
        }
    }
    for (i = 0; i < qIdx; i++) players[quarterFinalLosers[i]].rank = currentRank++;

    // 1/8决赛负者：9~16名
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "1/8决赛") == 0) {
            int idx = findPlayerIndex(matches[i].loser);
            if (idx != -1 && rIdx < 8) round16Losers[rIdx++] = idx;
        }
    }
    for (i = 0; i < rIdx; i++) players[round16Losers[i]].rank = currentRank++;

    // 兜底异常选手
    for (i = 0; i < playerCount; i++) {
        if (players[i].rank == 0) players[i].rank = currentRank++;
    }
}

// 查找选手下标
int findPlayerIndex(const char* name) {
    int i;
    for (i = 0; i < playerCount; i++) {
        if (strcmp(players[i].name, name) == 0)
            return i;
    }
    return -1;
}

// 冒泡排序：按名次升序
void sortPlayers() {
    int i, j;
    Player temp;
    for (i = 0; i < playerCount - 1; i++) {
        for (j = 0; j < playerCount - 1 - i; j++) {
            if (players[j].rank > players[j + 1].rank) {
                temp = players[j];
                players[j] = players[j + 1];
                players[j + 1] = temp;
            }
        }
    }
}

// 打印菜单（和实验文档示例完全一致）
void printMenu() {
    system("cls");
    printf("========================================\n");
    printf("     2025年世乒赛数据处理系统\n");
    printf("========================================\n");
    printf("  1--输出选手信息及名次\n");
    printf("  2--输出所有比赛信息\n");
    printf("  3--查询选手比赛信息\n");
    printf("  4--按国籍/地区分类统计运动员人数\n");
    printf("  0--保存文件并退出\n");
    printf("========================================\n");
}

// 功能1：输出名次 + 写入output.txt（生成在Debug文件夹）
void outputPlayers() {
    FILE* fp;
    int i;
    sortPlayers();

    printf("\n=====2025年世乒赛16强选手信息=====\n");
    printf("%-24s %-16s %s\n", "姓名", "国籍/地区", "名次");
    printf("------------------------------------------------------------\n");
    for (i = 0; i < playerCount; i++) {
        printf("%-24s %-16s %d\n",
            players[i].name,
            players[i].nation,
            players[i].rank);
    }

    fp = fopen("output.txt", "w");
    if (fp == NULL) {
        printf("\n警告：无法创建output.txt文件！\n");
        return;
    }
    fprintf(fp, "=====2025年世乒赛16强选手名次表=====\n");
    fprintf(fp, "%-24s %-16s %s\n", "姓名", "国籍/地区", "名次");
    fprintf(fp, "------------------------------------------------------------\n");
    for (i = 0; i < playerCount; i++) {
        fprintf(fp, "%-24s %-16s %d\n",
            players[i].name,
            players[i].nation,
            players[i].rank);
    }
    fclose(fp);
    printf("\n选手名次信息已写入 output.txt\n");
}

// 功能2：分轮次打印全部比赛
void outputMatches() {
    int i;
    printf("\n=====1/8决赛及结果如下：=====\n");
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "1/8决赛") == 0) {
            int wIdx = findPlayerIndex(matches[i].winner);
            int lIdx = findPlayerIndex(matches[i].loser);
            printf("%s(%s) -----%s(%s)  %d:%d\n",
                matches[i].winner, players[wIdx].nation,
                matches[i].loser, players[lIdx].nation,
                matches[i].winnerScore, matches[i].loserScore);
        }
    }

    printf("\n=====1/4决赛及结果如下：=====\n");
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "1/4决赛") == 0) {
            int wIdx = findPlayerIndex(matches[i].winner);
            int lIdx = findPlayerIndex(matches[i].loser);
            printf("%s(%s) -----%s(%s)  %d:%d\n",
                matches[i].winner, players[wIdx].nation,
                matches[i].loser, players[lIdx].nation,
                matches[i].winnerScore, matches[i].loserScore);
        }
    }

    printf("\n=====半决赛及结果如下：=====\n");
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "半决赛") == 0) {
            int wIdx = findPlayerIndex(matches[i].winner);
            int lIdx = findPlayerIndex(matches[i].loser);
            printf("%s(%s) -----%s(%s)  %d:%d\n",
                matches[i].winner, players[wIdx].nation,
                matches[i].loser, players[lIdx].nation,
                matches[i].winnerScore, matches[i].loserScore);
        }
    }

    printf("\n=====冠亚军决赛及结果如下：=====\n");
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "决赛") == 0) {
            int wIdx = findPlayerIndex(matches[i].winner);
            int lIdx = findPlayerIndex(matches[i].loser);
            printf("%s(%s) -----%s(%s)  %d:%d\n",
                matches[i].winner, players[wIdx].nation,
                matches[i].loser, players[lIdx].nation,
                matches[i].winnerScore, matches[i].loserScore);
        }
    }
}

// 功能3：查询选手所有对局
void queryPlayer() {
    char name[MAX_NAME_LEN];
    int i, found = 0;
    printf("\n请输入要查找的运动员的姓名：");
    fgets(name, MAX_NAME_LEN, stdin);
    name[strcspn(name, "\n")] = '\0';

    printf("\n===== %s 的全部对局 =====\n", name);
    printf("%-12s %-20s %s\n", "轮次", "对手", "比分&胜负");
    printf("--------------------------------------------------\n");
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].winner, name) == 0) {
            printf("%-12s %-20s %d:%d 胜\n",
                matches[i].round, matches[i].loser,
                matches[i].winnerScore, matches[i].loserScore);
            found = 1;
        }
        else if (strcmp(matches[i].loser, name) == 0) {
            printf("%-12s %-20s %d:%d 负\n",
                matches[i].round, matches[i].winner,
                matches[i].winnerScore, matches[i].loserScore);
            found = 1;
        }
    }
    if (!found)
        printf("未找到该选手的任何比赛记录！\n");
}

// 功能4：按国籍统计人数
void statByNation() {
    typedef struct {
        char nation[MAX_NATION_LEN];
        int count;
    } NationStat;
    NationStat stats[MAX_PLAYERS];
    int statCount = 0, i, j, flag;

    for (i = 0; i < playerCount; i++) {
        flag = 0;
        for (j = 0; j < statCount; j++) {
            if (strcmp(stats[j].nation, players[i].nation) == 0) {
                stats[j].count++;
                flag = 1;
                break;
            }
        }
        if (!flag) {
            strcpy(stats[statCount].nation, players[i].nation);
            stats[statCount].count = 1;
            statCount++;
        }
    }

    printf("\n===== 按国籍/地区分类统计 =====\n");
    printf("%-16s %s\n", "国籍/地区", "运动员数");
    printf("----------------------------------------\n");
    for (i = 0; i < statCount; i++)
        printf("%-16s %d\n", stats[i].nation, stats[i].count);
}

// 功能0：保存选手到Debug下player.txt
void savePlayersToPlayerTxt() {
    FILE* fp;
    int i;
    sortPlayers();
    fp = fopen("player.txt", "w");
    if (fp == NULL) {
        printf("警告：无法创建player.txt文件！\n");
        return;
    }
    fprintf(fp, "===== 2025年世乒赛,16强选手存档 =====\n");
    fprintf(fp, "%-24s %-16s %s\n", "姓名", "国籍/地区", "名次");
    fprintf(fp, "------------------------------------------------------------\n");
    for (i = 0; i < playerCount; i++) {
        fprintf(fp, "%-24s %-16s %d\n",
            players[i].name,
            players[i].nation,
            players[i].rank);
    }
    fclose(fp);
}