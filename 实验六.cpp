#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_CUSTOMERS 300
#define MAX_WINDOWS 5
#define OPEN_TIME 480   // 9:00 = 480分钟
#define CLOSE_TIME 1020 // 17:00 = 1020分钟

// 客户结构体
typedef struct {
    int id;
    int arrive_time;    // 到达时间（分钟，从9:00起）
    int service_time;   // 业务办理耗时（分钟）
    int start_time;     // 开始办理时间
    int wait_time;      // 等待时间
    int window_id;      // 服务窗口编号
} Customer;

// 窗口结构体
typedef struct {
    int id;
    int queue[MAX_CUSTOMERS];
    int front, rear;
    int total_served;
    int current_customer_id;
    int finish_time;
} Window;

// 全局变量
Customer customers[MAX_CUSTOMERS];
int customer_count = 0;
Window windows[MAX_WINDOWS];
int window_count;
int max_queue_length = 0;
int total_wait_time = 0;
int total_customers = 0;
long long simulation_time = 0;

// 函数声明
void init_windows(int n);
void generate_customers();
int is_empty(Window* w);
int is_full(Window* w);
void enqueue(Window* w, int customer_id);
int dequeue(Window* w);
int get_queue_length(Window* w);
void assign_customer_to_window(int customer_id);
void process_windows();
void update_statistics();
void write_log();
void print_summary();

int main() {
    srand((unsigned)time(NULL));

    printf("===== 银行排队调度模拟系统 =====\n");
    printf("营业时间: 9:00 - 17:00\n\n");

    // 设置窗口数量
    window_count = 3; // 可调整
    init_windows(window_count);

    // 生成客户数据
    generate_customers();
    printf("共生成 %d 位客户\n\n", customer_count);

    // 模拟仿真
    printf("开始仿真...\n");
    for (simulation_time = OPEN_TIME; simulation_time < CLOSE_TIME; simulation_time++) {
        // 检查是否有客户到达
        for (int i = 0; i < customer_count; i++) {
            if (customers[i].arrive_time == simulation_time) {
                assign_customer_to_window(i);
                total_customers++;
            }
        }

        // 处理各窗口业务
        process_windows();

        // 更新统计
        update_statistics();
    }

    // 处理剩余客户
    int remaining = 1;
    while (remaining) {
        remaining = 0;
        for (int i = 0; i < window_count; i++) {
            if (!is_empty(&windows[i])) {
                remaining = 1;
                process_windows();
                update_statistics();
                break;
            }
        }
        simulation_time++;
    }

    // 输出结果
    print_summary();
    write_log();

    printf("\n仿真完成！统计结果已写入日志文件。\n");
    return 0;
}

// 初始化窗口
void init_windows(int n) {
    window_count = n;
    for (int i = 0; i < n; i++) {
        windows[i].id = i + 1;
        windows[i].front = windows[i].rear = 0;
        windows[i].total_served = 0;
        windows[i].current_customer_id = -1;
        windows[i].finish_time = 0;
    }
}

// 生成随机客户数据
void generate_customers() {
    int time = OPEN_TIME;
    customer_count = 0;

    while (time < CLOSE_TIME && customer_count < MAX_CUSTOMERS) {
        // 随机到达间隔（1-8分钟）
        int interval = rand() % 8 + 1;
        time += interval;

        if (time >= CLOSE_TIME) break;

        customers[customer_count].id = customer_count + 1;
        customers[customer_count].arrive_time = time;
        // 业务时长 3-15 分钟
        customers[customer_count].service_time = rand() % 13 + 3;
        customers[customer_count].start_time = 0;
        customers[customer_count].wait_time = 0;
        customers[customer_count].window_id = -1;
        customer_count++;
    }
}

// 队列操作
int is_empty(Window* w) {
    return w->front == w->rear;
}

int is_full(Window* w) {
    return (w->rear + 1) % MAX_CUSTOMERS == w->front;
}

void enqueue(Window* w, int customer_id) {
    if (is_full(w)) {
        printf("警告: 窗口 %d 队列已满\n", w->id);
        return;
    }
    w->queue[w->rear] = customer_id;
    w->rear = (w->rear + 1) % MAX_CUSTOMERS;
}

int dequeue(Window* w) {
    if (is_empty(w)) return -1;
    int id = w->queue[w->front];
    w->front = (w->front + 1) % MAX_CUSTOMERS;
    return id;
}

int get_queue_length(Window* w) {
    return (w->rear - w->front + MAX_CUSTOMERS) % MAX_CUSTOMERS;
}

