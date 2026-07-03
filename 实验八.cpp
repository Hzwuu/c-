#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_CARS 1000
#define MAX_PLATE 20
#define FILENAME "parkingLot.txt"
#define OUTPUT_FILENAME "parkingLot_new.txt"
#define HOURLY_RATE 10
#define DAILY_CAP_HOURS 8
#define ZOMBIE_DAYS 7

// 车辆信息结构体
typedef struct {
    char plate[MAX_PLATE];
    time_t entryTime;
    time_t exitTime;
    double fee;
    int isExited;  // 0-未出场, 1-已出场
} ParkingRecord;

// 全局变量
ParkingRecord records[MAX_CARS];
int recordCount = 0;

// 函数声明
void loadData();
void saveData();
void vehicleEntry();
void vehicleExit();
void queryVehicle();
void sortRecords();
void totalFees();
void zombieCars();
void displayMenu();
void displayRecord(ParkingRecord* record);
int findVehicle(const char* plate);
int isPlateExist(const char* plate);
void trim(char* str);
double calculateFee(time_t entry, time_t exit);
int getParkingHours(time_t entry, time_t exit);
int getParkingDays(time_t entry, time_t exit);
void safeGetInput(char* buffer, int size);
int safeScanfInt(int* value);

// 安全获取输入
void safeGetInput(char* buffer, int size) {
    if (fgets(buffer, size, stdin) != NULL) {
        trim(buffer);
    }
    else {
        buffer[0] = '\0';
    }
}

// 安全的整数输入
int safeScanfInt(int* value) {
    char buffer[20];
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        return sscanf(buffer, "%d", value);
    }
    return 0;
}

// 主函数
int main() {
    // 加载数据
    loadData();

    int choice;
    while (1) {
        displayMenu();
        printf("请选择操作 (1-8): ");
        if (safeScanfInt(&choice) != 1) {
            printf("输入无效，请输入数字！\n");
            while (getchar() != '\n');
            continue;
        }
        getchar();  // 清除缓冲区

        switch (choice) {
        case 1:
            vehicleEntry();
            break;
        case 2:
            vehicleExit();
            break;
        case 3:
            queryVehicle();
            break;
        case 4:
            sortRecords();
            break;
        case 5:
            totalFees();
            break;
        case 6:
            zombieCars();
            break;
        case 7:
            saveData();
            printf("数据已保存到 %s\n", OUTPUT_FILENAME);
            break;
        case 8:
            saveData();
            printf("感谢使用智能停车系统！\n");
            return 0;
        default:
            printf("无效选择，请重新输入！\n");
        }
        printf("\n");
    }
    return 0;
}

// 显示菜单
void displayMenu() {
    printf("========================================\n");
    printf("        智能停车场管理系统\n");
    printf("========================================\n");
    printf("1. 车辆入库\n");
    printf("2. 车辆出库\n");
    printf("3. 车辆查询\n");
    printf("4. 记录排序（按入场时间）\n");
    printf("5. 费用汇总\n");
    printf("6. 僵尸车统计\n");
    printf("7. 数据导出\n");
    printf("8. 退出系统\n");
    printf("========================================\n");
}

