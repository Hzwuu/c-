#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_SLOTS 20      // 最大槽位数
#define MAX_NAME_LEN 50   // 充电宝名称最大长度（未使用，但为了结构体可扩展性保留）
#define CHARGE_RATE 20    // 每次充电模拟增加的百分比
#define FULL_BATTERY 100  // 满电状态
#define LOW_BATTERY 50    // 低电量阈值

// 充电宝状态枚举
typedef enum {
    IDLE,       // 空闲
    RENTING,    // 租借中
    CHARGING    // 充电中
} Status;

// 充电宝结构体
typedef struct {
    int id;                 // 编号（从1开始）
    Status status;          // 当前状态
    int battery;            // 当前电量（0-100），仅在柜内有效（IDLE或CHARGING）
    time_t rentStartTime;   // 租借开始时间，仅在RENTING状态有效
} PowerBank;

// 全局变量
PowerBank g_banks[MAX_SLOTS];
int g_slotCount;            // 实际槽位数（N）
time_t g_currentTime;       // 当前模拟时间，用于充电推进和计时

// 函数声明
void initializeSystem(int n);
void displayAllBanks();
void rentPowerBank();
void returnPowerBank();
void advanceTime(int minutes);
void chargeAllBanks();
void showMenu();
int isValidId(int id);

int main() {
    int choice;
    int n;
    int advanceMinutes;

    // 初始化随机数生成器
    srand((unsigned)time(NULL));

    printf("=== 智能共享充电宝租借柜模拟系统 ===\n");
    printf("请输入充电宝槽位数 (1-%d): ", MAX_SLOTS);
    if (scanf("%d", &n) != 1) {
        // 处理输入错误
        printf("无效的槽位数，使用默认值 %d。\n", MAX_SLOTS);
        n = MAX_SLOTS;
    }
    if (n < 1 || n > MAX_SLOTS) {
        printf("无效的槽位数，使用默认值 %d。\n", MAX_SLOTS);
        n = MAX_SLOTS;
    }

    initializeSystem(n);
    g_currentTime = time(NULL); // 初始化当前模拟时间

    while (1) {
        showMenu();
        printf("请选择操作: ");
        if (scanf("%d", &choice) != 1) {
            // 处理输入错误
            printf("无效选择，请重新输入。\n");
            getchar(); // 清除输入缓冲区
            continue;
        }
        getchar(); // 清除输入缓冲区

        switch (choice) {
        case 1:
            displayAllBanks();
            break;
        case 2:
            rentPowerBank();
            break;
        case 3:
            returnPowerBank();
            break;
        case 4:
            printf("请输入要推进的分钟数: ");
            if (scanf("%d", &advanceMinutes) != 1) {
                // 处理输入错误
                printf("请输入正数分钟。\n");
                getchar(); // 清除输入缓冲区
                continue;
            }
            if (advanceMinutes > 0) {
                advanceTime(advanceMinutes);
            }
            else {
                printf("请输入正数分钟。\n");
            }
            break;
        case 0:
            printf("正在退出系统...\n");
            return 0;
        default:
            printf("无效选择，请重新输入。\n");
            break;
        }
        printf("\n");
    }

    return 0;
}

/**
 * 初始化系统
 * @param n 充电宝槽位数
 */
void initializeSystem(int n) {
    g_slotCount = n;
    for (int i = 0; i < n; i++) {
        g_banks[i].id = i + 1;
        g_banks[i].status = IDLE;
        g_banks[i].battery = FULL_BATTERY;
        g_banks[i].rentStartTime = 0;
    }
    printf("系统初始化完成，共 %d 个充电宝已就绪。\n", n);
}

/**
 * 显示所有充电宝信息
 */
void displayAllBanks() {
    printf("\n===== 充电宝状态一览 =====\n");
    printf("ID\t状态\t\t电量\n");
    printf("-----------------------------\n");
    for (int i = 0; i < g_slotCount; i++) {
        printf("%d\t", g_banks[i].id);
        switch (g_banks[i].status) {
        case IDLE:
            printf("空闲\t\t%d%%\n", g_banks[i].battery);
            break;
        case RENTING:
            printf("租借中\t\t-\n");
            break;
        case CHARGING:
            printf("充电中\t\t%d%%\n", g_banks[i].battery);
            break;
        default:
            printf("未知状态\n");
            break;
        }
    }
    printf("============================\n");
}

/**
 * 租借充电宝
 */
