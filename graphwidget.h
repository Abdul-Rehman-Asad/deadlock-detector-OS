#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QMap>
#include <QSet>
#include <QVector>
#include <QPointF>
#include <QString>
#include "resourcemanager.h"


class GraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphWidget(QWidget *parent = nullptr);

    void updateGraph(const ResourceManager *rm);
    void setDeadlockedPids(const QVector<int> &pids);
    void setDeadlockCycles(const QVector<QVector<int>> &cycles);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    const ResourceManager *m_rm;

    QMap<int, QPointF>   m_nodePos;
    QMap<int, QSet<int>> m_wfg;
    QVector<int>         m_deadlocked;
    QVector<QVector<int>>m_cycles;

    int     m_dragNode;
    QPointF m_dragOffset;

    void layoutNodes();
    void drawArrow(QPainter &painter, QPointF from, QPointF to,
                   bool isDeadlockEdge, float nodeRadius);
    bool isDeadlockEdge(int from, int to) const;

    static const float NODE_RADIUS;
};

#endif
