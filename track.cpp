#include "track.h"

#include <QAudioDecoder>

Track::Track(QWidget *parent)
    : QCustomPlot(parent)
    , decoder(new QAudioDecoder(this))
{

    wavePlot = addGraph();

    QBrush FillBrush(QColor(100,100,100));
    this->setBackground(FillBrush);
    QPen ThePen(QColor(127,255,0));
    wavePlot->setPen(ThePen);

    yAxis->setVisible(false);
    xAxis->setVisible(false);


    PlayRect = new QCPItemRect(this);
    PlayRect->topLeft->setType(QCPItemPosition::ptViewportRatio);
    PlayRect->bottomRight->setType(QCPItemPosition::ptViewportRatio);


    QPen RectPen(QColor(255,255,255,150));
    QBrush RectBrush(QColor(200,200,200,75));

    RectPen.setWidth(3);
    PlayRect->topLeft->setCoords(0,0);
    PlayRect->bottomRight->setCoords(1,1);
    PlayRect->setPen(RectPen);
    PlayRect->setBrush(RectBrush);



    wavePlot->setBrush(FillBrush);
    timGenericTick = new QTimer(this);
    timGenericTick->setInterval(10);
    timGenericTick->setSingleShot(false);

    timEndTick = new QTimer(this);
    timEndTick->setInterval(1000);
    timEndTick->setSingleShot(false);

    connect(timGenericTick,&QTimer::timeout,this,&Track::TimerTick);
    connect(timEndTick,&QTimer::timeout,this,&Track::EndSlide);

    SecsTxt = new QCPItemText(this);
    SecsTxt->setPositionAlignment(Qt::AlignTop|Qt::AlignLeft);
    SecsTxt->position->setType(QCPItemPosition::ptViewportRatio);
    SecsTxt->position->setCoords(0.02, 0.05);
    SecsTxt->setText("Ready");
    SecsTxt->setFont(QFont(font().family(), 10));
    SecsTxt->setColor(QColor(255,255,255));
    SecsTxt->setClipToAxisRect(false);
    DoSlide = false;

    //wavePlot->setPen(ThePen);

}

Track::~Track()
{
    delete decoder;
    // wavePlot delete auto ?
}

void Track::setSource(const QAudioBuffer &inbuffer)
{
    buffer = inbuffer;


    setBuffer();


    startPlaying(((float)buffer.duration()) / 1e+6);

}

void Track::setBuffer()
{
    samples.clear();
    qreal peak = getPeakValue(buffer.format());
    const float *data = buffer.constData<float>();
    int count = buffer.sampleCount();

    for (int i=0; i<count; i++){
        double val = ((double)data[i])/peak;
        samples.append(val);
    }

}

void Track::plot()
{
    QVector<double> x(samples.size());
    for (int i=0; i<x.size(); i++)
        x[i] = i;
    wavePlot->addData(x, samples);
    yAxis->setRange(QCPRange(-0.6, 0.6));

    xAxis->setRange(QCPRange(0, samples.size()));
    replot();
}

void Track::startPlaying(float TimeInSecs)
{
    //TickAdd = 1.f/( TimeInSecs / 0.025f );
    TotSecs = TimeInSecs;


    timGenericTick->start();

    timEndTick->start((int)(TimeInSecs * 1000));


}

void Track::TimerTick()
{
    if (!DoSlide)
        return;

    float RemSecs = ((float)timEndTick->remainingTime()) / 1000.f;
    float CurrentPos = TotSecs - RemSecs;
    TickSet = CurrentPos/TotSecs;

    PlayRect->topLeft->setCoords(TickSet,0);
    SetTimeLabel(CurrentPos,TotSecs);



    replot();


}

void Track::EndSlide()
{

    timGenericTick->stop();
    timEndTick->stop();
    PlayRect->topLeft->setCoords(1,0);
    SetTimeLabel(TotSecs,TotSecs);

    replot();


}

void Track::SetTimeLabel(float Cur, float Remaining)
{
    SecsTxt->setText(QString::number(Cur,'f',1) + " / " + QString::number(Remaining,'f',1) + " (sec)");


}

/**
 * https://stackoverflow.com/questions/46947668/draw-waveform-from-raw-data-using-qaudioprobe
 * @brief Track::getPeakValue
 * @param format
 * @return The peak value
 */
qreal Track::getPeakValue(const QAudioFormat &format)
{
    qreal ret(0);
    if (format.isValid()){
        switch (format.sampleType()) {
            case QAudioFormat::Unknown:
            break;
            case QAudioFormat::Float:
                if (format.sampleSize() != 32) // other sample formats are not supported
                    ret = 0;
                else
                    ret = 1.00003;
            break;
            case QAudioFormat::SignedInt:
                if (format.sampleSize() == 32)
#ifdef Q_OS_WIN
                    ret = INT_MAX;
#endif
#ifdef Q_OS_UNIX
                    ret = SHRT_MAX;
#endif
                else if (format.sampleSize() == 16)
                    ret = SHRT_MAX;
                else if (format.sampleSize() == 8)
                    ret = CHAR_MAX;
                break;
            case QAudioFormat::UnSignedInt:
                if (format.sampleSize() == 32)
                    ret = UINT_MAX;
                else if (format.sampleSize() == 16)
                    ret = USHRT_MAX;
                else if (format.sampleSize() == 8)
                    ret = UCHAR_MAX;
            break;
        default:
            break;
        }
    }
    return ret;
}
