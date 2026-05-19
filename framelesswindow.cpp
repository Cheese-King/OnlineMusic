#include "framelesswindow.h"

FramelessWindow::FramelessWindow(QWindow * parent) : QQuickWindow(parent)
{

    this->setFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    //Qt::Window：声明这是一个顶层窗口。
    //Qt::FramelessWindowHint：移除默认窗口边框和标题栏（无框窗口）。
    //Qt::WindowMinMaxButtonsHint：保留最小化和最大化按钮（需配合自定义绘制）。
    this->setColor(Qt::transparent);//设置窗口背景为透明
    this->show();
}

void FramelessWindow::mousePressEvent(QMouseEvent *event)//鼠标按压窗口
{
    this->start_pos = event->globalPosition();//记录鼠标按下时的全局坐标
    this->old_pos = this->position();//old_pos记录当前窗口位置
    this->old_size = this->size();//old_size记录当前窗口大小

    event->ignore();//允许事件event接着传递

    QQuickWindow::mousePressEvent(event);//调用基类事件处理，确保qml的交互功能
}

void FramelessWindow::mouseReleaseEvent(QMouseEvent *event)//鼠标释放
{
    this->old_pos = this->position();//更新窗口坐标
    QQuickWindow::mouseReleaseEvent(event);//调用基类事件处理，确保qml的交互功能
}

void FramelessWindow::mouseMoveEvent(QMouseEvent *event)//鼠标移动事件
{
    QPointF pos = event->position();//存储两个double型坐标，继承自QPoint
    if(event->buttons() & Qt::LeftButton)//按下左键
    {
        // 改变大小
        this->setWindowGeometry(event->globalPosition());//根据鼠标的屏幕全局坐标，直接设置窗口的位置
        if(pos.y() <= 80) {
            this->setPosition((this->old_pos - this->start_pos + event->globalPosition()).toPoint());
            //计算窗口位置，当前窗口坐标+鼠标偏移量，偏移量等于
        }

    }
    else
    {
        this->mouse_pos = this->getMousePos(pos);// 获取鼠标在边框的位置（枚举值）
        this->setCursorIcon();// 根据位置设置调整大小的光标
    }
    QQuickWindow::mouseMoveEvent(event);//调用基类事件处理，确保qml的交互功能
}

void FramelessWindow::setWindowGeometry(const QPointF &pos)//窗口几何设置函数,pos来自globalPosition()？
{
    QPointF offset = this->start_pos - pos;//计算鼠标移动的偏移量，存x和y两个值

    if(offset.x() == 0 && offset.y() == 0) return;//无偏移直接返回

    static auto set_geometry_func = [this](const QSize & size,const QPointF &pos)
    //lambda函数
    //被声明为静态，不能访问非静态成员，通过this访问，this指向FramelessWindow实例
    //lambda函数后面的影响外部变量，但是受const控制
    {
        QPointF t_pos = this->old_pos;//临时位置

        QSize t_size = minimumSize();//临时大小，初始为最小size
            // 宽度调整逻辑：若新宽度大于最小宽度，使用计算值；否则根据鼠标位置调整位置
        if(size.width() > minimumWidth())
        {
            t_pos.setX(pos.x());//直接使用鼠标当前全局 x 坐标作为新窗口左上角 x 坐标
            t_size.setWidth(size.width());// 更新窗口宽度为新计算值
        }
        else if(this->mouse_pos == LEFT)//拖动左框边
        {
            t_pos.setX(this->old_pos.x()+this->old_size.width()-minimumWidth());//计算，调整前的位置+调整前的宽度-最小宽度
        }

        if(size.height() > minimumHeight())
        {
            t_pos.setY(pos.y());
            t_size.setHeight(size.height());
        }

        else if(this->mouse_pos == TOP)
        {
            t_pos.setY(this->old_pos.y()+this->old_size.height()-minimumHeight());
        }

        this->setGeometry(t_pos.x(),t_pos.y(),t_size.width(),t_size.height());
        this->update();
    };

    switch (this->mouse_pos) //看鼠标位置，选择调整方式
    {
    case TOPLEFT: set_geometry_func(this->old_size + QSize(offset.x(),offset.y()),
                          this->old_pos - offset);//左上角，同时设置横竖坐标，大小
        break;
    case TOP: set_geometry_func(this->old_size + QSize(0,offset.y()),
                          this->old_pos - QPointF(0,offset.y()));
        break;
    case TOPRIGHT: set_geometry_func(this->old_size - QSize(offset.x(),-offset.y()),
                          this->old_pos - QPointF(0,offset.y()));
        break;
    case LEFT: set_geometry_func(this->old_size + QSize(offset.x(),0),
                          this->old_pos - QPointF(offset.x(),0));
        break;
    case RIGHT: set_geometry_func(this->old_size - QSize(offset.x(),0),
                          this->position());
        break;
    case BOTTOMLEFT: set_geometry_func(this->old_size + QSize(offset.x(),-offset.y()),
                          this->old_pos - QPointF(offset.x(),0));
        break;
    case BOTTOM: set_geometry_func(this->old_size + QSize(0,-offset.y()),
                          this->position());
        break;
    case BOTTOMRIGHT: set_geometry_func(this->old_size - QSize(offset.x(),offset.y()),
                          this->position());
        break;
    default:
        break;
    }

}

