#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include <QNetworkAccessManager>                  //网络访问管理
#include <QNetworkReply>                          //网络响应处理
#include <QObject>                                //qt对象基类
#include <QDebug>                                 //调试输出
#include <QFile>                                  //文件操作
#include <QDir>                                   //目录操作
#include <QFileInfo>                              //文件信息
#include <QThread>                                //线程

class DownloadTask  : public QObject {
    Q_OBJECT

    //            属性      name         指定一个成员函数用于读取          指定一个函数用于设置该属性的值    指定一个信号属性值变换的时候触发   表明该属性不能在派生类中被重写。
    Q_PROPERTY(TaskStatus status            READ status                 WRITE setStatus              NOTIFY statusChanged       FINAL)
    Q_PROPERTY(QString      url             READ url                    WRITE setUrl                 NOTIFY urlChanged          FINAL)
    Q_PROPERTY(QString fileName             READ fileName               WRITE setFileName            NOTIFY fileNameChanged     FINAL)
    Q_PROPERTY(QString savePath             READ savePath               WRITE setSavePath            NOTIFY savePathChanged     FINAL)
    Q_PROPERTY(double progressValue         READ progressValue                                       NOTIFY progressValueChanged   FINAL)

public:
    enum TaskStatus//定义了一个枚举类型 TaskStatus，用于表示下载任务的不同状态。使用 Q_ENUM 宏将枚举类型注册到 Qt 的元对象系统中，以便在 QML 等环境中使用。
    {
        Normal = 0,
        Loading,
        Paused,
        Cancel,
        Ready,
        Error
    };
    Q_ENUM(TaskStatus);

    explicit DownloadTask();

    //带父对象的构造函数
    DownloadTask(QObject *parent);

    //地址，路劲，文件名
    DownloadTask(const QString &url,const QString &savePath,const QString &fileName);
    ~DownloadTask();//析构释放资源
    //公共办法
    Q_INVOKABLE void startDownload();//下载
    Q_INVOKABLE void pauseDownload();//暂停
    Q_INVOKABLE void cancelDownload();//取消下载

    TaskStatus status() const;
    void setStatus(TaskStatus newStatus);//获取DowmLoadTask状态

    QString url() const;
    void setUrl(const QString &newUrl);//下载链接

    QString fileName() const;
    void setFileName(const QString &newfileName);//文件名

    QString savePath() const;
    void setSavePath(const QString &newSavePath);//保存路径


    double progressValue() const;
    void setProgressValue(double newProgressValue);//下载进度

signals:
    // 下载成功信号，当下载成功时发出该信号，携带下载的文件名和保存路径。
    void downloadRelay(const QString &filename, const QString& savePath);
    // 下载完成
    void downloadFinished();
    // 下载错误
    void downloadError(const QString &error, const QString &filename);

    void statusChanged(int);//当任务状态改变时发出该信号，携带新的状态值

    void urlChanged(QString);//当下载链接改变时发出该信号，携带新的链接

    void fileNameChanged(QString);//当文件名改变时发出该信号，携带新的文件名

    void savePathChanged(QString);//保存路径

    void progressValueChanged(double);

public slots:

    //定义槽函数
    void onDownloadReadyRead();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onFinished();
private:

    QNetworkAccessManager *m_manger = nullptr;
    QNetworkReply *m_reply = nullptr;
    QFile m_file;
    QString m_url = "";
    QString m_fileName = "";
    QString m_savePath = "";
    double m_progressValue = 0;//下载进度百分比
    TaskStatus m_status = TaskStatus::Normal;
    int m_progressByte = 0;//本次下载会话中已接收的数据字节数
    int m_bufferByte = 0;//累计已下载的字节数
    bool m_paused = true;

};


#endif // DOWNLOADTASK_H
