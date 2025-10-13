
// *********
// lexer.cpp
// *********

#include "lexer.hpp"
#include "error.hpp"
#include "f40_converter.hpp"
#include <fstream>
#include <format>
#include <cctype>   // Per funzioni su char
#include <cwctype>  // Per funzioni su wide char (iswspace, iswdigit, etc.)
#include <charconv> // Per std::from_chars
#include <string>
#include <stdexcept> // Per std::invalid_argument, std::out_of_range
#include <map>
#include <algorithm> // Per std::replace
#include <set>

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

// --- Funzione Helper per il parsing dei suffissi di tipo ---
static std::string parse_type_suffix(const std::vector<CharInfo>& char_stream, size_t& current_pos)
{
	if (current_pos >= char_stream.size() || !iswalpha(static_cast<wint_t>(char_stream[current_pos].value))) {
		return "";
	}

	std::string potential_suffix;
	size_t temp_pos = current_pos;
	while (temp_pos < char_stream.size() && iswalnum(static_cast<wint_t>(char_stream[temp_pos].value))) {
		potential_suffix += static_cast<char>(char_stream[temp_pos].value);
		temp_pos++;
	}

	// Lista di suffissi conosciuti (validi e non validi, il parser deciderà)
	const static std::set<std::string> known_suffixes = {
		"u8", "s8", "u16", "s16", "f40", 
		"u32", "s32", "u64", "s64"
	};

	if (known_suffixes.count(potential_suffix)) {
		current_pos = temp_pos; // Consuma il suffisso
		return potential_suffix;
	}

	return ""; // Non è un suffisso conosciuto, non consumare nulla
}


// --- Implementazione dei Metodi della Classe Lexer ---

const std::vector<Token>& Lexer::get_token_stream() const 
{
	return m_token_stream;
}

// Gestisce numeri esadecimali (0x, $).
void Lexer::parse_esadecimal(const std::vector<CharInfo>& char_stream, size_t& i, bool mcpp_compatibility) 
{
	const auto& start_ci = char_stream[i];
	const size_t start_pos = i;

	size_t prefix_len = (char_stream[i].value == '$') ? 1 : 2;

	size_t current_pos = i + prefix_len;
	std::string cleaned_str;
	bool is_real = false;
	bool has_decimal_point = false;
	bool has_exponent = false;

	// Analizza il corpo del numero
	while (current_pos < char_stream.size()) 
	{
		char32_t c = char_stream[current_pos].value;

		if (c == '\'' && !mcpp_compatibility) 
		{
			current_pos++;
			continue;
		}

		if (c == '.') 
		{
			if (has_decimal_point || has_exponent) 
			{
				ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::UnexpectedToken, char_stream[current_pos].row, char_stream[current_pos].col, "Punto decimale non valido in numero esadecimale");
				break;
			}
			has_decimal_point = true;
			is_real = true;
			cleaned_str += '.';
			current_pos++;
			continue;
		}

		if (c == 'p' || c == 'P') 
		{
			if (has_exponent) 
			{
				ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::UnexpectedToken, char_stream[current_pos].row, char_stream[current_pos].col, "Esponente multiplo non valido");
				break;
			}
			has_exponent = true;
			is_real = true;
			cleaned_str += c;
			current_pos++;
			if (current_pos < char_stream.size() && (char_stream[current_pos].value == '+' || char_stream[current_pos].value == '-')) 
			{
				cleaned_str += static_cast<char>(char_stream[current_pos].value);
				current_pos++;
			}
			continue;
		}

		if (!iswxdigit(static_cast<wint_t>(c))) 
		{
			break; // Fine del numero
		}

		cleaned_str += static_cast<char>(c);
		current_pos++;
	}

	// Dopo aver parsato il numero, cerca un suffisso di tipo
	std::string suffix = parse_type_suffix(char_stream, current_pos);

	if (cleaned_str.empty()) 
	{
		// MODIFICA: Gestione specifica per identificatori che iniziano con '$'
		if (start_ci.value == '$' && current_pos < char_stream.size())
		{
			char32_t offending_char = char_stream[current_pos].value;
			if (iswalpha(static_cast<wint_t>(offending_char)) || offending_char == '_')
			{
				ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::Tokenizing, ErrorMessage::InvalidIdentifierStart, start_ci.row, start_ci.col);
				// Consuma il resto dell'identificatore non valido per evitare errori a cascata
				while (current_pos < char_stream.size() && (iswalnum(static_cast<wint_t>(char_stream[current_pos].value)) || char_stream[current_pos].value == '_'))
				{
					current_pos++;
				}
				i = current_pos - 1;
				return;
			}
		}
		
		ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::InvalidNumberFormat, start_ci.row, start_ci.col, "Numero esadecimale incompleto dopo il prefisso");
		i = start_pos + prefix_len - 1;
		return;
	}

	char last_char = cleaned_str.back();
	if (last_char == '.' || tolower(last_char) == 'p') 
	{
		ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::InvalidNumberFormat, start_ci.row, start_ci.col, "Il numero esadecimale termina in modo inaspettato");
	}

	Token t;
	t.file_idx = start_ci.file_idx;
	t.row = start_ci.row;
	t.col = start_ci.col;
	t.pos_start = start_pos;
	t.pos_end = current_pos - 1;
	t.type_suffix = suffix;

	if (is_real) 
	{
		t.type = eToken::T_REAL;
		std::string str_to_convert = "0x" + cleaned_str;
		double val;
		try 
		{
			size_t pos = 0;
			val = std::stod(str_to_convert, &pos);
			if (pos < str_to_convert.length()) 
			{
				 ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::InvalidNumberFormat, t.row, t.col, "Caratteri non validi nel numero reale esadecimale: " + str_to_convert);
				 t.value = 0.0;
				 t.f40_representation = "Error";
			} 
			else 
			{
				t.value = val;
				t.f40_representation = F40Converter::convertDoubleToMBF_string(val);
			}
		} 
		catch (const std::invalid_argument&) 
		{
			ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::InvalidNumberFormat, t.row, t.col, "Impossibile convertire il numero reale esadecimale: " + str_to_convert);
			t.value = 0.0;
			t.f40_representation = "Error";
		} 
		catch (const std::out_of_range&) 
		{
			ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::InvalidNumberFormat, t.row, t.col, "Valore fuori range per numero reale esadecimale: " + str_to_convert);
			t.value = 0.0;
			t.f40_representation = "Error";
		}
	} 
	else 
	{ // Intero
		t.type = eToken::T_INTEGER;
		uint64_t val;
		try 
		{
			val = std::stoull(cleaned_str, nullptr, 16);
			t.value = val;
		} 
		catch (...) 
		{
			ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::InvalidNumberFormat, t.row, t.col, "Impossibile convertire l'intero esadecimale: " + cleaned_str);
			t.value = (uint64_t)0;
		}
	}

	m_token_stream.push_back(t);
	i = current_pos - 1;
}

