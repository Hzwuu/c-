#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_TEXT 1000      // 输入文本最大长度
#define MAX_WORDS 200      // 最大单词数
#define MAX_WORD_LEN 50    // 单个单词最大长度

// 情感词库定义
const char* positive_words[] = { "good", "nice", "happy" };
const char* negative_words[] = { "bad", "sad", "worse" };
const int POS_COUNT = 3;
const int NEG_COUNT = 3;

// 分词函数：使用strtok进行多分隔符分词
int tokenize(char* text, char words[][MAX_WORD_LEN]) {
    int count = 0;
    char* token;
    char* context = nullptr; // strtok_s 的上下文指针

    // 定义分隔符：空格、逗号、句号、问号、感叹号
    const char* delimiters = " ,.?!";

    // 第一次调用strtok
    token = strtok_s(text, delimiters, &context);

    while (token != NULL && count < MAX_WORDS) {
        // 过滤空字符串（strtok会自动跳过连续分隔符，但为安全起见再检查一次）
        if (strlen(token) > 0) {
            // 复制单词并转换为小写（归一化处理）
            strcpy(words[count], token);
            // 转小写
            for (int i = 0; words[count][i]; i++) {
                words[count][i] = tolower(words[count][i]);
            }
            count++;
        }
        token = strtok_s(NULL, delimiters, &context);
    }

    return count;
}

// 计算平均单词长度
double calc_avg_length(char words[][MAX_WORD_LEN], int word_count) {
    if (word_count == 0) return 0.0;

    int total_chars = 0;
    for (int i = 0; i < word_count; i++) {
        total_chars += strlen(words[i]);
    }

    return (double)total_chars / word_count;
}

// 统计情感词出现次数
void count_sentiment_words(char words[][MAX_WORD_LEN], int word_count,
    int* pos_count, int* neg_count) {
    *pos_count = 0;
    *neg_count = 0;

    for (int i = 0; i < word_count; i++) {
        // 检查是否为积极词
        for (int j = 0; j < POS_COUNT; j++) {
            if (strcmp(words[i], positive_words[j]) == 0) {
                (*pos_count)++;
                break;  // 避免重复计数
            }
        }

        // 检查是否为消极词
        for (int j = 0; j < NEG_COUNT; j++) {
            if (strcmp(words[i], negative_words[j]) == 0) {
                (*neg_count)++;
                break;
            }
        }
    }
}

// 判断情感倾向
const char* determine_sentiment(int pos_count, int neg_count) {
    if (pos_count > neg_count) {
        return "偏积极";
    }
    else if (neg_count > pos_count) {
        return "偏消极";
    }
    else {
        return "中性";
    }
}

// 打印结果
void print_results(int word_count, double avg_length,
    int pos_count, int neg_count, const char* sentiment) {
    printf("\n===== 文本分析结果 =====\n");
    printf("单词总数量：%d\n", word_count);
    printf("单词平均长度：%.2f\n", avg_length);
    printf("积极词汇出现次数：%d\n", pos_count);
    printf("消极词汇出现次数：%d\n", neg_count);
    printf("文本情感倾向：%s\n", sentiment);
}

int main() {
    char input[MAX_TEXT];
    char words[MAX_WORDS][MAX_WORD_LEN];
    int word_count;
    double avg_length;
    int pos_count, neg_count;
    const char* sentiment;

    printf("请输入一段英文文本（支持空格、,.?!等标点）：\n");

    // 读取整行输入（包含空格）
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("读取输入失败！\n");
        return 1;
    }

    // 去除末尾的换行符（fgets会保留换行符）
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }

    // 1. 分词处理（会修改原始字符串，所以如果需要保留原文，应提前备份）
    word_count = tokenize(input, words);

    // 2. 计算平均长度
    avg_length = calc_avg_length(words, word_count);

    // 3. 统计情感词
    count_sentiment_words(words, word_count, &pos_count, &neg_count);

    // 4. 判断情感倾向
    sentiment = determine_sentiment(pos_count, neg_count);

    // 5. 输出结果
    print_results(word_count, avg_length, pos_count, neg_count, sentiment);

    return 0;
}