// 去除字符串首尾空格
void trim(char* str) {
    if (str == NULL) return;

    // 去除前导空格
    char* start = str;
    while (isspace((unsigned char)*start)) start++;

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    // 去除尾部空格
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

// 查找车辆索引
int findVehicle(const char* plate) {
    for (int i = 0; i < recordCount; i++) {
        if (strcmp(records[i].plate, plate) == 0) {
            return i;
        }
    }
    return -1;
}

// 检查车牌是否存在（包括已出场）
int isPlateExist(const char* plate) {
    return findVehicle(plate) != -1;
}

// 计算停车时长（小时）
int getParkingHours(time_t entry, time_t exit) {
    double diff = difftime(exit, entry);
    int hours = (int)(diff / 3600);
    if (diff > hours * 3600) {
        hours++;  // 不足1小时按1小时计
    }
    return hours;
}

// 计算停车天数
int getParkingDays(time_t entry, time_t exit) {
    double diff = difftime(exit, entry);
    return (int)(diff / 86400);
}

// 计算停车费用
double calculateFee(time_t entry, time_t exit) {
    if (entry >= exit) return 0;

    int totalHours = getParkingHours(entry, exit);
    int totalDays = getParkingDays(entry, exit);

    double totalFee = 0;

    if (totalDays == 0) {
        // 当天停车
        if (totalHours > 8) {
            totalFee = DAILY_CAP_HOURS * HOURLY_RATE;  // 封顶
        }
        else {
            totalFee = totalHours * HOURLY_RATE;
        }
    }
    else {
        // 跨天停车
        // 计算第一天的费用
        time_t dayEnd = entry + 86400;  // 第二天0点
        int firstDayHours = getParkingHours(entry, dayEnd);
        if (firstDayHours > 8) {
            totalFee += DAILY_CAP_HOURS * HOURLY_RATE;
        }
        else {
            totalFee += firstDayHours * HOURLY_RATE;
        }

        // 计算中间完整天的费用
        for (int d = 1; d < totalDays; d++) {
            totalFee += DAILY_CAP_HOURS * HOURLY_RATE;
        }

        // 计算最后一天的费用
        time_t lastDayStart = entry + totalDays * 86400;
        int lastDayHours = getParkingHours(lastDayStart, exit);
        if (lastDayHours > 8) {
            totalFee += DAILY_CAP_HOURS * HOURLY_RATE;
        }
        else {
            totalFee += lastDayHours * HOURLY_RATE;
        }
    }

    return totalFee;
}

// 加载数据
void loadData() {
    FILE* fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        printf("未找到数据文件 %s，将创建新数据文件\n", FILENAME);
        return;
    }

    recordCount = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL && recordCount < MAX_CARS) {
        char plate[MAX_PLATE];
        long entry, exit;
        double fee;
        int exited;

        if (sscanf(line, "%[^|]|%ld|%ld|%lf|%d", plate, &entry, &exit, &fee, &exited) == 5) {
            strcpy(records[recordCount].plate, plate);
            records[recordCount].entryTime = (time_t)entry;
            records[recordCount].exitTime = (time_t)exit;
            records[recordCount].fee = fee;
            records[recordCount].isExited = exited;
            recordCount++;
        }
    }
    fclose(fp);
    printf("成功加载 %d 条停车记录\n", recordCount);
}

// 保存数据
void saveData() {
    FILE* fp = fopen(OUTPUT_FILENAME, "w");
    if (fp == NULL) {
        printf("错误：无法打开文件 %s 进行写入！\n", OUTPUT_FILENAME);
        return;
    }

    for (int i = 0; i < recordCount; i++) {
        fprintf(fp, "%s|%ld|%ld|%.2f|%d\n",
            records[i].plate,
            (long)records[i].entryTime,
            (long)records[i].exitTime,
            records[i].fee,
            records[i].isExited);
    }
    fclose(fp);
}

// 格式化时间字符串
void formatTime(time_t t, char* buffer, size_t size) {
    struct tm tm_info;
    if (localtime_s(&tm_info, &t) == 0) {
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);
    }
    else {
        strcpy(buffer, "时间无效");
    }
}

// 显示单条记录
void displayRecord(ParkingRecord* record) {
    char entryStr[50], exitStr[50];

    formatTime(record->entryTime, entryStr, sizeof(entryStr));

    if (record->isExited) {
        formatTime(record->exitTime, exitStr, sizeof(exitStr));
        printf("车牌: %-10s 入场: %s  出场: %s  费用: %.2f元  状态: 已出场\n",
            record->plate, entryStr, exitStr, record->fee);
    }
    else {
        printf("车牌: %-10s 入场: %s  状态: 停车中\n",
            record->plate, entryStr);
    }
}

// 车辆入库
void vehicleEntry() {
    char plate[MAX_PLATE];
    printf("请输入车牌号: ");
    safeGetInput(plate, sizeof(plate));

    if (strlen(plate) == 0) {
        printf("错误：车牌号不能为空！\n");
        return;
    }

    // 检查是否已在停车场内（未出场）
    int index = findVehicle(plate);
    if (index != -1 && records[index].isExited == 0) {
        printf("错误：车辆 %s 已在停车场内，禁止重复入库！\n", plate);
        return;
    }

    // 如果车辆已出场，更新记录
    if (index != -1 && records[index].isExited == 1) {
        records[index].entryTime = time(NULL);
        records[index].exitTime = 0;
        records[index].fee = 0;
        records[index].isExited = 0;
        printf("车辆 %s 重新入库成功！\n", plate);
        return;
    }

    // 新车入库
    if (recordCount >= MAX_CARS) {
        printf("错误：停车场已满！\n");
        return;
    }

    strcpy(records[recordCount].plate, plate);
    records[recordCount].entryTime = time(NULL);
    records[recordCount].exitTime = 0;
    records[recordCount].fee = 0;
    records[recordCount].isExited = 0;
    recordCount++;

    char timeStr[50];
    formatTime(records[recordCount - 1].entryTime, timeStr, sizeof(timeStr));
    printf("车辆 %s 入库成功！时间: %s\n", plate, timeStr);
}