// Gestisce sia il prefisso '%' che '0b'
void Lexer::parse_binary(const std::vector<CharInfo>& char_stream, size_t& i, bool mcpp_compatibility) 
{
	const auto& start_ci = char_stream[i];
	const size_t start_pos = i;

	size_t prefix_len = 0;
	std::string prefix_str;

	// Rileva il prefisso (% o 0b)
	if (start_ci.value == '%') 
	{
		prefix_len = 1;
		prefix_str = "%";
	} 
	else if (start_ci.value == '0' && i + 1 < char_stream.size() && tolower(char_stream[i + 1].value) == 'b') 
	{
		prefix_len = 2;
		prefix_str = "0b";
	} 
	else 
	{
		// Sicurezza: non dovrebbe accadere se chiamato correttamente da tokenize
		return;
	}

	size_t current_pos = i + prefix_len;
	std::string cleaned_str;

	while (current_pos < char_stream.size()) 
	{
		char32_t c = char_stream[current_pos].value;

		// Ignora l'apice solo se la compatibilità mcpp è disattivata
		if (c == '\'' && !mcpp_compatibility) 
		{
			current_pos++;
			continue;
		}

		// Solo '0' e '1' sono permessi
		if (c == '0' || c == '1') 
		{
			cleaned_str += static_cast<char>(c);
			current_pos++;
		} 
		else 
		{
			break; // Fine del numero binario
		}
	}

	// Dopo aver parsato il numero, cerca un suffisso di tipo
	std::string suffix = parse_type_suffix(char_stream, current_pos);

	// Validazione: il numero non può essere vuoto dopo il prefisso
	if (cleaned_str.empty()) 
	{
		ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::InvalidNumberFormat, start_ci.row, start_ci.col, "Numero binario incompleto dopo il prefisso '" + prefix_str + "'");
		i = start_pos + prefix_len - 1; // Riprocessa dal carattere dopo il prefisso
		return;
	}

	Token t;
	t.file_idx = start_ci.file_idx;
	t.row = start_ci.row;
	t.col = start_ci.col;
	t.pos_start = start_pos;
	t.pos_end = current_pos - 1;
	t.type = eToken::T_INTEGER;
	t.type_suffix = suffix;

	uint64_t val;
	try 
	{
		val = std::stoull(cleaned_str, nullptr, 2); // Base 2
		t.value = val;
	} 
	catch (...) 
	{ // Cattura std::invalid_argument e std::out_of_range
		ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::InvalidNumberFormat, t.row, t.col, "Impossibile convertire il numero binario o valore fuori range: " + cleaned_str);
		t.value = (uint64_t)0;
	}

	m_token_stream.push_back(t);
	i = current_pos - 1; // Imposta l'indice all'ultimo carattere consumato
}

