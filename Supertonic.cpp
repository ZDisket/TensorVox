#include "Supertonic.h"
#include "ext/json.hpp"

#include <algorithm>
#include <fstream>
#include <random>
#include <filesystem>

using json = nlohmann::json;

namespace {
std::wstring ToWide(const std::string& value)
{
    return std::wstring(value.begin(), value.end());
}

template<typename T>
Ort::Value CreateTensor(Ort::MemoryInfo& mem_info, std::vector<T>& data, const std::vector<int64_t>& shape)
{
    return Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());
}
}

bool Supertonic::Initialize(const std::string& onnxDir, ETTSRepo::Enum InTTSRepo)
{
    CurrentRepo = InTTSRepo;
    config_loaded_ = LoadConfig(onnxDir);
    models_loaded_ = LoadModels(onnxDir);

    std::filesystem::path p(onnxDir);
    BasePath = p.parent_path().generic_string(); // generic_string always returns in forward slash style.

    CurrentLoadedVoice = "";

    return config_loaded_ && models_loaded_;
}

bool Supertonic::LoadConfig(const std::string& onnxDir)
{
    std::ifstream file(onnxDir + "/tts.json");
    if (!file.is_open()) {
        return false;
    }

    json cfg;
    file >> cfg;

    sample_rate_ = cfg["ae"]["sample_rate"].get<int>();
    base_chunk_size_ = cfg["ae"]["base_chunk_size"].get<int>();
    chunk_compress_factor_ = cfg["ttl"]["chunk_compress_factor"].get<int>();
    latent_dim_ = cfg["ttl"]["latent_dim"].get<int>();





    return true;
}

bool Supertonic::LoadModels(const std::string& onnxDir)
{
    duration_predictor_ = std::make_unique<ONNXModel>();
    text_encoder_ = std::make_unique<ONNXModel>();
    vector_estimator_ = std::make_unique<ONNXModel>();

    const std::wstring dp_path = ToWide(onnxDir + "/duration_predictor.onnx");
    const std::wstring text_enc_path = ToWide(onnxDir + "/text_encoder.onnx");
    const std::wstring vector_est_path = ToWide(onnxDir + "/vector_estimator.onnx");

    const bool dp_loaded = duration_predictor_->Load(dp_path, "supertonic_dp");
    const bool text_loaded = text_encoder_->Load(text_enc_path, "supertonic_text_enc");
    const bool vector_loaded = vector_estimator_->Load(vector_est_path, "supertonic_vector_est");

    _overrideDevices(duration_predictor_->IsGPUDevice(), duration_predictor_->GetDeviceName());

    return dp_loaded && text_loaded && vector_loaded;
}

bool Supertonic::LoadStyle(const std::string& stylePath)
{
    std::ifstream file(stylePath);
    if (!file.is_open()) {
        return false;
    }

    json style_json;
    file >> style_json;

    style_.ttl_shape = style_json["style_ttl"]["dims"].get<std::vector<int64_t>>();
    style_.dp_shape = style_json["style_dp"]["dims"].get<std::vector<int64_t>>();

    auto ttl_data_nested = style_json["style_ttl"]["data"].get<std::vector<std::vector<std::vector<float>>>>();
    style_.ttl_data.clear();
    for (const auto& batch : ttl_data_nested) {
        for (const auto& row : batch) {
            style_.ttl_data.insert(style_.ttl_data.end(), row.begin(), row.end());
        }
    }

    auto dp_data_nested = style_json["style_dp"]["data"].get<std::vector<std::vector<std::vector<float>>>>();
    style_.dp_data.clear();
    for (const auto& batch : dp_data_nested) {
        for (const auto& row : batch) {
            style_.dp_data.insert(style_.dp_data.end(), row.begin(), row.end());
        }
    }

    style_.loaded = true;
    return true;
}