// 车辆出库
void vehicleExit() {
    char plate[MAX_PLATE];
    printf("请输入车牌号: ");
    safeGetInput(plate, sizeof(plate));

    if (strlen(plate) == 0) {
        printf("错误：车牌号不能为空！\n");
        return;
    }

    int index = findVehicle(plate);
    if (index == -1) {
        printf("错误：未找到车辆 %s 的记录！\n", plate);
        return;
    }

    if (records[index].isExited == 1) {
        printf("错误：车辆 %s 已出场！\n", plate);
        return;
    }

    // 计算费用
    time_t now = time(NULL);
    records[index].exitTime = now;
    records[index].fee = calculateFee(records[index].entryTime, now);
    records[index].isExited = 1;

    char exitStr[50];
    formatTime(now, exitStr, sizeof(exitStr));

    int hours = getParkingHours(records[index].entryTime, now);
    printf("车辆 %s 出库成功！\n", plate);
    printf("出场时间: %s\n", exitStr);
    printf("停车时长: %d 小时\n", hours);
    printf("停车费用: %.2f 元\n", records[index].fee);
}

// 查询车辆（仅未出场）
void queryVehicle() {
    char plate[MAX_PLATE];
    printf("请输入车牌号: ");
    safeGetInput(plate, sizeof(plate));

    if (strlen(plate) == 0) {
        printf("错误：车牌号不能为空！\n");
        return;
    }

    int index = findVehicle(plate);
    if (index == -1) {
        printf("错误：未找到车辆 %s 的记录！\n", plate);
        return;
    }

    if (records[index].isExited == 1) {
        printf("车辆 %s 已出场，无法查询停车状态\n", plate);
        return;
    }

    printf("\n当前停车信息：\n");
    displayRecord(&records[index]);
}

// 记录排序（按入场时间升序）
void sortRecords() {
    if (recordCount == 0) {
        printf("暂无停车记录\n");
        return;
    }

    // 冒泡排序
    for (int i = 0; i < recordCount - 1; i++) {
        for (int j = 0; j < recordCount - 1 - i; j++) {
            if (records[j].entryTime > records[j + 1].entryTime) {
                ParkingRecord temp = records[j];
                records[j] = records[j + 1];
                records[j + 1] = temp;
            }
        }
    }

    printf("\n按入场时间升序排序结果：\n");
    printf("----------------------------------------\n");
    for (int i = 0; i < recordCount; i++) {
        displayRecord(&records[i]);
    }
    printf("----------------------------------------\n");
}

// 费用汇总
void totalFees() {
    double total = 0;
    int exitedCount = 0;

    for (int i = 0; i < recordCount; i++) {
        if (records[i].isExited == 1) {
            total += records[i].fee;
            exitedCount++;
        }
    }

    printf("\n费用汇总：\n");
    printf("已出场车辆数: %d 辆\n", exitedCount);
    printf("总停车费用: %.2f 元\n", total);
}

// 僵尸车统计
void zombieCars() {
    time_t now = time(NULL);
    int zombieCount = 0;
    int days = 0;

    printf("\n僵尸车清单（停车时长 ≥ %d 天）：\n", ZOMBIE_DAYS);
    printf("----------------------------------------\n");

    for (int i = 0; i < recordCount; i++) {
        if (records[i].isExited == 0) {
            days = (int)(difftime(now, records[i].entryTime) / 86400);
            if (days >= ZOMBIE_DAYS) {
                zombieCount++;
                char entryStr[50];
                formatTime(records[i].entryTime, entryStr, sizeof(entryStr));
                printf("车牌: %-10s 入场时间: %s  已停留: %d 天\n",
                    records[i].plate, entryStr, days);
            }
        }
    }

    if (zombieCount == 0) {
        printf("未发现僵尸车\n");
    }
    else {
        printf("----------------------------------------\n");
        printf("僵尸车总数: %d 辆\n", zombieCount);
    }
}