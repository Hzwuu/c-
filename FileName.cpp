#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DAYS 100
#define MONTH "2025-06"

// 每日天气记录结构体
struct WeatherRecord {
    char date[11];
    float max_temp;
    float min_temp;
    char weatherType[10];
    float precipitation;
};

// 每月天气统计结构体
struct WeatherRecord_m {
    char ym[8];
    float ex_maxtemp;
    float ex_mintemp;
    float ave_maxtemp;
    float ave_mintemp;
    float sum_precipitation;
};

// 全局变量
struct WeatherRecord days[MAX_DAYS];
int dayCount = 0;

float ex_maxtemp, ex_mintemp;
float ave_maxtemp, ave_mintemp;
float sum_precipitation;

// 函数声明
void initWeatherData();          // 新增：直接在代码中初始化数据
void calcMonthlyStats();
void showExtremeMaxDate();
void showHotDaysStats();
void showWeatherTypeStats();
void queryByDate();
void updateYearlyMonthFile();
int isLeapYear(int year);
int getDaysOfMonth(int year, int month);
void printMenu();

// ==================== 主函数 ====================
int main() {
    initWeatherData();           // 直接初始化数据，不读文件
    calcMonthlyStats();

    int choice;
    do {
        printMenu();
        printf("请选择操作（0-5）：");
        scanf_s("%d", &choice);
        getchar();

        switch (choice) {
        case 1: showExtremeMaxDate(); break;
        case 2: showHotDaysStats(); break;
        case 3: showWeatherTypeStats(); break;
        case 4: queryByDate(); break;
        case 5: updateYearlyMonthFile(); break;
        case 0: printf("程序退出。\n"); break;
        default: printf("无效选择，请重新输入！\n");
        }
        printf("\n");
    } while (choice != 0);

    return 0;
}

// ==================== 新增：直接在代码中初始化数据 ====================
void initWeatherData() {
    // 直接在这里定义所有天气数据，不需要外部文件
    struct WeatherRecord sampleData[] = {
        {"2025-06-01", 27.5, 20.0, "多云", 0.0},
        {"2025-06-02", 30.0, 22.5, "晴", 0.0},
        {"2025-06-03", 33.5, 24.0, "晴", 0.0},
        {"2025-06-04", 35.0, 26.0, "晴", 0.0},
        {"2025-06-05", 34.0, 25.5, "多云", 0.0},
        {"2025-06-06", 28.0, 21.0, "雨", 15.5},
        {"2025-06-07", 26.5, 19.5, "雨", 22.0},
        {"2025-06-08", 29.0, 20.5, "多云", 0.0},
        {"2025-06-09", 31.0, 23.0, "晴", 0.0},
        {"2025-06-10", 36.0, 27.0, "晴", 0.0}
    };

    // 计算数据条数
    dayCount = sizeof(sampleData) / sizeof(sampleData[0]);

    // 复制到全局数组
    for (int i = 0; i < dayCount; i++) {
        days[i] = sampleData[i];
    }

    printf("已加载 %d 条天气数据（数据内嵌在代码中）\n", dayCount);
}

// ==================== 其他函数（与之前相同） ====================

void calcMonthlyStats() {
    if (dayCount == 0) {
        printf("没有天气数据！\n");
        return;
    }

    ex_maxtemp = days[0].max_temp;
    ex_mintemp = days[0].min_temp;
    ave_maxtemp = 0;
    ave_mintemp = 0;
    sum_precipitation = 0;

    for (int i = 0; i < dayCount; i++) {
        if (days[i].max_temp > ex_maxtemp)
            ex_maxtemp = days[i].max_temp;
        if (days[i].min_temp < ex_mintemp)
            ex_mintemp = days[i].min_temp;
        ave_maxtemp += days[i].max_temp;
        ave_mintemp += days[i].min_temp;
        sum_precipitation += days[i].precipitation;
    }

    ave_maxtemp /= dayCount;
    ave_mintemp /= dayCount;

    printf("\n================ 2025年6月 基本天气信息 ================\n");
    printf("极端最高气温：%.1f℃\n", ex_maxtemp);
    printf("极端最低气温：%.1f℃\n", ex_mintemp);
    printf("平均最高气温：%.1f℃\n", ave_maxtemp);
    printf("平均最低气温：%.1f℃\n", ave_mintemp);
    printf("月降水总量：%.1f mm\n", sum_precipitation);
    printf("=========================================================\n\n");
}

