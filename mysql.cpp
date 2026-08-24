#include "mysql.h"
#include <QDebug>
#include <QSqlQuery>
#include <QDateTime>
//类外初始化p
MySql * const MySql::p = new MySql("mysql.db");

MySql::MySql(QObject *parent) : QObject(parent)
{

}

MySql::~MySql()
{
    if(p!=NULL)
    {
        delete p;
    }
}

MySql::MySql(QString dbName, QObject *parent)
{
    //安装sqlite的数据库驱动（目的：确定操作哪类数据库）
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    //创建具体的基于Sqlite的数据库名
    db.setDatabaseName(dbName);

    //打开数据库
    if(db.open())
    {
        qDebug()<<"open db ok"<<endl;
    }
    else {
        qDebug()<<"open db error:"<<db.lastError().text()<<endl;
    }
}

MySql  *MySql::getMySql()
{
    return p;
}

void MySql::creatMySql()
{
    //定义一个sql语句的预处理对象
    QSqlQuery query;
    //组装SQL语句 //创建 管理员表（用户名，密码，权限等级：1一级 2二级 3三级）
    QString sql = "CREATE TABLE IF NOT EXISTS admin(name TEXT NOT NULL PRIMARY KEY,pwd TEXT NOT NULL,"
                  "level INTEGER NOT NULL DEFAULT 3)";
    //使用query对象执行sql语句
    if(query.exec(sql))
    {
        qDebug()<<"creat table admin ok"<<endl;
    }
    else {
        qDebug()<<"create table admin error:"<<query.lastError().text()<<endl;
    }

    //旧库迁移：admin 表已存在但没有 level 列时，补充该列（默认三级管理员）
    QSqlQuery pragmaQuery;
    pragmaQuery.exec("PRAGMA table_info(admin)");
    bool hasLevel = false;
    while (pragmaQuery.next())
    {
        if (pragmaQuery.value(1).toString() == "level")
        {
            hasLevel = true;
            break;
        }
    }
    if (!hasLevel)
    {
        QSqlQuery alterQuery;
        if (alterQuery.exec("ALTER TABLE admin ADD COLUMN level INTEGER NOT NULL DEFAULT 3"))
        {
            qDebug()<<"migrate table admin add column level ok"<<endl;
        }
        else {
            qDebug()<<"migrate table admin add column level error:"<<alterQuery.lastError().text()<<endl;
        }
    }

    //插入默认一级管理员 root（已存在则忽略，并确保其权限为一级）
    query.prepare("INSERT OR IGNORE INTO admin (name, pwd, level) VALUES (:name, :pwd, :level)");
    query.bindValue(":name","root");
    query.bindValue(":pwd","123456");
    query.bindValue(":level", 1);
    query.exec();
    query.prepare("UPDATE admin SET level = :level WHERE name = :name");
    query.bindValue(":name","root");
    query.bindValue(":level", 1);
    query.exec();

    //创建 员工表（卡号，姓名，年龄，性别，部门，余额）
    sql = "CREATE TABLE IF NOT EXISTS employee(card TEXT NOT NULL PRIMARY KEY,name TEXT NOT NULL,age TEXT NOT NULL,"
          "sex TEXT NOT NULL,department TEXT NOT NULL,balance TEXT NOT NULL)";
    //使用query对象执行sql语句
    if(query.exec(sql))
    {
        qDebug()<<"creat table employee ok"<<endl;
    }
    else {
        qDebug()<<"create table employee error:"<<query.lastError().text()<<endl;
    }

    //创建 考勤表（卡号，姓名，部门，时间，考勤状态）
    sql = "CREATE TABLE IF NOT EXISTS attendance(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,card TEXT NOT NULL,name TEXT NOT NULL,department TEXT NOT NULL,"
          "e_time TEXT NOT NULL,status TEXT NOT NULL)";
    //使用query对象执行sql语句
    if(query.exec(sql))
    {
        qDebug()<<"creat table attendance ok"<<endl;
    }
    else {
        qDebug()<<"create table attendance error:"<<query.lastError().text()<<endl;
    }

    //创建 商品表 (商品号，商品名称，单价，库存，商品信息)
    sql = "CREATE TABLE IF NOT EXISTS product(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,name TEXT NOT NULL,price TEXT NOT NULL,"
          "number TEXT NOT NULL,information TEXT NOT NULL)";
    //使用query对象执行sql语句
    if(query.exec(sql))
    {
        qDebug()<<"creat table product ok"<<endl;
    }
    else {
        qDebug()<<"create table product error:"<<query.lastError().text()<<endl;
    }

    //种子数据：商品表为空时插入 50 条测试商品（5 个类别 × 10 编号）
    QSqlQuery seedQuery;
    seedQuery.exec("SELECT COUNT(*) FROM product");
    if(seedQuery.next() && seedQuery.value(0).toInt() == 0)
    {
        const char* sqlSeed =
            "WITH RECURSIVE cnt(x) AS (SELECT 1 UNION ALL SELECT x+1 FROM cnt WHERE x<50) "
            "INSERT INTO product (name, price, number, information) "
            "SELECT "
            "  CASE (x % 5) "
            "    WHEN 0 THEN '矿泉水' "
            "    WHEN 1 THEN '可乐' "
            "    WHEN 2 THEN '方便面' "
            "    WHEN 3 THEN '薯片' "
            "    ELSE '牛奶' "
            "  END || '_' || x, "
            "  printf('%.2f', 2.0 + (x % 20)), "
            "  10 + (x * 3 % 80), "
            "  '测试商品 ' || x "
            "FROM cnt;";
        if(!seedQuery.exec(sqlSeed))
        {
            qDebug()<<"seed product data error:"<<seedQuery.lastError().text()<<endl;
        }
        else
        {
            qDebug()<<"seed product data: inserted 50 rows"<<endl;
        }
    }

    //创建 打卡规则表（单行 id=1，8 个时间字段）
    sql = "CREATE TABLE IF NOT EXISTS attendance_rule("
          "id INTEGER NOT NULL PRIMARY KEY CHECK(id=1),"
          "sign_in_start TEXT NOT NULL, sign_in_end TEXT NOT NULL,"
          "late_start TEXT NOT NULL, late_end TEXT NOT NULL,"
          "early_start TEXT NOT NULL, early_end TEXT NOT NULL,"
          "sign_out_start TEXT NOT NULL, sign_out_end TEXT NOT NULL)";
    if(query.exec(sql))
    {
        qDebug()<<"creat table attendance_rule ok"<<endl;
    }
    else {
        qDebug()<<"create table attendance_rule error:"<<query.lastError().text()<<endl;
    }
    //插入默认打卡规则（已存在则忽略）：
    //签到 08:00-09:00 / 迟到 09:00-12:00 / 早退 12:00-18:00 / 签退 18:00-23:00
    query.prepare("INSERT OR IGNORE INTO attendance_rule "
                  "(id, sign_in_start, sign_in_end, late_start, late_end, "
                  " early_start, early_end, sign_out_start, sign_out_end) "
                  "VALUES (1, '08:00','09:00','09:00','12:00','12:00','18:00','18:00','23:00')");
    query.exec();

    //创建 问题反馈表（员工提交，管理员在异常反馈箱中查看）
    sql = "CREATE TABLE IF NOT EXISTS feedback("
          "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
          "card TEXT NOT NULL, name TEXT NOT NULL, department TEXT NOT NULL,"
          "title TEXT NOT NULL, type TEXT NOT NULL, content TEXT NOT NULL,"
          "f_time TEXT NOT NULL)";
    if(query.exec(sql))
    {
        qDebug()<<"creat table feedback ok"<<endl;
    }
    else {
        qDebug()<<"create table feedback error:"<<query.lastError().text()<<endl;
    }

    //创建 人脸特征表（员工人脸录入时写入，feature 为 Dlib 128 维特征序列化字节）
    sql = "CREATE TABLE IF NOT EXISTS face_info("
          "card TEXT NOT NULL PRIMARY KEY,"
          "name TEXT NOT NULL,"
          "feature BLOB NOT NULL,"
          "reg_time TEXT NOT NULL)";
    if(query.exec(sql))
    {
        qDebug()<<"creat table face_info ok"<<endl;
    }
    else {
        qDebug()<<"create table face_info error:"<<query.lastError().text()<<endl;
    }
}

