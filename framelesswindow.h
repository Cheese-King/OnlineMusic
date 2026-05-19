#ifndef FRAMELESSWINDOW_H
#define FRAMELESSWINDOW_H

#include <QQuickWindow>

class FramelessWindow : public QQuickWindow
{
    Q_OBJECT//作用：启用 Qt 的元对象系统，支持信号槽机制、属性系统、动态类型识别等功能
    Q_PROPERTY(MousePosition mouse_pos READ getMouse_pos WRITE setMouse_pos NOTIFY mouse_posChanged FINAL)
    //MousePosition：属性类型，为类内定义的枚举类型。
    //READ getMouse_pos：读取属性值的函数为 getMouse_pos()。
    //WRITE setMouse_pos：设置属性值的函数为 setMouse_pos()。
    //NOTIFY mouse_posChanged：当属性值变化时，发出 mouse_posChanged 信号。
    //FINAL：该属性不可被派生类重写。


public:
    enum MousePosition {
        NORMAL = 0,TOPLEFT = 1,TOP,TOPRIGHT,LEFT,RIGHT,BOTTOMLEFT,BOTTOM,BOTTOMRIGHT
    };
    Q_ENUM(MousePosition);//Q_ENUM 宏：将枚举暴露给 Qt 元对象系统，以便在 QML 中直接使用枚举值
    FramelessWindow(QWindow * parent = nullptr);
    MousePosition getMouse_pos() const;
    void setMouse_pos(MousePosition newMouse_pos);

signals:
    void mouse_posChanged();//当 mouse_pos 属性值变化时发出信号，用于通知 QML 界面更新（如光标样式、交互状态）。

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
private:

    void setWindowGeometry(const QPointF &pos);
    void setCursorIcon();
    MousePosition getMousePos(const QPointF &pos);

    // 缩放边距
    int step = 8;
    // 鼠标的大概位置
    MousePosition mouse_pos = NORMAL;
    // 起始位置
    QPointF start_pos;
    // 旧位置
    QPointF old_pos;
    // 旧大小
    QSize old_size;

};

#endif // FRAMELESSWINDOW_H
