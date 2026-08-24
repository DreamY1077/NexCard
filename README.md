# NexCard新卡智联桌面系统

基于 Qt + dlib 的一卡通与考勤管理上位机系统，对接 STM32+RFID 读卡硬件。

## 负责内容
Qt 上位机界面开发与全部功能实现（登录/考勤/消费/充值/管理后台/人脸识别）

## 技术栈
- C++14 / Qt Widgets 5.12
- Qt SQL / Qt SerialPort / Qt Multimedia
- QThread 多线程 / SQLite / Dlib 19.24
- QSS 皮肤 / 跨平台

## 构建说明

### 1. 准备模型文件
由于 dlib 模型文件较大（>100MB），未上传至仓库。请下载以下文件放入 `models/` 目录：

- `shape_predictor_68_face_landmarks.dat`（99.7MB）
  - 下载：<http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2>
- `dlib_face_recognition_resnet_model_v1.dat`（22.5MB）
  - 下载：<http://dlib.net/files/dlib_face_recognition_resnet_model_v1.dat.bz2>
- `mmod_human_face_detector.dat`（730KB，可选，增强戴眼镜场景检测）
  - 下载：<http://dlib.net/files/mmod_human_face_detector.dat.bz2>

下载后解压缩（.bz2 → .dat），放入项目 `models/` 目录。

### 2. dlib 源码
`thirdparty/dlib/` 目录已包含 dlib 19.24.9 完整源码。如使用更稳定版本可替换：
```bash
git submodule add https://github.com/davisking/dlib.git thirdparty/dlib
```

### 3. 编译
使用 Qt Creator 打开 `Smart_Card_Pass.pro` 即可构建（建议 **Release 模式**）。

## 项目结构
```
.
├── adminfeedbackwidget.*      # 管理员反馈管理
├── adminofemployee.*          # 员工信息管理（含人脸录入）
├── employeepunchwidget.*      # 员工人脸考勤打卡
├── facedetector.*             # dlib 封装（人脸检测/特征）
├── cameraframesurface.*       # 摄像头帧捕获
├── facerecognitionworker.*    # 识别工作线程
├── faceregisterdialog.*       # 人脸录入对话框
├── mysql.*                    # SQLite 数据库层
├── myserial.*                 # 串口读卡单例
├── employeepagewidget.*       # 员工主控
├── adminpagewidget.*          # 管理员主控
├── widget.*                   # 登录窗口
└── thirdparty/dlib/           # dlib 19.24.9 源码
```

## 功能特性
- 🔐 多级权限管理（一/二/三级管理员）
- 💳 员工刷卡/人脸登录
- 👤 人脸识别考勤（HOG + MMOD 双检测、128维特征）
- 🛒 刷卡消费（事务原子性）
- 💰 余额充值
- 📊 考勤规则配置（签到/迟到/早退/签退时段）
- 🔍 关键字模糊搜索、批量操作
- 🌐 商家管理、问题反馈
