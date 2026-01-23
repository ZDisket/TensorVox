#include "VITSEvo.h"
#include <cstddef>
#include <string>


std::vector<int64_t> ZeroPadVec(const std::vector<int32_t> &InIDs)
{
    std::vector<int64_t> NewIDs;
    NewIDs.reserve(InIDs.size() * 2);

    NewIDs.push_back(0);

    for (auto CharID : InIDs)
    {

        NewIDs.push_back((int64_t)CharID);
        NewIDs.push_back(0);


    }

    return NewIDs;

}


namespace {
std::size_t CountElements(const std::vector<std::int64_t>& shape)
{
    std::size_t total = 1;
    for (auto dim : shape) {
        total *= static_cast<std::size_t>(dim);
    }
    return total;
}
}

bool VITSEvo::Initialize(const std::string &SavedModelFolder, ETTSRepo::Enum InTTSRepo)
{

    std::wstring WidePath( SavedModelFolder.begin(), SavedModelFolder.end() );

    CurrentRepo = InTTSRepo;

    return Load(WidePath);

}




TFTensor<float> VITSEvo::DoInference(const std::vector<int32_t> &InputIDs, const std::vector<float> &ArgsFloat, const std::vector<int32_t> ArgsInt, int32_t SpeakerID, int32_t EmotionID)
{
    // VITS EVO uses zero interspersion

    std::vector<int64_t> RealIDs = ZeroPadVec(InputIDs);


    // Call the ONNX inference fun

    std::vector<float> AudioFrames = Predict(RealIDs, (int64_t)SpeakerID);
    const int64_t output_count = (int64_t)AudioFrames.size();


    TFTensor<float> RetTensor;

    RetTensor.Shape = {output_count};
    RetTensor.TotalSize = output_count;


    RetTensor.Data = AudioFrames;

    return RetTensor;

}

std::vector<float> VITSEvo::Predict(std::vector<std::int64_t>& text_ids, int64_t SpeakerID)
{
    if (text_ids.empty()) {
        return {};
    }

    const std::int64_t batch = 1;
    const std::int64_t seq_len = static_cast<std::int64_t>(text_ids.size());

    std::vector<std::int64_t> text_shape{ batch, seq_len };
    std::vector<std::int64_t> text_lengths_shape{ batch };
    std::vector<std::int64_t> text_lengths_data{ seq_len };

    Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
    std::vector<Ort::Value> input_tensors;

    input_tensors.emplace_back(Ort::Value::CreateTensor<std::int64_t>(mem_info,
                                                                      text_ids.data(), text_ids.size(), text_shape.data(), text_shape.size()));
    input_tensors.emplace_back(Ort::Value::CreateTensor<std::int64_t>(mem_info,
                                                                      text_lengths_data.data(), text_lengths_data.size(), text_lengths_shape.data(), text_lengths_shape.size()));

    std::vector<std::string> InputNames = { "text", "text_lengths" };

    if (SpeakerID != -1)
    {
        // This is a multi speaker model.

        std::vector<int64_t> speaker_id_data = {SpeakerID};
        std::vector<int64_t> speaker_id_shape = {batch};

        input_tensors.emplace_back(
            Ort::Value::CreateTensor<std::int64_t>(mem_info,
                                                   speaker_id_data.data(), speaker_id_data.size(), speaker_id_shape.data(), speaker_id_shape.size())
            );

        InputNames.push_back("sid");

    }

    auto output_tensors = Forward(input_tensors, InputNames);
    if (output_tensors.empty() || !output_tensors[0].IsTensor()) {
        return {};
    }

    auto output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
    std::size_t output_count = CountElements(output_shape);
    const float* output_ptr = output_tensors[0].GetTensorData<float>();


    return std::vector<float>(output_ptr, output_ptr + output_count);
}