bool MySql::checkAdmin(QString name, QString pwd)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM admin WHERE name=:name AND pwd=:pwd");
    query.bindValue(":name",name);
    query.bindValue(":pwd",pwd);
    if(query.exec())
    {
        qDebug()<<"执行查询语句成功"<<endl;
    }
    else {
        qDebug()<<"执行查询语句失败："<<query.lastError().text()<<endl;
    }
    //遍历查询结果（是否存在name和pwd）
    while(query.next())
    {
        return true;//存在 找到一行
    }
    return false;
}

bool MySql::checkCard(QString card)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM employee WHERE card=:card");
    query.bindValue(":card",card);
    if(query.exec())
    {
        qDebug()<<"执行查询语句成功"<<endl;
    }
    else {
        qDebug()<<"执行查询语句失败："<<query.lastError().text()<<endl;
    }
    //遍历查询结果（是否存在card）
    while(query.next())
    {
        return true;//存在 找到一行
    }
    return false;
}

void MySql::insertAdminData(QString name, QString pwd, int level)
{
    if(name.isEmpty())
    {
        qDebug()<<"用户名不能为空"<<endl;
        return;
    }
    if(pwd.isEmpty())
    {
        qDebug()<<"密码不能为空"<<endl;
        return;
    }
    QSqlQuery query;
    query.prepare("INSERT INTO admin (name, pwd, level) VALUES (:name, :pwd, :level)");
    query.bindValue(":name",name);
    query.bindValue(":pwd",pwd);
    query.bindValue(":level", level);
    if(query.exec())
    {
        qDebug()<<"执行插入语句成功，新纪录ID："<<query.lastInsertId().toInt()<<endl;;
    }
    else {
        qDebug()<<"执行插入语句失败："<<query.lastError().text()<<endl;;
    }
}

