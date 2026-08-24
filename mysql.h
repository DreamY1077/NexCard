#ifndef MYSQL_H
#define MYSQL_H

#include <QObject>
#include <QSqlDatabase>//数据库核心头文件
#include <QSqlQuery>//数据库操作语句头文件
#include <QSqlError>//数据库错误信息
#include <QStringList>
#include <QList>
#include <QByteArray>

//打卡规则（4 个时段，时间格式 "HH:mm"）
//默认：签到 08:00-09:00 / 迟到 09:00-12:00 / 早退 12:00-18:00 / 签退 18:00-23:00
struct AttendanceRule
{
    QString signInStart;   //签到开始
    QString signInEnd;     //签到结束
    QString lateStart;     //迟到开始
    QString lateEnd;       //迟到结束
    QString earlyStart;    //早退开始
    QString earlyEnd;      //早退结束
    QString signOutStart;  //签退开始
    QString signOutEnd;    //签退结束
};

//人脸特征记录（员工人脸注册后入库，feature 为 Dlib 128 维 float 序列化字节）
struct FaceRecord
{
    QString card;          //员工卡号
    QString name;          //姓名（冗余便于显示）
    QByteArray feature;    //128 维特征（float32 × 128 = 512 字节）
};

class MySql : public QObject
{
    Q_OBJECT

private:
    //设置单例模式
    //1、私有化 构造函数与析构函数
    explicit MySql(QObject *parent = nullptr);
    ~MySql();
    //2、设置静态 私有 只读 指针变量 ，保存唯一实例地址
    //初始化数据库
    MySql(QString dbName,QObject *parent = nullptr);
    static MySql * const p;
public:
    //3、设置静态 公共 方法， 提供获取唯一实例地址的方法
    static MySql *getMySql(void);

    //创建数据库
    void creatMySql();
    //确认账号、密码
    bool checkAdmin(QString name,QString pwd);
    //确认卡号
    bool checkCard(QString card);
    //添加管理员账号、密码（level：1一级 2二级 3三级，新注册默认三级）
    void insertAdminData(QString name,QString pwd,int level = 3);
    //查询管理员权限等级（1一级 2二级 3三级，未找到返回0）
    int findAdminLevel(QString name);
    //修改管理员权限等级（root 一级管理员不允许被修改）
    bool updateAdminLevel(QString name,int newLevel);
    //员工注册
    void insertEmployeeData(QString card,QString name,QString age,QString sex,QString department);
    //获取员工上一次考勤状态
    QString getAttendanceStatus(QString card);
    //获取某卡号当天的全部考勤状态（用于当天打卡去重）
    QStringList getTodayAttendanceStatus(QString card);
    //插入员工考勤打卡
    bool insertAttendance(QString card,QString name,QString department,QString status);
    //查询员工姓名
    QString findEmployeeName(QString card);
    //查询员工部门
    QString findEmployeeDepartment(QString card);
    //获取打卡规则（不存在则返回默认规则）
    AttendanceRule getAttendanceRule();
    //更新打卡规则（id=1 单行）
    bool updateAttendanceRule(const AttendanceRule &rule);
    //提交问题反馈（员工端）
    bool insertFeedback(const QString &card, const QString &name, const QString &department,
                        const QString &title, const QString &type, const QString &content);

    //========== 人脸特征（face_info 表）==========
    //保存/更新某员工的人脸特征（feature 为 Dlib 128 维 float 序列化字节）
    bool saveFaceInfo(const QString &card, const QString &name, const QByteArray &feature);
    //删除某员工的人脸特征（员工被删除时同步调用）
    bool deleteFaceInfo(const QString &card);
    //获取全部人脸特征（打卡识别时逐条比对）
    QList<FaceRecord> getAllFaceInfo();
    //某卡号是否已录入人脸
    bool hasFaceInfo(const QString &card);

signals:

public slots:
};

#endif // MYSQL_H