void showExtremeMaxDate() {
    printf("\n【2025-06 极端最高气温对应日期】\n");
    printf("极端最高气温：%.1f℃\n", ex_maxtemp);
    printf("出现日期：");
    int found = 0;
    for (int i = 0; i < dayCount; i++) {
        if (days[i].max_temp == ex_maxtemp) {
            if (found) printf("、");
            printf("%s", days[i].date);
            found = 1;
        }
    }
    if (!found) printf("未找到");
    printf("\n");
}

void showHotDaysStats() {
    printf("\n【2025-06 高温天气统计（>=35℃）】\n");
    int hotCount = 0;
    printf("高温日期：");
    for (int i = 0; i < dayCount; i++) {
        if (days[i].max_temp >= 35.0) {
            if (hotCount > 0) printf("、");
            printf("%s(%.1f℃)", days[i].date, days[i].max_temp);
            hotCount++;
        }
    }
    if (hotCount == 0) {
        printf("无高温天气");
    }
    printf("\n高温天数：%d 天\n", hotCount);
}

void showWeatherTypeStats() {
    printf("\n【2025-06 天气类型分类统计】\n");
    struct {
        char type[10];
        int count;
    } stats[20] = { 0 };
    int statCount = 0;

    for (int i = 0; i < dayCount; i++) {
        int found = 0;
        for (int j = 0; j < statCount; j++) {
            if (strcmp(stats[j].type, days[i].weatherType) == 0) {
                stats[j].count++;
                found = 1;
                break;
            }
        }
        if (!found && statCount < 20) {
            strcpy_s(stats[statCount].type, sizeof(stats[statCount].type), days[i].weatherType);
            stats[statCount].count = 1;
            statCount++;
        }
    }

    printf("%-10s %-10s\n", "天气类型", "天数");
    printf("----------------------\n");
    for (int i = 0; i < statCount; i++) {
        printf("%-10s %-10d\n", stats[i].type, stats[i].count);
    }
}

void queryByDate() {
    char date[11];
    printf("\n【按日期查询天气】\n");
    printf("请输入日期（格式：2025-06-01）：");
    scanf_s("%s", date, (unsigned)_countof(date));
    getchar();

    for (int i = 0; i < dayCount; i++) {
        if (strcmp(days[i].date, date) == 0) {
            printf("\n日期：%s\n", days[i].date);
            printf("最高气温：%.1f℃\n", days[i].max_temp);
            printf("最低气温：%.1f℃\n", days[i].min_temp);
            printf("天气类型：%s\n", days[i].weatherType);
            printf("降水量：%.1f mm\n", days[i].precipitation);
            return;
        }
    }
    printf("未找到 %s 的天气数据！\n", date);
}

void updateYearlyMonthFile() {
    printf("\n【年度每月天气记录更新】\n");
    // 这个功能仍然需要读写文件，因为要保存到 WeatherMonth.txt
    // 但即使没有这个文件，程序也能正常运行其他功能

    FILE* fp = fopen("WeatherMonth.txt", "r");
    int exists = (fp != NULL);

    if (exists) {
        fclose(fp);
        printf("发现 WeatherMonth.txt 文件。\n");
        printf("当前月份统计信息：\n");
        printf("极端最高：%.1f 极端最低：%.1f 平均最高：%.1f 平均最低：%.1f 降水：%.1f\n",
            ex_maxtemp, ex_mintemp, ave_maxtemp, ave_mintemp, sum_precipitation);
        printf("（如需写入文件，可手动创建 WeatherMonth.txt）\n");
    }
    else {
        printf("未找到 WeatherMonth.txt，如需保存月度数据请手动创建该文件。\n");
        printf("当前月份统计信息：\n");
        printf("极端最高：%.1f 极端最低：%.1f 平均最高：%.1f 平均最低：%.1f 降水：%.1f\n",
            ex_maxtemp, ex_mintemp, ave_maxtemp, ave_mintemp, sum_precipitation);
    }
}

int isLeapYear(int year) {
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

int getDaysOfMonth(int year, int month) {
    int days[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2 && isLeapYear(year))
        return 29;
    return days[month];
}

void printMenu() {
    printf("==================== 气象数据统计系统 ====================\n");
    printf("1. 2025-06 极端最高气温对应日期\n");
    printf("2. 2025-06 高温天气统计\n");
    printf("3. 2025-06 天气类型分类统计\n");
    printf("4. 按日期查询天气\n");
    printf("5. 年度每月天气记录更新\n");
    printf("0. 退出\n");
    printf("==========================================================\n");
}