#include "FastSpeech2.h"



FastSpeech2::FastSpeech2()
{
	FastSpeech = nullptr;
}

FastSpeech2::FastSpeech2(const std::string & SavedModelFolder)
{
	Initialize(SavedModelFolder);
}


bool FastSpeech2::Initialize(const std::string & SavedModelFolder)
{
	try {
		FastSpeech = new Model(SavedModelFolder);
	}
	catch (...) {
		FastSpeech = nullptr;
		return false;

	}
	return true;
}

TFTensor<float> FastSpeech2::DoInference(const std::vector<int32_t>& InputIDs, int32_t SpeakerID, float Speed, float Energy, float F0)
{
	if (!FastSpeech)
		throw std::exception("Tried to do inference on unloaded or invalid model!");

	// Convenience reference so that we don't have to constantly derefer pointers.
	Model& Mdl = *FastSpeech;

	// Define the tensors
	Tensor input_ids{ Mdl,"serving_default_input_ids" };
	Tensor energy_ratios{ Mdl,"serving_default_energy_ratios" };
	Tensor f0_ratios{ Mdl,"serving_default_f0_ratios" };
	Tensor speaker_ids{ Mdl,"serving_default_speaker_ids" };
	Tensor speed_ratios{ Mdl,"serving_default_speed_ratios" };

	// This is the shape of the input IDs, our equivalent to tf.expand_dims.
	std::vector<int64_t> InputIDShape = { 1, (int64_t)InputIDs.size() };

	input_ids.set_data(InputIDs, InputIDShape);
	energy_ratios.set_data(std::vector<float>{ Energy });
	f0_ratios.set_data(std::vector<float>{F0});
	speaker_ids.set_data(std::vector<int32_t>{SpeakerID});
	speed_ratios.set_data(std::vector<float>{Speed});

	// Define output tensor
	Tensor output{ Mdl,"StatefulPartitionedCall" };


	// Vector of input tensors
	std::vector<Tensor*> inputs = { &input_ids,&speaker_ids,&speed_ratios,&f0_ratios,&energy_ratios };


	// Do inference
	FastSpeech->run(inputs, output);

	// Define output and return it
	TFTensor<float> Output = VoxUtil::CopyTensor<float>(output);

	// We could just straight out define it in the return statement, but I like it more this way
	return Output;
}

FastSpeech2::~FastSpeech2()
{
	if (FastSpeech)
		delete FastSpeech;
}
