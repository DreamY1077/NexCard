# NexCard（新卡智联）UI 界面优化方案

> 生成时间：2026-08-21 ｜ 基于全部 .ui 文件静态分析
> 性质：方案文档（仅规划，不修改代码）

---

## 一、界面现状盘点

| 区域 | 文件 | 现状 |
|------|------|------|
| 登录主窗口 | `widget.ui` | 600×400 固定尺寸，背景图 + 顶部标题 + 三个 RadioButton 切换，QStackedWidget 承载三个登录子页 |
| 刷卡登录页 | `logincardwidget.ui` | **绝对定位**（手写 geometry），"请刷卡"提示 + 卡号输入框 + 登录按钮，样式内联 |
| 管理员登录页 | `loginadminwidget.ui` | 绝对定位，账号密码输入 + 登录按钮，样式内联 |
| 员工注册页 | `employeeregisterwidget.ui` | 绝对定位，多字段表单，样式内联 |
| 管理员主控 | `adminpagewidget.ui` | 1024×800，背景图 + 顶部标题/欢迎语/退出 + 左侧树菜单(QDockWidget+QTreeWidget) + 右侧 QTabWidget，**QSS 较完整但全部内联在 .ui 中** |
| 员工主控 | `employeepagewidget.ui` | 结构同上，QSS 与 admin 端**不完全一致**（部分选择器带 `#pageTabWidget` 前缀、部分不带） |
| 业务页面（约 14 个） | `adminof*.ui` / `employee*.ui` | 无自带样式，依赖主控 Tab 的 QSS 继承；弹出的子窗口（新增/修改类）为**独立顶层窗口，样式脱节** |
| 资源 | `res.qrc` | 4 张图（controlMag.jpg / loginMag.jpg / loginMag2.jpg / titleMag.png），另有 `LoginBackgroundImage.png` 未被引用 |

---

## 二、问题清单（按影响排序）

| 级别 | 问题 | 说明 |
|------|------|------|
| 高 | **样式重复散落** | 同一套按钮/输入框 QSS 在登录页、两个主控、业务页中重复粘贴 5+ 份，改一处漏一处 |
| 高 | **两套主控样式漂移** | admin 端 QSS 全部限定 `QTabWidget#pageTabWidget` 作用域，员工端混用全局/局部选择器，长期维护必然观感分叉 |
| 高 | **登录页绝对定位、不可缩放** | 登录窗口固定 600×400，子页控件全手写坐标；换高分屏/改窗口尺寸即错位 |
| 中 | **无状态反馈色** | 表格无斑马纹；考勤签到/签退、权限等级、库存不足等关键状态无颜色区分，扫一眼看不清 |
| 中 | **无图标体系** | 树菜单、按钮纯文字，侧边栏视觉单调，层级感弱 |
| 中 | **欢迎语未利用** | welLabel 固定显示"您好"，未显示登录者姓名/角色/等级徽章 |
| 中 | **子窗口风格脱节** | AdminOfSetEmp / AdminOfAddAtt / AdminOfAddShop / 注册页等独立窗口不继承主控 QSS，观感像"另一个程序" |
| 中 | **弹窗原生样式** | QMessageBox / QInputDialog 全部系统默认，与整体风格不搭 |
| 低 | **无高 DPI 适配** | main.cpp 无 `AA_EnableHighDpiScaling`，高分屏下界面发虚/错位 |
| 低 | **无字体规范** | 字号/字体在各处硬编码（14pt/15px/36px 混用），未建立字号阶梯 |
| 低 | **资源混乱** | res.qrc 无目录组织，1 张图片未引用 |
| 低 | **交互细节缺失** | 输入框无回车提交、无 tooltip、无快捷键 |

---

## 三、优化目标与设计规范（Design Token）

统一的设计系统，用一套"变量"驱动全局：

