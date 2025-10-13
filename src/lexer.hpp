
// *********
// lexer.hpp
// *********

#pragma once

#include <string>
#include <vector>
#include <variant>
#include <cstdint>
#include "streamer.hpp"

// --- Enum Class per i Tipi di Token ---
enum class eToken 
{
		// Tipi di base
		T_CHAR			,	// Un singolo carattere (es. 'a', '\n')
		T_EOF			,	// Fine del file (End of File)

		// Tipi futuri
		T_IDENTIFIER	,	// a-z, A-Z, _
		T_INTEGER		,	// 123, 0xff
		T_REAL			,	// 1.23
		T_STRING		,	// "..."
		T_KEYWORD		,	// if, else, while...

		// Operatori a 3 caratteri
		T_ELLIPSIS		,	// ...

		// Operatori a 2 caratteri
		T_SHIFT_RIGHT	,   // >>
		T_SHIFT_LEFT	,	// <<
		T_ASSIGN		,	// :=
		T_COMPARE		,	// ?=
		T_NOT_EQUAL		,	// !=
		T_MOD			,	// %%
		T_LESS_EQUAL_THAN	, // <=
		T_GREATER_EQUAL_THAN, // >=
		T_SCOPE_RES		,	// ::
		T_RETURN		,	// ->
		T_AND			,	// &&
		T_OR			,	// ||
		T_INCREMENT		,	// ++
		T_DECREMENT		,	// --
		T_BIT_AND		,	// %&
		T_BIT_OR		,	// %|
		T_BIT_XOR		,	// %^
		T_BIT_NOT		,	// %~, %-

		// Operatori a 1 carattere
		T_LESS_THAN		,	// <
		T_GREATER_THAN	,	// >
		T_PLUS			,	// +
		T_MINUS			,	// -
		T_MUL			,	// *
		T_DIV			,	// /
		T_NOT			,	// !
		T_ADDRESS		,	// @
		T_POINTER		,	// ^

		// Parentesi
		T_P0			,	// (
		T_P1			,	// )
		T_Q0			,	// [
		T_Q1			,	// ]
		T_G0			,	// {
		T_G1			,	// }

		// Punteggiatura
		T_SEMICOLON		,	// ;
		T_COLON			,	// :
		T_COMMA			,	// ,
		
		T_ASM_BLOCK		,	// Contenuto di un blocco asm { ... }
		T_UTF8           	// Carattere non riconosciuto come parte di un altro token
};

// --- Struttura del Token ---
struct Token {
		eToken type; // Il tipo di token (dall'enum eToken)

		// Informazioni sulla posizione per il debug e la gestione degli errori
		size_t file_idx;  // Indice del file, ereditato da CharInfo
		size_t row;       // Numero di riga, ereditato da CharInfo
		size_t col;       // Numero di colonna, ereditato da CharInfo
		size_t pos_start; // Indice di inizio nel vettore m_char_stream
		size_t pos_end;   // Indice di fine (uguale a pos_start per token singoli)

		// Valore del token
		std::variant<
			std::monostate, // Per token che non hanno un valore (es. EOF, punteggiatura)
			char32_t,       // Per T_CHAR e T_UTF8
			std::string,    // Per identificatori, stringhe, keyword, ecc.
			uint64_t,       // Per numeri interi
			double          // Per numeri reali
		> value;

		std::string type_suffix; // Per letterali post-fissi (es. "u8")

		// Rappresentazione opzionale in formato F40
		std::string f40_representation;
};

// --- Classe Lexer ---
class Lexer 
{
public:
		Lexer() = default;

		// MODIFICA: Accetta il flag di compatibilità mcpp
		// Esegue l'analisi lessicale sul vettore di CharInfo fornito.
		void tokenize(const std::vector<CharInfo>& char_stream, bool mcpp_compatibility);

		// Salva il contenuto del token_stream in un file di debug.
		// Modificato per accettare lo streamer e risolvere i nomi dei file.
		void dump_to_file(const std::string& input_filepath, const Streamer& streamer) const;

		// Restituisce un riferimento costante al vettore di token generato.
		const std::vector<Token>& get_token_stream() const;

private:
		std::vector<Token> m_token_stream; // Vettore contenente i token generati

		// Funzione helper per analizzare un numero esadecimale
		void parse_esadecimal(const std::vector<CharInfo>& char_stream, size_t& i, bool mcpp_compatibility);
		
		// Funzione helper per analizzare un numero binario
		void parse_binary(const std::vector<CharInfo>& char_stream, size_t& i, bool mcpp_compatibility);

		// Funzione helper per analizzare un numero intero o in virgola mobile
		void parse_integer_float(const std::vector<CharInfo>& char_stream, size_t& i, bool mcpp_compatibility);

		// Funzione helper per analizzare una stringa o un carattere
		void parse_string(const std::vector<CharInfo>& char_stream, size_t& i);

		// Funzione helper per analizzare un identificatore o una keyword
		void parse_identifier_or_keyword(const std::vector<CharInfo>& char_stream, size_t& i);

		// Funzioni helper per l'analisi degli operatori
		bool try_parse_operator3(const std::vector<CharInfo>& char_stream, size_t& i);
		bool try_parse_operator2(const std::vector<CharInfo>& char_stream, size_t& i);
		bool try_parse_operator1(const std::vector<CharInfo>& char_stream, size_t& i);
};
