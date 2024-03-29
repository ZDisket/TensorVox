#include "vits.h"

std::vector<int64_t> VITS::ZeroPadVec(const std::vector<int32_t> &InIDs)
{
    std::vector<int64_t> NewIDs;
    NewIDs.reserve(InIDs.size() * 2);

    NewIDs.push_back(0);

    for (auto CharID : InIDs)
    {

        NewIDs.push_back((int64_t)CharID);
        NewIDs.push_back(0);


    }
    // Add final 0
   // NewIDs.push_back(0);


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
    // without this memory consumption is 4x
    torch::NoGradGuard no_grad;

    // TorchMoji hidden states are added to ArgsFloat
    const bool UsesTorchMoji = ArgsFloat.size() > 1;

    std::vector<int64_t> PaddedIDs;


    // Our current TM-enabled models don't use zero interspersion
    if (UsesTorchMoji)
        PaddedIDs.assign(InputIDs.begin(),InputIDs.end());
    else
        PaddedIDs = ZeroPadVec(InputIDs);


    std::vector<int64_t> inLen = { (int64_t)PaddedIDs.size() };


    // ZDisket: Is this really necessary?
    torch::TensorOptions Opts = torch::TensorOptions().requires_grad(false);

    auto InIDS = torch::tensor(PaddedIDs, Opts).unsqueeze(0);
    auto InLens = torch::tensor(inLen, Opts);
    auto InLenScale = torch::tensor({ ArgsFloat[0]}, Opts);



    std::vector<torch::jit::IValue> inputs{ InIDS,InLens,InLenScale };

    if (SpeakerID != -1){
        auto InSpkid = torch::tensor({SpeakerID},Opts);
        inputs.push_back(InSpkid);
    }

    if (EmotionID != -1){
        auto InEmid = torch::tensor({EmotionID},Opts);
        inputs.push_back(InEmid);
    }

    // Handle TorchMoji Emb
    if (UsesTorchMoji){
        // Make a copy stripping first elem
        std::vector<float> TMHidden(ArgsFloat.begin() + 1, ArgsFloat.end());

        auto InMoji = torch::tensor(TMHidden,Opts).unsqueeze(0);
        inputs.push_back(InMoji);

    }

    // Infer

    c10::IValue Output = Model.get_method("infer_ts")(inputs);

    // Output = tuple (audio,att)

    auto OutputT = Output.toTuple();

    // Grab audio
    // [1, frames] -> [frames]
    auto AuTens = OutputT.get()->elements()[0].toTensor().squeeze();

    // Grab Attention
    // [1, 1, x, y] -> [x, y] -> [y,x] -> [1, y, x]
    auto AttTens = OutputT.get()->elements()[1].toTensor().squeeze().transpose(0,1).unsqueeze(0);

    Attention = VoxUtil::CopyTensor<float>(AttTens);

    return VoxUtil::CopyTensor<float>(AuTens);

}
