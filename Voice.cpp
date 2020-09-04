#include "Voice.h"
#include "ext/ZCharScanner.h"


const std::vector<int32_t> enPhonemeIDs = { 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76,
77, 78, 79, 80, 81, 82, 83, 84, 85,  86, 87, 88, 89, 90, 91, 92,93, 94, 95, 96, 97,
98,99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136,
137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149 };


std::vector<int32_t> Voice::PhonemesToID(const std::string & InTxt)
{
	ZStringDelimiter Delim(InTxt);
	Delim.AddDelimiter(" ");

	std::vector<int32_t> VecPhones;
	VecPhones.reserve(Delim.szTokens());

	for (const auto& Pho : Delim.GetTokens()) 
	{
		size_t ArrID = 0;

        if (VoxUtil::FindInVec<std::string>(Pho, enPhonemes, ArrID))
            VecPhones.push_back(enPhonemeIDs[ArrID]);
		else
			cout << "Voice::PhonemesToID() WARNING: Unknown phoneme " << Pho << endl;



	}
	// Prevent out of range error in single word input
	if (VecPhones.size() > 1)
	{
		if (VecPhones[VecPhones.size() - 1] != 148)
			VecPhones.push_back(148);
	}
	else 
	{
		VecPhones.push_back(148);

	}


	return VecPhones;

}

Voice::Voice(const std::string & VoxPath, const string &inName)
{
	MelPredictor.Initialize(VoxPath + "/melgen");
	Vocoder.Initialize(VoxPath + "/vocoder");
	Processor.Initialize(VoxPath + "/g2p.fst");
    VoxInfo = VoxUtil::ReadModelJSON(VoxPath + "/info.json");
    Name = inName;

}

std::vector<float> Voice::Vocalize(const std::string & Prompt, float Speed, int32_t SpeakerID, float Energy, float F0)
{

	std::string PhoneticTxt = Processor.ProcessTextPhonetic(Prompt);

	TFTensor<float> Mel = MelPredictor.DoInference(PhonemesToID(PhoneticTxt), SpeakerID, Speed, Energy, F0);

	TFTensor<float> AuData = Vocoder.DoInference(Mel);


	int64_t Width = AuData.Shape[0];
	int64_t Height = AuData.Shape[1];
	int64_t Depth = AuData.Shape[2];
	//int z = 0;

	std::vector<float> AudioData;
	AudioData.resize(Height);

	// Code to access 1D array as if it were 3D
	for (int64_t x = 0; x < Width;x++)
	{
		for (int64_t z = 0;z < Depth;z++)
		{
			for (int64_t y = 0; y < Height;y++) {
				int64_t Index = x * Height * Depth + y * Depth + z;
				AudioData[(size_t)y] = AuData.Data[(size_t)Index];

			}

		}
	}


	return AudioData;
}

Voice::~Voice()
{
}
