#include "attention.h"


Attention::Attention(QWidget *parent) : QCustomPlot(parent)
{

    QBrush FillBrush(QColor(100,100,100));
    this->setBackground(FillBrush);
    QColor White(255,255,255);
    QPen AxisPen(QColor(150,150,150));
    xAxis->setTickLabelColor(White);
    yAxis->setTickLabelColor(White);

    xAxis->setBasePen(AxisPen);
    yAxis->setBasePen(AxisPen);

    xAxis->setLabel("Decoder timestep");
    yAxis->setLabel("Encoder timestep");

    xAxis->setLabelColor(White);
    yAxis->setLabelColor(White);
    QFont Fnt = QFont(font().family(), 10);

    xAxis->setLabelFont(QFont(font().family(), 9));
    yAxis->setLabelFont(QFont(font().family(), 9));

    yAxis->setTickPen(AxisPen);
    xAxis->setTickPen(AxisPen);

    yAxis->setSubTickPen(AxisPen);
    xAxis->setSubTickPen(AxisPen);



}

void Attention::DoPlot(const TFTensor<float> &Alignment)
{
    const auto& Shp = Alignment.Shape;




    Map->data()->setSize((int32_t)Shp[2],(int32_t)Shp[1]);

    Map->data()->setRange(QCPRange(0.0,(double)Shp[2]),QCPRange(0.0,(double)Shp[1]));
    for (int64_t x = 0; x < Shp[2];x++)
    {
        for (int64_t y = 0;y < Shp[1];y++)
        {
            size_t i = x + Shp[2]*y;
            Map->data()->setCell(x,y,(double)Alignment.Data[i]);

        }


    }
    Map->setDataRange(QCPRange(0.0,1.0));
    xAxis->setRange(QCPRange(0.0,(double)Shp[2]));

    yAxis->setRange(QCPRange(0.0,(double)Shp[1]));

    rescaleAxes();

    replot();


}

