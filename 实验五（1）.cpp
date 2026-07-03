#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PLAYERS 16
#define MAX_MATCHES 15
#define MAX_NAME_LEN 30
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
void savePlayers();
int findPlayerIndex(const char* name);
void addPlayer(const char* name, const char* nation);

int main() {
    int choice;
    int loadSuccess;
   
   

    printf("========================================\n");
    printf("     2025年世乒赛数据处理系统\n");
    printf("========================================\n\n");

    // 1. 从文件读取选手信息
    printf("正在加载选手信息...\n");
    loadSuccess = loadPlayersFromFile();
    if (!loadSuccess) {
        printf("错误：无法加载选手信息，程序退出！\n");
        printf("请确保 players.txt 文件存在且格式正确。\n");
        return 1;
    }
    printf("? 成功加载 %d 名选手信息\n\n", playerCount);

    // 2. 从文件读取比赛信息
    printf("正在加载比赛信息...\n");
    loadSuccess = loadMatchesFromFile();
    if (!loadSuccess) {
        printf("错误：无法加载比赛信息，程序退出！\n");
        printf("请确保 matches.txt 文件存在且格式正确。\n");
        return 1;
    }
    printf("? 成功加载 %d 场比赛信息\n\n", matchCount);

    // 3. 计算名次
    printf("正在计算选手名次...\n");
    calculateRank();
    printf("? 名次计算完成！\n\n");

    system("pause");

    // 菜单循环
    do {
        printMenu();
        printf("请选择功能（0-4）：");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("输入无效，请重新选择！\n");
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
            savePlayers();
            printf("数据已保存，程序退出！\n");
            break;
        default:
            printf("无效选择，请重新输入！\n");
            break;
        }

        if (choice != 0) {
            printf("\n");
            system("pause");
        }

    } while (choice != 0);

    return 0;
}

// 从文件读取选手信息
int loadPlayersFromFile() {
    FILE* fp;
    char line[MAX_LINE_LEN];
    char name[MAX_NAME_LEN];
    char nation[MAX_NATION_LEN];
    int count = 0;

    fp = fopen("players.txt", "r");
    if (fp == NULL) {
        printf("  未找到 players.txt 文件！\n");
        return 0;
    }

    // 跳过表头（如果有）
    fgets(line, sizeof(line), fp);

    // 逐行读取：姓名 国籍
    while (fgets(line, sizeof(line), fp) != NULL) {
        // 去除换行符
        line[strcspn(line, "\n")] = 0;
        line[strcspn(line, "\r")] = 0;

        if (strlen(line) == 0) continue;

        // 解析：姓名 国籍（用空格或Tab分隔）
        if (sscanf(line, "%s %s", name, nation) == 2) {
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

// 从文件读取比赛信息
int loadMatchesFromFile() {
    FILE* fp;
    char line[MAX_LINE_LEN];
    char round[MAX_ROUND_LEN];
    char winner[MAX_NAME_LEN];
    char loser[MAX_NAME_LEN];
    int wScore, lScore;
    int count = 0;

    fp = fopen("matches.txt", "r");
    if (fp == NULL) {
        printf("  未找到 matches.txt 文件！\n");
        return 0;
    }

    // 跳过表头（如果有）
    fgets(line, sizeof(line), fp);

    // 逐行读取
    while (fgets(line, sizeof(line), fp) != NULL) {
        // 去除换行符
        line[strcspn(line, "\n")] = 0;
        line[strcspn(line, "\r")] = 0;

        if (strlen(line) == 0) continue;

        // 解析格式：轮次 胜者 负者 胜者得分:负者得分
        if (sscanf(line, "%s %s %s %d:%d",
            round, winner, loser, &wScore, &lScore) == 5) {
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

// 计算选手名次
void calculateRank() {
    int i, j;
    int rank = 1;
    int halfFinalLosers[2] = { 0 };
    int quarterFinalLosers[4] = { 0 };
    int round16Losers[8] = { 0 };
    int hIdx = 0, qIdx = 0, rIdx = 0;
    int found;

    // 先重置所有选手名次
    for (i = 0; i < playerCount; i++) {
        players[i].rank = 0;
    }

    // 找出冠军（决赛胜者）
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "决赛") == 0) {
            int idx = findPlayerIndex(matches[i].winner);
            if (idx != -1) players[idx].rank = 1;
            idx = findPlayerIndex(matches[i].loser);
            if (idx != -1) players[idx].rank = 2;
            break;
        }
    }

    // 找出半决赛负者（第3-4名）
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "半决赛") == 0) {
            int idx = findPlayerIndex(matches[i].loser);
            if (idx != -1 && hIdx < 2) {
                halfFinalLosers[hIdx++] = idx;
            }
        }
    }
    rank = 3;
    for (i = 0; i < hIdx; i++) {
        players[halfFinalLosers[i]].rank = rank++;
    }

    // 找出1/4决赛负者（第5-8名）
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "1/4决赛") == 0) {
            int idx = findPlayerIndex(matches[i].loser);
            if (idx != -1 && qIdx < 4) {
                quarterFinalLosers[qIdx++] = idx;
            }
        }
    }
    rank = 5;
    for (i = 0; i < qIdx; i++) {
        players[quarterFinalLosers[i]].rank = rank++;
    }

    // 找出1/8决赛负者（第9-16名）
    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].round, "1/8决赛") == 0) {
            int idx = findPlayerIndex(matches[i].loser);
            if (idx != -1 && rIdx < 8) {
                round16Losers[rIdx++] = idx;
            }
        }
    }
    rank = 9;
    for (i = 0; i < rIdx; i++) {
        players[round16Losers[i]].rank = rank++;
    }

    // 检查是否有选手名次未被赋值
    for (i = 0; i < playerCount; i++) {
        if (players[i].rank == 0) {
            players[i].rank = rank++;
        }
    }
}

