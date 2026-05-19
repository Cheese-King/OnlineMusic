#include "downloadtaskthread.h"

Downloadtaskthread::Downloadtaskthread()
{

}

Downloadtaskthread::Downloadtaskthread(const QString &url, const QString &savePath,const QString &fileName)
{
    m_url = url;//保存下载url
    m_fileName = fileName;//保存文件名

    m_downloadTask = new DownloadTask(url,savePath,fileName);// 创建具体的下载任务实例

    //连接DowmloadTask，改变下载状态
    connect(m_downloadTask,&DownloadTask::statusChanged,this,&Downloadtaskthread::setStatus);

    //进度条改变
    connect(m_downloadTask,&DownloadTask::progressValueChanged,this,&Downloadtaskthread::setProgressValue);

    //中继信息
    connect(m_downloadTask,&DownloadTask::downloadRelay,this,&Downloadtaskthread::downloadRelay);

    //错误
    connect(m_downloadTask,&DownloadTask::downloadError,this,&Downloadtaskthread::downloadError);
}

Downloadtaskthread::~Downloadtaskthread()
{
    // 内存清理
    if(m_downloadTask)//若存在下载状态
    {
        if(m_downloadTask->status() == DownloadTask::TaskStatus::Loading)//如果下载状态
        {
            m_downloadTask->pauseDownload();//暂停
        }
        m_downloadTask->deleteLater();//删除下载任务
    }
    if(m_thread)//存在线程
    {
        if(m_thread->isRunning())//如果在运行
        {
            m_thread->quit();//通知线程退出循环
            m_thread->wait();//阻塞 等待线程结束，避免强制 退出
        }
        m_thread->deleteLater();
    }
}

int Downloadtaskthread::status() const//获取当前状态
{
    return m_status;
}

void Downloadtaskthread::setStatus(int newStatus)
{
    if (m_status == newStatus)
        return;

    m_status = newStatus;
    emit statusChanged(m_status);// 发射状态变化信号（通知外部状态更新）
}

QString Downloadtaskthread::savePath() const//获取保存路径
{
    return m_savePath;
}

void Downloadtaskthread::setSavePath(const QString &newSavePath)//更新保存路径
{
    if (m_savePath == newSavePath)
        return;
    m_savePath = newSavePath;
    emit savePathChanged();// 发射状态变化信号（通知外部状态更新）
}


void Downloadtaskthread::start()
{
    std::unique_lock<std::mutex> locker(m_mutex);//互斥锁，确保同一时间只有一个线程能修改共享资源，避免竞态条件，保证线程安全

    if(m_thread == nullptr)
    {
        m_thread = new QThread();
    }
    qDebug() << "启动" << m_thread->thread() << " 启动线程: " << QThread::currentThread();

        if(m_thread != m_downloadTask->thread())//若当前线程不是下载任务的线程
    {


        m_downloadTask->moveToThread(m_thread);//将下载任务对象移动到新线程（关键操作）

        //线程启动时触发下载
        connect(m_thread,&QThread::started,m_downloadTask,&DownloadTask::startDownload);

                // 信号发送者：下载任务对象  发送的信号：下载完成时发出的信号      信号接收者：线程对象        接收的槽函数：Lambda表达式
        connect(m_downloadTask,         &DownloadTask::downloadFinished,      m_thread,           [=](){
                     qDebug() << "已退出线程: "<< QThread::currentThread();
                     m_thread->quit();//通知线程退出事件循环
                     m_thread->wait();//等待
//                   m_downloadTask->moveToThread(this->thread());
        });
    }
    m_thread->start();//启动线程
}

void Downloadtaskthread::pause()//暂停
{
    std::lock_guard<std::mutex> locker(m_mutex);//互斥锁

    if(m_thread->isRunning())//当线程正在运行
    {
        m_downloadTask->pauseDownload();//进行downloadtask的pauseDownload（）函数，84行
    }
}

double Downloadtaskthread::getProgressValue() const//获取当前进度值
{
    return m_progressValue;
}

//更新当前进度
void Downloadtaskthread::setProgressValue(double newProgressValue)
{
    if (qFuzzyCompare(m_progressValue, newProgressValue))//浮点值近似比较（避免精度误差误判）
        return;

    m_progressValue = newProgressValue;//更新进度

    emit progressValueChanged(m_progressValue);//发射进度变化信号
}

//获取文件名
QString Downloadtaskthread::getFileName() const
{
    return m_fileName;
}

//更新文件名
void Downloadtaskthread::setFileName(const QString &newFileName)
{
    if (m_fileName == newFileName)
        return;
    m_fileName = newFileName;
    emit fileNameChanged(m_fileName);
}
