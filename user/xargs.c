#include "kernel/types.h"
#include "kernel/param.h" // 包含 MAXARG 定义
#include "user/user.h"

#define MAX_LINE_LEN 512 // 每行输入的最大长度

int main(int argc, char *argv[])
{
    char *cmd_args[MAXARG];      // 存储最终执行命令的参数列表
    char line_buf[MAX_LINE_LEN]; // 缓存从标准输入读取的一行数据
    int i, n;
    int line_len = 0; // 当前行缓冲区已填充的长度

    // 复制 xargs 自身的命令行参数到 cmd_args。
    // argv[0] 是 "xargs"，argv[1] 是实际要执行的命令名。
    // cmd_args[0] 将是命令名，cmd_args[1]... 是其固定参数。
    for (i = 1; i < argc; i++)
    {
        cmd_args[i - 1] = argv[i];
    }
    // cmd_args[argc - 1] 将是为 stdin 行数据保留的位置
    while ((n = read(0, &line_buf[line_len], 1)) > 0)
    {
        if (line_buf[line_len] == '\n')
        {
            line_buf[line_len] = 0; // 终止当前行字符串

            // 将当前行作为额外参数追加到命令参数列表
            cmd_args[argc - 1] = line_buf;
            cmd_args[argc] = 0; // 参数列表必须以 NULL 结尾

            // 创建子进程并执行命令
            if (fork() == 0)
            {
                exec(cmd_args[0], cmd_args);
                fprintf(2, "xargs: exec 失败\n"); // exec 失败说明命令无法执行
                exit(1);
            }
            else
            {
                wait(0); // 父进程等待子进程完成
            }
            line_len = 0; // 重置缓冲区，准备读取下一行
        }
        else
        {
            line_len++;
            if (line_len >= MAX_LINE_LEN)
            {
                // 行太长，超出缓冲区限制
                fprintf(2, "xargs: 行过长\n");
                exit(1);
            }
        }
    }
    // 处理最后一行可能没有换行符的情况
    if (line_len > 0)
    {
        line_buf[line_len] = 0;
        cmd_args[argc - 1] = line_buf;
        cmd_args[argc] = 0;

        if (fork() == 0)
        {
            exec(cmd_args[0], cmd_args);
            fprintf(2, "xargs: exec 失败\n");
            exit(1);
        }
        else
        {
            wait(0);
        }
    }
    exit(0); 
}