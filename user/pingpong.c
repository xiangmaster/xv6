#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc != 1)
    // 检查命令行参数个数，不为1则输出error
    {
        fprintf(2, "Error!\n");
        exit(1);
    }
    int p[2];   // 整型数组p存放管道的两个文件描述符
    pipe(p);    // 创建管道 p[0]表示读，p[1]表示写
    int pid = fork();

    // 子进程
    if (pid == 0)    
    {
        close(p[0]);    // 子进程需要写，使用关闭读端
        char tmp = 'x';
        if (write(p[1], &tmp, 1))
            fprintf(0,"%d: received ping\n", getpid()); 
            // 向标准输出打印消息，包含子进程PID
        close(p[1]);
    }

    // 父进程
    else    
    {
        wait((int *)0); // 等待子进程结束
        close(p[1]);
        char tmp;
        if (read(p[0], &tmp, 1))
            fprintf(0, "%d: received pong\n", getpid());
            // 向标准输出打印消息，包含父进程PID
        close(p[0]);
    }
}