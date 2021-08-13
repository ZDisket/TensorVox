#include "MultiBandMelGAN.h"
#define IF_EXCEPT(cond,ex) if (cond){throw std::exception(ex);}



bool MultiBandMelGAN::Initialize(const std::string & VocoderPath)
{
	try {
        MelGAN = std::make_unique<cppflow::model>(VocoderPath);
	}
	catch (...) {
		return false;

	}
	return true;


}

TFTensor<float> MultiBandMelGAN::DoInference(const TFTensor<float>& InMel)
{
    IF_EXCEPT(!MelGAN, "Tried to infer MB-MelGAN on uninitialized model!!!!")

    // Convenience reference so that we don't have to constantly derefer pointers.
    cppflow::model& Mdl = *MelGAN;


    cppflow::tensor input_mels{ InMel.Data, InMel.Shape};


    auto out_audio = Mdl({{"serving_default_mels:0",input_mels}}, {"StatefulPartitionedCall:0"})[0];
    TFTensor<float> RetTensor = VoxUtil::CopyTensor<float>(out_audio);

    return RetTensor;





}

MultiBandMelGAN::MultiBandMelGAN()
{
	MelGAN = nullptr;
}


MultiBandMelGAN::~MultiBandMelGAN()
{


}
