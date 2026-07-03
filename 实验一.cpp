#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// 函数声明
int GenRandomInRange(int minVal, int maxVal);
void ConductSurveySampling(int TotalPopulationSize, int SampleSize);

// 1. 生成指定区间内的随机整数 [minVal, maxVal]
int GenRandomInRange(int minVal, int maxVal) {
    // 确保 minVal < maxVal
    if (minVal > maxVal) {
        int temp = minVal;
        minVal = maxVal;
        maxVal = temp;
    }
    // 区间长度: maxVal - minVal + 1
    return rand() % (maxVal - minVal + 1) + minVal;
}

// 2. 通用抽样器：从 1 ~ TotalPopulationSize 中不重复抽取 SampleSize 个ID
void ConductSurveySampling(int TotalPopulationSize, int SampleSize) {
    // 参数合法性检查
    if (TotalPopulationSize <= 0 || SampleSize <= 0) {
        printf("错误：总人数和样本量必须为正整数！\n");
        return;
    }
    if (SampleSize > TotalPopulationSize) {
        printf("错误：样本量不能超过总人数！\n");
        return;
    }

    // 动态分配标记数组，记录哪些ID已被抽中 (使用bool类型)
    bool* isSelected = (bool*)calloc(TotalPopulationSize + 1, sizeof(bool));
    if (isSelected == NULL) {
        printf("内存分配失败！\n");
        return;
    }

    int* samples = (int*)malloc(SampleSize * sizeof(int));
    if (samples == NULL) {
        printf("内存分配失败！\n");
        free(isSelected);
        return;
    }

    int count = 0;
    while (count < SampleSize) {
        int candidate = GenRandomInRange(1, TotalPopulationSize);
        if (!isSelected[candidate]) {
            isSelected[candidate] = true;
            samples[count] = candidate;
            count++;
        }
    }

    // 输出抽中的样本ID（按抽中顺序）
    printf("\n--- 抽中的样本ID（按抽取顺序） ---\n");
    for (int i = 0; i < SampleSize; i++) {
        printf("%d ", samples[i]);
        if ((i + 1) % 10 == 0) printf("\n");
    }
    printf("\n");

    // 进阶功能：模拟满意度调查 (预设满意率为60%)
    // 对每个被抽中的用户，再次利用随机数判断是否满意
    int satisfiedCount = 0;
    // 设定满意阈值为 60%，即随机数 1~100 中 <=60 表示满意
    const int SATISFY_THRESHOLD = 60;

    for (int i = 0; i < SampleSize; i++) {
        int satisfaction = GenRandomInRange(1, 100);
        if (satisfaction <= SATISFY_THRESHOLD) {
            satisfiedCount++;
        }
    }

    double satisfactionRate = (double)satisfiedCount / SampleSize * 100.0;
    printf("--- 满意度分析（预设满意概率60%%，基于随机模拟） ---\n");
    printf("样本量: %d, 满意人数: %d, 满意率: %.2f%%\n",
        SampleSize, satisfiedCount, satisfactionRate);

    // 释放动态内存
    free(isSelected);
    free(samples);
}

int main() {
    // 设置随机种子，使每次运行产生不同的随机序列
    srand((unsigned int)time(NULL));

    printf("===== 实验一：抽样问卷调查系统 =====\n\n");

    // 1. 基础随机函数与大规模分布统计
    printf("--- 1. 随机数分布统计 (10000次, 范围1~100) ---\n");
    const int TOTAL_TESTS = 10000;
    const int MIN_VAL = 1;
    const int MAX_VAL = 100;
    int frequency[MAX_VAL + 1] = { 0 }; // 下标0不使用，1~100计数

    for (int i = 0; i < TOTAL_TESTS; i++) {
        int num = GenRandomInRange(MIN_VAL, MAX_VAL);
        frequency[num]++;
    }

    // 打印统计结果（为了简洁，这里只打印每个数字的出现次数，可观察分布）
    // 实际中可粗略观察均匀性，由于输出太长，此处采用分段汇总方式展示
    printf("各数字出现次数（每10个数字汇总一次，便于观察均匀性）：\n");
    for (int i = 1; i <= MAX_VAL; i += 10) {
        int sum = 0;
        for (int j = i; j < i + 10 && j <= MAX_VAL; j++) {
            sum += frequency[j];
        }
        printf("  %2d ~ %2d : %d 次\n", i, (i + 9 > MAX_VAL ? MAX_VAL : i + 9), sum);
    }
    // 也可打印每个数字的详细计数，但会很长，所以选择汇总方式
    // 若希望详细，可取消注释下方循环
    /*
    printf("\n每个数字的详细计数：\n");
    for (int i = 1; i <= MAX_VAL; i++) {
        printf("%d:%d ", i, frequency[i]);
        if (i % 10 == 0) printf("\n");
    }
    */
    printf("\n");

    // 2. 模拟特定人群的“小比例抽样” (总人数10000, 样本50)
    printf("--- 2. 小比例抽样 (总人数10000, 样本量50) ---\n");
    ConductSurveySampling(10000, 50);
    printf("\n");

    // 3. 通用抽样器，从键盘接收参数
    printf("--- 3. 通用抽样器 (自定义总人数和样本量) ---\n");
    int population, sample;
    printf("请输入总人数 N: ");
    scanf_s("%d", &population);
    printf("请输入样本量 n: ");
    scanf_s("%d", &sample);

    ConductSurveySampling(population, sample);

    return 0;
}