// Gestisce numeri interi decimali e in virgola mobile.
void Lexer::parse_integer_float(const std::vector<CharInfo>& char_stream, size_t& i, bool mcpp_compatibility)
{
	const auto& start_ci = char_stream[i];
	const size_t start_pos = i;

	std::string cleaned_str;
	bool is_real = false;
	bool has_decimal_point = false;
	bool has_exponent = false;
	size_t current_pos = i;

	// Analizza il corpo del numero
	while (current_pos < char_stream.size())
	{
		char32_t c = char_stream[current_pos].value;

		if (c == '\'' && !mcpp_compatibility)
		{
			current_pos++;
			continue;
		}

		if (c == '.')
		{
			if (has_decimal_point || has_exponent)
			{
				ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::UnexpectedToken, char_stream[current_pos].row, char_stream[current_pos].col, "Punto decimale non valido");
				break;
			}
			has_decimal_point = true;
			is_real = true;
			cleaned_str += '.';
			current_pos++;
			continue;
		}

		if (c == 'e' || c == 'E')
		{
			if (has_exponent)
			{
				ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::UnexpectedToken, char_stream[current_pos].row, char_stream[current_pos].col, "Esponente multiplo non valido");
				break;
			}
			has_exponent = true;
			is_real = true;
			cleaned_str += c;
			current_pos++;
			if (current_pos < char_stream.size() && (char_stream[current_pos].value == '+' || char_stream[current_pos].value == '-'))
			{
				cleaned_str += static_cast<char>(char_stream[current_pos].value);
				current_pos++;
			}
			continue;
		}

		if (!iswdigit(static_cast<wint_t>(c)))
		{
			break; // Fine del numero
		}

		cleaned_str += static_cast<char>(c);
		current_pos++;
	}
	
	// Dopo aver parsato il numero, cerca un suffisso di tipo
	std::string suffix = parse_type_suffix(char_stream, current_pos);

	// Se il numero inizia con un punto, is_real deve essere true
	if (char_stream[start_pos].value == '.')
	{
		is_real = true;
	}

	char last_char = cleaned_str.back();
	if (last_char == '.' || tolower(last_char) == 'e')
	{
		ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::InvalidNumberFormat, start_ci.row, start_ci.col, "Il numero termina in modo inaspettato");
	}

	Token t;
	t.file_idx = start_ci.file_idx;
	t.row = start_ci.row;
	t.col = start_ci.col;
	t.pos_start = start_pos;
	t.pos_end = current_pos - 1;
	t.type_suffix = suffix;

	if (is_real)
	{
		t.type = eToken::T_REAL;
		double val;
		auto [ptr, ec] = std::from_chars(cleaned_str.data(), cleaned_str.data() + cleaned_str.size(), val);

		if (ec != std::errc() || ptr != cleaned_str.data() + cleaned_str.size())
		{
			ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::InvalidNumberFormat, t.row, t.col, "Impossibile convertire il numero reale: " + cleaned_str);
			t.value = 0.0;
			t.f40_representation = "Error";
		}
		else
		{
			t.value = val;
			t.f40_representation = F40Converter::convertDoubleToMBF_string(val);
		}
	}
	else // Intero
	{
		t.type = eToken::T_INTEGER;
		uint64_t val;
		try
		{
			val = std::stoull(cleaned_str, nullptr, 10);
			t.value = val;
		}
		catch (...)
		{
			ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingNumber, ErrorMessage::InvalidNumberFormat, t.row, t.col, "Impossibile convertire l'intero o valore fuori range: " + cleaned_str);
			t.value = (uint64_t)0;
		}
	}

	m_token_stream.push_back(t);
	i = current_pos - 1;
}

