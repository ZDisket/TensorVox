#include "vits.h"


std::vector<int64_t> VITS::ZeroPadVec(const std::vector<int32_t> &InIDs)
{
    std::vector<int64_t> NewIDs;
    NewIDs.reserve(InIDs.size() * 2);

    for (auto CharID : InIDs)
    {
        NewIDs.push_back(0);
        NewIDs.push_back((int64_t)CharID);

    }
    // Add final 0
    NewIDs.push_back(0);

    return NewIDs;

}

VITS::VITS()
{

}

bool VITS::Initialize(const std::string &SavedModelFolder, ETTSRepo::Enum InTTSRepo)
{
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().

        Model = torch::jit::load(SavedModelFolder);

    }
    catch (const c10::Error& e) {
        return false;

    }

    CurrentRepo = InTTSRepo;
    return true;
}

TFTensor<float> VITS::DoInference(const std::vector<int32_t> &InputIDs, const std::vector<float> &ArgsFloat, const std::vector<int32_t> ArgsInt, int32_t SpeakerID, int32_t EmotionID)
{
    std::vector<int64_t> PaddedIDs = ZeroPadVec(InputIDs);
    std::vector<int64_t> inLen = { (int64_t)PaddedIDs.size() };


    // ZDisket: Is this really necessary?
    torch::TensorOptions Opts = torch::TensorOptions().requires_grad(false);

    auto InIDS = torch::tensor(PaddedIDs, Opts).unsqueeze(0);
    auto InLens = torch::tensor(inLen, Opts);
    auto InLenScale = torch::tensor({ ArgsFloat[0]}, Opts);


    std::vector<torch::jit::IValue> inputs{ InIDS,InLens,InLenScale };

    // Infer

    c10::IValue Output = Model.get_method("infer_ts")(inputs);

    // Output = tuple (audio,att)

    auto OutputT = Output.toTuple();

    // Grab audio
    // [1, frames] -> [frames]
    auto AuTens = OutputT.get()->elements()[0].toTensor().squeeze();

    // Grab Attention
    // [1, 1, x, y] -> [x, y]
    auto AttTens = OutputT.get()->elements()[1].toTensor().squeeze();

    Attention = VoxUtil::CopyTensor<float>(AttTens);

    return VoxUtil::CopyTensor<float>(AuTens);

}