void MySql::insertEmployeeData(QString card, QString name, QString age, QString sex, QString department)
{
    QSqlQuery query;
    query.prepare("INSERT INTO employee VALUES (:card, :name, :age, :sex, :department,:balance)");
    query.bindValue(":card",card);
    query.bindValue(":name",name);
    query.bindValue(":age",age);
    query.bindValue(":sex",sex);
    query.bindValue(":department",department);
    query.bindValue(":balance","0");
    if(query.exec())
    {
        qDebug()<<"执行插入语句成功，新纪录ID："<<query.lastInsertId().toInt()<<endl;;
    }
    else {
        qDebug()<<"执行插入语句失败："<<query.lastError().text()<<endl;;
    }
}

QString MySql::getAttendanceStatus(QString card)
{
    QSqlQuery query;
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd");

    query.prepare("SELECT status FROM attendance "
                  "WHERE card = :card AND e_time LIKE :today "
                  "ORDER BY e_time DESC LIMIT 1");
    query.bindValue(":card",card);
    query.bindValue(":today",today + "%");

    bool ok = query.exec();
    qDebug() << "【查询考勤状态】执行结果：" << ok;
    if (!ok)
    {
        qDebug() << "SQL错误：" << query.lastError().text();
    }

    if(ok && query.next())
    {
        QString s = query.value("status").toString();
        qDebug() << "查到上一条打卡状态：" << s;
        return s;
    }
    qDebug() << "今日暂无打卡记录";
    return QString();
}

QStringList MySql::getTodayAttendanceStatus(QString card)
{
    QStringList list;
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QSqlQuery query;
    query.prepare("SELECT status FROM attendance "
                  "WHERE card = :card AND e_time LIKE :today");
    query.bindValue(":card", card);
    query.bindValue(":today", today + "%");
    if(query.exec())
    {
        while(query.next())
        {
            list << query.value("status").toString();
        }
    }
    return list;
}

bool MySql::insertAttendance(QString card,QString name,QString department,QString status)
{
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    qDebug()<<card<<endl;
    qDebug()<<name<<endl;
    qDebug()<<department<<endl;
    qDebug()<<time<<endl;
    qDebug()<<status<<endl;
    QSqlQuery query;
    query.prepare("INSERT INTO attendance (card, name, department, e_time, status) "
                  "VALUES (:card, :name, :department, :e_time, :status)");
    query.bindValue(":card",card);
    query.bindValue(":name",name);
    query.bindValue(":department",department);
    query.bindValue(":e_time",time);
    query.bindValue(":status",status);
    bool execResult = query.exec();
    if(!execResult)
    {
        qDebug() << "插入考勤失败原因：" << query.lastError().text();
    }
    return execResult;
}

QString MySql::findEmployeeName(QString card)
{
    QSqlQuery query;
    query.prepare("SELECT name FROM employee "
                  "WHERE card=:card");
    query.bindValue(":card",card);
    if(query.exec()&&query.next())
    {
        return query.value("name").toString();
    }
    return QString();
}

QString MySql::findEmployeeDepartment(QString card)
{
    QSqlQuery query;
    query.prepare("SELECT department FROM employee "
                  "WHERE card=:card");
    query.bindValue(":card",card);
    if(query.exec()&&query.next())
    {
        return query.value("department").toString();
    }
    return QString();
}

AttendanceRule MySql::getAttendanceRule()
{
    //默认规则（与建表时插入的默认值一致）
    AttendanceRule rule;
    rule.signInStart  = "08:00";
    rule.signInEnd    = "09:00";
    rule.lateStart    = "09:00";
    rule.lateEnd      = "12:00";
    rule.earlyStart   = "12:00";
    rule.earlyEnd     = "18:00";
    rule.signOutStart = "18:00";
    rule.signOutEnd   = "23:00";

    QSqlQuery query;
    if(query.exec("SELECT * FROM attendance_rule WHERE id=1"))
    {
        if(query.next())
        {
            rule.signInStart  = query.value("sign_in_start").toString();
            rule.signInEnd    = query.value("sign_in_end").toString();
            rule.lateStart    = query.value("late_start").toString();
            rule.lateEnd      = query.value("late_end").toString();
            rule.earlyStart   = query.value("early_start").toString();
            rule.earlyEnd     = query.value("early_end").toString();
            rule.signOutStart = query.value("sign_out_start").toString();
            rule.signOutEnd   = query.value("sign_out_end").toString();
        }
    }
    else {
        qDebug()<<"get attendance_rule error:"<<query.lastError().text()<<endl;
    }
    return rule;
}

