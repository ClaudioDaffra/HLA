
// *********
// compiler.hpp
// *********

#pragma once

#include <string>
#include <vector>
#include <memory>
#include "streamer.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "genCode.hpp"

// Enum per specificare il tipo di conversione richiesta
enum class ConversionType 
{
	None,
	F32_TO_F40,
	F64_TO_F40
};

class Compiler 
{
public:
	// Il costruttore accetta gli argomenti dalla riga di comando
	Compiler(int argc, char** argv);

	// Il metodo run() avvia il processo di compilazione
	int run();

private:
	void parse_command_line(int argc, char** argv);
	void print_help() const;
	void execute_pipeline();
	void perform_conversion() const; // Metodo unificato per le conversioni

	// Opzioni del compilatore
	std::string m_input_file;
	std::string m_output_file = "a.out"; // Default output
	bool m_help_requested = false;
	
	bool m_emit_debug_symbols = false;      // per il flag -g
	bool m_enable_optimizations = false;    // per il flag -O
	std::string m_target_architecture = "C64"; // per il flag -t
	size_t m_tab_width = 4;                 // per il flag -T, default 4
	std::string m_start_address;            // per il flag -s
	
	// Aggiunto flag per la compatibilità con mcpp
	bool m_mcpp_compatibility = false;      // per il flag -mcpp, default false

	// Opzioni per la conversione a F40
	ConversionType m_conversion_type = ConversionType::None;
	double m_conversion_input_value = 0.0; // Usiamo double per contenere sia float che double

	// Fasi del compilatore
	Streamer m_streamer;
	Lexer m_lexer;
	Parser m_parser;
	CodeGenerator m_code_generator;
	std::unique_ptr<Node> m_ast_root;

	// Elenco statico delle architetture supportate
	static const std::vector<std::string> SUPPORTED_ARCHITECTURES;

};

// end compiler.hpp