// #5 lexer string
void Lexer::parse_string(const std::vector<CharInfo>& char_stream, size_t& i)
{
	const auto& start_ci = char_stream[i];
	const size_t start_pos = i;
	const char32_t quote_char = start_ci.value;

	size_t current_pos = i + 1;
	std::u32string string_content; // Usa u32string per contare correttamente i caratteri

	bool closed = false;
	while (current_pos < char_stream.size())
	{
		const auto& current_ci = char_stream[current_pos];

		if (current_ci.value == quote_char)
		{
			closed = true;
			break;
		}

		// Gestisce le sequenze di escape
		if (current_ci.value == U'\\')
		{
			current_pos++;
			if (current_pos >= char_stream.size())
			{
				// Raggiunto EOF dopo un backslash, l'errore verrà gestito dal controllo !closed
				break;
			}
			const auto& escaped_ci = char_stream[current_pos];
			switch (escaped_ci.value)
			{
				case U'n': string_content += U'\n'; break;
				case U't': string_content += U'\t'; break;
				case U'r': string_content += U'\r'; break;
				case U'\\': string_content += U'\\'; break;
				case U'\'': string_content += U'\''; break;
				case U'"': string_content += U'"'; break;
				case U'v': string_content += U'\v'; break;     // Vertical Tab
				case U'f': string_content += U'\f'; break;     // Form Feed
				case U'e': string_content += U'\x1b'; break;   // Escape character (non-standard)
				default:
					// Per ora, aggiunge il carattere così com'è. Un compilatore reale potrebbe emettere un warning.
					string_content += escaped_ci.value;
					break;
			}
		}
		else
		{
			string_content += current_ci.value;
		}
		current_pos++;
	}

	if (!closed)
	{
		ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingString, ErrorMessage::UnclosedString, start_ci.row, start_ci.col);
		i = current_pos - 1; // Avanza i fino alla fine per fermare l'analisi
		return;
	}

	Token t;
	t.file_idx = start_ci.file_idx;
	t.row = start_ci.row;
	t.col = start_ci.col;
	t.pos_start = start_pos;
	t.pos_end = current_pos;

	if (quote_char == U'\'')
	{
		if (string_content.length() != 1)
		{
			ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::TokenizingString, ErrorMessage::InvalidCharacterLiteral, start_ci.row, start_ci.col, "Trovati " + std::to_string(string_content.length()) + " caratteri, atteso 1.");
			t.type = eToken::T_CHAR;
			t.value = (string_content.empty()) ? U'\0' : string_content[0];
		}
		else
		{
			t.type = eToken::T_CHAR;
			t.value = string_content[0];
		}
	}
	else // quote_char == U'"'
	{
		t.type = eToken::T_STRING;
		// Converte u32string di nuovo in una stringa utf-8 per il valore del token
		std::string utf8_string;
		for (char32_t c : string_content)
		{
			utf8_string += encode_utf8(c);
		}
		t.value = utf8_string;
	}

	m_token_stream.push_back(t);
	i = current_pos; // Imposta l'indice sull'apice di chiusura
}

// --- Mappe Statiche per la Ricerca degli Operatori ---
// Usiamo u32string per la corrispondenza con i CharInfo.value
static const std::map<std::u32string, eToken> ops3 = {
	{U"...", eToken::T_ELLIPSIS}
};

static const std::map<std::u32string, eToken> ops2 = {
	{U">>", eToken::T_SHIFT_RIGHT}, {U"<<", eToken::T_SHIFT_LEFT},
	{U":=", eToken::T_ASSIGN},      {U"?=", eToken::T_COMPARE}, {U"!=", eToken::T_NOT_EQUAL},
	{U"%%", eToken::T_MOD},         {U"<=", eToken::T_LESS_EQUAL_THAN},
	{U">=", eToken::T_GREATER_EQUAL_THAN}, {U"->", eToken::T_RETURN},
	{U"::", eToken::T_SCOPE_RES},
	{U"&&", eToken::T_AND},         {U"||", eToken::T_OR},
	{U"++", eToken::T_INCREMENT},
	{U"--", eToken::T_DECREMENT},
	{U"%&", eToken::T_BIT_AND},     {U"%|", eToken::T_BIT_OR},
	{U"%^", eToken::T_BIT_XOR},     {U"%~", eToken::T_BIT_NOT},
	{U"%-", eToken::T_BIT_NOT}
};

// Per gli operatori a 1 carattere, una mappa da char32_t è più efficiente.
static const std::map<char32_t, eToken> ops1 = {
	{U'<', eToken::T_LESS_THAN},   {U'>', eToken::T_GREATER_THAN},
	{U'+', eToken::T_PLUS},       {U'-', eToken::T_MINUS},
	{U'*', eToken::T_MUL},        {U'/', eToken::T_DIV},
	{U'!', eToken::T_NOT},        {U'@', eToken::T_ADDRESS},
	{U'^', eToken::T_POINTER},    {U'(', eToken::T_P0},
	{U')', eToken::T_P1},        {U'[', eToken::T_Q0},
	{U']', eToken::T_Q1},        {U'{', eToken::T_G0},
	{U'}', eToken::T_G1},        {U';', eToken::T_SEMICOLON},
	{U':', eToken::T_COLON},      {U',', eToken::T_COMMA}
};

// --- Nuove Routine per la Gestione degli Operatori ---

