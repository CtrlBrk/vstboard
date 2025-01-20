#ifndef AUDIOGRAPH_H
#define AUDIOGRAPH_H

#include <QWidget>
QT_FORWARD_DECLARE_CLASS(QChart)
QT_FORWARD_DECLARE_CLASS(QLineSeries)
QT_FORWARD_DECLARE_CLASS(QXYSeries)

namespace View {

    class AudioGraph : public QWidget {
    public:
    explicit AudioGraph(QWidget *parent = nullptr);
    void UpdateGraph(float *buffer, qint32 size, int serie=0);

private:
    QChart *m_chart = nullptr;
    QLineSeries *m_series = nullptr;
    QLineSeries *m_series2 = nullptr;

};
}

#endif // AUDIOGRAPH_H
