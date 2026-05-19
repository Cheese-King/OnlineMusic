#include "desktoplyric.h"
#include <QBitmap>//QBitmap 是一个用于处理位图的类，位图通常用于创建窗口的遮罩效果。
#include <QPainter>//QPainter 是 Qt 中用于进行 2D 绘图的类，可以用来绘制各种图形，如矩形、椭圆等。

DesktopLyric::DesktopLyric(QQuickWindow * parent) : QQuickWindow(parent)
{

     //qt::winodwstayontophint表示该窗口会始终显示在其他窗口的顶部；Qt::FramelessWindowHint 表示该窗口没有边框和标题栏。
    this->setFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

    this->setColor(Qt::transparent);//this->setColor(Qt::transparent);：将窗口的背景颜色设置为透明，这样窗口就可以显示出背后的内容。
    this->show();
}

void DesktopLyric::setFillMask(const int x,const int y,const int w,const int h)
{
//    qDebug() << "x: " << x << " y: " << y << " w: " <<w  << " h: " <<h;
    QBitmap bitmap(this->width(),this->height());//创建一个与窗口大小相同的位图对象
    QPainter painter(&bitmap);//创建一个 QPainter 对象，用于在位图上进行绘图操

    painter.setBrush(Qt::color0);//color0 黑色
    painter.drawRect(0, 0, this->width(), this->height());//坐标0，0开始等宽等高绘制
    painter.setBrush(Qt::color1); // 设置中间区域为不透明
    painter.drawRect(x, y, w, h);//在指定的位置绘制一个白色的矩形。
    painter.end();

    this->setMask(bitmap);//将创建好的位图设置为窗口的遮罩，这样窗口只有白色矩形区域是可见的，其他区域是透明的。
}


void DesktopLyric::setFillMask(const QJsonValue &o)
{

    QBitmap bitmap(this->width(),this->height());
    QPainter painter(&bitmap);
    painter.setBrush(Qt::color0);
    painter.drawRect(0, 0, this->width(), this->height());

    QJsonArray jsonArr = o.toArray();//尝试将传入的 QJsonValue 对象转换为 QJsonArray 对象

    if(jsonArr.isEmpty())//jsonArr为空
    {
//        qDebug() << "x: " << o["x"].toDouble() << " y: " << o["y"].toDouble() << " w: " <<o["w"].toDouble()  << " h: " <<o["h"].toDouble();
        painter.setBrush(Qt::color1);//将画笔的画刷设置为 Qt::color1，即不透明状态

        painter.drawRect(o["x"].toDouble(), o["y"].toDouble(), o["w"].toDouble(), o["h"].toDouble());//从 QJsonValue 对象中提取 x、y、w、h 四个属性值

    }
    else
    {
        for(int i = 0; i < jsonArr.count();i++ )//遍历循环
        {
            QJsonObject obj = jsonArr[i].toObject();
            painter.setBrush(Qt::color1);
            painter.drawRect(obj["x"].toDouble(), obj["y"].toDouble(), obj["w"].toDouble(), obj["h"].toDouble());
//            qDebug() << "cnt: " << i << " x: " << obj["x"].toDouble() << " y: " << obj["y"].toDouble() << " w: " <<obj["w"].toDouble()  << " h: " <<obj["h"].toDouble();
        }
    }
    painter.end();
    this->setMask(bitmap);
}

void DesktopLyric::mousePressEvent(QMouseEvent *event)
{
    this->m_startPos = event->globalPosition().toPoint();//event->globalPosition() 返回鼠标按下时的全局位置（相对于屏幕）

    this->m_oldPos = position();//position() 函数返回窗口当前的位置

    event->ignore();//忽略该事件，意味着该事件会继续传递给父对象或其他可能处理它的对象。

    QQuickWindow::mousePressEvent(event);////调用基类事件处理，确保qml的交互功能
}

void DesktopLyric::mouseReleaseEvent(QMouseEvent *event)
{
    this->m_oldPos = this->position();

    QQuickWindow::mouseReleaseEvent(event);
}

void DesktopLyric::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        //this->m_oldPos - this->m_startPos 计算出窗口相对于鼠标按下时的偏移量，再加上当前鼠标的全局位置 event->globalPosition().toPoint()，得到窗口的新位置。
        this->setPosition(this->m_oldPos - this->m_startPos + event->globalPosition().toPoint());
    }

    QQuickWindow::mouseMoveEvent(event);
}