TFTensor<float> Supertonic::DoInference(const std::vector<int32_t>& InputIDs, const std::vector<float>& ArgsFloat,
                                       const std::vector<int32_t> ArgsInt, int32_t SpeakerID, int32_t EmotionID)
{
    (void)EmotionID;
    EnsureSpeaker(SpeakerID);

    TFTensor<float> result;
    if (InputIDs.empty()) {
        return result;
    }
    if (!models_loaded_ || !config_loaded_ || !style_.loaded) {
        throw std::runtime_error("Supertonic not initialized or style not loaded");
    }

    int total_step = ArgsInt.empty() ? 6 : ArgsInt[0];
    float speed = ArgsFloat.empty() ? 1.05f : ArgsFloat[0];
    if (total_step < 1) {
        total_step = 1;
    }
    if (speed <= 0.0f) {
        speed = 1.0f;
    }

    std::vector<int64_t> text_ids;
    text_ids.reserve(InputIDs.size());
    for (auto id : InputIDs) {
        text_ids.push_back(static_cast<int64_t>(id));
    }

    const int64_t batch = 1;
    const int64_t seq_len = static_cast<int64_t>(text_ids.size());

    std::vector<int64_t> text_shape{batch, seq_len};
    std::vector<int64_t> text_mask_shape{batch, 1, seq_len};
    std::vector<float> text_mask(seq_len, 1.0f);

    Ort::MemoryInfo mem_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    std::vector<Ort::Value> dp_inputs;
    dp_inputs.emplace_back(CreateTensor(mem_info, text_ids, text_shape));
    dp_inputs.emplace_back(CreateTensor(mem_info, style_.dp_data, style_.dp_shape));
    dp_inputs.emplace_back(CreateTensor(mem_info, text_mask, text_mask_shape));

    auto dp_outputs = duration_predictor_->Forward(dp_inputs, {"text_ids", "style_dp", "text_mask"}, {"duration"});
    if (dp_outputs.empty() || !dp_outputs[0].IsTensor()) {
        return result;
    }

    auto duration_info = dp_outputs[0].GetTensorTypeAndShapeInfo();
    const float* duration_ptr = dp_outputs[0].GetTensorData<float>();
    float duration = duration_info.GetElementCount() ? duration_ptr[0] : 0.0f;
    duration /= speed;

    std::vector<Ort::Value> text_inputs;
    text_inputs.emplace_back(CreateTensor(mem_info, text_ids, text_shape));
    text_inputs.emplace_back(CreateTensor(mem_info, style_.ttl_data, style_.ttl_shape));
    text_inputs.emplace_back(CreateTensor(mem_info, text_mask, text_mask_shape));

    auto text_outputs = text_encoder_->Forward(text_inputs, {"text_ids", "style_ttl", "text_mask"}, {"text_emb"});
    if (text_outputs.empty() || !text_outputs[0].IsTensor()) {
        return result;
    }

    auto text_emb_info = text_outputs[0].GetTensorTypeAndShapeInfo();
    std::vector<int64_t> text_emb_shape = text_emb_info.GetShape();
    size_t text_emb_count = text_emb_info.GetElementCount();
    const float* text_emb_ptr = text_outputs[0].GetTensorData<float>();
    std::vector<float> text_emb_vec(text_emb_ptr, text_emb_ptr + text_emb_count);

    int64_t wav_len = static_cast<int64_t>(duration * sample_rate_);
    wav_len = std::max<int64_t>(wav_len, 1);
    int64_t chunk_size = static_cast<int64_t>(base_chunk_size_) * chunk_compress_factor_;
    int64_t latent_len = std::max<int64_t>(1, (wav_len + chunk_size - 1) / chunk_size);
    int64_t latent_dim = static_cast<int64_t>(latent_dim_) * chunk_compress_factor_;

    std::vector<int64_t> latent_shape{batch, latent_dim, latent_len};
    std::vector<int64_t> latent_mask_shape{batch, 1, latent_len};
    std::vector<float> latent_mask(latent_len, 1.0f);

    std::vector<float> xt(static_cast<std::size_t>(latent_dim * latent_len));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(0.0f, 1.0f);

    for (int64_t d = 0; d < latent_dim; ++d) {
        for (int64_t t = 0; t < latent_len; ++t) {
            xt[static_cast<std::size_t>(d * latent_len + t)] = dist(gen) * latent_mask[static_cast<std::size_t>(t)];
        }
    }

    std::vector<int64_t> scalar_shape{batch};
    std::vector<float> total_step_vec(batch, static_cast<float>(total_step));

    for (int step = 0; step < total_step; ++step) {
        std::vector<float> current_step_vec(batch, static_cast<float>(step));
        std::vector<Ort::Value> vector_inputs;
        vector_inputs.emplace_back(CreateTensor(mem_info, xt, latent_shape));
        vector_inputs.emplace_back(CreateTensor(mem_info, text_emb_vec, text_emb_shape));
        vector_inputs.emplace_back(CreateTensor(mem_info, style_.ttl_data, style_.ttl_shape));
        vector_inputs.emplace_back(CreateTensor(mem_info, text_mask, text_mask_shape));
        vector_inputs.emplace_back(CreateTensor(mem_info, latent_mask, latent_mask_shape));
        vector_inputs.emplace_back(CreateTensor(mem_info, total_step_vec, scalar_shape));
        vector_inputs.emplace_back(CreateTensor(mem_info, current_step_vec, scalar_shape));

        auto vector_outputs = vector_estimator_->Forward(
            vector_inputs,
            {"noisy_latent", "text_emb", "style_ttl", "text_mask", "latent_mask", "total_step", "current_step"},
            {"denoised_latent"}
        );

        if (vector_outputs.empty() || !vector_outputs[0].IsTensor()) {
            return result;
        }

        auto denoised_info = vector_outputs[0].GetTensorTypeAndShapeInfo();
        const float* denoised_ptr = vector_outputs[0].GetTensorData<float>();
        size_t denoised_count = denoised_info.GetElementCount();
        xt.assign(denoised_ptr, denoised_ptr + denoised_count);
    }

    result.Data = xt;
    result.Shape = latent_shape;
    result.TotalSize = xt.size();
    return result;
}

void Supertonic::EnsureSpeaker(int32_t SpeakerID)
{
    std::string RequestedSpeakerJSON = std::to_string(SpeakerID) + ".json";

    if (RequestedSpeakerJSON != CurrentLoadedVoice){
        LoadStyle(BasePath + "/styles/" + RequestedSpeakerJSON);
        CurrentLoadedVoice = RequestedSpeakerJSON;
    }


}
