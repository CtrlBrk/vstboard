#include "audiograph.h"
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QVBoxLayout>

namespace View {


AudioGraph::AudioGraph(QWidget *parent) :
    QWidget(parent),
    m_chart(new QChart),
    m_series(new QLineSeries),
    m_series2(new QLineSeries)
{
    // setAttribute(Qt::WA_DeleteOnClose);

    auto chartView = new QChartView(m_chart);

    auto axisX = new QValueAxis;
    axisX->setRange(0, 600);
    axisX->setLabelFormat("%g");
    // axisX->setTitleText("Samples");

    auto axisY = new QValueAxis;
    axisY->setRange(-1, 1);
    // axisY->setTitleText("Audio level");

    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);

    m_chart->addSeries(m_series);
    m_series->attachAxis(axisX);
    m_series->attachAxis(axisY);

    m_chart->addSeries(m_series2);
    m_series2->attachAxis(axisX);
    m_series2->attachAxis(axisY);

    m_chart->legend()->hide();

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(chartView);
}

void AudioGraph::UpdateGraph(float *buffer, unsigned long sampleCount, int serie)
{

    QList<QPointF> m_buffer;

    if (m_buffer.isEmpty()) {
        m_buffer.reserve(sampleCount);
        for (int i = 0; i < sampleCount; ++i)
            m_buffer.append(QPointF(i, 0));
    }

    if(sampleCount>600) sampleCount=600;
    unsigned long i=sampleCount;

    while(i>0) {
        --i;
        if(serie==0) {
            m_buffer[i].setY(buffer[i]);
        } else {
            m_buffer[i].setY(buffer[i]+.1f);
        }

    }
    if(serie==0) {
        m_series->replace(m_buffer);
    } else {
        m_series2->replace(m_buffer);
    }
}


}
