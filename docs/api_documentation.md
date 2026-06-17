
# 德州扑克服务端 API 文档
#Texas-Holdem-AI-System

本文档描述德州扑克源码中服务端与客户端之间的主要通信接口。所有通信基于 **WebSocket** 或 **TCP Socket**，消息格式为 **JSON**。

---

## 目录

1. [通用规范](#通用规范)
2. [登录与认证](#登录与认证)
3. [房间管理](#房间管理)
4. [牌桌操作](#牌桌操作)
5. [俱乐部系统](#俱乐部系统)
6. [锦标赛系统](#锦标赛系统)
7. [后台管理接口](#后台管理接口)
8. [错误码表](#错误码表)

---

## 通用规范

### 请求格式


{
  "cmd": "命令名称",
  "seq": 12345,
  "data": { ... 具体参数 ... }
}


字段	类型	说明
cmd	string	命令名称，见各接口定义
seq	int	消息序列号，客户端自增
data	object	请求数据


响应格式
{
  "cmd": "命令名称",
  "seq": 12345,
  "code": 0,
  "msg": "success",
  "data": { ... 返回数据 ... }
}
字段	类型	说明
code	int	0=成功，非0=错误码
msg	string	提示信息
登录与认证
1. 客户端登录
请求 cmd = "login"
{
  "cmd": "login",
  "seq": 1,
  "data": {
    "username": "player001",
    "password": "md5_encrypted",
    "device_id": "uuid_12345",
    "language": "en_US"
  }
}


响应
{
  "cmd": "login",
  "seq": 1,
  "code": 0,
  "data": {
    "user_id": 10086,
    "nickname": "扑克王",
    "avatar": "https://cdn.example.com/avatar.png",
    "chips": 15000,
    "vip_level": 3,
    "session_token": "eyJhbGciOiJIUzI1NiIs..."
  }
}


2. 获取用户信息
请求 cmd = "get_user_info"
{
  "cmd": "get_user_info",
  "seq": 2,
  "data": {
    "user_id": 10086
  }
}
响应
{
  "cmd": "get_user_info",
  "seq": 2,
  "code": 0,
  "data": {
    "user_id": 10086,
    "nickname": "扑克王",
    "chips": 14800,
    "total_hands": 1250,
    "win_rate": 0.62,
    "total_profit": 3500
  }
}

房间管理
3. 获取房间列表
请求 cmd = "get_room_list"
{
  "cmd": "get_room_list",
  "seq": 10,
  "data": {
    "game_type": "classic",
    "page": 1,
    "page_size": 20
  }
}
响应
{
  "cmd": "get_room_list",
  "seq": 10,
  "code": 0,
  "data": {
    "total": 45,
    "rooms": [
      {
        "room_id": 1001,
        "room_name": "初级场",
        "blind_small": 10,
        "blind_big": 20,
        "min_buyin": 1000,
        "max_buyin": 5000,
        "current_players": 6,
        "max_players": 9,
        "status": "playing"
      }
    ]
  }
}

4. 加入房间
请求 cmd = "join_room"

{
  "cmd": "join_room",
  "seq": 11,
  "data": {
    "room_id": 1001,
    "password": ""  // 可选，私密房间需要
  }
}
响应
{
  "cmd": "join_room",
  "seq": 11,
  "code": 0,
  "data": {
    "table_id": 5001,
    "seat_id": 3,
    "chips_at_table": 5000
  }
}

5. 离开房间
请求 cmd = "leave_room"

{
  "cmd": "leave_room",
  "seq": 12,
  "data": {
    "table_id": 5001
  }
}
响应 返回标准 code: 0 表示成功。

牌桌操作
6. 玩家行动（下注/弃牌等）
请求 cmd = "player_action"
{
  "cmd": "player_action",
  "seq": 20,
  "data": {
    "table_id": 5001,
    "action": "bet",
    "amount": 100,
    "action_type": "fold|check|call|bet|raise|all_in"
  }
}


json
{
  "cmd": "player_action",
  "seq": 20,
  "code": 0,
  "data": {
    "new_pot": 2500,
    "player_chips_left": 4900
  }
}
7. 牌桌状态推送（服务端主动推送）
推送 cmd = "table_state"

json
{
  "cmd": "table_state",
  "seq": 0,
  "data": {
    "table_id": 5001,
    "round": "preflop",
    "community_cards": [],
    "pot": 1500,
    "current_bet": 100,
    "current_player_seat": 3,
    "players": [
      {
        "seat_id": 1,
        "user_id": 10001,
        "chips": 5000,
        "bet": 0,
        "status": "active",
        "hand_cards": []  // 仅自己能看到自己的手牌
      }
    ]
  }
}
8. 发牌推送
推送 cmd = "deal_cards"

json
{
  "cmd": "deal_cards",
  "seq": 0,
  "data": {
    "hand_cards": [
      { "suit": "spades", "rank": "A" },
      { "suit": "hearts", "rank": "K" }
    ]
  }
}
9. 公共牌推送
推送 cmd = "community_cards"

json
{
  "cmd": "community_cards",
  "seq": 0,
  "data": {
    "round": "flop",
    "cards": [
      { "suit": "clubs", "rank": "10" },
      { "suit": "diamonds", "rank": "5" },
      { "suit": "spades", "rank": "2" }
    ]
  }
}
10. 摊牌/结算推送
推送 cmd = "showdown"

json
{
  "cmd": "showdown",
  "seq": 0,
  "data": {
    "winners": [
      {
        "seat_id": 3,
        "user_id": 10086,
        "hand_rank": "flush",
        "hand_cards": [...],
        "win_amount": 3500
      }
    ],
    "player_chips_update": [
      { "seat_id": 3, "chips": 8500 },
      { "seat_id": 1, "chips": 1500 }
    ]
  }
}
俱乐部系统
11. 创建俱乐部
请求 cmd = "create_club"

json
{
  "cmd": "create_club",
  "seq": 30,
  "data": {
    "club_name": "扑克精英社",
    "description": "欢迎高手加入",
    "min_vip_level": 0,
    "join_type": "apply"  // apply=申请加入, direct=直接加入
  }
}
响应

json
{
  "cmd": "create_club",
  "seq": 30,
  "code": 0,
  "data": {
    "club_id": 2001,
    "invite_code": "ABC123"
  }
}
12. 加入俱乐部
请求 cmd = "join_club"

json
{
  "cmd": "join_club",
  "seq": 31,
  "data": {
    "club_id": 2001,
    "invite_code": "ABC123"
  }
}
响应

json
{
  "cmd": "join_club",
  "seq": 31,
  "code": 0,
  "data": {
    "club_name": "扑克精英社",
    "member_count": 156
  }
}
13. 创建俱乐部牌桌
请求 cmd = "create_club_table"

json
{
  "cmd": "create_club_table",
  "seq": 32,
  "data": {
    "club_id": 2001,
    "blind_small": 50,
    "blind_big": 100,
    "min_buyin": 5000,
    "max_buyin": 20000,
    "max_players": 9,
    "is_private": true
  }
}
响应 返回 table_id，同普通房间。

锦标赛系统
14. 获取锦标赛列表
请求 cmd = "get_tournament_list"

json
{
  "cmd": "get_tournament_list",
  "seq": 40,
  "data": {
    "type": "mtt",  // sng 或 mtt
    "status": "registering"  // registering, playing, finished
  }
}
响应

json
{
  "cmd": "get_tournament_list",
  "seq": 40,
  "code": 0,
  "data": {
    "tournaments": [
      {
        "tournament_id": 3001,
        "name": "每日免费赛",
        "type": "mtt",
        "buyin": 0,
        "fee": 0,
        "prize_pool": 100000,
        "start_time": "2026-06-13 20:00:00",
        "registered_count": 45,
        "max_players": 200,
        "status": "registering"
      }
    ]
  }
}
15. 报名锦标赛
请求 cmd = "register_tournament"

json
{
  "cmd": "register_tournament",
  "seq": 41,
  "data": {
    "tournament_id": 3001
  }
}
响应

json
{
  "cmd": "register_tournament",
  "seq": 41,
  "code": 0,
  "data": {
    "registration_id": 8001,
    "table_id": 6001
  }
}
后台管理接口
后台接口基于 HTTP RESTful，前缀为 /api/admin。

16. 管理员登录
http
POST /api/admin/login
Content-Type: application/json

{
  "username": "admin",
  "password": "admin123"
}
响应

json
{
  "code": 0,
  "data": {
    "token": "admin_jwt_token"
  }
}
17. 查询玩家列表
http
GET /api/admin/players?page=1&size=20
Authorization: Bearer {token}
响应

json
{
  "code": 0,
  "data": {
    "total": 1500,
    "list": [
      {
        "user_id": 10086,
        "nickname": "扑克王",
        "chips": 150000,
        "register_time": "2026-01-01 10:00:00",
        "status": "active"
      }
    ]
  }
}
18. 修改玩家金币
http
POST /api/admin/player/chips
Authorization: Bearer {token}

{
  "user_id": 10086,
  "amount": 10000,  // 正数增加，负数扣除
  "reason": "补偿"
}
响应 返回标准 code: 0 表示成功。

错误码表
错误码	说明
0	成功
1001	未登录或登录过期
1002	用户名或密码错误
1003	账号被禁用
2001	房间不存在
2002	房间已满
2003	金币不足
2004	已在其他房间
2005	房间密码错误
3001	俱乐部不存在
3002	没有权限
3003	邀请码无效
4001	锦标赛未开始
4002	锦标赛已报满
4003	已报名该锦标赛
5001	参数错误
5002	操作过于频繁
9999	系统内部错误
WebSocket 连接示例
JavaScript 客户端示例
javascript
const ws = new WebSocket('ws://your-server.com:10001');

ws.onopen = () => {
  // 登录
  ws.send(JSON.stringify({
    cmd: 'login',
    seq: 1,
    data: {
      username: 'player001',
      password: '5f4dcc3b5aa765d61d8327deb882cf99',
      device_id: 'web_client'
    }
  }));
};

ws.onmessage = (event) => {
  const response = JSON.parse(event.data);
  console.log('收到消息:', response);
  
  if (response.cmd === 'table_state') {
    // 更新界面
    updateTableUI(response.data);
  }
};
联系支持
如 API 对接中遇到问题，可通过以下方式联系我们：

联系方式	账号
📧 Email: ttpoker40@gmail.com
💬 Telegram: @alibabama401
*最后更新：2026-06-17 | 版本：v1.0*

