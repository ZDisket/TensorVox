#include "voxer.h"

void Voxer::run()
{
    pAttItem->setBackgroundColor(InProcessColor);
    std::vector<float> Audat = pAttVoice->Vocalize(Prompt.toStdString(),Speed,SpeakerID,Energy,F0);
    pAttItem->setBackgroundColor(DoneColor);
    emit Done(Audat);

}

Voxer::Voxer()
{

}
