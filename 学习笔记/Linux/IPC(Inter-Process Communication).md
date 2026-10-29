## 1.管道
>管道是Linux最古老的进程间通信(IPC)方式,本质是内核中的一块缓冲区,通过两个文件描述符访问:一个只读端,一个只写端。*数据单向流动*:写端写入,读端读出。
```
写进程 ──write(fd[1])──▶ [ 内核管道缓冲区 ] ──read(fd[0])──▶ 读进程
```
#### 特点
- 半双工:数据只能单向流动,要双向通信需要建两条管道。
- 字节流:没有消息边界,读写都按字节处理。
- 先进先出:读出的数据会从管道中移除,不能 lseek。
- 有容量上限:Linux 默认 64KB(可用 fcntl(F_SETPIPE_SZ) 调整)。
- 自带同步:管道空时 read 阻塞,管道满时 write 阻塞。
- 管道分两种:匿名管道(pipe)和命名管道(FIFO)。
#### 匿名管道(pipe)
创建方式(函数签名)
```c
#include <unistd.h>
int pipe(int fd[2]);   // 成功返回0,失败返回-1 fd[0]是读端口，fd[1]是写端
```
##### 特点：
- 匿名管道没有名字,只能通过 fork 继承 fd 的方式在有亲缘关系的进程(父子、兄弟)之间使用。
##### 用例一：
```c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    if (pipe(fd) == -1) { perror("pipe"); return 1; }

    pid_t pid = fork();
    if (pid == 0) {
        // 子进程:读
        close(fd[1]);                     // 关闭不用的写端
        char buf[128];
        ssize_t n = read(fd[0], buf, sizeof(buf) - 1);
        buf[n] = '\0';
        printf("child received: %s\n", buf);
        close(fd[0]);
    } else {
        // 父进程:写
        close(fd[0]);                     // 关闭不用的读端
        const char *msg = "hello from parent";
        write(fd[1], msg, strlen(msg));
        close(fd[1]);
        wait(NULL);
    }
    return 0;
}
```
>关键点:必须在 fork 之前调用 pipe,这样父子才共享这对 fd;并且双方都要关掉自己不用的那一端,否则会导致读端永远等不到 EOF，因为如果有写端打开，且没有数据，调用read会一直阻塞等待
##### 用例二：实现 shell 的 ls | wc -l
dup2将后一个fd重定向到前一个fd
```c
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pipe(fd);

    if (fork() == 0) {
        // 子进程1:执行 ls,输出到管道
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);   // stdout → 管道写端
        close(fd[1]);
        execlp("ls", "ls", NULL);
        _exit(1);
    }

    if (fork() == 0) {
        // 子进程2:执行 wc -l,从管道读
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);    // stdin ← 管道读端
        close(fd[0]);
        execlp("wc", "wc", "-l", NULL);
        _exit(1);
    }

    // 父进程必须关闭两端,否则 wc 收不到 EOF 会一直阻塞
    close(fd[0]);
    close(fd[1]);
    wait(NULL);
    wait(NULL);
    return 0;
}
```
>假设fd[0] = 3 fd[1] = 4
>dup2(4,1)执行之后，
>1 -> 匿名管道写端      4 -> 匿名管道写端
>根据上面提到，管道写端通常只保留一个，需要关掉多余的管道写端fd
##### 用例三：双向管道
```c
int p2c[2], c2p[2];     // parent→child, child→parent
pipe(p2c);
pipe(c2p);

if (fork() == 0) {
    close(p2c[1]);      // 子进程:从 p2c 读,往 c2p 写
    close(c2p[0]);
    // ... read(p2c[0]) / write(c2p[1]) ...
} else {
    close(p2c[0]);      // 父进程:往 p2c 写,从 c2p 读
    close(c2p[1]);
    // ... write(p2c[1]) / read(c2p[0]) ...
}
```
#### 命名管道(fifo)
>匿名管道的限制是只能用于亲缘进程。命名管道在文件系统中有一个路径名(类型为 p 的特殊文件),因此任意无亲缘关系的进程都可以通过打开这个路径来通信。
- 接口签名
```c
#include <sys/stat.h>
int mkfifo(const char *path, mode_t mode);   
/*
只读方式 open 会阻塞,直到有进程以写方式打开(反之亦然)。
加 O_NONBLOCK 可避免阻塞。
打开之后,读写规则和匿名管道完全一致。
*/
```
##### 用例一：独立进程通信
>写进程
```c
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    mkfifo("/tmp/myfifo", 0666);              // 已存在会返回错误,可忽略
    int fd = open("/tmp/myfifo", O_WRONLY);   // 阻塞直到有读者
    const char *msg = "hello via fifo";
    write(fd, msg, strlen(msg));
    close(fd);
    return 0;
}
```
>读进程
```c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("/tmp/myfifo", O_RDONLY);   // 阻塞直到有写者
    char buf[128];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    buf[n] = '\0';
    printf("received: %s\n", buf);
    close(fd);
    unlink("/tmp/myfifo");                    // 用完删除
    return 0;
}
```
>常见的坑:
- 忘记关闭不用的端:读进程的 read 永远不返回 0,程序卡死。这是最常见的 bug。
- SIGPIPE:读端全关了还在写,进程被信号杀死。需要时用 signal(SIGPIPE, SIG_IGN) 忽略,此时 write 会返回 -1 并设置 errno = EPIPE。
- 把管道当消息队列用:管道是字节流,没有边界。如果需要分隔消息,要自己设计协议(定长、长度前缀或分隔符)。
- 多个写者大块写入:超过 PIPE_BUF 的写入不保证原子,可能交错。
- 通信速度:数据要经过"用户 → 内核 → 用户"两次拷贝,对大数据量不如共享内存高效。
## 2.信号
>信号是一种软件层面的异步通知机制,用来通知进程"某件事发生了"。它是 IPC 中唯一的异步通信方式,信息量极小:只携带"是哪个信号"这一个编号,不携带数据(实时信号可附带少量数据)。
#### 信号的来源
- 键盘:Ctrl+C(SIGINT)、Ctrl+\(SIGQUIT)、Ctrl+Z(SIGTSTP)
- 硬件异常:非法内存访问(SIGSEGV)、除零(SIGFPE)
- 系统调用/命令:kill、raise、abort、alarm
- 内核事件:子进程退出(SIGCHLD)、管道破裂(SIGPIPE)、定时器到期(SIGALRM)
#### 支持的信号
*可以通过kill -l命令查看支持的信号*
#### 信号的三种处理方式
>进程收到信号后,可以:
- 执行默认动作(终止、忽略、暂停、产生 core dump 等)
- 忽略信号(SIG_IGN)
- 捕获信号:执行自定义的信号处理函数
#### 信号的发送
```c
#include <signal.h>
int kill(pid_t pid, int sig);   // 向指定进程发信号
/*
kill 的 pid 参数:
> 0:发给该进程
== 0:发给同进程组的所有进程
== -1:发给所有有权限的进程
< -1:发给进程组 ID 为 |pid| 的所有进程
*/
int raise(int sig);             // 向自己发信号
unsigned int alarm(unsigned int seconds);  // 定时发 SIGALRM
void abort(void);               // 向自己发 SIGABRT
```
#### 捕获信号：signal与sigaction
>signal较为简单
```c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void handler(int sig) {
    // 注意:这里只应调用异步信号安全函数,printf 其实不安全,示例仅为演示
    printf("caught signal %d\n", sig);
}
/*
signal 在不同系统上语义略有差异(是否自动重置处理函数、是否重启被中断的系统调用),可移植性差。
*/
int main() {
    signal(SIGINT, handler);   // 捕获 Ctrl+C
    while (1) {
        printf("running...\n");
        sleep(1);
    }
}
```
>sigaction更复杂健全
```c
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void handler(int sig) {
    const char msg[] = "got SIGINT\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);   // write 是异步信号安全的
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);     // 处理期间额外阻塞的信号集
    sa.sa_flags = SA_RESTART;     // 被中断的系统调用自动重启
    sigaction(SIGINT, &sa, NULL);

    while (1) pause();            // 挂起等待信号
}
```
#### 信号的生命周期:产生、未决、递达
```
产生 ──▶ 未决(pending) ──▶ 递达(delivery)并处理
              ▲
              │ 若被阻塞(blocked),则一直停在未决状态
```
- 未决:信号已产生但还没处理。
- 阻塞:进程可以通过信号屏蔽字(mask) 暂时阻塞某些信号,被阻塞的信号保持未决,直到解除阻塞。
- 递达:真正执行处理动作。
>注意:普通信号(1~31)不排队,同一信号在未决期间多次产生只记一次;实时信号(34~64)才会排队。
#### 信号处理函数的注意事项
- 只能调用异步信号安全函数:如 write、\_exit、signal、kill 等。printf、malloc、free 都不安全(它们内部用锁或全局状态,可能在被打断时死锁或破坏数据)。
- 尽量简单:常见做法是在处理函数里只设置一个 volatile sig_atomic_t 标志,主循环里检查该标志再做实际工作。
```c
   volatile sig_atomic_t quit = 0;
   void handler(int sig) { quit = 1; }
   // main: while (!quit) { ... }
```
- 保存并恢复 errno:处理函数可能打断主流程中刚设置 errno 的位置。
- 被中断的系统调用:慢速系统调用(read、accept 等)被信号打断会返回 -1 且 errno == EINTR,需要重试或使用 SA_RESTART。
- fork/exec 的继承:fork 后子进程继承信号处理方式和屏蔽字;exec 后被捕获的信号会重置为默认,被忽略的保持忽略。
## 3.信号量(semaphore)
>信号量(semaphore)是一个受内核保护的计数器,用于进程/线程间的同步与互斥,而不是传输数据。它控制的是"有多少个资源可用"。

