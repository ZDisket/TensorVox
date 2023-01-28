#include "istftnettorch.h"

bool iSTFTNetTorch::Initialize(const std::string &VocoderPath)
{
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().

        Model = torch::jit::load(VocoderPath + ".pt");

    }
    catch (const c10::Error& e) {
        return false;

    }

    return true;

}

TFTensor<float> iSTFTNetTorch::DoInference(const TFTensor<float> &InMel)
{
    auto TorchMel = torch::tensor(InMel.Data).reshape(InMel.Shape).unsqueeze(0); // [1, frames, n_mels]


    at::Tensor Output = Model({TorchMel}).toTensor(); // (audio frames)

    TFTensor<float> Tens = VoxUtil::CopyTensor<float>(Output);

    return Tens;


}

iSTFTNetTorch::iSTFTNetTorch()
{

}