bool Lexer::try_parse_operator3(const std::vector<CharInfo>& char_stream, size_t& i)
{
	if (i + 2 >= char_stream.size())
	{
		return false;
	}

	std::u32string op_str;
	op_str += char_stream[i].value;
	op_str += char_stream[i + 1].value;
	op_str += char_stream[i + 2].value;

	auto it = ops3.find(op_str);
	if (it != ops3.end())
	{
		const auto& ci = char_stream[i];
		Token t;
		t.type = it->second;
		t.file_idx = ci.file_idx;
		t.row = ci.row;
		t.col = ci.col;
		t.pos_start = i;
		t.pos_end = i + 2;
		m_token_stream.push_back(t);
		i += 2; // Avanza di 2, il for loop aggiungerà 1
		return true;
	}
	return false;
}

bool Lexer::try_parse_operator2(const std::vector<CharInfo>& char_stream, size_t& i)
{
	if (i + 1 >= char_stream.size())
	{
		return false;
	}

	std::u32string op_str;
	op_str += char_stream[i].value;
	op_str += char_stream[i + 1].value;

	auto it = ops2.find(op_str);
	if (it != ops2.end())
	{
		const auto& ci = char_stream[i];
		Token t;
		t.type = it->second;
		t.file_idx = ci.file_idx;
		t.row = ci.row;
		t.col = ci.col;
		t.pos_start = i;
		t.pos_end = i + 1;
		m_token_stream.push_back(t);
		i += 1; // Avanza di 1, il for loop aggiungerà 1
		return true;
	}
	return false;
}

bool Lexer::try_parse_operator1(const std::vector<CharInfo>& char_stream, size_t& i)
{
	const auto& ci = char_stream[i];
	auto it = ops1.find(ci.value);
	if (it != ops1.end())
	{
		Token t;
		t.type = it->second;
		t.file_idx = ci.file_idx;
		t.row = ci.row;
		t.col = ci.col;
		t.pos_start = i;
		t.pos_end = i;
		m_token_stream.push_back(t);
		return true;
	}
	return false;
}

// Mappa statica per le keyword
static const std::map<std::string, eToken> keywords = {
	{"for",   eToken::T_KEYWORD},
	{"while", eToken::T_KEYWORD},
	{"if",    eToken::T_KEYWORD},
	{"else",  eToken::T_KEYWORD},
	{"ret",   eToken::T_KEYWORD},
	{"fn",    eToken::T_KEYWORD},
	{"sys",   eToken::T_KEYWORD},
	{"ns",    eToken::T_KEYWORD},
	{"jmp",   eToken::T_KEYWORD}
	
	// "asm" è gestito come caso speciale, non più qui
};

void Lexer::parse_identifier_or_keyword(const std::vector<CharInfo>& char_stream, size_t& i)
{
	const auto& start_ci = char_stream[i];
	const size_t start_pos = i;
	size_t current_pos = i;
	std::string identifier_str;

	while (current_pos < char_stream.size())
	{
		char32_t c = char_stream[current_pos].value;
		if (iswalnum(static_cast<wint_t>(c)) || c == '_')
		{
			identifier_str += encode_utf8(c);
			current_pos++;
		}
		else
		{
			break;
		}
	}

	// --- Logica speciale per 'asm' ---
	if (identifier_str == "asm") {
		size_t lookahead_pos = current_pos;

		while (lookahead_pos < char_stream.size() && iswspace(static_cast<wint_t>(char_stream[lookahead_pos].value))) {
			lookahead_pos++;
		}

		if (lookahead_pos < char_stream.size() && char_stream[lookahead_pos].value == U'{') {
			size_t content_start_pos = lookahead_pos + 1;
			size_t content_end_pos = content_start_pos;
			int brace_level = 1;

			while (content_end_pos < char_stream.size()) {
				char32_t c = char_stream[content_end_pos].value;
				if (c == U'{') {
					brace_level++;
				} else if (c == U'}') {
					brace_level--;
					if (brace_level == 0) {
						break;
					}
				}
				content_end_pos++;
			}

			if (brace_level != 0) {
				ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::Tokenizing, ErrorMessage::UnclosedAsmBlock, start_ci.row, start_ci.col);
				i = content_end_pos - 1;
				return;
			}

			std::string asm_content;
			for (size_t k = content_start_pos; k < content_end_pos; ++k) {
				asm_content += encode_utf8(char_stream[k].value);
			}

			Token t;
			t.type = eToken::T_ASM_BLOCK;
			t.file_idx = start_ci.file_idx;
			t.row = start_ci.row;
			t.col = start_ci.col;
			t.pos_start = start_pos;
			t.pos_end = content_end_pos;
			t.value = asm_content;
			m_token_stream.push_back(t);

			i = content_end_pos;
			return;
		}
	}

	Token t;
	t.file_idx = start_ci.file_idx;
	t.row = start_ci.row;
	t.col = start_ci.col;
	t.pos_start = start_pos;
	t.pos_end = current_pos - 1;
	
	auto it = keywords.find(identifier_str);
	if (it != keywords.end())
	{
		t.type = eToken::T_KEYWORD;
	}
	else
	{
		t.type = eToken::T_IDENTIFIER;
	}
	
	t.value = identifier_str;
	m_token_stream.push_back(t);
	i = current_pos - 1;
}

