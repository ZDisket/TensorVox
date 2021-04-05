#ifndef ATTENTION_H
#define ATTENTION_H

#include "ext/qcustomplot.h"
#include "VoxCommon.hpp"

class Attention : public QCustomPlot
{
public:
    Attention(QWidget *parent = nullptr);

    void DoPlot(const TFTensor<float>& Alignment);

    QCPColorMap* Map;

};

#endif // ATTENTION_H
