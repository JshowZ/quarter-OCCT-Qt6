#include "QTestWidget.h"
#include <QPainter>

QTestWidget::QTestWidget(QWidget *parent)
    : QWidget(parent)
{
    // 空构造函数
}

QTestWidget::~QTestWidget()
{
    // 空析构函数
}

void QTestWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    
    // 简单的绘制，显示这是一个测试控件
    QPainter painter(this);
    painter.setPen(Qt::blue);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(rect(), Qt::AlignCenter, "QTestWidget - Empty Widget");
}
