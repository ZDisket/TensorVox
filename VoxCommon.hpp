#pragma once
/*
 VoxCommon.hpp : Defines common data structures and constants to be used with TensorVox 
*/
#include <iostream>

#undef slots // https://github.com/pytorch/pytorch/issues/19405


#pragma warning(push, 0) // LibTorch spams us with warnings
#include <torch/script.h> // One-stop header.
#pragma warning(pop)

#define slots Q_SLOTS

#include <vector>
#include "ext/AudioFile.hpp"
#include "ext/CppFlow/ops.h"
#include "ext/CppFlow/model.h"



#include <QMessageBox>



#define IF_RETURN(cond,ret) if (cond){return ret;}

const uint32_t CommonSampleRate = 48000;

namespace VoxCommon{
const std::string CommonLangConst = "_std";


}

// https://github.com/almogh52/rnnoise-cmake/blob/d981adb2e797216f456cfcf158f73761a29981f8/examples/rnnoise_demo.c#L31
const uint32_t RNNoiseFrameSize = 480;
typedef std::vector<std::tuple<std::string,cppflow::tensor>> TensorVec;

template<typename T>
struct TFTensor {
	std::vector<T> Data;
	std::vector<int64_t> Shape;
	size_t TotalSize;

};


namespace ETTSRepo {
enum Enum{
    TensorflowTTS = 0,
    CoquiTTS,
    jaywalnut310 // OG VITS repo
};

}
namespace EText2MelModel {
enum Enum{
    FastSpeech2 = 0,
    Tacotron2,
    VITS
};

}

namespace EVocoderModel{
enum Enum{
    MultiBandMelGAN = 0,
    MelGANSTFT, // there is no architectural changes so we can use mb-melgan class for melgan-stft
    NullVocoder // For fully E2E models
};
}

// ===========DEPRECATED===============
// Negative numbers denote character-based language, positive for phoneme based. Standard is char-equivalent language idx = negative(phn-based)
// In case of English, since -0 doesn't exist, we use -1.
// For example, German phonetic would be 3, and character based would be -3
// IPA-phn-based are mainly for Coqui
// ===========DEPRECATED===============
namespace ETTSLanguage{
enum Enum{
  GermanChar = -3,
  SpanishChar,
  EnglishChar,
  EnglishPhn,
  SpanishPhn,
  GermanPhn,
  EnglishIPA,
};

}

/* Language Spec Standard V1:
- Language is specified with a string from the JSON and the type is saved instead of relying
on ETTSLanguage enum.
-- The string is LanguageName-Method; for example English-StressedIPA, English-ARPA, German-Char
- Both pre-V1 standard and current are supported
- V1 Standard does not require changes in code to add new languages

*/

namespace ETTSLanguageType{
enum Enum{
    ARPA = 0,
    Char,
    IPA,
    GlobalPhone
};
}


struct ArchitectureInfo{
    int Repo;
    int Text2Mel;
    int Vocoder;

    // String versions of the info, for displaying.
    // We want boilerplate int index to str conversion code to be low.
    std::string s_Repo;
    std::string s_Text2Mel;
    std::string s_Vocoder;

};
struct VoiceInfo{
  std::string Name;
  std::string Author;
  int32_t Version;
  std::string Description;
  ArchitectureInfo Architecture;
  std::string Note;

  uint32_t SampleRate;

  std::string s_Language; // Language name = English-ARPA -> "English"
  std::string s_Language_Fullname; // Full language name = "English-ARPA"

  std::string EndPadding;
  int32_t LangType;



};

namespace VoxUtil {


    std::string U32ToStr(const std::u32string& InU32);
    std::u32string StrToU32(const std::string& InStr);

    std::vector<std::string> GetLinedFile(const std::string& Path);

    VoiceInfo ReadModelJSON(const std::string& InfoFilename);



    // Copy PyTorch tensor

    template<typename D>
    TFTensor<D> CopyTensor(at::Tensor& InTens){
        D* Data = InTens.data<D>();
        std::vector<int64_t> Shape = InTens.sizes().vec();

        size_t TotalSize = 1;

        for (const int64_t& Dim : Shape)
            TotalSize *= Dim;

        std::vector<D> DataVec = std::vector<D>(Data,Data + TotalSize);

        return TFTensor<D>{DataVec,Shape,TotalSize};


    }


    // Copy CppFlow (TF) tensor
	template<typename F>
    TFTensor<F> CopyTensor(cppflow::tensor& InTens)
	{
		std::vector<F> Data = InTens.get_data<F>();
        std::vector<int64_t> Shape = InTens.shape().get_data<int64_t>();
		size_t TotalSize = 1;
		for (const int64_t& Dim : Shape)
			TotalSize *= Dim;

		return TFTensor<F>{Data, Shape, TotalSize};


	}

    template<typename VXVec1>
    bool FindInVec(VXVec1 In, const std::vector<VXVec1>& Vec, size_t& OutIdx, size_t start = 0) {
		for (size_t xx = start;xx < Vec.size();xx++)
		{
			if (Vec[xx] == In) {
				OutIdx = xx;
				return true;

			}

		}


		return false;

	}
    template<typename VXVec1, typename X>
    bool FindInVec2(VXVec1 In, const std::vector<X>& Vec, size_t& OutIdx, size_t start = 0) {
        for (size_t xx = start;xx < Vec.size();xx++)
        {
            if (Vec[xx] == In) {
                OutIdx = xx;
                return true;

            }

        }


        return false;

    }

	void ExportWAV(const std::string& Filename, const std::vector<float>& Data, unsigned SampleRate);
}
