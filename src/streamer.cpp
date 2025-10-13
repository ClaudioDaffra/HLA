
// *********
// streamer.cpp
// *********

#include "streamer.hpp"
#include "error.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <format>
#include <string_view>
#include <charconv>

// --- Funzione Helper per la codifica UTF-8 ---
// Converte un codepoint UTF-32 in una stringa UTF-8.
static std::string encode_utf8(char32_t c) 
{
    std::string out;
    if (c < 0x80) {
        out += static_cast<char>(c);
    } else if (c < 0x800) {
        out += static_cast<char>(0xC0 | (c >> 6));
        out += static_cast<char>(0x80 | (c & 0x3F));
    } else if (c < 0x10000) {
        out += static_cast<char>(0xE0 | (c >> 12));
        out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (c & 0x3F));
    } else if (c < 0x110000) {
        out += static_cast<char>(0xF0 | (c >> 18));
        out += static_cast<char>(0x80 | ((c >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (c & 0x3F));
    }
    return out;
}


Streamer::Streamer() = default;

void Streamer::set_tab_width(size_t width)
{
    if (width > 0) {
        m_tab_width = width;
    }
}

const std::vector<CharInfo>& Streamer::get_char_stream() const
{
    return m_char_stream;
}

const std::string& Streamer::get_filename_by_index(size_t index) const
{
    if (index < m_filenames.size()) {
        return m_filenames[index];
    }
    static const std::string empty_string = "";
    return empty_string;
}

size_t Streamer::get_or_add_filename_idx(const std::string& filename)
{
    for (size_t i = 0; i < m_filenames.size(); ++i) {
        if (m_filenames[i] == filename) {
            return i;
        }
    }
    m_filenames.push_back(filename);
    return m_filenames.size() - 1;
}

void Streamer::stream_file(const std::string& filepath)
{
    std::ifstream file_stream(filepath, std::ios::binary); // Apri in modalità binaria
    if (!file_stream) {
        ErrorHandler::get().push_error(Sender::Loader, ErrorType::Fatal, Action::CheckFileExists, ErrorMessage::FileNotFound, 0, 0, filepath);
        return;
    }

    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    std::string file_content = buffer.str();

    // Imposta il file iniziale
    (void)get_or_add_filename_idx(filepath);
    ErrorHandler::get().set_current_file(filepath);

    process_buffer(file_content);
}

void Streamer::dump_to_file(const std::string& input_filepath) const
{
    std::string output_filename = input_filepath + ".streamer.debug";
    std::ofstream out_file(output_filename);

    if (!out_file) {
        ErrorHandler::get().push_error(Sender::Compiler, ErrorType::Error, Action::Coding, ErrorMessage::FileNotFound, 0, 0, "Could not open debug stream file: " + output_filename);
        return;
    }

    out_file << "--- Begin Debug Stream ---\n";
    out_file << std::format("[{:<15}][Row/Col  ][Lvl][Hex Val  ] : Char\n", "File");
    out_file << "------------------------------------------------------\n";

    for (const auto& ci : m_char_stream) {
        std::string filename = get_filename_by_index(ci.file_idx);
        if (filename.length() > 15) {
            filename = "..." + filename.substr(filename.length() - 12);
        }

        std::string char_repr;
        if (ci.value == U'\n') {
            char_repr = "\\n";
        } else if (ci.value == U'\t') {
            char_repr = "\\t";
        } else {
            // Usa la funzione di codifica per rappresentare correttamente tutti i caratteri
            char_repr = encode_utf8(ci.value);
        }
        
        std::string hex_val_str = std::format("0x{:X}", static_cast<uint32_t>(ci.value));

        std::string output_line = std::format(
            "[{:<15}][{:04}/{:04}][{:02}] {:<9} : {}",
            filename,
            ci.row,
            ci.col,
            ci.level,
            hex_val_str,
            char_repr
        );
        out_file << output_line << '\n';
    }
    out_file << "--- End Debug Stream ---\n";
}

void Streamer::process_buffer(const std::string& buffer)
{
    size_t current_row = 1;
    size_t current_col = 1;
    size_t current_level = 0;
    size_t current_file_idx = 0; // Indice del file principale
    bool is_start_of_line = true;

    const char* ptr = buffer.c_str();
    const char* end = ptr + buffer.length();

    while (ptr < end)
    {
        // Gestione della direttiva #line del preprocessore
        if (is_start_of_line && *ptr == '#') {
            std::string_view line_view(ptr, end - ptr);

            if (line_view.starts_with("#line ")) {
                const char* directive_ptr = ptr + 6; // Salta "#line "

                while (directive_ptr < end && (*directive_ptr == ' ' || *directive_ptr == '\t')) directive_ptr++;

                size_t new_line_number = 0;
                auto [p_num, ec] = std::from_chars(directive_ptr, end, new_line_number);

                if (ec == std::errc()) {
                    directive_ptr = p_num;
                    while (directive_ptr < end && (*directive_ptr == ' ' || *directive_ptr == '\t')) directive_ptr++;

                    if (directive_ptr < end && *directive_ptr == '"') {
                        directive_ptr++; // Salta "
                        const char* filename_start = directive_ptr;
                        while (directive_ptr < end && *directive_ptr != '"') directive_ptr++;
                        
                        if (directive_ptr < end) { // Trovato " di chiusura
                            std::string new_filename(filename_start, directive_ptr - filename_start);
                            directive_ptr++; // Salta " di chiusura

                            // Consuma il resto della riga della direttiva, incluso il newline
                            while (directive_ptr < end && *directive_ptr != '\n' && *directive_ptr != '\r') directive_ptr++;
                            if (directive_ptr < end && *directive_ptr == '\r' && ptr + 1 < end && *(ptr + 1) == '\n') directive_ptr++;
                            if (directive_ptr < end && (*directive_ptr == '\n' || *directive_ptr == '\r')) directive_ptr++;
                            
                            ptr = directive_ptr;

                            // Aggiorna lo stato per la riga successiva
                            current_row = new_line_number;
                            current_file_idx = get_or_add_filename_idx(new_filename);
                            current_col = 1;
                            current_level = 0;
                            is_start_of_line = true;
                            
                            continue; // Ricomincia il ciclo per la nuova riga
                        }
                    }
                }
            }
        }

        // Gestione standard dei caratteri
        is_start_of_line = false;

        if (*ptr == '\r' && ptr + 1 < end && *(ptr + 1) == '\n') ptr++;

        if (*ptr == '\n' || *ptr == '\r') {
            m_char_stream.push_back({U'\n', current_file_idx, current_row, current_col, current_level});
            current_row++;
            current_col = 1;
            current_level = 0;
            is_start_of_line = true;
            ptr++;
            continue;
        }

        if (*ptr == '\t') {
            size_t spaces_to_add = m_tab_width - ((current_col - 1) % m_tab_width);
            m_char_stream.push_back({U'\t', current_file_idx, current_row, current_col, current_level});
            current_col += spaces_to_add;
            ptr++;
            continue;
        }

        const char* current_char_ptr = ptr;
        char32_t u32_char = decode_utf8(ptr, end);

        if (u32_char != U'\0') {
            m_char_stream.push_back({u32_char, current_file_idx, current_row, current_col, current_level});
            current_col++;
        } else {
            unsigned char invalid_byte = static_cast<unsigned char>(*current_char_ptr);
            std::string extra = std::format("byte: 0x{:02x}", invalid_byte);
            ErrorHandler::get().push_error(Sender::Loader, ErrorType::Critical, Action::Tokenizing, ErrorMessage::InvalidUtf8Sequence, current_row, current_col, extra);
            return;
        }
    }

    // Assicura che il file termini sempre con un newline
    if (!m_char_stream.empty() && m_char_stream.back().value != U'\n') {
        m_char_stream.push_back({U'\n', current_file_idx, current_row, current_col, current_level});
    }
}

char32_t Streamer::decode_utf8(const char*& ptr, const char* end)
{
    unsigned char c = *ptr;
    
    if (c < 0x80) {
        ptr++;
        return c;
    }
    
    if ((c & 0xE0) == 0xC0) {
        if (ptr + 1 < end && (ptr[1] & 0xC0) == 0x80) {
            char32_t val = ((ptr[0] & 0x1F) << 6) | (ptr[1] & 0x3F);
            ptr += 2;
            return val;
        }
    }
    
    if ((c & 0xF0) == 0xE0) {
        if (ptr + 2 < end && (ptr[1] & 0xC0) == 0x80 && (ptr[2] & 0xC0) == 0x80) {
            char32_t val = ((ptr[0] & 0x0F) << 12) | ((ptr[1] & 0x3F) << 6) | (ptr[2] & 0x3F);
            ptr += 3;
            return val;
        }
    }
    
    if ((c & 0xF8) == 0xF0) {
        if (ptr + 3 < end && (ptr[1] & 0xC0) == 0x80 && (ptr[2] & 0xC0) == 0x80 && (ptr[3] & 0xC0) == 0x80) {
            char32_t val = ((ptr[0] & 0x07) << 18) | ((ptr[1] & 0x3F) << 12) | ((ptr[2] & 0x3F) << 6) | (ptr[3] & 0x3F);
            ptr += 4;
            return val;
        }
    }

    return U'\0';
}

// end streamer.cpp
