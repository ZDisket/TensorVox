#ifndef TRACK_H
#define TRACK_H
#include "ext/qcustomplot.h"
#include <QAudioBuffer>


// Copied from https://stackoverflow.com/questions/50277132/qt-audio-file-to-wave-like-audacity

class QAudioDecoder;

class Track : public QCustomPlot
{
    Q_OBJECT

public:
    Track(QWidget *parent = Q_NULLPTR);
    ~Track();
    void setSource(const QAudioBuffer &inbuffer);

public:
    void setBuffer();
    void plot();
    void startPlaying(float TimeInSecs);


public slots:
    void TimerTick();
    void EndSlide();
private:
    void SetTimeLabel(float Cur, float Remaining);
    QTimer* timGenericTick;
    QTimer* timEndTick;

    float TickSet;
    float TotSecs;

    QCPItemRect* PlayRect;
    QCPItemText* SecsTxt;

    qreal getPeakValue(const QAudioFormat& format);

    QAudioDecoder *decoder;
    QAudioBuffer buffer;
    QVector<double> samples;
    QCPGraph *wavePlot;
};
#endif // TRACK_H
