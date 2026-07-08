#pragma once
// =============================================================================
//  Game/Logic/RunHistory.h
//
//  @version 8.1
//  Regista runs completadas (campanha vencida) num ficheiro CSV append-only,
//  em Development/Runs/runs.csv. Formato plano/legivel, git-diffable,
//  consistente com o estilo ja usado no projecto (dev_log.txt).
//
//  Colunas: timestamp,campaign_name,campaign_id,elapsed_seconds,elapsed_formatted
//  Uma linha por run. Cabecalho escrito automaticamente se o ficheiro nao
//  existir ainda.
// =============================================================================
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <filesystem>

namespace logic {

// Formata segundos como MM:SS.mmm (ex: 125.487f -> "02:05.487").
inline std::string formatElapsed(float seconds) {
    if (seconds < 0.0f) seconds = 0.0f;
    int totalMs = (int)(seconds * 1000.0f + 0.5f);
    int ms = totalMs % 1000;
    int totalSec = totalMs / 1000;
    int sec = totalSec % 60;
    int min = totalSec / 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << min << ":"
        << std::setfill('0') << std::setw(2) << sec << "."
        << std::setfill('0') << std::setw(3) << ms;
    return oss.str();
}

// Timestamp actual formatado "YYYY-MM-DD HH:MM:SS" (hora local).
inline std::string currentTimestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tmBuf{};
#if defined(_WIN32)
    localtime_s(&tmBuf, &t);
#else
    localtime_r(&t, &tmBuf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// Escreve/acrescenta um registo de run ao ficheiro CSV. Cria a pasta e o
// cabecalho automaticamente se necessario. Retorna false em caso de erro
// de I/O (nao deve travar o jogo — o chamador pode ignorar o retorno).
inline bool recordRun(const std::string& csvPath, const std::string& campaignName,
                      const std::string& campaignID, float elapsedSeconds) {
    try {
        std::filesystem::path p(csvPath);
        if (p.has_parent_path())
            std::filesystem::create_directories(p.parent_path());
    } catch (...) {
        return false;
    }

    bool needsHeader = !std::filesystem::exists(csvPath);

    std::ofstream f(csvPath, std::ios::app);
    if (!f.is_open()) return false;

    if (needsHeader) {
        f << "timestamp,campaign_name,campaign_id,elapsed_seconds,elapsed_formatted\n";
    }

    f << currentTimestamp() << ","
      << campaignName << ","
      << campaignID << ","
      << elapsedSeconds << ","
      << formatElapsed(elapsedSeconds) << "\n";

    return true;
}

} // namespace logic
