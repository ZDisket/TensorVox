#include "devits.h"

DEVITS::DEVITS()
{

}

TFTensor<float> DEVITS::DoInferenceDE(const std::vector<int32_t> &InputIDs, const TFTensor<float> &MojiIn, const TFTensor<float> &BERTIn, const std::vector<float> &ArgsFloat, const std::vector<int32_t> ArgsInt, int32_t SpeakerID, int32_t EmotionID)
{
    // without this memory consumption is 4x
    torch::NoGradGuard no_grad;



    std::vector<int64_t> PaddedIDs;


    PaddedIDs = ZeroPadVec(InputIDs);


    std::vector<int64_t> inLen = { (int64_t)PaddedIDs.size() };


    // ZDisket: Is this really necessary?
    torch::TensorOptions Opts = torch::TensorOptions().requires_grad(false);

    auto InIDS = torch::tensor(PaddedIDs, Opts).unsqueeze(0);
    auto InLens = torch::tensor(inLen, Opts);
    auto MojiHidden = torch::tensor(MojiIn.Data);
    auto BERTHidden = torch::tensor(BERTIn.Data).reshape(BERTIn.Shape);

    std::vector<int64_t> BERTSz = {BERTIn.Shape[1]};
    auto BERTLens = torch::tensor(BERTSz);

    auto InLenScale = torch::tensor({ ArgsFloat[0]}, Opts);



    std::vector<torch::jit::IValue> inputs{ InIDS,InLens, MojiHidden, BERTHidden, BERTLens, InLenScale };

    if (SpeakerID != -1){
        auto InSpkid = torch::tensor({SpeakerID},Opts);
        inputs.push_back(InSpkid);
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
