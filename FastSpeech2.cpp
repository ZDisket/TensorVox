#include "FastSpeech2.h"



FastSpeech2::FastSpeech2()
{
}


TFTensor<float> FastSpeech2::DoInference(const std::vector<int32_t>& InputIDs,const std::vector<float>& ArgsFloat,const std::vector<int32_t> ArgsInt, int32_t SpeakerID , int32_t EmotionID)
{
    if (!CurrentMdl)
        throw std::exception("Tried to do inference on unloaded or invalid model!");

    // Convenience reference so that we don't have to constantly derefer pointers.
    cppflow::model& Mdl = *CurrentMdl;

    // This is the shape of the input IDs, our equivalent to tf.expand_dims.

    std::vector<int64_t> InputIDShape = { 1, (int64_t)InputIDs.size() };

    // Define the tensors
    cppflow::tensor input_ids{InputIDs, InputIDShape };
    cppflow::tensor energy_ratios{ ArgsFloat[1] };
    cppflow::tensor f0_ratios{ArgsFloat[2]};
    cppflow::tensor speaker_ids{ SpeakerID };
    cppflow::tensor speed_ratios{ ArgsFloat[0] };
    cppflow::tensor* emotion_ids = nullptr;






    // Vector of input tensors
    TensorVec Inputs = {{"serving_default_input_ids:0",input_ids},
                        {"serving_default_speaker_ids:0",speaker_ids},
                        {"serving_default_energy_ratios:0",energy_ratios},
                        {"serving_default_f0_ratios:0",f0_ratios},
                        {"serving_default_speed_ratios:0",speed_ratios}};

    // This is a multi-emotion model
    if (EmotionID != -1)
    {
        emotion_ids = new cppflow::tensor{EmotionID};
        Inputs.push_back({"serving_default_emotion_ids:0",emotion_ids});


    }





    // Do inference
    auto Outputs = Mdl(Inputs,{"StatefulPartitionedCall:0","StatefulPartitionedCall:1","StatefulPartitionedCall:2"});

    // Define output and return it
    TFTensor<float> Output = VoxUtil::CopyTensor<float>(Outputs[1]);

    // We allocated the emotion_ids cppflow::tensor dynamically, delete it
    if (emotion_ids)
        delete emotion_ids;

    // We could just straight out define it in the return statement, but I like it more this way

    return Output;
}

FastSpeech2::~FastSpeech2()
{

}
