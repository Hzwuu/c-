#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

#define MAX_WORDS 500
#define MAX_NAME 30
#define MAX_LINE 256
#define RANK_FILE "rankings.txt"
#define WORD_FILE "words.txt"

// 单词结构体
typedef struct {
    char english[50];
    char chinese[50];
} Word;

// 玩家结构体
typedef struct {
    char name[MAX_NAME];
    int score;
} Player;

int wordCount = 0;
Word words[MAX_WORDS];

// ---------- 函数声明 ----------
void loadWords();
void clearInputBuffer();
void menu();
void playGame();
void showRankings();
void addPlayerRecord();
void clearRankings();
int comparePlayers(const void* a, const void* b);
void saveRankings(Player players[], int count);
int loadRankings(Player players[]);

// ---------- 主函数 ----------
int main() {
    srand((unsigned int)time(NULL));
    loadWords();
    if (wordCount == 0) {
        printf("警告：单词库为空，请检查 %s 文件。\n", WORD_FILE);
    }
    int choice;
    do {
        menu();
        printf("请选择操作(1-5): ");
        scanf("%d", &choice);
        clearInputBuffer();
        switch (choice) {
        case 1: playGame(); break;
        case 2: showRankings(); break;
        case 3: addPlayerRecord(); break;
        case 4: clearRankings(); break;
        case 5: printf("退出程序，感谢使用！\n"); break;
        default: printf("无效输入，请重新选择。\n");
        }
    } while (choice != 5);
    return 0;
}

// ---------- 菜单显示 ----------
void menu() {
    printf("\n========== 英语单词闯关系统 ==========\n");
    printf("1. 闯关答题\n");
    printf("2. 查看排行榜\n");
    printf("3. 添加玩家记录\n");
    printf("4. 清空排行榜\n");
    printf("5. 退出程序\n");
    printf("======================================\n");
}

// ---------- 加载单词库 ----------
void loadWords() {
    FILE* fp = fopen(WORD_FILE, "r");
    if (!fp) {
        printf("无法打开单词文件 %s，请确保文件存在。\n", WORD_FILE);
        return;
    }
    char line[MAX_LINE];
    wordCount = 0;
    while (fgets(line, sizeof(line), fp) && wordCount < MAX_WORDS) {
        // 去除行尾换行符
        line[strcspn(line, "\n")] = '\0';
        // 按逗号分割英文和中文
        char* comma = strchr(line, ',');
        if (!comma) continue;
        *comma = '\0';
        // 复制英文
        strcpy(words[wordCount].english, line);
        // 复制中文，跳过前导空格
        char* p = comma + 1;
        while (*p == ' ') p++;
        strcpy(words[wordCount].chinese, p);
        wordCount++;
    }
    fclose(fp);
    printf("成功加载 %d 个单词。\n", wordCount);
}

// ---------- 清空输入缓冲区 ----------
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// ---------- 闯关答题 ----------
void playGame() {
    if (wordCount == 0) {
        printf("单词库为空，无法进行闯关。请检查 %s 文件。\n", WORD_FILE);
        return;
    }
    int totalQuestions = 5;
    if (totalQuestions > wordCount) totalQuestions = wordCount;
    int score = 0;
    int usedIndices[MAX_WORDS] = { 0 };
    printf("\n===== 闯关开始 (共 %d 题) =====\n", totalQuestions);
    for (int i = 0; i < totalQuestions; i++) {
        // 随机抽取不重复的单词
        int idx;
        do {
            idx = rand() % wordCount;
        } while (usedIndices[idx]);
        usedIndices[idx] = 1;

        char answer[50];
        printf("\n第 %d 题：%s\n请输入对应的英文单词: ", i + 1, words[idx].chinese);
        fgets(answer, sizeof(answer), stdin);
        answer[strcspn(answer, "\n")] = '\0';

        // 忽略大小写比较
        if (strcasecmp(answer, words[idx].english) == 0) {
            printf("回答正确！\n");
            score += 10;
        }
        else {
            printf("回答错误，正确答案是：%s\n", words[idx].english);
        }
    }
    printf("\n闯关结束，您的得分：%d 分\n", score);

    // 保存玩家记录
    char name[MAX_NAME];
    printf("请输入您的姓名保存成绩：");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    if (strlen(name) == 0) {
        strcpy(name, "匿名玩家");
    }

    Player players[100];
    int count = loadRankings(players);
    strcpy(players[count].name, name);
    players[count].score = score;
    count++;

    qsort(players, count, sizeof(Player), comparePlayers);
    saveRankings(players, count);
    printf("成绩已保存！\n");
}

// ---------- 查看排行榜 ----------
void showRankings() {
    Player players[100];
    int count = loadRankings(players);
    if (count == 0) {
        printf("排行榜暂无数据。\n");
        return;
    }
    printf("\n========== 排行榜 ==========\n");
    printf("名次\t姓名\t\t得分\n");
    for (int i = 0; i < count; i++) {
        printf("%d\t%s\t\t%d\n", i + 1, players[i].name, players[i].score);
    }
    printf("============================\n");
}

// ---------- 添加玩家记录（手动） ----------
void addPlayerRecord() {
    Player players[100];
    int count = loadRankings(players);
    char name[MAX_NAME];
    int score;
    printf("请输入玩家姓名：");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    if (strlen(name) == 0) {
        printf("姓名不能为空，取消添加。\n");
        return;
    }
    printf("请输入得分：");
    scanf("%d", &score);
    clearInputBuffer();

    strcpy(players[count].name, name);
    players[count].score = score;
    count++;

    qsort(players, count, sizeof(Player), comparePlayers);
    saveRankings(players, count);
    printf("玩家记录添加成功！\n");
}

// ---------- 清空排行榜 ----------
void clearRankings() {
    printf("确定要清空所有排行榜数据吗？(y/n): ");
    char confirm;
    scanf("%c", &confirm);
    clearInputBuffer();
    if (tolower(confirm) != 'y') {
        printf("操作取消。\n");
        return;
    }
    FILE* fp = fopen(RANK_FILE, "w");
    if (!fp) {
        printf("无法打开排行榜文件。\n");
        return;
    }
    fclose(fp);
    printf("排行榜已清空！\n");
}

// ---------- 比较函数用于qsort (降序) ----------
int comparePlayers(const void* a, const void* b) {
    Player* pa = (Player*)a;
    Player* pb = (Player*)b;
    return pb->score - pa->score;
}

// ---------- 保存排行榜 ----------
void saveRankings(Player players[], int count) {
    FILE* fp = fopen(RANK_FILE, "w");
    if (!fp) {
        printf("无法保存排行榜文件。\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s,%d\n", players[i].name, players[i].score);
    }
    fclose(fp);
}

// ---------- 加载排行榜 ----------
int loadRankings(Player players[]) {
    FILE* fp = fopen(RANK_FILE, "r");
    if (!fp) {
        return 0;
    }
    int count = 0;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp) && count < 100) {
        line[strcspn(line, "\n")] = '\0';
        char* comma = strchr(line, ',');
        if (!comma) continue;
        *comma = '\0';
        strcpy(players[count].name, line);
        players[count].score = atoi(comma + 1);
        count++;
    }
    fclose(fp);
    return count;
}