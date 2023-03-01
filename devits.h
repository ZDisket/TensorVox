#ifndef DEVITS_H
#define DEVITS_H
#include "vits.h"

class DEVITS : public VITS
{
public:
    DEVITS();

    /*
    Do inference on a DE-VITS model.

    -> InputIDs: Input IDs of tokens for inference
    -> SpeakerID: ID of the speaker in the model to do inference on. If single speaker, always leave at 0. If multispeaker, refer to your model.
    -> MojiIn: TorchMoji hidden states size [tm]
    -> BERTIn: BERT hidden states size [1, n_tokens, channels]
    -> ArgsFloat[0]: Length scale.

    <- Returns: TFTensor<float> with shape {frames} of audio data
    */
    TFTensor<float> DoInferenceDE(const std::vector<int32_t>& InputIDs, const TFTensor<float>& MojiIn, const TFTensor<float>& BERTIn,const std::vector<float>& ArgsFloat,const std::vector<int32_t> ArgsInt, int32_t SpeakerID = 0, int32_t EmotionID = -1);
};

#endif // DEVITS_H
