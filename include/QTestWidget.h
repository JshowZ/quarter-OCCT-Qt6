#pragma once

#include <QWidget>

/**
 * @class QTestWidget
 * @brief 一个空的测试QWidget类
 */
class QTestWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针，默认为nullptr
     */
    explicit QTestWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~QTestWidget();

protected:
    /**
     * @brief 重写paintEvent事件
     * @param event 绘图事件
     */
    void paintEvent(QPaintEvent *event) override;
};