void Lexer::tokenize(const std::vector<CharInfo>& char_stream, bool mcpp_compatibility) 
{
	m_token_stream.clear();

	for (size_t i = 0; i < char_stream.size(); ++i) 
	{
		const auto& ci = char_stream[i];

		if (iswspace(static_cast<wint_t>(ci.value))) 
		{
			continue;
		}

		if (ci.value == U'/' && i + 1 < char_stream.size()) 
		{
			const auto& next_ci = char_stream[i + 1];
			if (next_ci.value == U'/') 
			{
				i += 2;
				while (i < char_stream.size() && char_stream[i].value != U'\n') i++;
				if (i < char_stream.size()) i--;
				continue;
			}
			if (next_ci.value == U'*') 
			{
				size_t start_line = ci.row;
				size_t start_col = ci.col;
				i += 2;
				bool closed = false;
				while (i + 1 < char_stream.size()) 
				{
					if (char_stream[i].value == U'*' && char_stream[i + 1].value == U'/') 
					{
						i += 1;
						closed = true;
						break;
					}
					i++;
				}
				if (!closed) 
				{
					ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Fatal, Action::Tokenizing, ErrorMessage::UnclosedComment, start_line, start_col);
					return;
				}
				continue;
			}
		}

		char32_t next_char = (i + 1 < char_stream.size()) ? char_stream[i + 1].value : 0;

		bool is_binary_0b = (ci.value == '0' && tolower(next_char) == 'b');

		if (is_binary_0b) 
		{
			parse_binary(char_stream, i, mcpp_compatibility);
			continue;
		}

		bool is_hex_start = (ci.value == '$') || (ci.value == '0' && tolower(next_char) == 'x');
		if (is_hex_start) 
		{
			parse_esadecimal(char_stream, i, mcpp_compatibility);
			continue;
		}

		bool is_dec_float_start = iswdigit(static_cast<wint_t>(ci.value)) || (ci.value == '.' && i + 1 < char_stream.size() && iswdigit(static_cast<wint_t>(char_stream[i+1].value)));
		if (is_dec_float_start) 
		{
			parse_integer_float(char_stream, i, mcpp_compatibility);
			continue;
		}
		
		if (ci.value == U'"' || ci.value == U'\'')
		{
			parse_string(char_stream, i);
			continue;
		}
		
		if (try_parse_operator3(char_stream, i) ||
			try_parse_operator2(char_stream, i) ||
			try_parse_operator1(char_stream, i))
		{
			continue;
		}
		
		if (iswalpha(static_cast<wint_t>(ci.value)) || ci.value == '_')
		{
			parse_identifier_or_keyword(char_stream, i);
			continue;
		}
		
		// Gestione speciale per gli operatori bitwise che iniziano con '%' ma non sono numeri binari
		if (ci.value == '%') {
			parse_binary(char_stream, i, mcpp_compatibility);
			continue;
		}

		Token t;
		t.type = eToken::T_UTF8;
		t.file_idx = ci.file_idx;
		t.row = ci.row;
		t.col = ci.col;
		t.pos_start = i;
		t.pos_end = i;
		t.value = ci.value;
		m_token_stream.push_back(t);
	}

	if (!m_token_stream.empty())
	{
		std::vector<Token> merged_stream;
		merged_stream.reserve(m_token_stream.size());

		for (size_t i = 0; i < m_token_stream.size(); ++i)
		{
			Token current_token = m_token_stream[i];

			if (current_token.type == eToken::T_STRING)
			{
				while (i + 1 < m_token_stream.size() && m_token_stream[i + 1].type == eToken::T_STRING)
				{
					const Token& next_token = m_token_stream[i + 1];
					std::get<std::string>(current_token.value) += std::get<std::string>(next_token.value);
					current_token.pos_end = next_token.pos_end;
					i++;
				}
			}
			merged_stream.push_back(current_token);
		}
		m_token_stream = std::move(merged_stream);
	}

	Token eof_token;
	eof_token.type = eToken::T_EOF;
	if (!char_stream.empty()) 
	{
		const auto& last_ci = char_stream.back();
		eof_token.file_idx = last_ci.file_idx;
		eof_token.row = last_ci.row;
		eof_token.col = last_ci.col + 1;
	} 
	else 
	{
		eof_token.file_idx = 0;
		eof_token.row = 1;
		eof_token.col = 1;
	}
	eof_token.pos_start = char_stream.size();
	eof_token.pos_end = char_stream.size();
	m_token_stream.push_back(eof_token);
}

