#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h" // 包含 struct dirent 和 DIRSIZ 定义

// 递归查找指定文件名的函数
// path: 当前搜索的目录路径
// target_name: 要查找的文件名
void find(char *path, char *target_name)
{
    char buf[512], *p;
    int fd;
    struct dirent de; // 目录项
    struct stat st;   // 文件状态
    // 打开当前路径
    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: 无法打开 %s\n", path);
        return;
    }
    // 获取文件状态
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: 无法获取 %s 的状态\n", path);
        close(fd);
        return;
    }
    if (st.type != T_DIR)
    {
        // 如果不是目录，检查是否是目标文件本身
        // 从路径中提取文件名部分进行比较
        char *file_name_ptr = path;
        for (char *s = path; *s; s++)
        {
            if (*s == '/')
            {
                file_name_ptr = s + 1;
            }
        }
        if (strcmp(file_name_ptr, target_name) == 0)
        {
            fprintf(1, "%s\n", path); // 匹配成功，打印路径
        }
        close(fd);
        return;
    }

    // 目录处理：准备缓冲区以拼接子路径 (path + '/' + de.name + '\0')
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf))
    {
        fprintf(2, "find: 路径过长\n");
        close(fd);
        return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    // 遍历目录项
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if (de.inum == 0)
            continue; // 跳过未使用的目录项

        // 避免 '.' 和 '..' 造成的无限递归
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;
        // 将目录项名称拼接到缓冲区，形成新的完整路径
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0; // 确保字符串以空字符结尾
        // 获取子路径的文件状态
        if (stat(buf, &st) < 0)
        {
            fprintf(2, "find: 无法获取 %s 的状态\n", buf);
            continue;
        }
        if (st.type == T_FILE)
        {
            // 如果是文件，比较其名称
            if (strcmp(de.name, target_name) == 0)
            {
                fprintf(1, "%s\n", buf); // 匹配成功，打印完整路径
            }
        }
        else if (st.type == T_DIR)
        {
            // 如果是子目录，递归调用 find
            find(buf, target_name);
        }
    }
    close(fd); // 关闭目录文件描述符
}

int main(int argc, char *argv[])
{
    // 检查参数数量：程序名、起始路径、目标文件名
    if (argc != 3)
    {
        fprintf(2, "用法: find <路径> <文件名>\n");
        exit(1);
    }

    // 调用查找函数
    find(argv[1], argv[2]);
    exit(0);
}