*两个核心原子操作*:
- P 操作(wait / down):计数器减 1。若减之前已经是 0,则阻塞等待。
- V 操作(post / up):计数器加 1,并唤醒一个等待者。
*基本用法*

| 类型         | 初值  | 用途                    |
| ---------- | --- | --------------------- |
| 二值信号量(互斥锁) | 1   | 保护临界区进程，同一时刻仅允许一个进程访问 |
| 计数信号量      | N   | 管理N个同类资源/限制并发数量       |
| 同步信号量      | 0   | 一方等待另一方，同步执行顺序        |
#### 信号量三套api
System V信号量\<sys/sem.h\> 复杂度较高，接口老旧
POSIX有名信号量和POSIX无名信号量\<semaphore.h\> 复杂度低，单次控制一个信号量
## 4.消息队列

#### 什么是消息队列

消息队列是内核中维护的一个**消息链表**,进程可以向其中**发送消息**,也可以从中**接收消息**。和管道的最大区别:

- 管道是**字节流**,没有边界;消息队列传递的是**一条条独立的消息**,有明确边界。
- 消息可以带**类型/优先级**,接收方可以按类型挑选,不一定要先进先出。
- 发送方和接收方**不必同时存在**:消息留在队列里,直到被读走或队列被删除。
- 内核自动做同步:队列满时发送阻塞,队列空时接收阻塞。

