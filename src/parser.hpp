// *********
// parser.hpp
// *********

#pragma once

#include "lexer.hpp"
#include "ast.hpp"
#include "streamer.hpp"
#include "symbolTable.hpp"
#include <vector>
#include <memory>
#include <string>
#include <set>

class Parser {
public:
	Parser() = default;
	// Il metodo principale che esegue il parsing del flusso di token
	// e restituisce la radice dell'albero sintattico (AST).
	// L'uso di unique_ptr garantisce la gestione automatica della memoria.
	std::unique_ptr < Node > parse(const std::vector < Token > & tokens,
		const Streamer & streamer);

	// Salva una rappresentazione dei passaggi del parser in un file di debug.
	void dump_ast_to_file(const std::string & input_filepath) const;

	// Salva la symbol table in un file di debug.
	void dump_symbol_table_to_file(const std::string& input_filepath) const;

	// Eccezione interna per gestire gli errori di parsing e il recupero.
	struct ParseError {};

private:
	const std::vector < Token > * m_tokens = nullptr;
	const Streamer * m_streamer = nullptr;
	size_t m_current_pos = 0;
	SymbolTable m_symbol_table; // Aggiunta della symbol table
	std::string m_current_namespace; // Per il name mangling
	Node* m_current_function = nullptr; // Puntatore al nodo della funzione corrente

	// Vettori per la gestione di etichette e salti all'interno di una funzione
	std::vector<Node*> m_jumps; // Memorizza i puntatori ai nodi ND_JUMP
	std::set<std::string> m_labels;             // Memorizza i nomi delle etichette definite
	
	bool m_in_panic_mode = false; // Flag per la gestione della cascata di errori

	// Vettore per memorizzare i log dei passaggi del parser per il dump
	mutable std::vector < std::string > m_dump_log;

	// Funzioni helper per il parsing
	const Token & peek() const;
	const Token & peek_next() const; // Funzione helper per guardare avanti
	const Token & consume();
	bool match(eToken type);
	bool match_keyword(const std::string & keyword);
	const Token & expect(eToken type);

	// Metodi per la struttura grammaticale
	std::unique_ptr < Node > parse_top_level_statement();
	std::unique_ptr < Node > parse_namespace();
	std::unique_ptr < Node > parse_function();
	std::unique_ptr < Node > parse_sys_function();
	std::unique_ptr < Node > parse_statement();
	std::unique_ptr < Node > parse_return_statement();
	std::unique_ptr < Node > parse_label_statement();
	std::unique_ptr < Node > parse_jump_statement();
	std::unique_ptr < Node > parse_if_statement();
	std::unique_ptr < Node > parse_declaration();
	std::unique_ptr < Node > parse_const_declaration();
	std::shared_ptr < Type > parse_type_specifier();
	std::unique_ptr < Node > parse_asm_statement();
	std::unique_ptr < Node > parse_expr_statement();
	std::unique_ptr < Node > parse_expr();
	std::unique_ptr < Node > parse_assign();
	std::unique_ptr < Node > parse_logical_or_and();
	std::unique_ptr < Node > parse_equality();
	std::unique_ptr < Node > parse_relational();
	std::unique_ptr < Node > parse_shift();
	std::unique_ptr < Node > parse_add_sub();
	std::unique_ptr < Node > parse_mul_div_mod();
	std::unique_ptr < Node > parse_bitwise_and();
	std::unique_ptr < Node > parse_bitwise_xor();
	std::unique_ptr < Node > parse_bitwise_or();
	std::unique_ptr < Node > parse_unary();
	std::unique_ptr < Node > parse_postfix();
	std::unique_ptr < Node > parse_term();
};

