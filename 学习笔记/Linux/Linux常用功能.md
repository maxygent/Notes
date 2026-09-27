## 查看文件
### `less` 
- +n 行号   从行号开始
- +/s 字符串   从第一个匹配开始
### `head`
- -n 行数
### `tail`
- -n 行数
- -f 持续追踪尾部
## 重定向
`\>`
- stdout重定向   以覆盖模式
`\>\>`
- 以追加模式
`2>`
- stderr 重定向
`2>>`
- 以追加模式
`<`
- 从文件中读取stdin
## 命令连接
`;`
- 从左到右依次执行  不管失败与否
`&&`
- 从左到右依次执行  任一失败则终止
`||`
- 前一个失败则执行后面的
`|`
- 将前一个命令的stdout重定向到下一个命令的stdin
## 引号
`'  '` 
- 单引号内容按普通字符处理  ${}也不会展开  
`"  "`
- 双引号会展开变量 ${}  变量实际使用时建议尽量用双引号包裹
## 用户权限
`sudo`
- 临时提权  
- -i 进去root权限的shell
## 所有权和所属组
`chown`
- 位置参数一  目标值 格式为所有者:用户组  用户组可以省略代表不修改
- 位置参数二  对象路径
- -R  对目录内对象递归操作
`chmod`
- 权限分为r(read) w(write) x(execute) 对于目录来说  r(可以列出目录下内容) w(可以修改目录的内容，创建删除等) x(是否能进入目录) 用ll显示的一般是10位drwxrwxr--  第一位是d代表目录  为空代表文件  后面权限对应所有者 所属组 其他
- 位置参数一 权限 可以使用755(一般目录或文件)  或者采用\[u(所有者)g(所属组)o(其他用户)a(所有用户)\]\[+(增加)-(减少)=(指定)\]rwx来修改权限
## 软件包管理
#### 国内镜像源
- 清华大学镜像站：https://mirrors.tuna.tsinghua.edu.cn/
- 南京大学镜像站：https://mirror.nju.edu.cn/
- 校园网联合镜像站：https://mirrors.cernet.edu.cn/
- 阿里云镜像站：https://developer.aliyun.com/mirror/

#### 配置格式
替换内容一般在各大镜像站的文档里会帮忙写好，例如下图的 TUNA 镜像站。选好对应的 Ubuntu 版本后，复制镜像站提供的配置文件即可。
##### 新版(DEB822 格式)
新版软件包仓库配置文件位置在：/etc/apt/sources.list.d/ubuntu.sources
##### 旧版
传统格式软件包仓库配置文件位置在：/etc/apt/sources.list
更换后记得`sudo apt update`

#### 下载软件
`sudo apt install` 
- -y 无需确认(默认需要)
- 可以使用空格隔开安装多个软件
`apt search`
- 搜索软件包
#### 升级软件包
`sudo apt upgrade`
#### 卸载和清理
`sudo apt list --installed`
- 列出已安装的软件包
`sudo apt remove 软件包`
- 卸载软件包
`sudo apt purge 软件包`
- 连带配置也会清除
`sudo apt autoremove`
- 清理无用依赖项
## ssh
`scp`
- 可以使用scp来传输文件 用法与cp基本一样 但是需要使用`username@Ipaddress:目录`指定远端目录
`ssh-keygen`
- 用于生成密钥  可以配置免密登录  将生成的公钥复制到远程主机的.ssh目录
`ssh-copy-id user@remote-host`
- 会自动检索本地的./ssh/\*.pub密钥 并使用密码登录把密钥写入远程主机的.ssh/authorized_keys 并配置好权限

`~/.ssh/authorized_keys`  服务器允许登录的公钥
`~/.ssh/config`  客户端 SSH 配置
`/etc/ssh/sshd_config`  SSH 服务端配置
- `PasswordAuthentication`  控制密码认证 关闭则只能使用密钥登录
- `PermitRootLogin`  控制 root 远程登录
## 网络下载工具
`curl`
- 有许多其他功能
`wget`
- 参数简单
`aria2c`
- 专业下载工具 下载功能更强大
## 打包和压缩
`tar`
- 仅创建归档文件  可以调用gzip等实现打包加上压缩
- -c 创建归档  -x 解包归档 -v 显示正在处理的文件 -z 使用gzip解压/压缩      -f 跟上归档文件名   参数几乎总是一起使用的 例如`tar -cvzf test.gz test/`        `tar -xvzf test.gz`
`zip`
- 最常用
`7z`
- 压缩慢  但是压缩率高
## 文本处理
`wc`（意为 word count）
- -l：只统计行数 -w：只统计单词数 -c：只统计字节数 -m：只统计字符数       默认统计前三项
`grep` (文本检索)
- 位置参数一  搜索内容的正则表达式
- 位置参数二  想搜索文件的路径
- -i  忽略大小写
- -n  显示行号
- -r  递归搜索目录
- -v  显示不匹配的行
- -w  只匹配完整单词
- -F  按普通字符串匹配，不解析正则表达式
- -C  行数显示匹配行前后各 n 行
`sed` (文本替换 默认不修改 仅输出到stdout)
- 位置参数一  替换方式
- 位置参数二  想修改文件的路径
- -i  直接修改源文件  保存后不可恢复！
- 用法 sed -i "s/test/ts/g"   最后一个/g代表替换全部
## 符号链接和硬链接
符号链接又称软连接 其inode指向另一个inode 相当于windows快捷方式  本身并不持有任何数据  源文件被删除后 软连接就会失效
硬链接  两个目录指向同一个inode两者平级
`ln` (创建链接)
- -s 创建符号链接  不加则是硬链接 
- `ln -s test.txt a.txt`  创建一个 a.txt 指向 test.txt
## 后台任务
#### 挂起与恢复 
终端程序运行时按`ctrl + z`进程会被挂起(后台且暂停)  
也可以通过`fg %[1 2 3]`恢复对应进程或者`bg %[1 2 3]`在后台恢复运行
#### 后台任务
启动进程的时候用 `&` 即可以创建后台进程 例如`sleep 1000 &`
可以使用jobs查看所有后台进程  其中`+`标记的是最近启动的  也是fg bg的默认目标
`-`标记的是最近启动的前一个  用于递补
#### 不挂断任务
使用nohup启动任务  即使终端关闭也会继续运行  `nohup sleep 30`
#### 终端复用器
推荐tmux

