# NexCard（新卡智联）— 简历项目经历文档

> 适用于简历的项目描述，含中文版（通用/详细两个版本）与英文版，可直接摘用。

---

## 一、项目总结（技术摘要）

**NexCard（新卡智联）** 是一款基于 **Qt Widgets / C++11** 开发的"一卡通"桌面管理系统，面向企业内部场景，将 **员工考勤打卡、刷卡消费、余额充值** 与 **管理员后台管理** 整合为一套带 **串口 IC 读卡器硬件** 的完整系统。

| 维度 | 内容 |
|------|------|
| 技术栈 | Qt Widgets 5.12、Qt SQL、Qt SerialPort、C++11、qmake + MinGW 64-bit |
| 数据库 | SQLite 单文件（首次运行自动建 4 张表 + 默认管理员） |
| 硬件对接 | 串口 IC 读卡器（COM5 / 115200 / 8N1），正则解析卡号 `c1-70-74-6` 格式 |
| 代码规模 | 24 个 .cpp / 24 个 .h / 20 个 .ui，自研为主 |
| 架构 | 登录层（QStackedWidget 三页）→ 主控层（QTreeWidget + QTabWidget 懒加载）→ 业务模块 → MySql / MySerial 单例 |

**核心功能：**
- **员工端**：刷卡登录、个人信息查看、一键打卡（按当日最后一条状态自动切换签到/签退）、商品库刷卡购买、余额充值
- **管理端**：账号密码登录、管理员注册、串口配置与调试（波特率/数据位/校验位/流控、收发日志）、员工信息管理、考勤管理、商品管理（均含查询/增删改）
- **公共**：员工自助注册（刷卡 + 填信息）、串口读卡解析

**设计亮点：**
1. **单例模式**贯穿底层：`MySql`（数据库）与 `MySerial`（串口）均为全局唯一实例，跨页面共享连接与会话
2. **信号槽驱动页面流转**：登录成功发 `loginOK`、读卡器发 `cardIdReceived`，全系统统一监听，解耦页面间通信
3. **标签页懒加载**：左侧树形菜单点击后才创建页面加入 Tab，关闭置空指针，避免内存常驻
4. **事务保证购买一致性**：刷卡购买走 BEGIN / COMMIT / ROLLBACK，扣余额与扣库存原子完成
5. **参数化 SQL**：绝大多数 SQL 使用 `bindValue` 绑定，防注入
6. **界面友好性**：关键操作均有校验提示与二次确认弹窗，表格列自动铺满

---

## 二、简历项目经历（中文 · 通用版）

### 项目：NexCard 一卡通智能管理系统（Qt 桌面端） | 独立开发
**技术栈：** C++11 / Qt Widgets 5.12 / Qt SQL / Qt SerialPort / SQLite / qmake

**项目描述：**
面向企业内部的一卡通管理系统，对接串口 IC 读卡器硬件，实现员工刷卡登录、考勤打卡、刷卡消费、余额充值与管理员后台管理的全流程闭环，全程独立设计开发。

**核心职责与成果：**
- **架构设计**：采用"登录层 → 主控层 → 业务模块 → 单例底层"四层结构，基于 QStackedWidget 实现刷卡登录/管理员登录/员工注册三页切换，QTreeWidget + QTabWidget 实现树形菜单与多标签工作区，页面懒加载并按需释放，控制内存占用。
- **硬件对接（串口读卡）**：基于 Qt SerialPort 封装全局唯一串口单例，支持自动枚举可用串口、可视化配置波特率/数据位/校验位/流控；利用正则表达式从读卡器原始数据流中稳健解析卡号（如 `c1-70-74-6`），缓冲按行处理保证数据完整性，并以信号槽将卡号广播给登录、注册等所有业务页面。
- **数据层设计**：基于 SQLite 设计 4 张业务表（管理员/员工/考勤/商品），首次运行自动建表并初始化默认管理员；查询语句全部采用参数绑定（bindValue）防止 SQL 注入。
- **核心业务实现**：① 考勤打卡——按卡号查询当日最后一条记录，自动判断本次为"签到"或"签退"；② 刷卡购买——使用 BEGIN/COMMIT/ROLLBACK 事务保证"扣减余额 + 扣减库存"的原子性与数据一致性，含库存不足、余额不足等多重校验；③ 余额充值——金额合法性校验与二次确认。
- **管理端功能**：实现员工信息、考勤记录、商品信息的全量 CRUD，支持关键字模糊搜索、勾选批量删除（主键存于 QTableWidgetItem UserRole，删除时回读），全部操作带确认弹窗防误删。
- **工程质量**：全部源码由个人编写（24 cpp / 24 h / 20 ui），关键操作均有输入校验与错误提示，代码结构清晰、模块职责单一。