| Token | 建议值 | 用途 |
|-------|--------|------|
| 主色 Primary | `#4287d8`（现有蓝） | 主按钮、选中态、焦点框 |
| 辅色 Secondary | `#73a8dd` | 次级描边、链接 |
| 成功 Success | `#3cb371` / `#27ae60` | 签到、正常、已支付 |
| 警示 Warning | `#f0ad4e` | 库存不足、待处理 |
| 危险 Danger | `#d9534f` | 签退(可选)、删除、错误、迟到 |
| 文字主/次/弱 | `#222222` / `#666666` / `#999999` | 标题/正文/辅助 |
| 背景/卡片 | `rgba(255,255,255,140~230)` | 保持现有半透明玻璃感 |
| 字体 | Microsoft YaHei UI / 11pt 基准 | 标题 22pt / 正文 11pt / 辅助 9pt 阶梯 |
| 圆角 | 8px（控件）/ 12px（按钮）/ 16px（卡片） | 统一观感 |

**核心做法**：抽取一份**全局样式表 `app.qss`**（放入资源 qrc），在 `main.cpp` 加载，统一设置所有控件基类样式；各页面只保留少量覆盖样式，删掉重复内联 QSS。

---

## 四、分层优化方案

### 4.1 全局样式层（收益最大，建议最先做）

新建 `res/app.qss`（qrc 注册，main.cpp 一次性加载），覆盖：

- `QPushButton`（主/次/危险三种对象名约定：`#primaryBtn` / 默认 / `#dangerBtn`）
- `QLineEdit` / `QComboBox`（含下拉面板、hover、focus 主色描边）
- `QTableWidget`（斑马纹 `alternate-background-color`、表头加粗、item 圆角 hover）
- `QTreeWidget`（菜单条目高亮、缩进、图标间距）
- `QTabWidget` / `QTabBar`（页签、关闭按钮 hover 红色）
- `QMessageBox`（按钮、标题图标配色）
- `QToolTip`、`QScrollBar`、`QCheckBox` / `QRadioButton`（统一 indicator 绘制）
- 全局字体设置

> 注意：全局 QSS 会同时作用到 admin/employee 两个主控及其业务页，**先统一再删重复**，避免中间态观感变化过大。逐个页面编译验证。

### 4.2 登录页重构（widget.ui + 三个登录子页）

```
┌───────────────────────────────────────────┐
│            背景图（全窗口平铺）              │
│   ┌───────────────────────────────────┐   │
│   │          ▤ 新卡智联                │   │
│   │     NexCard 企业一卡通考勤系统       │   │
│   │  ○ 刷卡登录   ○ 管理员登录   ○ 员工注册│   │
│   │  ┌───────────────────────────┐    │   │
│   │  │   登录卡片（半透明圆角）      │    │   │
│   │  │   [账号图标] 输入框         │    │   │
│   │  │   [密码图标] 输入框         │    │   │
│   │  │        [ 登 录 ]          │    │   │
│   │  └───────────────────────────┘    │   │
│   └───────────────────────────────────┘   │
└───────────────────────────────────────────┘
```

- 三个登录子页从**绝对定位改为 QVBoxLayout/QFormLayout 布局**，窗口可缩放且始终居中
- 输入框加前置图标（`QLineEdit::addAction` 用 QStyle 标准图标）
- 登录按钮改**主色实心**（hover 加深、pressed 下沉），支持**回车提交**
- 登录窗口支持拖拽缩放（去掉 `setFixedSize`），子页自适应

### 4.3 主控页优化（adminpagewidget / employeepagewidget）

- **顶部标题栏**：左侧品牌 Logo + 名称；右侧显示登录身份徽章（如"管理员：root · 一级"），welLabel 动态赋值
- **侧边菜单**：
  - 为每个菜单项配图标（QStyle 标准图标 `SP_*` 或内置 16×16 图标资源），分组折叠
  - 当前选中项高亮（主色浅底 + 左侧竖条指示）
