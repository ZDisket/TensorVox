#include "SupertonicTextProcessor.h"
#include "ext/json.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

using json = nlohmann::json;

// Available languages for multilingual TTS
const std::vector<std::string> AVAILABLE_LANGS = {"en", "ko", "es", "pt", "fr"};

namespace {
bool IsAsciiSpace(char32_t cp)
{
    if (cp > 0x7F) {
        return false;
    }
    return std::isspace(static_cast<unsigned char>(cp)) != 0;
}

bool IsEmoji(char32_t cp)
{
    return cp >= 0x1F000 && cp <= 0x1FFFF;
}

bool IsTerminalPunct(char32_t cp)
{
    switch (cp) {
    case U'.':
    case U'!':
    case U'?':
    case U';':
    case U':':
    case U',':
    case U'\'':
    case U'"':
    case U')':
    case U']':
    case U'}':
    case U'>':
    case U'\u2026': // …
    case U'\u3002': // 。
    case U'\u300D': // 」
    case U'\u300F': // 』
    case U'\u3011': // 】
    case U'\u3009': // 〉
    case U'\u300B': // 》
    case U'\u203A': // ›
    case U'\u00BB': // »
    case U'\u201C': // “
    case U'\u201D': // ”
    case U'\u2018': // ‘
    case U'\u2019': // ’
        return true;
    default:
        return false;
    }
}

void ReplaceAll(std::u32string& text, const std::u32string& from, const std::u32string& to)
{
    if (from.empty()) {
        return;
    }
    size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::u32string::npos) {
        text.replace(pos, from.size(), to);
        pos += to.size();
    }
}

std::u32string TrimSpaces(const std::u32string& text)
{
    size_t start = 0;
    while (start < text.size() && text[start] == U' ') {
        ++start;
    }
    size_t end = text.size();
    while (end > start && text[end - 1] == U' ') {
        --end;
    }
    return text.substr(start, end - start);
}

std::u32string NormalizeSpaces(const std::u32string& text)
{
    std::u32string result;
    result.reserve(text.size());
    bool prev_space = false;
    for (char32_t cp : text) {
        if (IsAsciiSpace(cp)) {
            if (!prev_space) {
                result.push_back(U' ');
                prev_space = true;
            }
            continue;
        }
        prev_space = false;
        result.push_back(cp);
    }
    return result;
}

std::u32string AsciiToU32(const std::string& text)
{
    std::u32string result;
    result.reserve(text.size());
    for (unsigned char ch : text) {
        result.push_back(static_cast<char32_t>(ch));
    }
    return result;
}
}

SupertonicTextProcessor::SupertonicTextProcessor(const std::string& unicode_indexer_json_path)
{
    std::ifstream file(unicode_indexer_json_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + unicode_indexer_json_path);
    }
    json j;
    file >> j;
    indexer_ = j.get<std::vector<int64_t>>();
}

SupertonicTextProcessor::Result SupertonicTextProcessor::Process(
    const std::vector<std::string>& text_list,
    const std::vector<std::string>& lang_list
    )
{
    std::vector<std::u32string> processed_texts;
    processed_texts.reserve(text_list.size());
    for (size_t i = 0; i < text_list.size(); ++i) {
        processed_texts.push_back(preprocessText(text_list[i], lang_list[i]));
    }
    return buildResult(processed_texts);
}

SupertonicTextProcessor::Result SupertonicTextProcessor::Process(
    const std::vector<std::u32string>& text_list,
    const std::vector<std::string>& lang_list
    )
{
    std::vector<std::u32string> processed_texts;
    processed_texts.reserve(text_list.size());
    for (size_t i = 0; i < text_list.size(); ++i) {
        processed_texts.push_back(preprocessText(text_list[i], lang_list[i]));
    }
    return buildResult(processed_texts);
}

std::u32string SupertonicTextProcessor::preprocessText(const std::string& text, const std::string& lang)
{
    return preprocessText(ansiToU32(text), lang);
}

std::u32string SupertonicTextProcessor::preprocessText(const std::u32string& text, const std::string& lang)
{
    std::u32string result = normalizeText(text);

    if (!result.empty()) {
        char32_t last_char = result.back();
        if (!IsTerminalPunct(last_char)) {
            result.push_back(U'.');
        }
    }

    bool valid_lang = false;
    for (const auto& available_lang : AVAILABLE_LANGS) {
        if (lang == available_lang) {
            valid_lang = true;
            break;
        }
    }
    if (!valid_lang) {
        throw std::runtime_error("Invalid language: " + lang + ". Available: en, ko, es, pt, fr");
    }

    std::u32string tag_lang = AsciiToU32(lang);
    std::u32string result_with_tags;
    result_with_tags.reserve(result.size() + tag_lang.size() * 2 + 5);
    result_with_tags += U"<";
    result_with_tags += tag_lang;
    result_with_tags += U">";
    result_with_tags += result;
    result_with_tags += U"</";
    result_with_tags += tag_lang;
    result_with_tags += U">";
    return result_with_tags;
}