void FramelessWindow::setCursorIcon()//光标设置函数
{
    static bool isSet = false;//避免光标重复设置
    switch (this->mouse_pos)
    {

        //左上角和右下角
    case TOPLEFT:
    case BOTTOMRIGHT:
        this->setCursor(Qt::SizeFDiagCursor);//调用 setCursor 函数将当前窗口的光标，Qt::SizeFDiagCursor，这是一个倾斜的双向箭头光标
        isSet = true;
        break;

        //上边界和下边界
    case TOP:
    case BOTTOM:
        this->setCursor(Qt::SizeVerCursor);//Qt::SizeVerCursor，这是一个垂直方向的双向箭头光标，表明可以沿垂直方向调整窗口大小。
        isSet = true;
        break;

        //右上角和左下角
    case TOPRIGHT:
    case BOTTOMLEFT:
        this->setCursor(Qt::SizeBDiagCursor);
        isSet = true;
        break;

        //水平方向，左右
    case LEFT:
    case RIGHT:
        this->setCursor(Qt::SizeHorCursor);//Qt::SizeHorCursor,水平方向箭头
        isSet = true;
        break;

    default:
        if(isSet) {
            isSet = false;
            this->unsetCursor();
        }
        break;
    }
}

FramelessWindow::MousePosition FramelessWindow::getMousePos(const QPointF &pos)//鼠标位置检测
{
    int x = pos.x();
    int y = pos.y();
    int w = this->width();
    int h = this->height();

    MousePosition mouse_pos = NORMAL;//表示默认情况下鼠标处于窗口的普通区域。非边框，step=8

    if(x >= 0 && x <= this->step && y >= 0 && y <= this->step) {
        mouse_pos = TOPLEFT;
    } else if(x > this->step && x < (w - this->step) && y >= 0 && y <= this->step) {
        mouse_pos = TOP;
    } else if(x >= (w - this->step) && x <= w && y >= 0 && y <= this->step) {
        mouse_pos = TOPRIGHT;
    } else if(x >= 0 && x <= this->step &&  y > this->step && y < (h - this->step)) {
        mouse_pos = LEFT;
    } else if(x >= (w - this->step) && x <= w &&  y > this->step && y < (h - this->step)) {
        mouse_pos = RIGHT;
    } else if(x >= 0 && x <= this->step &&  y >= (h - this->step) && y < h) {
        mouse_pos = BOTTOMLEFT;
    } else if(x > this->step && x < (w - this->step) && y >= (h - this->step) && y <= h) {
        mouse_pos = BOTTOM;
    } else if(x >= (w - this->step) && x <= w && y >= (h - this->step) && y <= h) {
        mouse_pos = BOTTOMRIGHT;
    }
    return mouse_pos;//返回检测鼠标的枚举值

}

FramelessWindow::MousePosition FramelessWindow::getMouse_pos() const//获取鼠标位置
{
    return mouse_pos;
}

void FramelessWindow::setMouse_pos(MousePosition newMouse_pos)
{
    if (mouse_pos == newMouse_pos)//判断前后鼠标前后位置是否改变，若值相等则直接返回，不做操作
        return;
    mouse_pos = newMouse_pos;
    emit mouse_posChanged();// 发送信号，通知鼠标位置变化（用于QML绑定）
}
