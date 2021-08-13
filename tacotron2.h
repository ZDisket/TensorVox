#ifndef TACOTRON2_H
#define TACOTRON2_H

#include "melgen.h"

class Tacotron2 : public MelGen
{
private:

    TFTensor<float> DoInferenceTFTTS(const std::vector<int32_t>& InputIDs,int32_t SpeakerID = 0, int32_t EmotionID = -1);
    TFTensor<float> DoInferenceCoqui(const std::vector<int32_t>& InputIDs);



public:
    Tacotron2();
    TFTensor<float> Attention;

    /*
    Do inference on a Tacotron2 model.

    -> InputIDs: Input IDs of tokens for inference
    -> SpeakerID: ID of the speaker in the model to do inference on. If single speaker, always leave at 0. If multispeaker, refer to your model.

    <- Returns: TFTensor<float> with shape {1,<len of mel in frames>,80} containing contents of mel spectrogram.
    */
    TFTensor<float> DoInference(const std::vector<int32_t>& InputIDs,const std::vector<float>& ArgsFloat,const std::vector<int32_t> ArgsInt, int32_t SpeakerID = 0, int32_t EmotionID = -1);

};

#endif // TACOTRON2_H