## 系统资源查询
`top`
- 默认安装  界面朴素 一般使用`htop`或者其他格式各样的\*top
`kill`
- 使用pid匹配进程 默认发送的是SIGTERM 如果进程卡死 可以使用`kill -9`发送SIGKILL来杀死进程
`pkill`
- 可以使用正则来匹配进程名
`du` / `df`
- 查询磁盘的使用情况 通过使用`df -h`查看系统存储使用情况  但是未挂载的则不会显示  `du -sh 文件或目录`查看其占用大小
`lsblk`
- 查看磁盘情况 即使未挂载也会显示
`iotop`
- 查看磁盘读写 类似 top
## 磁盘挂载与卸载
`sudo mount /dev/sdb1 /data/backup`
- 用lsblk等命令查看未挂载的磁盘  并将它们挂载到空的目录下(因为被挂载目录不可用，所以通常选择挂载在空目录)
`sudo umount /dev/sdb1`
- 可以选择挂载点也可以选择挂载的磁盘   在拔出磁盘之前最好先卸载  防止有正在进行的IO操作
## 环境变量与变量
`printenv / env`
- 可以查看当前环境变量
- 只有环境变量才可以被子进程看到 
- 使用`key=value`可以设置变量  使用`export key=value`可以设置环境变量 注意=前后不能有空格  仅对某个变量追加可以使用`key=value:${key}`  通常环境变量使用`:`分隔
#### 特殊环境变量
`PATH`
- 默认运行程序的时候  会去PATH目录下找对应的程序   可以使用`which`在PATH中查找某个程序的位置
## .bashrc
启动终端的时候默认加载的配置文件   需要长期使用的环境变量等可以设置在其中
也可以使用`alias key=value`设置别名
## 实用工具
`watch`
- 监视文件 -n 指定间隔(秒)
`tee`
- 输出到文件的同时还会回显在终端
`diff`
- 比较文件差异  
`uname`
- -a 显示全部系统信息
`CTRL + D`
- 终端里按该快捷键会输出一个EOF(结束符)  在root模式下可以退出root  在ssh中可以登出
## 系统服务管理
如果希望一个程序长期在后台运行、开机自动启动  或者程序崩溃后自动重新启动    通常需要将它配置成一个系统服务
现代 Ubuntu 等 Linux 发行版主要使用 systemd 管理系统服务  最常用的工具是：
`systemctl`        启动、停止、重启和查看服务
`systemd service`   定义一个系统服务
`systemctl enable`  设置开机自动启动
系统服务在`/etc/systemd/system/*.service`可以看到


## 系统日志和故障排查
如果某个服务启动失败、系统突然重启、硬件出现异常  第一件事通常应该是查看日志
常见工具和位置包括:
`journalctl` 查看 systemd 收集的系统和服务日志
`dmesg`     查看 Linux 内核产生的信息，硬件和驱动故障排查中非常常用
`/var/log`  传统日志文件所在目录



## 定时和自动执行任务
如果希望 Linux 在指定时间自动执行某个命令，例如：
每天凌晨自动备份
每小时执行一次脚本
每周自动清理文件
可以了解：
`cron / crontab` 经典的 Linux 定时任务工具
`systemd timer`  systemd 提供的定时任务机制
`at` 在未来某个时间只执行一次任务
简单的周期任务使用 cron 非常常见  而已经使用 systemd 管理的服务通常也可以搭配 systemd timer


## 网络配置和排障
网络问题种类很多，不同问题通常需要不同工具：
`ifconfig / ip addr` 查看网络接口和 IP 地址
`ip route` 查看和管理路由
`ping` 测试目标是否能够连通
`ss` 查看网络连接和监听端口
`dig / nslookup` 查询 DNS
`traceroute / tracepath` 查看数据包经过的网络路径
`tcpdump` 抓取和分析网络数据包
`nmap` 探测网络中的主机和开放端口


## 防火墙
如果希望限制哪些网络连接能够进入或离开 Linux 系统，就需要配置防火墙。
常见工具包括：
`ufw`  Ubuntu 中非常易用的防火墙管理工具
`nftables`  现代 Linux 内核主要使用的底层防火墙框架
`iptables`  旧教程中非常常见的防火墙工具

## 远程文件同步
`rsync`
- 增量更新


## 硬件和驱动
如果需要查看 Linux 识别到了哪些硬件，或者排查设备和驱动问题，可以了解
`lspci`  查看 PCI / PCIe 设备
`lsusb`  查看 USB 设备
`lscpu`  查看 CPU 信息
`dmidecode`  查看 BIOS、内存、主板等 DMI 信息
`lsmod` 查看当前已经加载的内核模块
`modprobe` 加载或卸载内核模块
`dmesg`  查看内核和驱动产生的信息