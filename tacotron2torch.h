#ifndef TACOTRON2TORCH_H
#define TACOTRON2TORCH_H
#include "melgen.h"

class Tacotron2Torch : public MelGen
{
private:
   torch::jit::script::Module Model;

public:

    TFTensor<float> Attention;


    Tacotron2Torch();
    /*
    Initialize and load the model

    -> SavedModelFolder: Folder where the TorchScript models are exported
    <- Returns: (bool)Success
    */
    bool Initialize(const std::string& SavedModelFolder, ETTSRepo::Enum InTTSRepo);


    /*
    Do inference on a Tacotron2 model.

    -> InputIDs: Input IDs of tokens for inference
    -> SpeakerID: ID of the speaker in the model to do inference on. If single speaker, always leave at 0. If multispeaker, refer to your model.

    <- Returns: TFTensor<float> with shape [n_mels, frames] containing contents of mel spectrogram.
    */
    TFTensor<float> DoInference(const std::vector<int32_t>& InputIDs,const std::vector<float>& ArgsFloat,const std::vector<int32_t> ArgsInt, int32_t SpeakerID = 0, int32_t EmotionID = -1);

};

#endif // TACOTRON2TORCH_H
