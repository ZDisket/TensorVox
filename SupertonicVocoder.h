#pragma once

#include "ONNXModel.h"
#include "MultiBandMelGAN.h"
#include <string>

class SupertonicVocoder : public ONNXModel, public MultiBandMelGAN
{
public:
    bool Initialize(const std::string& vocoderPath);
    TFTensor<float> DoInference(const TFTensor<float>& InMel);
};