std::u32string SupertonicTextProcessor::normalizeText(const std::u32string& text)
{
    std::u32string result;
    result.reserve(text.size());

    for (char32_t cp : text) {
        switch (cp) {
        case U'\u2013': // –
        case U'\u2011': // ‑
        case U'\u2014': // —
            cp = U'-';
            break;
        case U'_':
        case U'[':
        case U']':
        case U'|':
        case U'/':
        case U'#':
        case U'\u2192': // →
        case U'\u2190': // ←
            cp = U' ';
            break;
        case U'\u201C': // “
        case U'\u201D': // ”
            cp = U'"';
            break;
        case U'\u2018': // ‘
        case U'\u2019': // ’
        case U'\u00B4': // ´
        case U'`':
            cp = U'\'';
            break;
        default:
            break;
        }
        result.push_back(cp);
    }

    std::u32string emoji_filtered;
    emoji_filtered.reserve(result.size());
    for (char32_t cp : result) {
        if (!IsEmoji(cp)) {
            emoji_filtered.push_back(cp);
        }
    }

    std::u32string symbol_filtered;
    symbol_filtered.reserve(emoji_filtered.size());
    for (char32_t cp : emoji_filtered) {
        if (cp == U'\u2665' || cp == U'\u2606' || cp == U'\u2661' || cp == U'\u00A9' || cp == U'\\') {
            continue;
        }
        symbol_filtered.push_back(cp);
    }

    ReplaceAll(symbol_filtered, U"@", U" at ");
    ReplaceAll(symbol_filtered, U"e.g.,", U"for example, ");
    ReplaceAll(symbol_filtered, U"i.e.,", U"that is, ");

    ReplaceAll(symbol_filtered, U" ,", U",");
    ReplaceAll(symbol_filtered, U" .", U".");
    ReplaceAll(symbol_filtered, U" !", U"!");
    ReplaceAll(symbol_filtered, U" ?", U"?");
    ReplaceAll(symbol_filtered, U" ;", U";");
    ReplaceAll(symbol_filtered, U" :", U":");
    ReplaceAll(symbol_filtered, U" '", U"'");

    while (symbol_filtered.find(U"\"\"") != std::u32string::npos) {
        ReplaceAll(symbol_filtered, U"\"\"", U"\"");
    }
    while (symbol_filtered.find(U"''") != std::u32string::npos) {
        ReplaceAll(symbol_filtered, U"''", U"'");
    }
    while (symbol_filtered.find(U"``") != std::u32string::npos) {
        ReplaceAll(symbol_filtered, U"``", U"`");
    }

    std::u32string normalized = NormalizeSpaces(symbol_filtered);
    normalized = TrimSpaces(normalized);
    return normalized;
}

std::u32string SupertonicTextProcessor::ansiToU32(const std::string& text)
{
    std::u32string result;
    size_t i = 0;
    while (i < text.size()) {
        uint32_t codepoint = 0;
        unsigned char c = static_cast<unsigned char>(text[i]);
        codepoint = c;
        i += 1;
        result.push_back(static_cast<char32_t>(codepoint));
    }
    return result;
}

// Hangul syllable decomposition constants (Unicode Standard Annex #15)
static const uint32_t HANGUL_SBASE = 0xAC00;
static const uint32_t HANGUL_LBASE = 0x1100;
static const uint32_t HANGUL_VBASE = 0x1161;
static const uint32_t HANGUL_TBASE = 0x11A7;
static const int HANGUL_LCOUNT = 19;
static const int HANGUL_VCOUNT = 21;
static const int HANGUL_TCOUNT = 28;
static const int HANGUL_NCOUNT = HANGUL_VCOUNT * HANGUL_TCOUNT;
static const int HANGUL_SCOUNT = HANGUL_LCOUNT * HANGUL_NCOUNT;

// Latin character NFKD decompositions for Spanish, Portuguese, French
static const std::unordered_map<uint32_t, std::vector<char32_t>> LATIN_DECOMPOSITIONS = {
    {0x00C1, {0x0041, 0x0301}},
    {0x00C9, {0x0045, 0x0301}},
    {0x00CD, {0x0049, 0x0301}},
    {0x00D3, {0x004F, 0x0301}},
    {0x00DA, {0x0055, 0x0301}},
    {0x00E1, {0x0061, 0x0301}},
    {0x00E9, {0x0065, 0x0301}},
    {0x00ED, {0x0069, 0x0301}},
    {0x00F3, {0x006F, 0x0301}},
    {0x00FA, {0x0075, 0x0301}},
    {0x00C0, {0x0041, 0x0300}},
    {0x00C8, {0x0045, 0x0300}},
    {0x00CC, {0x0049, 0x0300}},
    {0x00D2, {0x004F, 0x0300}},
    {0x00D9, {0x0055, 0x0300}},
    {0x00E0, {0x0061, 0x0300}},
    {0x00E8, {0x0065, 0x0300}},
    {0x00EC, {0x0069, 0x0300}},
    {0x00F2, {0x006F, 0x0300}},
    {0x00F9, {0x0075, 0x0300}},
    {0x00C2, {0x0041, 0x0302}},
    {0x00CA, {0x0045, 0x0302}},
    {0x00CE, {0x0049, 0x0302}},
    {0x00D4, {0x004F, 0x0302}},
    {0x00DB, {0x0055, 0x0302}},
    {0x00E2, {0x0061, 0x0302}},
    {0x00EA, {0x0065, 0x0302}},
    {0x00EE, {0x0069, 0x0302}},
    {0x00F4, {0x006F, 0x0302}},
    {0x00FB, {0x0075, 0x0302}},
    {0x00C3, {0x0041, 0x0303}},
    {0x00D1, {0x004E, 0x0303}},
    {0x00D5, {0x004F, 0x0303}},
    {0x00E3, {0x0061, 0x0303}},
    {0x00F1, {0x006E, 0x0303}},
    {0x00F5, {0x006F, 0x0303}},
    {0x00C4, {0x0041, 0x0308}},
    {0x00CB, {0x0045, 0x0308}},
    {0x00CF, {0x0049, 0x0308}},
    {0x00D6, {0x004F, 0x0308}},
    {0x00DC, {0x0055, 0x0308}},
    {0x00E4, {0x0061, 0x0308}},
    {0x00EB, {0x0065, 0x0308}},
    {0x00EF, {0x0069, 0x0308}},
    {0x00F6, {0x006F, 0x0308}},
    {0x00FC, {0x0075, 0x0308}},
    {0x00C7, {0x0043, 0x0327}},
    {0x00E7, {0x0063, 0x0327}},
    };

