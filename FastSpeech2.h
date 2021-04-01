#pragma once

#include "melgen.h"


class FastSpeech2 : public MelGen
{

public:
	FastSpeech2();



	/*
	Do inference on a FastSpeech2 model.

	-> InputIDs: Input IDs of tokens for inference
	-> SpeakerID: ID of the speaker in the model to do inference on. If single speaker, always leave at 0. If multispeaker, refer to your model.
    -> (In ArgsFloat)Speed, Energy, F0: Parameters for FS2 inference. Leave at 1.f for defaults

	<- Returns: TFTensor<float> with shape {1,<len of mel in frames>,80} containing contents of mel spectrogram. 
	*/
    TFTensor<float> DoInference(const std::vector<int32_t>& InputIDs,const std::vector<float>& ArgsFloat,const std::vector<int32_t> ArgsInt, int32_t SpeakerID = 0, int32_t EmotionID = -1);



	~FastSpeech2();
};

