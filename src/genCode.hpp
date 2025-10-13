
// *********
// genCode.hpp
// *********

#pragma once

#include "ast.hpp"
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <variant>

class CodeGenerator
{
public:
	CodeGenerator() = default;

	// Genera il codice assembly dall'AST e lo scrive nel file di output.
	void generate(Node* root, const std::string& output_filename, const std::string& target_arch, const std::string& start_address);

private:
	// Routine per scrivere l'header del file assembly
	void generate_header(std::ofstream& out, const std::string& target_arch, const std::string& start_address);

	// Routine per scrivere il footer del file assembly
	void generate_footer(std::ofstream& out);

	// Funzioni helper per la generazione del codice
	void generate_node(Node* node, std::ofstream& out);
	void generate_variable_get(Node* node, std::ofstream& out);
	void generate_variable_set(Node* node, std::ofstream& out);
	void generate_binary_op(Node* node, std::ofstream& out);
	void generate_unary_op(Node* node, std::ofstream& out);
	void generate_var_decl(Node* node, std::ofstream& out);
	void generate_inc_dec(Node* node, std::ofstream& out);
	void generate_if_statement(Node* node, std::ofstream& out);

	// Funzioni helper per la generazione di funzioni
	void generate_function(Node* node, std::ofstream& out);
	void generate_sys_function(Node* node, std::ofstream& out);
	void generate_function_frame(Node* node, std::ofstream& out);
	void generate_function_prologue(Node* node, std::ofstream& out);
	void generate_function_epilogue(std::ofstream& out);

	// Struttura per memorizzare le informazioni sulle costanti reali
	struct RealConstantInfo {
		std::string label;
		std::string byte_representation; // es. "$8b,$00,$18,$93,$74"
		std::string original_value;      // es. "1024.768"
		std::string decimal_bytes;       // es. "[139 128 24 147 116]"
	};

	// Struttura per memorizzare le informazioni sulle costanti stringa
	struct StringConstantInfo {
		std::string label;
		std::string value;
	};

	// Struttura per memorizzare le informazioni sulle costanti globali
	struct GlobalConstantInfo {
		std::string name;
		std::string directive; // .byte, .word, .null
		std::string value;
		std::string comment;
	};

	// Mappa per tenere traccia delle costanti reali e dei loro label
	std::map<double, RealConstantInfo> m_real_constants;
	int m_real_constant_counter = 0;

	// Vettore per tenere traccia delle costanti stringa
	std::vector<StringConstantInfo> m_string_constants;
	int m_string_constant_counter = 0;

	// Vettore per tenere traccia delle costanti globali
	std::vector<GlobalConstantInfo> m_global_constants;

	// Mappe per la ricerca inversa delle costanti globali per valore
	std::map<double, std::string> m_global_real_constant_values;
	std::map<std::string, std::string> m_global_string_constant_values;

	// Contatore per generare etichette univoche per gli statement if
	int m_if_label_counter = 0;

	// Puntatore al nodo della funzione corrente durante la generazione del codice
	Node* m_current_function = nullptr;
};
