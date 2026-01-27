#include "SupertonicVocoder.h"

#include <cstddef>
#include <string>
#include <vector>

namespace {
std::wstring ToWide(const std::string& value)
{
    return std::wstring(value.begin(), value.end());
}

std::size_t CountElements(const std::vector<std::int64_t>& shape)
{
    std::size_t total = 1;
    for (auto dim : shape) {
        total *= static_cast<std::size_t>(dim);
    }
    return total;
}
}

bool SupertonicVocoder::Initialize(const std::string& vocoderPath)
{
    std::string model_path = vocoderPath;
    const std::string suffix = ".onnx";

    if (model_path.size() < suffix.size() ||
        model_path.compare(model_path.size() - suffix.size(), suffix.size(), suffix) != 0) {
        if (!model_path.empty() && model_path.back() != '/' && model_path.back() != '\\') {
            model_path += '/';
        }
        model_path += "vocoder.onnx";
    }

    std::wstring wide_path = ToWide(model_path);
    return static_cast<ONNXModel&>(*this).Load(wide_path, "supertonic_vocoder");
}

TFTensor<float> SupertonicVocoder::DoInference(const TFTensor<float>& InMel)
{
    TFTensor<float> result;
    if (InMel.Data.empty() || InMel.Shape.empty()) {
        return result;
    }

    Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
    std::vector<float> input_data = InMel.Data;
    std::vector<int64_t> input_shape = InMel.Shape;

    std::vector<Ort::Value> input_tensors;
    input_tensors.emplace_back(Ort::Value::CreateTensor<float>(mem_info, input_data.data(), input_data.size(), input_shape.data(), input_shape.size()));

    auto output_tensors = Forward(input_tensors);
    if (output_tensors.empty() || !output_tensors[0].IsTensor()) {
        return result;
    }

    auto output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
    std::size_t output_count = CountElements(output_shape);
    const float* output_ptr = output_tensors[0].GetTensorData<float>();

    result.Data.assign(output_ptr, output_ptr + output_count);

    // Return shape: [1, N]
    // Squeeze extra dim
    result.Shape = {output_shape[1]};

    result.TotalSize = output_count;


    return result;
}
