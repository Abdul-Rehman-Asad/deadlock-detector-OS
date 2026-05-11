#include "graphwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QtMath>
#include <QLinearGradient>
#include <QRadialGradient>

const float GraphWidget::NODE_RADIUS = 30.0f;

GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent), m_rm(nullptr), m_dragNode(-1)
{
    setMinimumSize(400, 300);
    setMouseTracking(true);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(15, 17, 26));
    setPalette(pal);
    setAutoFillBackground(true);
}

void GraphWidget::updateGraph(const ResourceManager *rm)
{
    m_rm = rm;
    if (rm) {
        m_wfg = rm->buildWaitForGraph();
        layoutNodes();
    } else {
        m_wfg.clear();
        m_nodePos.clear();
    }
    update();
}

void GraphWidget::setDeadlockedPids(const QVector<int> &pids)
{
    m_deadlocked = pids;
    update();
}

void GraphWidget::setDeadlockCycles(const QVector<QVector<int>> &cycles)
{
    m_cycles = cycles;
    update();
}


void GraphWidget::layoutNodes()
{
    if (!m_rm) return;

    const auto &procs = m_rm->processes();
    int n = procs.size();
    if (n == 0) { m_nodePos.clear(); return; }


    for (auto pid : m_nodePos.keys())
        if (!procs.contains(pid))
            m_nodePos.remove(pid);


    QVector<int> newPids;
    for (auto &p : procs)
        if (!m_nodePos.contains(p.id))
            newPids.append(p.id);

    float cx = width() / 2.0f;
    float cy = height() / 2.0f;
    float r  = qMin(cx, cy) * 0.65f;
    int   total = procs.size();
    int   idx = 0;

    for (auto &p : procs) {
        if (!m_nodePos.contains(p.id)) {
            float angle = 2.0f * M_PI * idx / total - M_PI / 2.0f;
            m_nodePos[p.id] = QPointF(cx + r * qCos(angle), cy + r * qSin(angle));
        }
        idx++;
    }
}

void GraphWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    layoutNodes();
}


void GraphWidget::mousePressEvent(QMouseEvent *event)
{
    QPointF pos = event->pos();
    m_dragNode = -1;
    for (auto it = m_nodePos.begin(); it != m_nodePos.end(); ++it) {
        if (QLineF(pos, it.value()).length() <= NODE_RADIUS + 4) {
            m_dragNode = it.key();
            m_dragOffset = it.value() - pos;
            break;
        }
    }
}

void GraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragNode >= 0) {
        m_nodePos[m_dragNode] = event->pos() + m_dragOffset;
        update();
    }
    setCursor(m_dragNode >= 0 ? Qt::ClosedHandCursor : Qt::ArrowCursor);
}

void GraphWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_dragNode = -1;
    setCursor(Qt::ArrowCursor);
}


bool GraphWidget::isDeadlockEdge(int from, int to) const
{
    for (auto &cycle : m_cycles) {
        for (int i = 0; i + 1 < cycle.size(); i++) {
            if (cycle[i] == from && cycle[i+1] == to)
                return true;
        }
    }
    return false;
}


