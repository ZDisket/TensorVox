#include "voxer.h"
using namespace std::chrono;

void Voxer::run()
{

    pAttItem->setBackgroundColor(InProcessColor);

    high_resolution_clock::time_point Start = high_resolution_clock::now();
    std::vector<float> Audat = pAttVoice->Vocalize(Prompt.toStdString(),Speed,SpeakerID,Energy,F0);
    high_resolution_clock::time_point End = high_resolution_clock::now();


    pAttItem->setBackgroundColor(DoneColor);
    emit Done(Audat,duration_cast<duration<double>>(End - Start));

}

Voxer::Voxer()
{

}
