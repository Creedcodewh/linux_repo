#include "head.h"

// 初始化客户端队列函数
int init_clientQueue(clientQueue_t *pclientQueue)
{
    // 将客户端队列的计时器初始化为0
    pclientQueue->timer = 0;
    // 使用-1填充客户端数组
    memset(pclientQueue->client, -1, sizeof(pclientQueue->client));
    // 使用0填充索引数组
    memset(pclientQueue->index, 0, sizeof(pclientQueue->index));
    // 循环遍历时间片数组，初始化每个时间片的头尾指针和大小
    for (int i = 0; i < TIME_SLICE; ++i)
    {
        pclientQueue->time_out[i].head = NULL;
        pclientQueue->time_out[i].tail = NULL;
        pclientQueue->time_out[i].size = 0;
    }
    return 0;
}

// 将网络文件描述符添加到客户端队列的函数
int fdAdd(int netFd, clientQueue_t *pclientQueue)
{
    // 为新的节点分配内存
    slotNode_t *newSlot = (slotNode_t *)calloc(1, sizeof(slotNode_t));
    // 检查内存分配是否成功
    if (newSlot == NULL)
    {
        return -1;
    }
    // 设置新节点的网络文件描述符
    newSlot->netFd = netFd;
    // 根据时间片计算索引
    pclientQueue->index[netFd] = (pclientQueue->timer + TIME_SLICE - 1) % TIME_SLICE;
    int index = (pclientQueue->timer + TIME_SLICE - 1) % TIME_SLICE;
    // 增加对应时间片的大小
    ++pclientQueue->time_out[index].size;
    // 尾插法，将新节点加入时间片的链表中
    if (pclientQueue->time_out[index].head == NULL)
    {
        pclientQueue->time_out[index].head = newSlot;
        pclientQueue->time_out[index].tail = newSlot;
    }
    else
    {
        pclientQueue->time_out[index].tail->next = newSlot;
        pclientQueue->time_out[index].tail = newSlot;
    }
    // 返回0表示添加成功
    return 0;
}

// 从客户端队列中删除指定的网络文件描述符
int fdDel(int netFd, clientQueue_t *pclientQueue)
{
    // 获取要删除的网络文件描述符的时间片索引
    int index = pclientQueue->index[netFd];
    // 重置索引为0
    pclientQueue->index[netFd] = 0;
    // 从对应时间片的头节点开始遍历
    slotNode_t *cur = pclientQueue->time_out[index].head;
    slotNode_t *pre = cur;
    // 检查头节点是否为空
    if (cur == NULL)
    {
        return 0;
    }
    // 如果头节点是要删除的节点
    if (cur->netFd == netFd)
    {
        // 将头节点指向下一个节点
        pclientQueue->time_out[index].head = cur->next;
        // 释放要删除的节点
        free(cur);
        // 减少对应时间片的大小
        --pclientQueue->time_out[index].size;
    }
    else
    {
        // 如果不是头节点，则遍历链表找到要删除的节点
        while (cur != NULL)
        {
            if (cur->netFd == netFd)
            {
                // 将前一个节点的next指针指向当前节点的下一个节点
                pre->next = cur->next;
                // 释放当前节点
                free(cur);
                // 减少对应时间片的大小
                --pclientQueue->time_out[index].size;
                break;
            }
            pre = cur;
            cur = cur->next;
        }
    }
    // 返回0表示删除成功
    return 0;
}

