#ifndef ISTFTNETTORCH_H
#define ISTFTNETTORCH_H
#include "MultiBandMelGAN.h"

class iSTFTNetTorch : public MultiBandMelGAN
{
private:
   torch::jit::script::Module Model;

public:
    bool Initialize(const std::string& VocoderPath);



    // Do MultiBand MelGAN inference including PQMF
    // -> InMel:  Mel spectrogram (shape [1, xx, 80])
    // <- Returns: Tensor data  [frames]
    virtual TFTensor<float> DoInference(const TFTensor<float>& InMel);
    iSTFTNetTorch();
};

#endif // ISTFTNETTORCH_H
