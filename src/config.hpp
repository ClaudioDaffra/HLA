
// *********
// config.hpp
// *********

#pragma once

#include <string_view>

// --- Informazioni sull'Applicazione ---
// Usiamo constexpr std::string_view per la massima efficienza.
// Queste sono costanti a tempo di compilazione senza overhead.

constexpr std::string_view APP_NAME = "High Level Assembler";
constexpr std::string_view COPYRIGHT_NOTICE = "Claudio Daffra [2025]";