void rentPowerBank() {
    int availableIds[MAX_SLOTS];
    int availableCount = 0;

    // 查找所有空闲充电宝
    for (int i = 0; i < g_slotCount; i++) {
        if (g_banks[i].status == IDLE) {
            availableIds[availableCount++] = g_banks[i].id;
        }
    }

    if (availableCount == 0) {
        printf("暂无可用充电宝。\n");
        return;
    }

    // 随机选择一个空闲充电宝
    int randomIndex = rand() % availableCount;
    int chosenId = availableIds[randomIndex];
    int index = chosenId - 1; // 因为id从1开始，数组下标从0开始

    // 更新状态
    g_banks[index].status = RENTING;
    g_banks[index].rentStartTime = g_currentTime;

    printf("租借成功！您已借出充电宝 ID: %d\n", chosenId);
    printf("租借开始时间: %s", ctime(&g_currentTime));
}

/**
 * 归还充电宝
 */
void returnPowerBank() {
    int id;
    printf("请输入要归还的充电宝编号: ");
    scanf("%d", &id);

    if (!isValidId(id)) {
        printf("错误：充电宝编号不存在。\n");
        return;
    }

    int index = id - 1;
    if (g_banks[index].status != RENTING) {
        printf("错误：该充电宝当前不是租借状态。\n");
        return;
    }

    // 计算租借时长（分钟）
    time_t endTime = g_currentTime;
    double diffSeconds = difftime(endTime, g_banks[index].rentStartTime);
    int minutes = (int)(diffSeconds / 60);
    if (diffSeconds < 0) {
        // 如果时间倒流（理论上不会发生），简单处理
        minutes = 0;
    }

    // 计算费用：不足1小时按1小时计，首小时2元，之后每小时1元
    int hours = minutes / 60;
    if (minutes % 60 != 0) {
        hours++; // 向上取整
    }
    int fee;
    if (hours <= 1) {
        fee = 2;
    }
    else {
        fee = 2 + (hours - 1) * 1;
    }

    // 随机生成归还电量（20%-90%）
    int returnBattery = 20 + rand() % 71; // 20-90

    // 更新充电宝状态
    g_banks[index].battery = returnBattery;
    if (returnBattery >= LOW_BATTERY) {
        g_banks[index].status = IDLE;
        printf("充电宝 %d 已归还，电量 %d%%，状态变更为：空闲。\n", id, returnBattery);
    }
    else {
        g_banks[index].status = CHARGING;
        printf("充电宝 %d 已归还，电量 %d%%，低于50%%，正在充电中。\n", id, returnBattery);
    }

    // 输出费用信息
    printf("租借时长: %d 分钟 (%.2f 小时)，费用: %d 元\n", minutes, minutes / 60.0, fee);
}

/**
 * 推进模拟时间
 * @param minutes 推进的分钟数
 */
void advanceTime(int minutes) {
    g_currentTime += minutes * 60; // 转换为秒
    chargeAllBanks();
    printf("时间已推进 %d 分钟。当前模拟时间: %s", minutes, ctime(&g_currentTime));
}

/**
 * 对所有充电中的充电宝进行充电模拟
 */
void chargeAllBanks() {
    for (int i = 0; i < g_slotCount; i++) {
        if (g_banks[i].status == CHARGING) {
            // 每次推进，电量增加20%
            g_banks[i].battery += CHARGE_RATE;
            if (g_banks[i].battery >= FULL_BATTERY) {
                g_banks[i].battery = FULL_BATTERY;
                g_banks[i].status = IDLE;
                printf("充电宝 %d 已充满，状态变更为：空闲。\n", g_banks[i].id);
            }
            else {
                // printf("充电宝 %d 当前电量: %d%%\n", g_banks[i].id, g_banks[i].battery); // 可选细节输出
            }
        }
    }
}

/**
 * 显示主菜单
 */
void showMenu() {
    printf("\n===== 智能共享充电宝租借柜 =====\n");
    printf("1. 查看所有充电宝状态\n");
    printf("2. 租借充电宝\n");
    printf("3. 归还充电宝\n");
    printf("4. 推进时间（模拟充电）\n");
    printf("0. 退出系统\n");
    printf("================================\n");
}

/**
 * 验证充电宝ID是否有效
 * @param id 充电宝编号
 * @return 1有效，0无效
 */
int isValidId(int id) {
    return (id >= 1 && id <= g_slotCount);
}