- **欢迎页**：从"欢迎使用"大字改为**信息面板**——当前身份、系统日期时间、常用功能快捷入口按钮
- **状态栏（可选）**：底部加 QStatusBar 显示串口连接状态（COM5 开/关）、数据库状态

### 4.4 表格状态着色（所有业务表格）

| 场景 | 规则 |
|------|------|
| 考勤状态 | 签到 → 绿色；签退 → 蓝色/灰色（用 `setForeground` 或 delegate） |
| 管理员权限 | 一级 → 主色加粗（如"一级管理员"徽章）；二级 → 蓝；三级 → 灰 |
| 商品库存 | `number ≤ 0` → 红色加粗"缺货"；`≤ 阈值` → 橙色 |
| 员工余额 | 低余额（如 <10）→ 橙色提示 |
| 表格通用 | 开启斑马纹 `setAlternatingRowColors(true)` + QSS `alternate-background-color` |

### 4.5 弹窗与子窗口统一

- **全局 QMessageBox 样式**：按钮圆角化、主按钮主色、图标与主题一致
- **子窗口改造**：新增/修改/注册类弹窗（AdminOfSetEmp、AdminOfAddAtt、AdminOfAddShop、AdminOfSetShop、AdminOfSetAdd 等）统一改为 `QDialog` + 布局管理器，套用全局 QSS，继承主窗口观感；统一 `WA_DeleteOnClose`
- 弹窗标题栏统一风格（可后期做 frameless 自绘，非必须）

### 4.6 细节与工程化

- `main.cpp` 增加 `QApplication::setAttribute(Qt::AA_EnableHighDpiScaling)` + `AA_UseHighDpiPixmaps`（Qt5 必需）
- 资源整理：res.qrc 按目录组织（`res/icons/`、`res/bg/`、`res/qss/`），清理未引用图片
- 交互细节：所有输入框支持回车提交、关键按钮加 tooltip、按钮统一最小尺寸
- 全局字体设置一次（`QApplication::setFont`）

---

## 五、分阶段实施路线（建议顺序）

| 阶段 | 内容 | 工作量 | 风险 |
|------|------|--------|------|
| 1 | 建 app.qss 全局样式 + main 加载 + 高 DPI | 中 | 低（先铺底，回归看每页） |
| 2 | 清理各 .ui 重复内联 QSS，保留少量覆盖 | 中 | 中（每页编译验证） |
| 3 | 登录页重构（布局化 + 卡片 + 图标 + 回车） | 中 | 中（布局改动多） |
| 4 | 主控页：身份徽章、菜单图标、欢迎页信息面板 | 小 | 低 |
| 5 | 表格状态着色 + 斑马纹 | 小 | 低 |
| 6 | 子窗口统一 QDialog + 弹窗 QSS | 中 | 低 |
| 7 | 资源整理、字体规范、tooltip/快捷键 | 小 | 低 |

> 每阶段独立可交付、可回滚；建议每阶段结束做一次完整编译 + 逐页面截图回归。

---

## 六、注意事项

1. **QSS 作用域**：app.qss 是全局的，必须兼顾 admin/employee 两端；如果某页需要特例，用 `对象名#ID` 局部覆盖，不要污染全局。
2. **绝对定位改布局**：登录子页（logincardwidget/loginadminwidget/employeeregisterwidget）从 geometry 改布局时，控件位置可能微调，需人工核对不遮挡。
3. **透明背景冲突**：主控页背景图 + 半透明控件是现有特色，全局 QSS 不要加不透明背景覆盖掉玻璃质感。
4. **Qt 版本**：工程用 Qt 5.12，QSS 支持 `alternate-background-color`、`rgba`；高 DPI 属性必须放 `main()` 中 QApplication 创建前。
5. **回归重点**：每个主控 Tab 内业务页、弹出的所有子窗口、登录三页切换、MessageBox 弹窗，逐项核对。

---

*本方案基于静态分析，实际效果以运行时验证为准。*