```
进程A ──msgsnd──▶ [ msg1 | msg2 | msg3 ... ] ──msgrcv──▶ 进程B
                      内核中的消息队列
```

#### Linux 中的两套消息队列

|                  | System V 消息队列         | POSIX 消息队列               |
| ---------------- | --------------------- | ------------------------ |
| 头文件              | `<sys/msg.h>`         | `<mqueue.h>`             |
| 创建/打开            | `msgget`              | `mq_open`                |
| 发送               | `msgsnd`              | `mq_send`                |
| 接收               | `msgrcv`              | `mq_receive`             |
| 控制/删除            | `msgctl`              | `mq_close` / `mq_unlink` |
| 标识               | key(`ftok` 生成)→ msqid | 路径名(如 `/myq`)            |
| 筛选方式             | 按**消息类型 mtype**       | 按**优先级**(高优先级先出)         |
| 异步通知             | 不支持                   | 支持(`mq_notify`,信号或线程回调)  |
| 能否用 select/epoll | 不能                    | 能(队列描述符本身是 fd)           |

新代码优先考虑 POSIX 消息队列,接口更简洁,也更容易与 `epoll` 结合。System V 在老项目里很常见。

#### System V 消息队列

##### 接口

```c
#include <sys/msg.h>

int msgget(key_t key, int msgflg);
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
int msgctl(int msqid, int cmd, struct msqid_ds *buf);
```

