#pragma once

#include "VoxCommon.hpp"
#include <memory>
class MultiBandMelGAN
{
private:
    std::unique_ptr<cppflow::model> MelGAN;


public:
    virtual bool Initialize(const std::string& VocoderPath);


	// Do MultiBand MelGAN inference including PQMF
	// -> InMel:  Mel spectrogram (shape [1, xx, 80])
	// <- Returns: Tensor data [4, xx, 1]
    virtual TFTensor<float> DoInference(const TFTensor<float>& InMel);

	MultiBandMelGAN();
	~MultiBandMelGAN();
};