// 查找选手索引
int findPlayerIndex(const char* name) {
    int i;
    for (i = 0; i < playerCount; i++) {
        if (strcmp(players[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// 按名次排序（冒泡排序）
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

// 打印菜单
void printMenu() {
    system("cls");
    printf("========================================\n");
    printf("     2025年世乒赛数据处理系统\n");
    printf("========================================\n");
    printf("  1 — 输出选手信息及名次\n");
    printf("  2 — 输出所有比赛信息\n");
    printf("  3 — 查询选手比赛信息\n");
    printf("  4 — 按国籍/地区分类统计运动员人数\n");
    printf("  0 — 保存文件并退出\n");
    printf("========================================\n");
}

// 功能1：输出选手信息及名次
void outputPlayers() {
    FILE* fp;
    int i;

    sortPlayers();

    printf("\n===== 选手名次表 =====\n");
    printf("%-4s%-36s%20s\n", "名次", "姓名", "国籍/地区");
    printf("------------------------------------------------------------------------------------------------\n");

    for (i = 0; i < playerCount; i++) {
        printf("%-4d%-30s%20s\n",
            players[i].rank,
            players[i].name,
            players[i].nation);
    }

    fp = fopen("output.txt", "w");
    if (fp == NULL) {
        printf("\n警告：无法创建output.txt文件！\n");
        return;
    }

    fprintf(fp, "===== 2025年世乒赛男单选手名次表 =====\n");
    // 固定宽度表头
    fprintf(fp, "%-4s%-25s%-20s\n", "名次", "姓名", "国籍/地区");
    fprintf(fp, "-------------------------------------------------------------\n");
    for (i = 0; i < playerCount; i++)
    {
        // 名次占4字符，姓名占25字符，国籍占20字符，全部左对齐
        fprintf(fp, "%-4d%-25s%-20s\n",
            players[i].rank,
            players[i].name,
            players[i].nation);
    }

    fclose(fp);
    printf("\n? 选手信息已写入 output.txt\n");
}

// 功能2：输出所有比赛信息
void outputMatches() {
    int i;

    printf("\n===== 全部比赛信息 =====\n");
    printf("轮次\t\t胜者\t\t\t负者\t\t\t比分\n");
    printf("-------------------------------------------------------------------\n");

    for (i = 0; i < matchCount; i++) {
        printf("%s\t%-16s\t%-16s\t%d:%d\n",
            matches[i].round,
            matches[i].winner,
            matches[i].loser,
            matches[i].winnerScore,
            matches[i].loserScore);
    }
}

// 功能3：查询选手比赛信息
void queryPlayer() {
    char name[MAX_NAME_LEN];
    int i;
    int found = 0;

    printf("\n请输入要查询的选手姓名：");
    scanf("%s", name);
    getchar();

    printf("\n===== %s 的比赛信息 =====\n", name);
    printf("轮次\t\t对手\t\t\t比分\n");
    printf("--------------------------------------------------\n");

    for (i = 0; i < matchCount; i++) {
        if (strcmp(matches[i].winner, name) == 0) {
            printf("%s\t%-16s\t%d:%d (胜)\n",
                matches[i].round,
                matches[i].loser,
                matches[i].winnerScore,
                matches[i].loserScore);
            found = 1;
        }
        else if (strcmp(matches[i].loser, name) == 0) {
            printf("%s\t%-16s\t%d:%d (负)\n",
                matches[i].round,
                matches[i].winner,
                matches[i].winnerScore,
                matches[i].loserScore);
            found = 1;
        }
    }

    if (!found) {
        printf("未找到 %s 的比赛信息！\n", name);
    }
}

// 功能4：按国籍/地区分类统计运动员人数
void statByNation() {
    typedef struct {
        char nation[MAX_NATION_LEN];
        int count;
    } NationStat;

    NationStat stats[MAX_PLAYERS];
    int statCount = 0;
    int i, j;
    int found;

    for (i = 0; i < playerCount; i++) {
        found = 0;
        for (j = 0; j < statCount; j++) {
            if (strcmp(stats[j].nation, players[i].nation) == 0) {
                stats[j].count++;
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(stats[statCount].nation, players[i].nation);
            stats[statCount].count = 1;
            statCount++;
        }
    }

    printf("\n===== 按国籍/地区分类统计 =====\n");
    printf("国籍/地区\t\t运动员人数\n");
    printf("----------------------------------------\n");

    for (i = 0; i < statCount; i++) {
        printf("%-16s\t%d人\n", stats[i].nation, stats[i].count);
    }
}

// 功能0：保存选手信息到player.txt
void savePlayers() {
    FILE* fp;
    int i;

    sortPlayers();

    fp = fopen("player.txt", "w");
    if (fp == NULL) {
        printf("警告：无法创建player.txt文件！\n");
        return;
    }

    fprintf(fp, "===== 2025年世乒赛男单选手信息 =====\n");
    fprintf(fp, "名次\t姓名\t\t\t国籍/地区\n");
    fprintf(fp, "------------------------------------------------\n");
    for (i = 0; i < playerCount; i++) {
        fprintf(fp, "%d\t%-16s\t%s\n",
            players[i].rank,
            players[i].name,
            players[i].nation);
    }

    fclose(fp);
    printf("? 选手信息已保存至 player.txt\n");
}