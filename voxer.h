#ifndef VOXER_H
#define VOXER_H

#include "Voice.h"
#include <QThread>

#include <QListWidgetItem>
#include <chrono>
#include "rnnoise.h"

const QColor DoneColor = QColor(0,128,0);
const QColor PlayingColor = QColor(168, 40, 94);
const QColor InProcessColor = QColor(0,0,255);

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
    uint32_t SampleRate;
    int32_t EmotionID;
    bool Denoise;
    QString EmotionOverride;

    // DANGER: If this is set, the item will not emit anything
    QString ExportFileName;

    float Amplify;
    Voxer();

    uint32_t CurrentID;

    std::vector<float> ForcedAudio;



signals:
    void Done(std::vector<float> AudioData,TFTensor<float> Mel,std::chrono::duration<double> infer_span,uint32_t ID);
    void AttentionReady(TFTensor<float> Att,uint32_t ID);

};

#endif // VOXER_H
