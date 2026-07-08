#pragma once
// =============================================================================
//  Game/Core/CampaignID.h
//
//  @version 8.1
//  Identificador determinístico de uma campanha, para diferenciar duas
//  campanhas com o mesmo nome ou versões diferentes da mesma campanha ao
//  longo do tempo (questão levantada por Rafael, sem resposta anterior).
//
//  ALGORITMO: FNV-1a 64-bit sobre a concatenação dos bytes crus de
//  campaign.txt + cada .lvl referenciado (pela ordem listada). Qualquer
//  alteração a um nível, ou à ordem/composição da campanha, muda o ID.
//  A mesma campanha (bytes identicos) produz sempre o mesmo ID, em
//  qualquer maquina (FNV-1a é puramente aritmético, sem dependencia de
//  plataforma — ao contrário de std::hash, que a norma C++ permite variar
//  entre execuções/compiladores e por isso NAO serve para isto).
//
//  Formato de saida: 16 caracteres hex (64 bits), ex: "a1b2c3d4e5f60718".
// =============================================================================
#include <cstdint>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

namespace core {

inline uint64_t fnv1a64(const uint8_t* data, size_t len, uint64_t seed = 14695981039346656037ull) {
    constexpr uint64_t PRIME = 1099511628211ull;
    uint64_t hash = seed;
    for (size_t i = 0; i < len; ++i) {
        hash ^= data[i];
        hash *= PRIME;
    }
    return hash;
}

inline std::string toHex16(uint64_t v) {
    static const char* digits = "0123456789abcdef";
    std::string out(16, '0');
    for (int i = 15; i >= 0; --i) {
        out[i] = digits[v & 0xF];
        v >>= 4;
    }
    return out;
}

// Le um ficheiro inteiro para um vector de bytes. Devolve vazio se nao existir.
inline std::vector<uint8_t> readFileBytes(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return {};
    size_t size = (size_t)f.tellg();
    f.seekg(0);
    std::vector<uint8_t> buf(size);
    if (size > 0) f.read(reinterpret_cast<char*>(buf.data()), (std::streamsize)size);
    return buf;
}

// Calcula o ID da campanha: hash(campaign.txt) encadeado com hash(cada .lvl).
// campaignDir: pasta onde vivem campaign.txt e os .lvl (ex: "Game/Assets/Levels").
// Devolve string vazia se campaign.txt nao existir/nao abrir.
inline std::string computeCampaignID(const std::string& campaignDir) {
    std::string campaignTxtPath = campaignDir + "/campaign.txt";
    std::vector<uint8_t> campaignBytes = readFileBytes(campaignTxtPath);
    if (campaignBytes.empty()) return "";

    uint64_t hash = fnv1a64(campaignBytes.data(), campaignBytes.size());

    // Reler campaign.txt como texto para extrair os nomes dos niveis, na ordem.
    std::istringstream iss(std::string(campaignBytes.begin(), campaignBytes.end()));
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        std::vector<uint8_t> lvlBytes = readFileBytes(campaignDir + "/" + line);
        // Encadeia o hash do nivel ao acumulado (seed = hash anterior), para
        // que a ORDEM dos niveis tambem influencie o resultado final.
        hash = fnv1a64(lvlBytes.data(), lvlBytes.size(), hash);
    }
    return toHex16(hash);
}

} // namespace core