void GraphWidget::drawArrow(QPainter &painter, QPointF from, QPointF to,
                            bool isDeadlock, float nodeRadius)
{
    QLineF line(from, to);
    if (line.length() < 1.0) return;


    QPointF dir = (to - from) / line.length();
    QPointF start = from + dir * nodeRadius;
    QPointF end   = to   - dir * nodeRadius;

    if (QLineF(start, end).length() < 1.0) return;


    QColor edgeColor = isDeadlock ? QColor(255, 60, 60) : QColor(80, 180, 255);
    int    lineWidth = isDeadlock ? 3 : 2;

    if (isDeadlock) {
        QPen glowPen(QColor(255, 60, 60, 60), lineWidth + 8);
        painter.setPen(glowPen);
        painter.drawLine(start, end);
    }

    painter.setPen(QPen(edgeColor, lineWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(start, end);


    double angle = qAtan2(end.y() - start.y(), end.x() - start.x());
    double arrowLen = 14.0;
    double arrowAngle = 0.4;

    QPointF p1 = end - QPointF(arrowLen * qCos(angle - arrowAngle),
                               arrowLen * qSin(angle - arrowAngle));
    QPointF p2 = end - QPointF(arrowLen * qCos(angle + arrowAngle),
                               arrowLen * qSin(angle + arrowAngle));

    painter.setBrush(edgeColor);
    painter.setPen(Qt::NoPen);
    QPolygonF arrowHead;
    arrowHead << end << p1 << p2;
    painter.drawPolygon(arrowHead);
}

void GraphWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient bg(0, 0, width(), height());
    bg.setColorAt(0, QColor(7, 20, 7));
    bg.setColorAt(1, QColor(15, 35, 15));
    painter.fillRect(rect(), bg);

    painter.setPen(QColor(57, 225, 20, 120));
    for (int x = 0; x < width(); x += 30)
        for (int y = 0; y < height(); y += 30)
            painter.drawPoint(x, y);

    if (!m_rm || m_rm->processes().isEmpty()) {
        painter.setPen(QColor(80, 100, 140));
        painter.setFont(QFont("Monospace", 12));
        painter.drawText(rect(), Qt::AlignCenter,
                         "Add processes to visualize\nthe Wait-For Graph");
        return;
    }

    for (auto fromIt = m_wfg.begin(); fromIt != m_wfg.end(); ++fromIt) {
        int fromPid = fromIt.key();
        if (!m_nodePos.contains(fromPid)) continue;

        for (int toPid : fromIt.value()) {
            if (!m_nodePos.contains(toPid)) continue;
            bool deadlock = isDeadlockEdge(fromPid, toPid);
            drawArrow(painter, m_nodePos[fromPid], m_nodePos[toPid],
                      deadlock, NODE_RADIUS);
        }
    }

    for (auto &p : m_rm->processes()) {
        if (!m_nodePos.contains(p.id)) continue;

        QPointF center = m_nodePos[p.id];
        bool isDeadlocked = p.isDeadlocked;
        bool isBlocked    = p.isBlocked;

        if (isDeadlocked) {
            QRadialGradient glow(center, NODE_RADIUS * 2.0);
            glow.setColorAt(0, QColor(255, 50, 50, 120));
            glow.setColorAt(1, QColor(255, 50, 50, 0));
            painter.setBrush(glow);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(center, NODE_RADIUS * 2.0, NODE_RADIUS * 2.0);
        }

        QRadialGradient grad(center - QPointF(4, 4), NODE_RADIUS * 1.5);
        if (isDeadlocked) {
            grad.setColorAt(0, QColor(200, 60, 60));
            grad.setColorAt(1, QColor(120, 20, 20));
        } else if (isBlocked) {
            grad.setColorAt(0, QColor(220, 150, 0));
            grad.setColorAt(1, QColor(130, 80, 0));
        } else {
            grad.setColorAt(0, QColor(50, 120, 220));
            grad.setColorAt(1, QColor(20, 60, 140));
        }

        painter.setBrush(grad);


        QColor borderColor = isDeadlocked ? QColor(255, 100, 100)
                             : isBlocked    ? QColor(255, 200, 0)
                                          : QColor(100, 180, 255);
        painter.setPen(QPen(borderColor, isDeadlocked ? 3 : 1.5));
        painter.drawEllipse(center, (double)NODE_RADIUS, (double)NODE_RADIUS);


        painter.setPen(Qt::white);
        QFont f("Monospace", 8, QFont::Bold);
        painter.setFont(f);
        QRectF textRect(center.x() - NODE_RADIUS, center.y() - 12,
                        NODE_RADIUS * 2, 24);
        painter.drawText(textRect, Qt::AlignCenter, p.name);


        if (isDeadlocked || isBlocked) {
            QFont sf("Monospace", 7);
            painter.setFont(sf);
            painter.setPen(isDeadlocked ? QColor(255, 150, 150) : QColor(255, 220, 100));
            painter.drawText(QRectF(center.x() - NODE_RADIUS,
                                    center.y() + NODE_RADIUS + 2,
                                    NODE_RADIUS * 2, 16),
                             Qt::AlignCenter,
                             isDeadlocked ? "DEADLOCK" : "BLOCKED");
        }
    }


    int lx = 10, ly = height() - 90;
    painter.setFont(QFont("Monospace", 8));

    auto drawLegend = [&](int x, int y, QColor c, const QString &label) {
        painter.setBrush(c);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(x, y, 12, 12);
        painter.setPen(QColor(180, 200, 240));
        painter.drawText(x + 18, y + 10, label);
    };

    drawLegend(lx, ly,      QColor(50, 120, 220), "Running");
    drawLegend(lx, ly + 18, QColor(220, 150, 0),  "Blocked");
    drawLegend(lx, ly + 36, QColor(200, 60, 60),  "Deadlocked");


    painter.setPen(QPen(QColor(80, 180, 255), 2));
    painter.drawLine(lx, ly + 58, lx + 30, ly + 58);
    painter.setPen(QColor(180, 200, 240));
    painter.drawText(lx + 35, ly + 63, "Wait-For");

    painter.setPen(QPen(QColor(255, 60, 60), 3));
    painter.drawLine(lx, ly + 76, lx + 30, ly + 76);
    painter.setPen(QColor(180, 200, 240));
    painter.drawText(lx + 35, ly + 81, "Deadlock Cycle");
}
