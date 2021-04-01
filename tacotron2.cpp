#include "tacotron2.h"

Tacotron2::Tacotron2()
{

}

TFTensor<float> Tacotron2::DoInference(const std::vector<int32_t> &InputIDs, const std::vector<float> &ArgsFloat, const std::vector<int32_t> ArgsInt, int32_t SpeakerID, int32_t EmotionID)
{
    if (!CurrentMdl)
        throw std::exception("Tried to do inference on unloaded or invalid model!");




    QString dastr;
    for (int32_t fish : InputIDs)
        dastr += QString::number(fish) + " ";



    // Convenience reference so that we don't have to constantly derefer pointers.
    Model& Mdl = *CurrentMdl;

    // Define the tensors

    Tensor input_ids{ Mdl,"serving_default_input_ids" };
    Tensor speaker_ids{ Mdl,"serving_default_speaker_ids" };
    Tensor input_lengths{Mdl,"serving_default_input_lengths"};
    Tensor* emotion_ids = nullptr;


    // This is a multi-emotion model
    if (EmotionID != -1)
    {
        emotion_ids = new Tensor{Mdl,"serving_default_emotion_ids"};
        emotion_ids->set_data(std::vector<int32_t>{EmotionID});

    }


    // This is the shape of the input IDs, our equivalent to tf.expand_dims.
    std::vector<int64_t> InputIDShape = { 1, (int64_t)InputIDs.size() };

    input_ids.set_data(InputIDs, InputIDShape);
    input_lengths.set_data(std::vector<int32_t>{(int32_t)InputIDs.size()});


    speaker_ids.set_data(std::vector<int32_t>{SpeakerID});

    // Define output tensor

    Tensor output{ Mdl,"StatefulPartitionedCall" };


    // Vector of input tensors
    std::vector<Tensor*> inputs = { &input_ids, &input_lengths,&speaker_ids};

    if (EmotionID != -1)
        inputs.push_back(emotion_ids);


    // Do inference

    CurrentMdl->run(inputs, output);

    // Define output and return it
    TFTensor<float> Output = VoxUtil::CopyTensor<float>(output);

    // We allocated the emotion_ids tensor dynamically, delete it
    if (emotion_ids)
        delete emotion_ids;

    // We could just straight out define it in the return statement, but I like it more this way

    return Output;

}
