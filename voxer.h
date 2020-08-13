#ifndef VOXER_H
#define VOXER_H

#include "Voice.h"
#include <QThread>

#include <QListWidgetItem>


// A Voxer is a thread spawned for the sole purpose of doing inference
class Voxer : public QThread
{
    Q_OBJECT

    void run() override;
public:

    Voice* pAttVoice;
    QListWidgetItem* pAttItem;
    QString Prompt;
    float Speed;
    float Energy;
    float F0;
    int32_t SpeakerID;
    Voxer();


signals:
    void Done(std::vector<float> AudioData);

};

#endif // VOXER_H
