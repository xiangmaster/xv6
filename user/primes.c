#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// 声明 primes 函数，并使用 __attribute__((noreturn)) 告诉编译器它不会正常返回
void primes(int left_pipe_read_fd) __attribute__((noreturn));

int main(int argc, char *argv[])
{
    int p[2]; // 用于根进程的管道：p[0] 是读取端，p[1] 是写入端
    int i;
    // 创建第一个管道，用于 main 函数向第一个 primes 进程发送数字
    if (pipe(p) < 0)
    {
        fprintf(2, "primes: main pipe failed\n");
        exit(1);
    }
    // 素数筛选
    if (fork() == 0)
    {
        // 子进程：它将从父进程的管道读取
        close(p[1]);  // 子进程不需要父进程的写入端，立即关闭
        primes(p[0]); // 将管道的读取端传递给 primes 函数
    }
    else
    {
        // 父进程: 负责生成的数字并写入管道
        close(p[0]); // 父进程不需要读取端，立即关闭
        for (i = 2; i <= 280; i++)
        { 
            if (write(p[1], &i, sizeof(i)) != sizeof(i))
            {
                fprintf(2, "primes: write to pipe failed\n");
                exit(1);
            }
        }
        close(p[1]); 
        wait(0); // 父进程等待子进程完成
        exit(0); // 父进程退出
    }
    exit(0); 
}
// primes 函数：处理素数筛选的递归函数
void primes(int left_pipe_read_fd)
{
    int prime;         // 当前进程找到的素数
    int n;             // 从左侧管道读取的数字
    int right_pipe[2]; // 用于当前进程向下一个 primes 进程发送数字的管道
    int pid;
    // 从左侧管道读取第一个数字，这将是当前进程要处理的素数
    if (read(left_pipe_read_fd, &prime, sizeof(prime)) == 0)
    {
        // 如果读取到0字节，左侧管道已关闭且没有更多数据
        close(left_pipe_read_fd);
        exit(0);
    }
    fprintf(1, "prime %d\n", prime); 
    // 为下一个 primes 进程创建管道
    if (pipe(right_pipe) < 0)
    {
        fprintf(2, "primes: child pipe failed\n");
        exit(1);
    }
    // 创建子进程来处理剩余的数字
    pid = fork();
    if (pid < 0)
    {
        fprintf(2, "primes: fork failed\n");
        exit(1);
    }

    if (pid == 0)
    {                             // 子进程 (下一个 primes 进程)
        close(left_pipe_read_fd); // 子进程不需要从其父进程的管道读取（已经通过 `primes(p[0])` 传递了新的 fd）
        close(right_pipe[1]);     // 子进程不需要向其自己的写入端写，它将从 `right_pipe[0]` 读取
        primes(right_pipe[0]); // 递归调用 primes，传递新的管道读取端
    }
    else
    {                         // 父进程 (当前 primes 进程)
        close(right_pipe[0]); // 父进程不需要从右侧管道读取
        // 从左侧管道读取剩余的数字，并筛选
        while (read(left_pipe_read_fd, &n, sizeof(n)) > 0)
        {
            if (n % prime != 0)
            { // 如果数字不是当前素数的倍数
                // 将这个数字写入到右侧管道，传递给下一个 primes 进程
                if (write(right_pipe[1], &n, sizeof(n)) != sizeof(n))
                {
                    fprintf(2, "primes: write to next pipe failed\n");
                    exit(1);
                }
            }
        }
        close(left_pipe_read_fd); // 所有数字都已从左侧管道读取完毕，关闭读取端
        close(right_pipe[1]);     // 所有筛选后的数字都已写入右侧管道，关闭写入端
        wait(0); 
        exit(0); 
    }
}