// --- Funzioni Helper per il Dump di Debug ---

static std::string token_type_to_string(eToken type) 
{
	switch (type) 
	{
		case eToken::T_CHAR:        return "T_CHAR";
		case eToken::T_EOF:         return "T_EOF";
		case eToken::T_IDENTIFIER:  return "T_IDENTIFIER";
		case eToken::T_INTEGER:     return "T_INTEGER";
		case eToken::T_REAL:        return "T_REAL";
		case eToken::T_STRING:      return "T_STRING";
		case eToken::T_KEYWORD:     return "T_KEYWORD";
		case eToken::T_UTF8:        return "T_UTF8";
		case eToken::T_ELLIPSIS:           return "T_ELLIPSIS";
		case eToken::T_SHIFT_RIGHT:        return "T_SHIFT_RIGHT";
		case eToken::T_SHIFT_LEFT:         return "T_SHIFT_LEFT";
		case eToken::T_ASSIGN:             return "T_ASSIGN";
		case eToken::T_COMPARE:            return "T_COMPARE";
		case eToken::T_NOT_EQUAL:          return "T_NOT_EQUAL";
		case eToken::T_MOD:                return "T_MOD";
		case eToken::T_LESS_EQUAL_THAN:    return "T_LESS_EQUAL_THAN";
		case eToken::T_GREATER_EQUAL_THAN: return "T_GREATER_EQUAL_THAN";
		case eToken::T_SCOPE_RES:          return "T_SCOPE_RES";
		case eToken::T_RETURN:             return "T_RETURN";
		case eToken::T_AND:                return "T_AND";
		case eToken::T_OR:                 return "T_OR";
		case eToken::T_INCREMENT:          return "T_INCREMENT";
		case eToken::T_DECREMENT:          return "T_DECREMENT";
		case eToken::T_BIT_AND:            return "T_BIT_AND";
		case eToken::T_BIT_OR:             return "T_BIT_OR";
		case eToken::T_BIT_XOR:            return "T_BIT_XOR";
		case eToken::T_BIT_NOT:            return "T_BIT_NOT";
		case eToken::T_LESS_THAN:          return "T_LESS_THAN";
		case eToken::T_GREATER_THAN:       return "T_GREATER_THAN";
		case eToken::T_PLUS:               return "T_PLUS";
		case eToken::T_MINUS:              return "T_MINUS";
		case eToken::T_MUL:                return "T_MUL";
		case eToken::T_DIV:                return "T_DIV";
		case eToken::T_NOT:                return "T_NOT";
		case eToken::T_ADDRESS:            return "T_ADDRESS";
		case eToken::T_POINTER:            return "T_POINTER";
		case eToken::T_P0:                 return "T_P0";
		case eToken::T_P1:                 return "T_P1";
		case eToken::T_Q0:                 return "T_Q0";
		case eToken::T_Q1:                 return "T_Q1";
		case eToken::T_G0:                 return "T_G0";
		case eToken::T_G1:                 return "T_G1";
		case eToken::T_SEMICOLON:          return "T_SEMICOLON";
		case eToken::T_COLON:              return "T_COLON";
		case eToken::T_COMMA:              return "T_COMMA";
		case eToken::T_ASM_BLOCK:          return "T_ASM_BLOCK";
		default:                           return "UNKNOWN";
	}
}

static std::string get_original_token_str(const Token& token, const std::vector<CharInfo>& char_stream) 
{
	if (token.pos_start > token.pos_end || token.pos_end >= char_stream.size()) return "<invalid_pos>";
	std::string original_str;
	for (size_t i = token.pos_start; i <= token.pos_end; ++i) 
	{
		original_str += encode_utf8(char_stream[i].value);
	}
	return original_str;
}