bool MySql::updateAttendanceRule(const AttendanceRule &rule)
{
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO attendance_rule "
                  "(id, sign_in_start, sign_in_end, late_start, late_end, "
                  " early_start, early_end, sign_out_start, sign_out_end) "
                  "VALUES (1, :s1, :s2, :l1, :l2, :e1, :e2, :o1, :o2)");
    query.bindValue(":s1", rule.signInStart);
    query.bindValue(":s2", rule.signInEnd);
    query.bindValue(":l1", rule.lateStart);
    query.bindValue(":l2", rule.lateEnd);
    query.bindValue(":e1", rule.earlyStart);
    query.bindValue(":e2", rule.earlyEnd);
    query.bindValue(":o1", rule.signOutStart);
    query.bindValue(":o2", rule.signOutEnd);
    if(!query.exec())
    {
        qDebug()<<"update attendance_rule error:"<<query.lastError().text()<<endl;
        return false;
    }
    return true;
}

bool MySql::insertFeedback(const QString &card, const QString &name, const QString &department,
                           const QString &title, const QString &type, const QString &content)
{
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QSqlQuery query;
    query.prepare("INSERT INTO feedback(card, name, department, title, type, content, f_time) "
                  "VALUES (:card, :name, :dept, :title, :type, :content, :time)");
    query.bindValue(":card", card);
    query.bindValue(":name", name);
    query.bindValue(":dept", department);
    query.bindValue(":title", title);
    query.bindValue(":type", type);
    query.bindValue(":content", content);
    query.bindValue(":time", time);
    if(!query.exec())
    {
        qDebug()<<"insert feedback error:"<<query.lastError().text()<<endl;
        return false;
    }
    return true;
}

bool MySql::saveFaceInfo(const QString &card, const QString &name, const QByteArray &feature)
{
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QSqlQuery query;
    //已存在则更新，否则插入
    query.prepare("INSERT OR REPLACE INTO face_info(card, name, feature, reg_time) "
                  "VALUES (:card, :name, :feature, :time)");
    query.bindValue(":card", card);
    query.bindValue(":name", name);
    query.bindValue(":feature", feature);
    query.bindValue(":time", time);
    if(!query.exec())
    {
        qDebug()<<"save face_info error:"<<query.lastError().text()<<endl;
        return false;
    }
    return true;
}

bool MySql::deleteFaceInfo(const QString &card)
{
    QSqlQuery query;
    query.prepare("DELETE FROM face_info WHERE card=:card");
    query.bindValue(":card", card);
    if(!query.exec())
    {
        qDebug()<<"delete face_info error:"<<query.lastError().text()<<endl;
        return false;
    }
    return true;
}

QList<FaceRecord> MySql::getAllFaceInfo()
{
    QList<FaceRecord> result;
    QSqlQuery query;
    if(!query.exec("SELECT card, name, feature FROM face_info"))
    {
        qDebug()<<"query face_info error:"<<query.lastError().text()<<endl;
        return result;
    }
    while(query.next())
    {
        FaceRecord r;
        r.card = query.value("card").toString();
        r.name = query.value("name").toString();
        r.feature = query.value("feature").toByteArray();
        result << r;
    }
    return result;
}

bool MySql::hasFaceInfo(const QString &card)
{
    QSqlQuery query;
    query.prepare("SELECT 1 FROM face_info WHERE card=:card");
    query.bindValue(":card", card);
    return query.exec() && query.next();
}

int MySql::findAdminLevel(QString name)
{
    QSqlQuery query;
    query.prepare("SELECT level FROM admin WHERE name=:name");
    query.bindValue(":name",name);
    if(query.exec()&&query.next())
    {
        return query.value("level").toInt();
    }
    return 0;
}

bool MySql::updateAdminLevel(QString name,int newLevel)
{
    //一级管理员（root）不允许被修改权限
    if(name == "root")
    {
        qDebug()<<"root 一级管理员权限不可修改"<<endl;
        return false;
    }
    if(newLevel != 2 && newLevel != 3)
    {
        qDebug()<<"非法权限等级："<<newLevel<<endl;
        return false;
    }
    QSqlQuery query;
    query.prepare("UPDATE admin SET level=:level WHERE name=:name");
    query.bindValue(":level",newLevel);
    query.bindValue(":name",name);
    return query.exec();
}
