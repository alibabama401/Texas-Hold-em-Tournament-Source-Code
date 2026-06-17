# 德州扑克源码部署指南
#Texas-Hold-em-Tournament-Source-Code

本指南帮助你从零开始部署德州扑克完整源码（客户端 + 服务端 + 数据库）。适用于 Ubuntu 20.04 / CentOS 7+ 环境。

---

## 目录 | Table of Contents

1. [环境要求](#环境要求)
2. [数据库部署](#数据库部署)
3. [服务端部署](#服务端部署)
4. [客户端编译](#客户端编译)
5. [配置修改](#配置修改)
6. [启动与验证](#启动与验证)
7. [常见问题](#常见问题)

---

## 环境要求

### 硬件最低配置（开发/小规模测试）

| 组件 | 要求 |
| :--- | :--- |
| CPU | 4核 @ 2.5GHz+ |
| 内存 | 8 GB |
| 硬盘 | 50 GB SSD |
| 带宽 | 10 Mbps |

### 硬件推荐配置（生产环境/1000+ 并发）

| 组件 | 要求 |
| :--- | :--- |
| CPU | 8核+ @ 3.0GHz+ |
| 内存 | 16 GB+ |
| 硬盘 | 100 GB+ SSD (RAID 10) |
| 带宽 | 50 Mbps+ |

### 软件要求

| 软件 | 版本 | 用途 |
| :--- | :--- | :--- |
| Ubuntu | 20.04 / 22.04 | 操作系统 |
| MySQL | 8.0+ | 主数据库 |
| Redis | 6.0+ | 缓存/会话 |
| Java | OpenJDK 11 | 后台服务 |
| GCC/G++ | 9.0+ | 编译 C++ 服务端 |
| CMake | 3.15+ | 构建工具 |
| Unity | 2020.3 LTS | 客户端编译 |

---

## 数据库部署

### 步骤1：安装 MySQL


# Ubuntu 20.04
sudo apt update
sudo apt install mysql-server-8.0 -y

# 启动并设置开机自启
sudo systemctl start mysql
sudo systemctl enable mysql

# 安全配置（设置 root 密码）
sudo mysql_secure_installation

步骤2：导入数据库脚本
# 登录 MySQL
mysql -u root -p

# 创建数据库（数据库名称可自定义，以实际脚本为准）
CREATE DATABASE poker_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE DATABASE poker_log_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

# 退出 MySQL
exit;

# 导入脚本（脚本路径以你的实际为准）
mysql -u root -p poker_db < /path/to/sql/poker_db.sql
mysql -u root -p poker_log_db < /path/to/sql/poker_log_db.sql


步骤3：安装 Redis
sudo apt install redis-server -y
sudo systemctl start redis
sudo systemctl enable redis

# 测试 Redis
redis-cli ping
# 应返回 PONG


服务端部署
服务端包含三个主要部分：

RoomServer（C++）：处理牌桌逻辑、游戏结算

LoginServer（C++/Java）：处理登录、路由

AdminServer（Java）：后台管理系统

步骤1：安装编译工具
sudo apt install build-essential cmake libmysqlclient-dev libhiredis-dev -y
步骤2：编译 C++ 服务端
# 进入服务端源码目录（以实际路径为准）
cd /path/to/server/cpp/

# 创建构建目录
mkdir build && cd build

# CMake 配置
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
make -j$(nproc)

# 编译完成后，可执行文件位于 build/bin/ 目录

步骤3：配置服务端参数
[Database]
mysql_host=127.0.0.1
mysql_port=3306
mysql_user=poker_user
mysql_password=your_password
mysql_db=poker_db

[Redis]
redis_host=127.0.0.1
redis_port=6379
redis_password=

[Server]
room_port=10001
login_port=10002
max_players=1000
log_level=INFO



步骤4：编译/运行 Java 后台服务

# 安装 Maven
sudo apt install maven -y

# 进入 Java 服务目录
cd /path/to/server/java/admin/

# 编译打包
mvn clean package -DskipTests

# 运行（生成的 jar 包在 target/ 目录）
java -jar target/admin-server-1.0.jar --server.port=8080



客户端编译
客户端使用 Unity 引擎，支持 iOS 和 Android。

步骤1：安装 Unity
下载并安装 Unity Hub

通过 Unity Hub 安装 Unity 2020.3 LTS 版本

添加模块：iOS Build Support / Android Build Support

步骤2：打开项目
打开 Unity Hub → Add → 选择客户端源码目录（如 client/unity/）

等待项目加载完成

步骤3：修改服务器地址
在 Unity 编辑器中，找到 Assets/Scripts/Network/NetworkConfig.cs：
public class NetworkConfig
{
    // 修改为你的服务端公网 IP 或域名
    public static string LOGIN_SERVER_IP = "your-server-ip";
    public static int LOGIN_SERVER_PORT = 10002;
    
    public static string ROOM_SERVER_IP = "your-server-ip";
    public static int ROOM_SERVER_PORT = 10001;
}


步骤4：构建客户端
Android（APK/AAB）
File → Build Settings → Android → Switch Platform

Player Settings 中配置包名、签名、版本号

Build → 生成 APK 或 AAB

iOS（IPA）
File → Build Settings → iOS → Switch Platform

Player Settings 中配置 Bundle Identifier、签名

Build → 生成 Xcode 项目

使用 Xcode 编译并导出 IPA

配置修改（重要参数）
修改游戏基础规则
文件：server/cpp/config/game_rules.json
{
  "blind_levels": [
    { "small_blind": 10, "big_blind": 20, "duration_seconds": 600 },
    { "small_blind": 20, "big_blind": 40, "duration_seconds": 600 },
    { "small_blind": 50, "big_blind": 100, "duration_seconds": 600 }
  ],
  "rake_percent": 5,
  "rake_cap": 100,
  "starting_chips": 1500,
  "min_players_for_start": 2,
  "max_players_per_table": 9
}


修改多语言文案
文件：https://github.com/alibabama401/Texas-Hold-em-Tournament-Source-Code

文件	语言
zh_CN.json	简体中文
zh_TW.json	繁体中文
en_US.json	英文
vi_VN.json	越南语
ko_KR.json	韩语


启动与验证
启动顺序
# 1. 启动 Redis
sudo systemctl start redis

# 2. 启动 MySQL（如未启动）
sudo systemctl start mysql

# 3. 启动 LoginServer
cd /path/to/server/cpp/build/bin/
./login_server --config=server.conf

# 4. 启动 RoomServer
./room_server --config=server.conf

# 5. 启动 AdminServer（Java）
cd /path/to/server/java/admin/
java -jar target/admin-server-1.0.jar


验证服务状态
# 检查端口监听
netstat -tlnp | grep -E "10001|10002|8080"

# 预期输出：
# tcp 0 0 0.0.0.0:10001  LISTEN  [room_server]
# tcp 0 0 0.0.0.0:10002  LISTEN  [login_server]
# tcp 0 0 0.0.0.0:8080   LISTEN  [java]


客户端连接测试
将编译好的客户端安装到手机或模拟器

输入测试账号登录（后台可创建测试账号）

创建或加入牌桌，验证完整流程

常见问题
Q1: 编译 C++ 时报错 mysql.h: No such file
解决方案：
sudo apt install libmysqlclient-dev
Q2: Redis 连接失败
检查：
redis-cli ping
# 应返回 PONG

# 检查 bind 配置
sudo nano /etc/redis/redis.conf
# 确保 bind 127.0.0.1 或你的服务器 IP

Q3: 客户端连接不上服务端
排查步骤：

检查服务端防火墙：sudo ufw allow 10001、sudo ufw allow 10002

检查客户端 NetworkConfig.cs 中的 IP 是否正确

使用 telnet your-server-ip 10001 测试端口连通性

Q4: 如何修改数据库密码？
ALTER USER 'poker_user'@'localhost' IDENTIFIED BY 'new_password';
FLUSH PRIVILEGES;
然后同步修改 server.conf 中的 mysql_password。

Q5: 服务端崩溃如何排查？
查看日志文件：
# 日志默认位置
cd /path/to/server/cpp/build/bin/logs/
tail -100 room_server.log

Q6: 支持 HTTPS/WSS 吗？
支持。需要在 Nginx 配置 SSL 反向代理，并修改客户端的 WebSocket 连接地址为 wss://your-domain.com。

联系支持
如遇到部署问题，可通过以下方式联系我们：

联系方式	账号
📧 Email: ttpoker40@gmail.com
💬 Telegram: @alibabama401
请在联系时附上错误日志和服务器环境信息。

*最后更新：2026-06-17 | 版本：v1.0*