消息结构必须以一个 `long` 类型的 `mtype` 开头:

```c
struct msgbuf {
    long mtype;        // 消息类型,必须 > 0
    char mtext[128];   // 消息数据,可以是任意结构
};
```

注意:`msgsz` 是 **mtext 部分的长度**,不包含 `mtype` 的 8 字节。

##### msgrcv 的 msgtyp 参数

| msgtyp | 含义 |
|---|---|
| `0` | 读队列中第一条消息(先进先出) |
| `> 0` | 读第一条类型等于 msgtyp 的消息 |
| `< 0` | 读类型 ≤ |msgtyp| 中**类型值最小**的第一条 |

##### 用例 1:两个进程收发消息

**发送端 sender.c**

```c
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msgbuf {
    long mtype;
    char mtext[128];
};

int main() {
    key_t key = ftok("/tmp", 'q');
    int msqid = msgget(key, IPC_CREAT | 0666);
    if (msqid == -1) { perror("msgget"); return 1; }

    struct msgbuf msg;
    msg.mtype = 1;
    strcpy(msg.mtext, "hello message queue");
    msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0);

    printf("sent: %s\n", msg.mtext);
    return 0;
}
```

**接收端 receiver.c**

```c
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msgbuf {
    long mtype;
    char mtext[128];
};

int main() {
    key_t key = ftok("/tmp", 'q');
    int msqid = msgget(key, IPC_CREAT | 0666);

    struct msgbuf msg;
    msgrcv(msqid, &msg, sizeof(msg.mtext), 1, 0);   // 只收类型 1,阻塞等待
    printf("received: %s\n", msg.mtext);

    msgctl(msqid, IPC_RMID, NULL);                  // 用完删除队列
    return 0;
}
```

两个进程不需要亲缘关系,只要用同一个 `ftok` 参数得到同一个 key 即可。

##### 用例 2:用 mtype 实现双向通信

一个队列里,用不同的 `mtype` 区分方向或接收者,避免自己读到自己发的消息:

```c
// 客户端发给服务器:mtype = 1
// 服务器回复客户端:mtype = 客户端的 pid

// 客户端
msg.mtype = 1;
sprintf(msg.mtext, "%d:request", getpid());
msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0);
msgrcv(msqid, &msg, sizeof(msg.mtext), getpid(), 0);   // 只收发给自己的回复

// 服务器
msgrcv(msqid, &msg, sizeof(msg.mtext), 1, 0);          // 收所有请求
// 解析出客户端 pid 后:
msg.mtype = client_pid;
msgsnd(msqid, &msg, len, 0);
```

这是 System V 消息队列最典型的用法:**一个队列服务多个客户端**。

##### 常用 flag

- `IPC_NOWAIT`:`msgsnd`/`msgrcv` 不阻塞,失败返回 -1,`errno` 为 `EAGAIN`(收)或队列满。
- `MSG_NOERROR`:接收时消息比缓冲区大则自动截断,否则 `msgrcv` 报 `E2BIG` 错误。

##### 命令行管理

```bash
ipcs -q              # 查看消息队列
ipcrm -q <msqid>     # 删除
```

System V 队列**不会随进程结束自动消失**(随内核存在,直到被删除或系统重启),所以忘记 `IPC_RMID` 会残留。

#### POSIX 消息队列

##### 接口

```c
#include <mqueue.h>

mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
int mq_send(mqd_t mqdes, const char *msg, size_t len, unsigned int prio);
ssize_t mq_receive(mqd_t mqdes, char *msg, size_t len, unsigned int *prio);
int mq_close(mqd_t mqdes);
int mq_unlink(const char *name);
int mq_getattr(mqd_t mqdes, struct mq_attr *attr);
int mq_notify(mqd_t mqdes, const struct sigevent *sevp);
```

