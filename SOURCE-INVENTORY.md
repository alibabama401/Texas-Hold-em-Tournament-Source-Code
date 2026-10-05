# SOURCE INVENTORY

本清单基于仓库提交 `217e628`（检查日期：2026-10-05）。

## 可直接证明

- C++ 游戏通信：`gameserver.cpp/.h`、`gameroot.cpp/.h`、`Processor.cpp`
- Tars 风格服务代理：`OuterFactoryImp.cpp`、`SocialServer.cpp`
- MySQL：`DBOperator.cpp/.h`、`robot.sql`
- Lua 资源：11 个 `.lua.bytes` 文件及对应 meta
- 移动端产品界面：`docs/Assets/Screenshots/001-009`
- API/部署文档：`docs/api_documentation.md`、`docs/deployment_guide.md`
- 工程资料：多语言、热更新、工程依赖、iOS 审核和 Lua 编码规范 Word 文档

## 不能由公开仓库独立证明

- 可直接编译运行的完整客户端和服务端
- 完整 SNG/MTT 状态机、桌间平衡与结算链路
- 酒店预订或“支持所有国内赛事”
- 已通过或保证通过 App Store 审核
- 部署文档列出的完整 Java/Redis/Unity 工程
- 生产性能、安全性和并发指标

## 需要修复的原仓库问题

- `DBOperator.cpp` 含示例数据库口令，发布前应改为环境变量或私有配置并轮换相关凭据。
- `LICENSE` 与 `License.md` 的授权条件和版权名称不一致。
- `CITATION.cff` 的功能摘要超出当前公开文件可验证范围。
- 原 README 有损坏的图片 Markdown，并包含无法由当前文件证明的酒店、全赛事兼容和 iOS 审核承诺。