static std::string get_converted_value_str(const Token& token)
{
	if (std::holds_alternative<uint64_t>(token.value))
	{
		return std::to_string(std::get<uint64_t>(token.value));
	}
	if (std::holds_alternative<double>(token.value))
	{
		std::string s = std::format("{}", std::get<double>(token.value));
		s.erase(s.find_last_not_of('0') + 1, std::string::npos);
		if (s.back() == '.') s.push_back('0');
		return s;
	}
	if (std::holds_alternative<char32_t>(token.value))
	{
		char32_t c = std::get<char32_t>(token.value);
		bool use_quotes = (token.type == eToken::T_CHAR);

		switch (c)
		{
			case U'\n': return use_quotes ? "'\\n'" : "\\n";
			case U'\t': return use_quotes ? "'\\t'" : "\\t";
			case U'\r': return use_quotes ? "'\\r'" : "\\r";
			case U'\\': return use_quotes ? "'\\\\'" : "\\\\";
			case U'\'': return use_quotes ? "'\\''" : "\\'";
			case U'"': return use_quotes ? "'\\\"'" : "\\\"";
			case U'\v': return use_quotes ? "'\\v'" : "\\v";
			case U'\f': return use_quotes ? "'\\f'" : "\\f";
			case U'\x1b': return use_quotes ? "'\\e'" : "\\e";
			default:
				if (c >= 32 && c < 127)
				{
					return use_quotes ? std::format("'{}'", static_cast<char>(c)) : std::format("{}", static_cast<char>(c));
				}
				else
				{
					return std::format("U+{:04X}", static_cast<uint32_t>(c));
				}
		}
	}
	if (std::holds_alternative<std::string>(token.value))
	{
		if (token.type == eToken::T_IDENTIFIER || token.type == eToken::T_KEYWORD)
		{
			return std::get<std::string>(token.value);
		}
		if (token.type == eToken::T_ASM_BLOCK)
		{
			return "{ ... }";
		}

		const std::string& original_str = std::get<std::string>(token.value);
		std::string escaped_str;
		escaped_str.reserve(original_str.length() + 2);
		escaped_str += '"';
		for (char c : original_str)
		{
			switch (c)
			{
				case '\n': escaped_str += "\\n"; break;
				case '\t': escaped_str += "\\t"; break;
				case '\r': escaped_str += "\\r"; break;
				case '\\': escaped_str += "\\\\"; break;
				case '"':  escaped_str += "\\\""; break;
				case '\'': escaped_str += "\\'"; break;
				case '\v': escaped_str += "\\v"; break;
				case '\f': escaped_str += "\\f"; break;
				case '\x1b': escaped_str += "\\e"; break;
				default:   escaped_str += c; break;
			}
		}
		escaped_str += '"';
		return escaped_str;
	}
	return "-";
}


void Lexer::dump_to_file(const std::string& input_filepath, const Streamer& streamer) const 
{
	std::string output_filename = input_filepath + ".lexer.debug";
	std::ofstream out_file(output_filename);

	if (!out_file) 
	{
		ErrorHandler::get().push_error(Sender::Lexer, ErrorType::Error, Action::Coding, ErrorMessage::FileNotFound, 0, 0, "Could not open debug lexer file: " + output_filename);
		return;
	}

	out_file << "--- Begin Lexer Debug ---\n";
	out_file << std::format("{:<15} | {:<10} | {:<12} | {:<20} | {:<20} | {:<10} | {:<25} | {}\n", "File", "Row/Col", "Stream Pos", "Token Type", "Converted Value", "Suffix", "F40 Repr", "Original Lexeme");
	out_file << std::string(140, '-') << "\n";

	for (const auto& token : m_token_stream) 
	{
		std::string filename = streamer.get_filename_by_index(token.file_idx);
		if (filename.length() > 15) 
		{
			filename = "..." + filename.substr(filename.length() - 12);
		}

		std::string pos_str = std::format("{:03}/{:03}", token.row, token.col);
		std::string stream_pos_str = std::format("{:04}-{:04}", token.pos_start, token.pos_end);
		
		std::string original_str_for_display = (token.type == eToken::T_EOF) ? "-" : get_original_token_str(token, streamer.get_char_stream());
		
		if (token.type == eToken::T_ASM_BLOCK) {
			original_str_for_display = "asm";
		}

		std::string converted_val_str = get_converted_value_str(token);
		if (converted_val_str == "-") {
			converted_val_str = original_str_for_display;
		}

		std::string f40_str = (token.type == eToken::T_REAL) ? token.f40_representation : "-";
		std::string suffix_str = token.type_suffix.empty() ? "-" : token.type_suffix;

		std::string output_line = std::format(
			"{:<15} | {:<10} | {:<12} | {:<20} | {:<20} | {:<10} | {:<25} | {}",
			filename,
			pos_str,
			stream_pos_str,
			token_type_to_string(token.type),
			converted_val_str,
			suffix_str,
			f40_str,
			original_str_for_display
		);
		out_file << output_line << '\n';

		if (token.type == eToken::T_ASM_BLOCK)
		{
			out_file << "{\n" << std::get<std::string>(token.value) << "\n}\n";
		}
	}
	out_file << "--- End Lexer Debug ---\n";
}
