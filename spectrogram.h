#ifndef SPECTROGRAM_H
#define SPECTROGRAM_H

#include "ext/qcustomplot.h"
#include "VoxCommon.hpp"

class Spectrogram : public QCustomPlot
{
public slots:
    void TimerTick();
    void EndSlide();
private:
    inline size_t Get2DIndex(size_t x,size_t y,size_t xSize);

    QCPItemRect* PlayRect;

    QTimer* timGenericTick;
    QTimer* timEndTick;

    float TotSecs;


public:
    bool DoSlide;
    Spectrogram(QWidget *parent = nullptr);

    void DoPlot(const TFTensor<float>& InMel,float TimeInSeconds);

    QCPColorMap* Map;
};

#endif // SPECTROGRAM_H