编译需要链接实时库:`gcc a.c -o a -lrt`

队列属性:

```c
struct mq_attr {
    long mq_flags;     // 0 或 O_NONBLOCK
    long mq_maxmsg;    // 队列最多容纳多少条消息
    long mq_msgsize;   // 每条消息最大字节数
    long mq_curmsgs;   // 当前消息数(只读)
};
```

##### 用例:带优先级的收发

**发送端**

```c
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>

int main() {
    struct mq_attr attr = { .mq_flags = 0, .mq_maxmsg = 10, .mq_msgsize = 128 };
    mqd_t mq = mq_open("/demo_mq", O_CREAT | O_WRONLY, 0644, &attr);
    if (mq == (mqd_t)-1) { perror("mq_open"); return 1; }

    mq_send(mq, "low priority",  strlen("low priority") + 1,  1);
    mq_send(mq, "high priority", strlen("high priority") + 1, 9);

    mq_close(mq);
    return 0;
}
```

**接收端**

```c
#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>

int main() {
    mqd_t mq = mq_open("/demo_mq", O_RDONLY);
    char buf[128];                       // 缓冲区必须 >= mq_msgsize
    unsigned int prio;

    for (int i = 0; i < 2; i++) {
        ssize_t n = mq_receive(mq, buf, sizeof(buf), &prio);
        if (n >= 0) printf("received (prio %u): %s\n", prio, buf);
    }

    mq_close(mq);
    mq_unlink("/demo_mq");
    return 0;
}
```

输出顺序是**先 high priority,后 low priority**,即使发送顺序相反。

> 注意:`mq_receive` 的缓冲区长度必须**不小于** `mq_msgsize`,否则返回 `EMSGSIZE` 错误。

##### mq_notify:异步通知

接收方不必阻塞等待,可以让内核在队列由空变非空时通知自己:

```c
struct sigevent sev;
sev.sigev_notify = SIGEV_SIGNAL;
sev.sigev_signo  = SIGUSR1;
mq_notify(mq, &sev);   // 注册一次性通知,触发后需要重新注册
```

注意只有队列**从空变为非空**时才触发,且是一次性的,并且同一时刻只能有一个进程注册。

### 查看

POSIX 消息队列可挂载到文件系统查看:

```bash
mkdir /dev/mqueue
mount -t mqueue none /dev/mqueue
ls /dev/mqueue
```

#### 消息队列 vs 管道

| | 管道 | 消息队列 |
|---|---|---|
| 数据形式 | 字节流 | 带边界的消息 |
| 读取顺序 | 严格 FIFO | 可按类型/优先级选择 |
| 亲缘关系 | 匿名管道需要 | 不需要 |
| 生命周期 | 随进程 | 随内核(需主动删除) |
| 发送方接收方是否需同时存在 | 通常需要 | 不需要 |
| 拷贝次数 | 2 次 | 2 次 |

#### 限制与常见坑

1. **有容量限制**:System V 受 `msgmax`(单条上限)、`msgmnb`(队列总字节)、`msgmni`(队列个数)限制,可查看 `/proc/sys/kernel/msgmax` 等;POSIX 受 `/proc/sys/fs/mqueue/` 下参数和 `ulimit -q` 限制。
2. **残留问题**:忘记 `msgctl(IPC_RMID)` 或 `mq_unlink`,队列一直占用内核资源,下次运行可能读到旧消息。
3. **结构体写错**:System V 的 `msgsz` 不含 `mtype`,常见错误是把 `sizeof(msg)` 传进去,导致越界或报错。
4. **`mtype` 必须大于 0**。
5. **不适合大数据量**:每条消息要在用户态与内核态间拷贝两次,大量或大块数据优先考虑共享内存。
6. **消息结构里不要放指针**:指针在另一个进程的地址空间里没有意义。
7. **System V 队列不是 fd**,不能用 `select`/`epoll` 监听,POSIX 的可以(Linux 特性)。

