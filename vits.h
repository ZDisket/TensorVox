#ifndef VITS_H
#define VITS_H


#pragma warning(push, 0) // LibTorch spams us with warnings
#include <torch/script.h> // One-stop header.
#pragma warning(pop)


#include "melgen.h"

// VITS is a fully E2E model; no separate vocoder needed
class VITS : public MelGen
{
private:
    torch::jit::script::Module Model;

public:
    TFTensor<float> Attention;

    VITS();

    // Since VITS runs on PyTorch, we override the loader
    /*
    Initialize and load the model

    -> SavedModelFolder: Not a folder, but path to TorchScripted .pt file
    <- Returns: (bool)Success
    */
    virtual bool Initialize(const std::string& SavedModelFolder, ETTSRepo::Enum InTTSRepo);


    /*
    Do inference on a VITS model.

    -> InputIDs: Input IDs of tokens for inference
    -> SpeakerID: ID of the speaker in the model to do inference on. If single speaker, always leave at 0. If multispeaker, refer to your model.
    -> ArgsFloat[0]: Length scale.

    <- Returns: TFTensor<float> with shape {frames} of audio data
    */
    TFTensor<float> DoInference(const std::vector<int32_t>& InputIDs,const std::vector<float>& ArgsFloat,const std::vector<int32_t> ArgsInt, int32_t SpeakerID = 0, int32_t EmotionID = -1);
};

#endif // VITS_H