static void DecomposeCharacter(char32_t codepoint, std::u32string& output)
{
    if (codepoint >= HANGUL_SBASE && codepoint < HANGUL_SBASE + HANGUL_SCOUNT) {
        uint32_t sIndex = static_cast<uint32_t>(codepoint) - HANGUL_SBASE;
        uint32_t lIndex = sIndex / HANGUL_NCOUNT;
        uint32_t vIndex = (sIndex % HANGUL_NCOUNT) / HANGUL_TCOUNT;
        uint32_t tIndex = sIndex % HANGUL_TCOUNT;

        output.push_back(static_cast<char32_t>(HANGUL_LBASE + lIndex));
        output.push_back(static_cast<char32_t>(HANGUL_VBASE + vIndex));
        if (tIndex > 0) {
            output.push_back(static_cast<char32_t>(HANGUL_TBASE + tIndex));
        }
        return;
    }

    auto it = LATIN_DECOMPOSITIONS.find(static_cast<uint32_t>(codepoint));
    if (it != LATIN_DECOMPOSITIONS.end()) {
        for (char32_t cp : it->second) {
            output.push_back(cp);
        }
        return;
    }

    output.push_back(codepoint);
}

std::vector<int64_t> SupertonicTextProcessor::codepointsToIds(const std::u32string& text)
{
    std::vector<int64_t> ids;
    ids.reserve(text.size());

    std::u32string decomposed;
    decomposed.reserve(text.size());
    for (char32_t cp : text) {
        DecomposeCharacter(cp, decomposed);
    }

    for (char32_t cp : decomposed) {
        if (cp < indexer_.size()) {
            int64_t id = indexer_[static_cast<size_t>(cp)];
            if (id >= 0) {
                ids.push_back(id);
            }
        }
    }

    return ids;
}

std::vector<std::vector<std::vector<float>>> SupertonicTextProcessor::getTextMask(
    const std::vector<int64_t>& text_ids_lengths
    )
{
    int64_t max_len = 0;
    for (auto len : text_ids_lengths) {
        if (len > max_len) {
            max_len = len;
        }
    }

    std::vector<std::vector<std::vector<float>>> mask;
    mask.reserve(text_ids_lengths.size());
    for (auto len : text_ids_lengths) {
        std::vector<std::vector<float>> batch_mask(1);
        batch_mask[0].resize(static_cast<size_t>(max_len));
        for (int64_t i = 0; i < max_len; ++i) {
            batch_mask[0][static_cast<size_t>(i)] = (i < len) ? 1.0f : 0.0f;
        }
        mask.push_back(std::move(batch_mask));
    }
    return mask;
}

SupertonicTextProcessor::Result SupertonicTextProcessor::buildResult(
    const std::vector<std::u32string>& text_list
    )
{
    Result result;
    std::vector<std::vector<int64_t>> all_ids;
    all_ids.reserve(text_list.size());
    result.lengths.reserve(text_list.size());

    for (size_t i = 0; i < text_list.size(); ++i) {
        std::vector<int64_t> ids = codepointsToIds(text_list[i]);
        result.lengths.push_back(static_cast<int64_t>(ids.size()));
        all_ids.push_back(std::move(ids));
    }

    int64_t max_len = 0;
    for (auto len : result.lengths) {
        if (len > max_len) {
            max_len = len;
        }
    }

    result.text_ids.resize(text_list.size());
    for (size_t i = 0; i < all_ids.size(); ++i) {
        result.text_ids[i].assign(static_cast<size_t>(max_len), 0);
        for (size_t j = 0; j < all_ids[i].size(); ++j) {
            result.text_ids[i][j] = all_ids[i][j];
        }
    }

    result.text_mask = getTextMask(result.lengths);
    return result;
}
