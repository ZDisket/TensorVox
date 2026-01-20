#pragma once

#include "ONNXModel.h"
#include "melgen.h"
#include <cstdint>
#include <vector>

class VITSEvo : public ONNXModel, public MelGen
{
public:
    // Since VITS EVO runs on ONNX we override the loader
    /*
    Initialize and load the model

    -> SavedModelFolder: Not a folder, but path to the .onnx file
    <- Returns: (bool)Success
    */
    virtual bool Initialize(const std::string& SavedModelFolder, ETTSRepo::Enum InTTSRepo);


    /*
    Do inference on a VITS model.

    -> InputIDs: Input IDs of tokens for inference
    -> SpeakerID: ID of the speaker in the model to do inference on. If single speaker, always leave at 0. If multispeaker, refer to your model.
    -> ArgsFloat[0]: Length scale. (not yet)

    <- Returns: TFTensor<float> with shape {frames} of audio data
    */
    TFTensor<float> DoInference(const std::vector<int32_t>& InputIDs,const std::vector<float>& ArgsFloat,const std::vector<int32_t> ArgsInt, int32_t SpeakerID = 0, int32_t EmotionID = -1);

    std::vector<float> Predict(std::vector<std::int64_t>& text_ids);
};