---

## 三、简历项目经历（中文 · 精简版，约 120 字）

**NexCard 一卡通智能管理系统（C++/Qt 桌面应用）｜独立开发**
基于 Qt Widgets 5.12 + C++11 + SQLite 独立开发的一卡通管理系统，对接串口 IC 读卡器，实现员工刷卡登录、自动签到/签退考勤、刷卡消费与余额充值、管理员后台 CRUD 全流程。采用单例模式封装数据库与串口、信号槽解耦页面通信、事务保证购买数据一致性、参数化 SQL 防注入，共 24 个 cpp / 24 个 h / 20 个 ui。

---

## 四、简历项目经历（English Version）

### Project: NexCard — Smart Card Integrated Management System (Qt Desktop)
**Tech Stack:** C++11 / Qt Widgets 5.12 / Qt SQL / Qt SerialPort / SQLite / qmake

**Overview:**
An in-house "all-in-one card" management system developed independently, integrating an IC card reader via serial port to deliver card-swipe login, attendance check-in/out, card-based payment, balance top-up, and full admin backend management.

**Key Contributions:**
- **Architecture:** Designed a 4-layer structure (Login → Main Control → Business Modules → Singleton Foundation). Used QStackedWidget for 3-page login flow and QTreeWidget + QTabWidget for a tree-menu / multi-tab workspace with lazy-loaded pages to keep memory usage low.
- **Hardware Integration:** Wrapped Qt SerialPort in a global singleton with auto port enumeration and GUI-based configuration (baud rate, data bits, parity, flow control). Parsed raw card-reader streams into card IDs (e.g. `c1-70-74-6`) using QRegularExpression with line-buffered handling, broadcasting card IDs to all pages via signal/slot.
- **Data Layer:** Designed 4 SQLite tables (admin/employee/attendance/product) with auto-creation on first run and a default admin account; used parameterized queries (`bindValue`) throughout to prevent SQL injection.
- **Core Business:** ① Attendance — queried the last record of the day to auto-switch between check-in and check-out; ② Card Payment — wrapped balance deduction and stock decrement in BEGIN/COMMIT/ROLLBACK transactions for atomicity, with stock/balance validation; ③ Top-up — amount validation with double confirmation.
- **Admin Features:** Full CRUD for employees, attendance, and products, keyword fuzzy search, checkbox batch deletion (IDs stored in item UserRole), and confirmation dialogs to prevent accidental deletion.
- **Quality:** Entire codebase written by hand (24 .cpp / 24 .h / 20 .ui) with consistent validation and error handling.

---

## 五、简历关键词 / 技能标签

C++11 · Qt Widgets · Qt 信号槽 · Qt SQL · SQLite · Qt SerialPort（串口通信）· 正则表达式 · 单例模式 · 事务处理 · 参数化查询（防注入）· QSS/UI 设计 · 面向对象设计 · 桌面应用开发 · 独立开发

---

## 六、面试可展开的技术细节（备用弹药）

| 技术点 | 可展开说明 |
|--------|-----------|
| 串口解析 | 按行缓冲 + 正则 `([0-9a-fA-F]{1,2}(?:-[0-9a-fA-F]{1,2})+)` 从读卡器数据中提取卡号，兼容无换行数据 |
| 事务购买 | BEGIN → 校验库存/余额 → 扣余额 → 逐商品扣库存 → COMMIT；任一步失败 ROLLBACK |
| 打卡状态机 | 查询当日最后一条状态：无记录或"签退"→ 本次"签到"；"签到"→ 本次"签退" |
| 单例模式 | `MySql::p` / `MySerial::m_serial` 静态指针 + `getInstance()`，跨页面共享数据库连接与串口会话 |
| 懒加载标签页 | 菜单点击才 `new` 页面并 `addTab`，关闭时指针置空 + `deleteLater()`，退出时倒序遍历删除避免索引错乱 |
| 防注入 | 查询语句统一 `prepare` + `bindValue` 参数绑定 |

---

*本文档基于源码静态分析生成，代码通读时间为 2026-08-15。*
