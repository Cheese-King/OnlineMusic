#include "downloadtask.h"


DownloadTask::DownloadTask()
{

}

DownloadTask::DownloadTask(QObject * parent) : QObject(parent) {

}

DownloadTask::DownloadTask(const QString &url, const QString &savePath,const QString &fileName) :
    m_url(url) , m_savePath(savePath) , m_fileName(fileName)
{

}

DownloadTask::~DownloadTask()
{
    if (m_reply)//任务进度
    {
        m_reply->deleteLater();//若不为空安全删除
    }
    if (m_manger)//网络请求
    {
        m_manger->deleteLater();//若不为空安全删除
    }

    qDebug() << " 已析构" ;
}

void DownloadTask::startDownload()
{
    qDebug() << "运行线程: "<< QThread::currentThreadId();//输出当前线程的ID

    if(m_status == TaskStatus::Loading)//下载状态，返回
        return;

    if(m_manger == nullptr)//判断是否为空
    {
        m_manger = new QNetworkAccessManager(this);//为空创建QNetworkAccessManager对象
    }

    // 设置状态
    setStatus(TaskStatus::Loading);//若处于下载或者刚创建一个QNetworkAccessManager对象，则认为任务正在下载，状态为loading
    m_paused = false;// 将暂停标志设置为false

    // 请求网络
    QNetworkRequest request;//设置一个网络请求对象

    request.setUrl(QUrl(m_url));//设置请求url

    request.setRawHeader(QByteArray("Range"), QString("bytes=" + QString::number(m_bufferByte) + "-").toLocal8Bit());

    m_reply = m_manger->get(request);
    qDebug() << "音乐"+ m_fileName + "开始下载"  ;

    // 准备写入文件
    QString fullPath = m_savePath + "/" + m_fileName;
    // 检查目录是否存在，如果不存在则创建
    QFileInfo fileInfo(fullPath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_file.setFileName(fullPath);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_reply->abort();
        return;
    }

    // 连接下载完成信号
    connect(m_reply,&QNetworkReply::finished,this,&DownloadTask::onFinished,
            Qt::UniqueConnection);
    // 连接可有新数据获取时
    connect(m_reply,&QNetworkReply::readyRead,this,&DownloadTask::onDownloadReadyRead,
            Qt::UniqueConnection);
    // 连接下载进度信号
    connect(m_reply,&QNetworkReply::downloadProgress,this,&DownloadTask::onDownloadProgress,
            Qt::UniqueConnection);
}

void DownloadTask::pauseDownload()
{
    if(m_status != TaskStatus::Loading) return;
    m_paused = true;
    setStatus(TaskStatus::Paused);
    if(m_reply) {
        m_reply->abort();
    }
    qDebug() << "暂停下载音乐：" << m_fileName << " 已下载byte: " << m_progressByte + m_bufferByte;
}

void DownloadTask::cancelDownload()
{
    setStatus(TaskStatus::Cancel);
    if(m_reply) {
        m_reply->abort();
    }

    qDebug() << "取消下载音乐：" << m_fileName;
}

void DownloadTask::onDownloadReadyRead()
{
    QByteArray data = m_reply->readAll(); // 读取缓冲区数据
    m_progressByte += data.length(); // 更新下载的数据长度
    m_file.write(data); // 把新的缓冲数据读入文件
//    qDebug() << "本次下载的数据长度~" << data.length() << " 累计下载数据 " << m_progressByte+m_bufferByte <<
//        "总数据：";
}

