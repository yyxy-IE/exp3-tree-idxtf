/**
 * 实验：目录树查看器（仿 Linux tree 命令）
 * 学号：__________  姓名：__________
 * 说明：请补全所有标记为 TODO 的函数体，不要修改其他代码。
 * 目录树查看器（仿 Linux tree 命令）
 * 完整实现版本（C语言，左孩子右兄弟二叉树）
 * 编译：gcc -o tree tree.c -std=c99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// ================== 二叉树结点定义 ==================
typedef struct FileNode {
    char *name;                  // 文件/目录名
    int isDir;                   // 1:目录 0:文件
    struct FileNode *firstChild; // 左孩子：第一个子项
    struct FileNode *nextSibling;// 右兄弟：下一个同层项
} FileNode;

// ================== 函数声明 ==================
FileNode* createNode(const char *name, int isDir);
int cmpNode(const void *a, const void *b);
FileNode* buildTree(const char *path);
void printTree(FileNode *node, const char *prefix, int isLast);
int countNodes(FileNode *root);
int countLeaves(FileNode *root);
int treeHeight(FileNode *root);
void countDirFile(FileNode *root, int *dirs, int *files);
void freeTree(FileNode *root);
char* getBaseName(void);

// ================== 需要补全的函数 ==================

// 创建新结点（分配内存、复制字符串、初始化指针）
FileNode* createNode(const char *name, int isDir) {
    // TODO: 实现
    FileNode *node = (FileNode*)malloc(sizeof(FileNode));
    if (!node) return NULL;
    node->name = (char*)malloc(strlen(name) + 1);
    strcpy(node->name, name);
    node->isDir = isDir;
    node->firstChild = NULL;
    node->nextSibling = NULL;
    return node;
}

// 比较函数，用于 qsort 对子项按名称排序
int cmpNode(const void *a, const void *b) {
    // TODO: 实现
    FileNode *na = *(FileNode**)a;
    FileNode *nb = *(FileNode**)b;
    return strcmp(na->name, nb->name);
}

// 递归构建目录树（核心难点）
FileNode* buildTree(const char *path) {
    // TODO: 实现
    // 步骤提示：
    // 1. opendir 打开目录，失败返回 NULL
    DIR *dir = opendir(path);
    if (!dir) return NULL;

    // 2. 从 path 提取目录名
    char *base = strrchr(path, '/');
    char nodeName[256];
    if (base == NULL) {
        strcpy(nodeName, path);
    } else {
        strcpy(nodeName, base + 1);
    }

    // 3. 创建当前目录结点
    FileNode *curNode = createNode(nodeName, 1);
    if (!curNode) {
        closedir(dir);
        return NULL;
    }

    // 临时存放子结点
    FileNode **childArr = NULL;
    int childCnt = 0;
    struct dirent *ent;

    // 4. 循环 readdir
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        // 5. 拼接完整路径
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, ent->d_name);

        // 6. 判断文件类型
        struct stat st;
        if (stat(fullPath, &st) < 0) continue;

        FileNode *child = NULL;
        if (S_ISDIR(st.st_mode)) {
            child = buildTree(fullPath);
        } else {
            child = createNode(ent->d_name, 0);
        }
        if (!child) continue;

        // 加入临时数组
        childArr = (FileNode**)realloc(childArr, (childCnt + 1) * sizeof(FileNode*));
        childArr[childCnt++] = child;
    }
    closedir(dir);

    // 9. 排序子结点
    if (childCnt > 0) {
        qsort(childArr, childCnt, sizeof(FileNode*), cmpNode);
    }

    // 10. 链接成兄弟链表
    if (childCnt > 0) {
        curNode->firstChild = childArr[0];
        for (int i = 0; i < childCnt - 1; i++) {
            childArr[i]->nextSibling = childArr[i+1];
        }
    }

    // 11. 释放临时数组
    free(childArr);
    return curNode;
}

// 树形输出（仿 tree 命令）
void printTree(FileNode *node, const char *prefix, int isLast) {
    // TODO: 实现
    // 步骤提示：
    // 1. 如果 node 为空，返回
    if (!node) return;

    // 2. 输出前缀、分支符号
    printf("%s", prefix);
    if (isLast) {
        printf("`-- ");
    } else {
        printf("|-- ");
    }
    printf("%s", node->name);
    if (node->isDir) printf("/");
    printf("\n");

    // 无孩子直接返回
    if (!node->firstChild) return;

    // 统计孩子总数
    int cnt = 0;
    FileNode *p = node->firstChild;
    while (p) { cnt++; p = p->nextSibling; }

    // 遍历递归打印
    p = node->firstChild;
    int idx = 0;
    while (p) {
        char newPrefix[1024] = {0};
        strcpy(newPrefix, prefix);
        if (isLast) {
            strcat(newPrefix, "    ");
        } else {
            strcat(newPrefix, "|   ");
        }
        int last = (++idx == cnt);
        printTree(p, newPrefix, last);
        p = p->nextSibling;
    }
}

// 统计二叉树结点总数
int countNodes(FileNode *root) {
    // TODO: 实现（递归）
    if (!root) return 0;
    int sum = 1;
    sum += countNodes(root->firstChild);
    sum += countNodes(root->nextSibling);
    return sum;
}


// 统计叶子结点数（firstChild == NULL 的结点）
int countLeaves(FileNode *root) {
    // TODO: 实现（递归）
    if (!root) return 0;
    int leaf = 0;
    if (root->firstChild == NULL) {
        leaf = 1;
    }
    leaf += countLeaves(root->firstChild);
    leaf += countLeaves(root->nextSibling);
    return leaf;
}

// 计算二叉树高度（根深度为1，空树高度为0）
int treeHeight(FileNode *root) {
    // TODO: 实现（递归）
    if (!root) return 0;
    int childH = treeHeight(root->firstChild);
    int sibH = treeHeight(root->nextSibling);
    return (childH + 1) > sibH ? (childH + 1) : sibH;
}

// 统计目录数和文件数（遍历整棵树）
void countDirFile(FileNode *root, int *dirs, int *files) {
    // TODO: 实现（递归）
    if (!root) return;
    if (root->isDir) {
        (*dirs)++;
    } else {
        (*files)++;
    }
    countDirFile(root->firstChild, dirs, files);
    countDirFile(root->nextSibling, dirs, files);
}

// 释放整棵树的内存
void freeTree(FileNode *root) {
    // TODO: 实现（递归释放左右子树，最后释放当前结点）
    if (!root) return;
    freeTree(root->firstChild);
    freeTree(root->nextSibling);
    free(root->name);
    free(root);
}

// 获取当前工作目录的“基本名称”（用于显示根结点名）
char* getBaseName(void) {
    // TODO: 实现
    // 提示：调用 getcwd(NULL,0) 获取绝对路径，提取最后一个 '/' 之后的部分
    // 注意释放 getcwd 分配的内存
    char *path = getcwd(NULL, 0);
    if (!path) return NULL;
    char *base = strrchr(path, '/');
    char *res;
    if (base == NULL) {
        res = (char*)malloc(strlen(path)+1);
        strcpy(res, path);
    } else {
        res = (char*)malloc(strlen(base+1)+1);
        strcpy(res, base+1);
    }
    free(path);
    return res;
}

int main(int argc, char *argv[]) {
    char targetPath[1024];
    if (argc >= 2) {
        strncpy(targetPath, argv[1], sizeof(targetPath)-1);
        targetPath[sizeof(targetPath)-1] = '\0';
    } else {
        if (getcwd(targetPath, sizeof(targetPath)) == NULL) {
            perror("getcwd");
            return 1;
        }
    }

    int len = strlen(targetPath);
    if (len > 0 && targetPath[len-1] == '/')
        targetPath[len-1] = '\0';

    struct stat st;
    if (stat(targetPath, &st) != 0) {
        perror("stat");
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "错误: %s 不是目录\n", targetPath);
        return 1;
    }

    FileNode *root = buildTree(targetPath);
    if (!root) {
        fprintf(stderr, "无法构建目录树\n");
        return 1;
    }

    // 输出根目录名
    char *displayName = NULL;
    if (argc >= 2) {
        displayName = root->name;
    } else {
        displayName = getBaseName();
    }
    printf("%s/\n", displayName);
    if (argc < 2) free(displayName);

    FileNode *child = root->firstChild;
    int childCount = 0;
    FileNode *tmp = child;
    while (tmp) { childCount++; tmp = tmp->nextSibling; }
    int idx = 0;
    while (child) {
        int isLast = (++idx == childCount);
        printTree(child, "", isLast);
        child = child->nextSibling;
    }

    int dirs = 0, files = 0;
    countDirFile(root, &dirs, &files);
    printf("\n%d 个目录, %d 个文件\n", dirs, files);
    printf("二叉树结点总数: %d\n", countNodes(root));
    printf("叶子结点数: %d\n", countLeaves(root));
    printf("树的高度: %d\n", treeHeight(root));

    freeTree(root);
    return 0;
}