// 分配客户到最短队列
void assign_customer_to_window(int customer_id) {
    int min_len = MAX_CUSTOMERS;
    int target = 0;

    for (int i = 0; i < window_count; i++) {
        int len = get_queue_length(&windows[i]);
        // 如果窗口空闲且无队列，优先分配
        if (windows[i].current_customer_id == -1 && len == 0) {
            target = i;
            break;
        }
        if (len < min_len) {
            min_len = len;
            target = i;
        }
    }

    enqueue(&windows[target], customer_id);
    customers[customer_id].window_id = windows[target].id;

    // 如果窗口空闲，立即开始服务
    if (windows[target].current_customer_id == -1) {
        int id = dequeue(&windows[target]);
        if (id != -1) {
            windows[target].current_customer_id = id;
            customers[id].start_time = (int)simulation_time;
            customers[id].wait_time = (int)simulation_time - customers[id].arrive_time;
            windows[target].finish_time = (int)simulation_time + customers[id].service_time;
            windows[target].total_served++;

            printf("时间 %d: 客户 %d 进入窗口 %d 办理 (等待 %d 分钟)\n",
                (int)simulation_time, id + 1, windows[target].id, customers[id].wait_time);
        }
    }
}

// 处理窗口业务
void process_windows() {
    for (int i = 0; i < window_count; i++) {
        if (windows[i].current_customer_id != -1 &&
            simulation_time >= windows[i].finish_time) {

            int id = windows[i].current_customer_id;
            printf("时间 %d: 客户 %d 离开窗口 %d\n",
                (int)simulation_time, id + 1, windows[i].id);

            total_wait_time += customers[id].wait_time;

            // 从队列取下一个客户
            int next = dequeue(&windows[i]);
            if (next != -1) {
                windows[i].current_customer_id = next;
                customers[next].start_time = (int)simulation_time;
                customers[next].wait_time = (int)simulation_time - customers[next].arrive_time;
                windows[i].finish_time = (int)simulation_time + customers[next].service_time;
                windows[i].total_served++;

                printf("时间 %d: 客户 %d 进入窗口 %d 办理 (等待 %d 分钟)\n",
                    (int)simulation_time, next + 1, windows[i].id, customers[next].wait_time);
            }
            else {
                windows[i].current_customer_id = -1;
                windows[i].finish_time = 0;
            }
        }
    }
}

// 更新最大队列长度统计
void update_statistics() {
    int total_len = 0;
    for (int i = 0; i < window_count; i++) {
        total_len += get_queue_length(&windows[i]);
        // 如果当前窗口正在服务，也算在队列长度中
        if (windows[i].current_customer_id != -1) {
            total_len += 1;
        }
    }
    if (total_len > max_queue_length) {
        max_queue_length = total_len;
    }
}

// 写入日志文件（使用 fopen_s）
void write_log() {
    FILE* fp;
    errno_t err = fopen_s(&fp, "bank_simulation.log", "w");
    if (err != 0 || fp == NULL) {
        printf("无法创建日志文件！\n");
        return;
    }

    fprintf(fp, "===== 银行排队调度模拟日志 =====\n");
    fprintf(fp, "营业时间: 9:00 - 17:00\n");
    fprintf(fp, "窗口数量: %d\n", window_count);
    fprintf(fp, "客户总数: %d\n", total_customers);
    fprintf(fp, "平均等待时间: %.2f 分钟\n",
        total_customers > 0 ? (float)total_wait_time / total_customers : 0);
    fprintf(fp, "最大排队长度: %d\n", max_queue_length);
    fprintf(fp, "\n各窗口服务统计:\n");
    for (int i = 0; i < window_count; i++) {
        fprintf(fp, "窗口 %d: 服务 %d 位客户\n",
            windows[i].id, windows[i].total_served);
    }

    fprintf(fp, "\n详细客户记录:\n");
    fprintf(fp, "客户ID\t到达时间\t服务时长\t等待时间\t窗口ID\n");
    for (int i = 0; i < customer_count; i++) {
        if (customers[i].window_id != -1) {
            fprintf(fp, "%d\t%d\t\t%d\t\t%d\t\t%d\n",
                customers[i].id,
                customers[i].arrive_time,
                customers[i].service_time,
                customers[i].wait_time,
                customers[i].window_id);
        }
    }

    fclose(fp);
}

// 控制台输出汇总
void print_summary() {
    printf("\n===== 仿真统计结果 =====\n");
    printf("窗口数量: %d\n", window_count);
    printf("客户总数: %d\n", total_customers);
    printf("平均等待时间: %.2f 分钟\n",
        total_customers > 0 ? (float)total_wait_time / total_customers : 0);
    printf("最大排队长度: %d\n", max_queue_length);
    printf("\n各窗口服务统计:\n");
    for (int i = 0; i < window_count; i++) {
        printf("窗口 %d: 服务 %d 位客户\n",
            windows[i].id, windows[i].total_served);
    }
}