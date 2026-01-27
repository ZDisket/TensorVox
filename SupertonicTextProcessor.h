#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Available languages for multilingual TTS
extern const std::vector<std::string> AVAILABLE_LANGS;

class SupertonicTextProcessor {
public:
    explicit SupertonicTextProcessor(const std::string& unicode_indexer_json_path);

    struct Result {
        std::vector<std::vector<int64_t>> text_ids;
        std::vector<std::vector<std::vector<float>>> text_mask;
        std::vector<int64_t> lengths;
    };

    Result Process(
        const std::vector<std::string>& text_list,
        const std::vector<std::string>& lang_list
        );

    Result Process(
        const std::vector<std::u32string>& text_list,
        const std::vector<std::string>& lang_list
        );

private:
    std::vector<int64_t> indexer_;

    std::u32string preprocessText(const std::string& text, const std::string& lang);
    std::u32string preprocessText(const std::u32string& text, const std::string& lang);
    std::u32string normalizeText(const std::u32string& text);
    std::u32string ansiToU32(const std::string& text);
    std::vector<int64_t> codepointsToIds(const std::u32string& text);
    std::vector<std::vector<std::vector<float>>> getTextMask(
        const std::vector<int64_t>& text_ids_lengths
        );

    Result buildResult(const std::vector<std::u32string>& text_list);
};
