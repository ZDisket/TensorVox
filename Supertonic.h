#pragma once

#include "ONNXModel.h"
#include "melgen.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Supertonic : public ONNXModel, public MelGen
{
public:
    bool Initialize(const std::string& onnxDir, ETTSRepo::Enum InTTSRepo) override;
    bool LoadStyle(const std::string& stylePath);
    TFTensor<float> DoInference(const std::vector<int32_t>& InputIDs, const std::vector<float>& ArgsFloat,
                                const std::vector<int32_t> ArgsInt, int32_t SpeakerID = 0, int32_t EmotionID = -1) override;

    int GetSampleRate() const { return sample_rate_; }

private:
    struct StyleData {
        std::vector<float> ttl_data;
        std::vector<int64_t> ttl_shape;
        std::vector<float> dp_data;
        std::vector<int64_t> dp_shape;
        bool loaded = false;
    };

    std::string BasePath;
    std::string CurrentLoadedVoice;

    void EnsureSpeaker(int32_t SpeakerID);

    bool LoadConfig(const std::string& onnxDir);
    bool LoadModels(const std::string& onnxDir);

    std::unique_ptr<ONNXModel> duration_predictor_;
    std::unique_ptr<ONNXModel> text_encoder_;
    std::unique_ptr<ONNXModel> vector_estimator_;
    bool models_loaded_ = false;
    bool config_loaded_ = false;

    int sample_rate_ = 0;
    int base_chunk_size_ = 0;
    int chunk_compress_factor_ = 0;
    int latent_dim_ = 0;

    StyleData style_;
};
