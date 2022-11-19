#include "spectrogram.h"



void Spectrogram::TimerTick()
{
    if (!DoSlide)
        return;

    float RemSecs = ((float)timEndTick->remainingTime()) / 1000.f;
    float CurrentPos = TotSecs - RemSecs;
    float TickSet = CurrentPos/TotSecs;

    PlayRect->topLeft->setCoords(TickSet,0);

    layer("Lay2")->replot();



}

void Spectrogram::EndSlide()
{
    timGenericTick->stop();
    timEndTick->stop();
    PlayRect->topLeft->setCoords(1,0);

    layer("Lay2")->replot();

}

size_t Spectrogram::Get2DIndex(size_t x, size_t y, size_t xSize)
{
      return x + xSize*y;
}


Spectrogram::Spectrogram(QWidget *parent) : QCustomPlot(parent)
{

    QBrush FillBrush(QColor(100,100,100));
    this->setBackground(FillBrush);
    QColor White(255,255,255);
    QPen AxisPen(QColor(150,150,150));
    xAxis->setTickLabelColor(White);
    yAxis->setTickLabelColor(White);

    xAxis->setBasePen(AxisPen);
    yAxis->setBasePen(AxisPen);

    yAxis->setLabel("Frequency");
    xAxis->setLabel("Time");


    // They show the wrong info

    xAxis->setTickLabels(false);
    yAxis->setTickLabels(false);


    xAxis->setTicks(false);
    yAxis->setTicks(false);
    xAxis->setLabelColor(White);
    yAxis->setLabelColor(White);
    QFont Fnt = QFont(font().family(), 10);

    xAxis->setLabelFont(Fnt);
    yAxis->setLabelFont(Fnt);



    PlayRect = new QCPItemRect(this);
    PlayRect->topLeft->setType(QCPItemPosition::ptViewportRatio);
    PlayRect->bottomRight->setType(QCPItemPosition::ptViewportRatio);



    // The rect is not visible without adding a layer, probably because we are using a more unusual type of plot
    addLayer("Lay2");

    QPen RectPen(QColor(255,255,255,150));
    QBrush RectBrush(QColor(200,200,200,75));

    RectPen.setWidth(3);
    PlayRect->topLeft->setCoords(0,0);
    PlayRect->bottomRight->setCoords(1,1);
    PlayRect->setPen(RectPen);
    PlayRect->setBrush(RectBrush);
    PlayRect->setLayer("Lay2");



    timGenericTick = new QTimer(this);
    timGenericTick->setInterval(10);
    timGenericTick->setSingleShot(false);

    timEndTick = new QTimer(this);
    timEndTick->setInterval(1000);
    timEndTick->setSingleShot(false);

    connect(timGenericTick,&QTimer::timeout,this,&Spectrogram::TimerTick);
    connect(timEndTick,&QTimer::timeout,this,&Spectrogram::EndSlide);

    DoSlide = false;


}

void Spectrogram::DoPlot(const TFTensor<float> &InMel, float TimeInSeconds)
{

    const TFTensor<float>& Mel = InMel;


    const auto& Shp = Mel.Shape;


    Map->data()->setSize((int32_t)Shp[2],(int32_t)Shp[1]);

    Map->data()->setRange(QCPRange(0.0,(double)Shp[1]),QCPRange(0.0,(double)Shp[2]));
    for (int64_t x = 0; x < Shp[2];x++)
    {
        for (int64_t y = 0;y < Shp[1];y++)
        {
            size_t i = Get2DIndex(x,y,Shp[2]);
            Map->data()->setCell(x,y,(double)Mel.Data[i]);

        }


    }
    Map->rescaleDataRange(true);





    rescaleAxes();

    replot();

    TotSecs = TimeInSeconds;


    PlayRect->setVisible(true);

    PlayRect->topLeft->setCoords(1,0);

    timGenericTick->start();

    timEndTick->start((int)(TimeInSeconds * 1000));





}
