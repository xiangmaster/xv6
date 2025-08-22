#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])  
{
    // 检查命令行参数的数量
    if (argc != 2) {
        fprintf(2, "Error! Need a param.\n"); // 打印错误信息到stderr
        exit(1); 
    }
    int ticks = atoi(argv[1]);  // 将命令行参数（字符串）转换为整数
    sleep(ticks);   // 调用sleep
    exit(0);
}