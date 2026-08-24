#!/usr/bin/env bash
# ============================================================================
# NexCard GitHub 推送脚本
# 用法：
#   1) 把 <YOUR_REPO_URL> 替换为你的仓库完整地址（带 .git）
#      例如 https://github.com/DreamY1077/NexCard.git
#   2) 在 Git Bash 中执行： bash push-to-github.sh
# ============================================================================

set -e

# TODO: 把下面这行替换为你的仓库地址
REPO_URL="https://github.com/DreamY1077/NexCard.git"

if [[ "$REPO_URL" == "<YOUR_REPO_URL>" ]]; then
    echo "❌ 请先打开本脚本，把 REPO_URL 替换为你的仓库地址"
    echo "   例如：REPO_URL=\"https://github.com/DreamY1077/NexCard.git\""
    exit 1
fi

cd "$(dirname "$0")"
echo "📂 当前目录: $(pwd)"

# 1) 初始化 git
if [ ! -d ".git" ]; then
    git init
    echo "✅ git init 完成"
else
    echo "ℹ️  已有 .git 目录，跳过 init"
fi

# 2) 配置提交用户（如果未设置）
if [ -z "$(git config user.name)" ]; then
    git config user.name "DreamY1077"
    git config user.email "1073772107@qq.com"
    echo "✅ git 用户配置完成（可改成你自己的）"
fi

# 3) 添加主分支
git branch -M main

# 4) 添加所有文件（受 .gitignore 过滤）
git add .
echo "✅ git add 完成"

# 5) 提交
git commit -m "feat: NexCard 智能卡人脸考勤管理系统 v4.0

- Qt 5.12 上位机：界面 + 全部功能（登录/考勤/消费/充值/管理/反馈）
- Dlib 19.24 人脸识别：HOG + MMOD 双检测、128维特征、本地离线
- 上位机串口对接 STM32+RFID 读卡硬件
- SQLite 数据层：7 张业务表、自动迁移、事务原子性
- 多线程识别（QThread + moveToThread）、5fps 限频
- 三级管理员权限、商家结算、问题反馈"

# 6) 配置远程仓库
if git remote get-url origin &>/dev/null; then
    git remote set-url origin "$REPO_URL"
    echo "ℹ️  更新 origin -> $REPO_URL"
else
    git remote add origin "$REPO_URL"
    echo "✅ 添加 origin -> $REPO_URL"
fi

# 7) 推送
echo ""
echo "🚀 开始推送到 GitHub ..."
git push -u origin main

echo ""
echo "🎉 完成！访问你的仓库查看：${REPO_URL%.git}"