#### 小结

- 消息队列传递**结构化、带边界的消息**,支持按类型或优先级取消息,发送接收双方无需同时在线。
- System V:`msgget/msgsnd/msgrcv/msgctl`,用 `mtype` 筛选,适合"一个队列多客户端"。
- POSIX:`mq_open/mq_send/mq_receive`,按优先级出队,支持 `mq_notify` 和 `epoll`,推荐新代码使用。
- 记得清理队列,注意容量限制,大数据量改用共享内存。

接下来可以讲**共享内存**(速度最快的 IPC),或者把管道、信号、信号量、消息队列、共享内存、socket 做一个完整对比表。
## 5.共享内存(shared memory)
### 解释
>共享内存是让多个进程把同一块物理内存映射到各自的虚拟地址空间,之后进程直接像访问普通内存一样读写,不再经过内核拷贝。
- 特点
	- 速度最快的 IPC:管道、消息队列每次传数据要"用户→内核→用户"拷贝两次,共享内存映射建立后读写零拷贝,不需要*系统调用*。
	- 自身不提供同步:内核不管谁先读谁先写,两个进程同时读写会产生*数据竞争*,必须配合信号量、互斥锁、原子操作等同步机制。
	- 不一定要亲缘关系:只要双方能找到同一个标识(key 或路径名)。
	- 生命周期随内核:进程退出后共享内存仍在,需要主动删除。
同样也有system V和POSIX两套实现接口
### 命令行管理
```bash
ipcs -m              # 查看共享内存
ipcrm -m <shmid>     # 删除
```
## 6.套接字(socket)
>socket(套接字)是一套通用的通信接口,本来为网络通信设计,但同样可以用于本机进程间通信。它是前面几种 IPC 中唯一原生支持跨主机的,也是功能最全面的一种:
- 双向通信(全双工):一个 socket 同时可读可写,不像管道要建两条。
- 有同步机制:读写自带阻塞,还能用 select/poll/epoll 多路复用,一个进程同时处理成千上万个连接。
- 可以不需要亲缘关系:通过地址(路径或 IP:端口)找到对方。
- 本机和跨机用同一套代码:只是地址类型不同。
>socket 在内核里表现为一个文件描述符,所以可以用read/write/close 操作,也可以用专用的 send/recv。

| 类型          | 地址族              | 地址形式  | 范围  | 特点          |
| ----------- | ---------------- | ----- | --- | ----------- |
| UNIX域socket | AF_UNIX          | 文件路径  | 本机  | 不经过网络协议栈，最快 |
| TCP socket  | AF_INET/AF_INFT6 | IP+端口 | 通用  | 可靠，面向连接，字节流 |
| UDP socket  | AF_INET/AF_INFT6 | IP+端口 | 通用  | 不可靠，无连接，数据报 |
#### 用例一：UNIX域socket通信
server.c
```c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/tmp/demo.sock"

int main() {
    int lfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (lfd == -1) { perror("socket"); return 1; }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCK_PATH);                       // 路径已存在会 bind 失败,先删除
    if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind"); return 1;
    }
    listen(lfd, 5);

    int cfd = accept(lfd, NULL, NULL);       // 阻塞等待客户端
    char buf[128];
    ssize_t n;
    while ((n = read(cfd, buf, sizeof(buf))) > 0) {
        write(cfd, buf, n);                  // 原样回显
    }

    close(cfd);
    close(lfd);
    unlink(SOCK_PATH);
    return 0;
}
```
>可以看到与网络通信的socket使用方法基本一致，只有地址和地址结构体等少量不同
### 总结
- socket 是最通用的 IPC:全双工、自带同步、可配合 epoll 做高并发,唯一支持跨主机。
- 本机通信用 Unix 域 socket(AF_UNIX),比回环 TCP 更快,还能传 fd、验证对端身份、用文件权限做访问控制。
- 流式 socket 没有消息边界,必须自己设计应用层协议来分包。
- 注意 unlink 残留文件、SIGPIPE、EINTR、fd 泄漏等常见问题。