void DownloadTask::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        double progress =  (double)(bytesReceived+m_bufferByte) / (bytesTotal+m_bufferByte) * 100;
        setProgressValue(progress);
    }
}
void DownloadTask::onFinished()//下载操作完成
{
    m_file.close();

    auto noError_func = [&]()//下载成功
    {
        qDebug() << "获取数据文件大小：" <<  m_reply->header(QNetworkRequest::ContentLengthHeader).toInt() + m_bufferByte;//获取文件总大小（原始大小 + 已缓冲字节）

        setStatus(TaskStatus::Ready);//设置状态为ready

        QString suffix = m_url.sliced(m_url.lastIndexOf('.',-1));//拓展名，从最后一位到 ‘.’

        QFileInfo fileInfo(m_file);//获取文件信息

        fileInfo.suffix();//获取拓展名

        QString newFileName = fileInfo.absoluteFilePath() + suffix;//绝对路径+拓展名

        if(QFile::exists(m_file.fileName()))//判断零食文件文件名是否存在
        {
            if(m_file.rename(newFileName))//用完整文件名命名
            {

                qDebug() << "音乐: \n"+ m_fileName + "下载完成" + m_file.fileName() ;//前面为变量，后面为绝对路径
                emit downloadRelay(m_fileName,"file:///" + m_file.fileName());

            }
            else
            {
                setStatus(TaskStatus::Error);//设置Error状态
                m_file.remove();//删除临时文件
                qDebug() << "音乐: \n"+ m_fileName + "写入失败"  ;
                emit downloadError("写入失败",m_fileName);
            }
        }
    };
    auto defaultError_func = [&]()//处理普通错误（非取消错误）
    {
        m_bufferByte += m_progressByte;//记录已下载字节

        setStatus(TaskStatus::Error);//设置为错误

//        m_progressByte = 0;
//        m_bufferByte = 0;
//        m_file.remove();
        qDebug() << "音乐: " << m_fileName + "下载失败" << m_reply->errorString() ;//进度error
        emit downloadError(m_reply->errorString(),m_fileName);
    };

    auto canceledError_func = [&]()//处理取消或者暂停
    {
        if(m_status == TaskStatus::Paused)
        {
            m_bufferByte += m_progressByte;//记录已下载字节
        }

        if(m_status == TaskStatus::Cancel)
        {
            m_progressByte = 0;
            m_bufferByte = 0;

            m_file.remove();
        }
    };

    //错误码分发
    switch(m_reply->error()) {

        case QNetworkReply::NoError:
            noError_func();//无错误，下载成功
            break;

        case QNetworkReply::OperationCanceledError://用户调用abort()或手动取消
            canceledError_func();// 区分暂停/取消，处理进度和文件
            break;

        default:
            defaultError_func();
            break;
    }

    m_progressByte = 0;// 重置本次下载的临时进度

    m_reply->deleteLater();//安全删除本次进度

    m_reply = nullptr;// 置空指针，防止野指针

    // 发送下载成功信号
    emit downloadFinished();// 通知上层逻辑任务已结束（无论成功或失败）

}

double DownloadTask::progressValue() const// 获取下载进度值的成员函数
{
    return m_progressValue;// 返回成员变量 m_progressValue，表示当前的下载进度值。
}

void DownloadTask::setProgressValue(double newProgressValue)
{
    if (qFuzzyCompare(m_progressValue, newProgressValue))//检查新的进度值与当前进度值是否近似相等
        return;

    m_progressValue = newProgressValue;//更新变量
    emit progressValueChanged(m_progressValue);
}

//获取状态函数
DownloadTask::TaskStatus DownloadTask::status() const
{
    return m_status;
}

//更新状态函数
void DownloadTask::setStatus(TaskStatus newStatus)
{
    if (m_status == newStatus)//判断新旧状态是否相等
        return;//相等直接返回不改变，优化性能

    m_status = newStatus;
    emit statusChanged(m_status);
}

// 获取下载链接的成员函数
QString DownloadTask::url() const
{
    return m_url;
}

//更新下载链接
void DownloadTask::setUrl(const QString &newUrl)
{
    if (m_url == newUrl)
        return;
    m_url = newUrl;
    emit urlChanged(m_url);
}


//文件名
QString DownloadTask::fileName() const
{
    return m_fileName;
}


//更新文件名
void DownloadTask::setFileName(const QString &newfileName)
{
    if (m_fileName == newfileName)
        return;
    m_fileName = newfileName;
    emit fileNameChanged(m_fileName);
}

//保存路径
QString DownloadTask::savePath() const
{
    return m_savePath;
}

//更新路径
void DownloadTask::setSavePath(const QString &newSavePath)
{
    if (m_savePath == newSavePath)
        return;
    m_savePath = newSavePath;
    emit savePathChanged(m_savePath);
}


