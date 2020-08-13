#include "voxer.h"

void Voxer::run()
{
    pAttItem->setBackgroundColor(QColor(0,0,255));
    std::vector<float> Audat = pAttVoice->Vocalize(Prompt.toStdString(),Speed,SpeakerID,Energy,F0);
    pAttItem->setBackgroundColor(QColor(0,128,0));
    emit Done(Audat);

}

Voxer::Voxer()
{

}
