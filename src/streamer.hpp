
// *********
// streamer.hpp
// *********

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

// Contiene le informazioni per ogni singolo carattere nel buffer di input.
struct CharInfo 
{
    char32_t value;      // Il carattere Unicode (UTF-32).
    size_t file_idx;     // Indice al nome del file in un'apposita tabella.
    size_t row;          // Numero di riga nel file di origine.
    size_t col;          // Numero di colonna nel file di origine.
    size_t level;        // Livello di indentazione (basato su spazi iniziali).
};

class Streamer 
{
public:
    Streamer();

    // Imposta la larghezza della tabulazione da usare nel calcolo delle colonne.
    void set_tab_width(size_t width);

    // Carica un file, lo processa e popola il vettore di CharInfo.
    void stream_file(const std::string& filepath);

    // Salva il contenuto del char_stream in un file di debug formattato.
    void dump_to_file(const std::string& input_filepath) const;

    // Restituisce un riferimento costante al vettore di caratteri.
    const std::vector<CharInfo>& get_char_stream() const;

    // Restituisce il nome del file dall'indice.
    const std::string& get_filename_by_index(size_t index) const;

private:
    // Converte un buffer di testo (UTF-8) in un vettore di CharInfo.
    void process_buffer(const std::string& buffer);

    // Decodifica un singolo carattere UTF-8 dalla stringa di input.
    char32_t decode_utf8(const char*& ptr, const char* end);

    // Aggiunge un nome di file alla tabella (se non presente) e ne restituisce l'indice.
    size_t get_or_add_filename_idx(const std::string& filename);

    std::vector<CharInfo> m_char_stream;
	
    std::vector<std::string> m_filenames; // Tabella dei nomi dei file per riferimento univoco.
	
    size_t m_tab_width = 4; // Larghezza di default della tabulazione
};

// end streamer.hpp
