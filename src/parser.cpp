// *********
// parser.cpp
// *********

#include "parser.hpp"
#include "error.hpp"
#include <variant>
#include <format>
#include <fstream>
#include <string>
#include <cstdint>
#include <cmath>
#include <set>
#include <stdexcept>
#include <algorithm>

// --- Dichiarazioni Forward delle Funzioni Helper Statiche ---

// Converte un tipo in stringa per i log del parser.
static std::string type_to_string_for_parser(const std::shared_ptr<Type>& type);

// Crea un nodo ND_CAST per avvolgere un altro nodo.
static std::unique_ptr<Node> create_cast_node(std::unique_ptr<Node> node_to_cast, std::shared_ptr<Type> target_type, std::vector<std::string>& dump_log);

// Applica le conversioni aritmetiche standard a due operandi (LHS e RHS).
static void apply_arithmetic_conversions(std::unique_ptr<Node>& lhs, std::unique_ptr<Node>& rhs, SymbolTable& symbol_table, std::vector<std::string>& dump_log);

// Esegue la valutazione delle costanti a tempo di compilazione (Constant Folding).
static std::unique_ptr<Node> try_constant_folding(std::unique_ptr<Node>& lhs, std::unique_ptr<Node>& rhs, NodeKind op, SymbolTable& symbol_table, std::vector<std::string>& dump_log, const Token& op_tok);

// Helper per verificare se un tipo C( un tipo intero (u8, s8, u16, s16)
static bool is_integer_type(const std::shared_ptr<Type>& t) {
	if (!t) return false;
	return t->kind >= TypeKind::TYPE_U8 && t->kind <= TypeKind::TYPE_S16;
}

//
// Entry point del parser.
// BNF: <program> ::= <top_level_statement>*
//
std::unique_ptr<Node> Parser::parse(const std::vector<Token>& tokens, const Streamer& streamer)
{
	m_tokens = &tokens;
	m_streamer = &streamer;
	m_current_pos = 0;
	m_dump_log.clear();
	m_current_function = nullptr;
	m_in_panic_mode = false;
	m_current_namespace = "";

	// Pulisce le strutture dati per etichette e salti
	m_jumps.clear();
	m_labels.clear();

	m_dump_log.push_back("Starting parser. Rule: <program> ::= <top_level_statement>*");

	auto program_node = std::make_unique<Node>();
	program_node->kind = NodeKind::ND_PROGRAM;

	Node head;
	Node* current_node = &head;

	while (peek().type != eToken::T_EOF)
	{
		auto node = parse_top_level_statement();
		if (node) {
			current_node->next = std::move(node);
			current_node = current_node->next.get();
		}
	}

	auto main_sym = m_symbol_table.find_symbol("main");
	if (!main_sym || main_sym->kind != SymbolKind::SYMBOL_FUNC) {
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::MissingMainFunction, 0, 0);
	}

	m_dump_log.push_back("Successfully matched EOF. Parsing complete.");
	program_node->body = std::move(head.next);
	return program_node;
}

//
// Analizza una dichiarazione a livello globale (funzione, variabile o blocco asm).
// BNF: <top_level_statement> ::= <namespace> | <function> | <sys_function> | <declaration> | <const_declaration> | <asm_statement>
//
std::unique_ptr<Node> Parser::parse_top_level_statement() {
	m_dump_log.push_back("Entering parse_top_level_statement(). Rule: <top_level_statement> ::= <namespace> | <function> | <sys_function> | <declaration> | <const_declaration> | <asm_statement>");
	try {
		if (peek().type == eToken::T_KEYWORD) {
			const auto& tok = peek();
			if (std::holds_alternative<std::string>(tok.value)) {
				const std::string& keyword = std::get<std::string>(tok.value);
				if (keyword == "fn") {
					consume(); // Consuma 'fn'
					return parse_function();
				}
				if (keyword == "sys") {
					consume(); // Consuma 'sys'
					return parse_sys_function();
				}
				if (keyword == "ns") {
					return parse_namespace();
				}
			}
		}

		if (peek().type == eToken::T_ASM_BLOCK) {
			return parse_asm_statement();
		}

		if (peek().type == eToken::T_IDENTIFIER && peek_next().type == eToken::T_COLON) {
			const auto& third_token = (*m_tokens)[m_current_pos + 2];

			if (third_token.type == eToken::T_POINTER) {
				return parse_declaration();
			}

			if (third_token.type == eToken::T_IDENTIFIER) {
				const std::string& name = std::get<std::string>(third_token.value);
				if (m_symbol_table.find_type(name)) {
					return parse_declaration();
				}
			}
			
			return parse_const_declaration();
		}

		const Token& tok = peek();
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, tok.row, tok.col, "Expected a namespace ('ns'), a function ('fn' or 'sys'), a global variable/constant declaration, or an asm block ('asm {}').");
		throw ParseError{};

	} catch (const ParseError&) {
		m_in_panic_mode = true;
		m_dump_log.push_back("Caught ParseError at top level. Synchronizing to next 'ns', 'fn', 'sys', 'asm {}' or ';'.");
		while (peek().type != eToken::T_EOF) {
			if (peek().type == eToken::T_SEMICOLON) {
				consume();
				break;
			}
			if (peek().type == eToken::T_ASM_BLOCK) {
				break;
			}
			if (peek().type == eToken::T_KEYWORD && std::holds_alternative<std::string>(peek().value)) {
				const std::string& keyword = std::get<std::string>(peek().value);
				if (keyword == "fn" || keyword == "sys" || keyword == "ns") {
					break;
				}
			}
			consume();
		}
		m_in_panic_mode = false;
		return nullptr;
	}
}

//
// Analizza una definizione di namespace.
// BNF: <namespace> ::= "ns" T_IDENTIFIER "{" (<function> | <sys_function>)* "}" ";"
//
std::unique_ptr<Node> Parser::parse_namespace()
{
	m_dump_log.push_back("Entering parse_namespace(). Rule: <namespace> ::= 'ns' T_IDENTIFIER '{' (<function> | <sys_function>)* '}' ';'");
	match_keyword("ns"); // Consuma 'ns'

	const Token& name_tok = expect(eToken::T_IDENTIFIER);
	const std::string ns_name = std::get<std::string>(name_tok.value);

	auto ns_node = std::make_unique<Node>();
	ns_node->kind = NodeKind::ND_NAMESPACE;
	auto ns_symbol = std::make_shared<Symbol>(ns_name, SymbolKind::SYMBOL_NAMESPACE, nullptr, m_symbol_table.get_current_scope()->level, true, &name_tok);
	if (!m_symbol_table.add_symbol(ns_symbol)) {
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::SymbolNameConflict, name_tok.row, name_tok.col, "Namespace name '" + ns_name + "' conflicts with an existing symbol.");
		throw ParseError{};
	}
	ns_node->symbol = ns_symbol;

	expect(eToken::T_G0);
	m_current_namespace = ns_name;

	Node body_head;
	Node* current_func = &body_head;
	while (peek().type != eToken::T_G1 && peek().type != eToken::T_EOF) {
		if (match_keyword("fn")) {
			current_func->next = parse_function();
		} else if (match_keyword("sys")) {
			current_func->next = parse_sys_function();
		} else {
			const Token& tok = peek();
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, tok.row, tok.col, "Only 'fn' or 'sys' functions are allowed inside a namespace.");
			throw ParseError{};
		}

		if (current_func->next) {
			current_func = current_func->next.get();
		}
	}
	ns_node->body = std::move(body_head.next);

	m_current_namespace = "";
	expect(eToken::T_G1);
	expect(eToken::T_SEMICOLON);
	m_dump_log.push_back(std::format("Finished parsing namespace '{}'.", ns_name));
	return ns_node;
}

//
// Analizza una definizione di funzione 'fn'.
// BNF: <function>     ::= "fn" T_IDENTIFIER "(" <param_list> ")" "->" <type_specifier> "{" <block> "}" ";"
//      <param_list>   ::= (<param> ("," <param>)*)?
//      <param>        ::= T_IDENTIFIER ":" <type_specifier>
//
std::unique_ptr<Node> Parser::parse_function()
{
	m_dump_log.push_back("Entering parse_function(). Rule: <function> ::= 'fn' T_IDENTIFIER '(' <param_list> ')' '->' <type_specifier> '{' <block> '}' ';'");

	// 'fn' keyword is already consumed by the caller
	m_dump_log.push_back("Matched 'fn'. Expecting function name (T_IDENTIFIER).");
	const Token& name_tok = expect(eToken::T_IDENTIFIER);
	const std::string func_name = std::get<std::string>(name_tok.value);
	const std::string final_func_name = m_current_namespace.empty()
	                                    ? func_name
	                                    : m_current_namespace + "." + func_name;

	m_dump_log.push_back(std::format("Final (mangled) function name is '{}'.", final_func_name));

	if (m_symbol_table.find_symbol(final_func_name)) {
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::SymbolNameConflict, name_tok.row, name_tok.col, "Symbol '" + final_func_name + "' is already defined.");
		throw ParseError{};
	}

	m_symbol_table.enter_scope(final_func_name);
	m_dump_log.push_back(std::format("Entered new scope for function '{}'.", final_func_name));

	// Pulisce i vettori per etichette e salti all'inizio di ogni funzione
	m_jumps.clear();
	m_labels.clear();
	m_dump_log.push_back("Cleared labels and jumps for new function scope.");

	std::vector<std::shared_ptr<Symbol>> locals_and_params_vec;
	std::vector<std::shared_ptr<Type>> parsed_param_types;
	std::shared_ptr<Type> ret_type;

	if (func_name == "main") {
		m_dump_log.push_back("Special handling for 'main' function.");
		expect(eToken::T_P0);

		if (peek().type != eToken::T_P1) {
			const Token& err_tok = peek();
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, err_tok.row, err_tok.col, "main function does not accept arguments; defaults are 'argc: u8, argv: ^^u16'");
			throw ParseError{};
		}
		expect(eToken::T_P1);
		m_dump_log.push_back("Matched '()'. Injecting default parameters for 'main'.");

		auto argc_type = m_symbol_table.find_type("u8");
		auto argc_sym = std::make_shared<Symbol>("argc", SymbolKind::SYMBOL_VAR, argc_type, m_symbol_table.get_current_scope()->level, false, &name_tok);
		m_symbol_table.add_symbol(argc_sym);
		locals_and_params_vec.push_back(argc_sym);
		parsed_param_types.push_back(argc_type);
		m_dump_log.push_back("  - Injected 'argc: u8'.");

		auto u16_type = m_symbol_table.find_type("u16");
		auto ptr_u16_type = m_symbol_table.get_pointer_type(u16_type);
		auto ptr_ptr_u16_type = m_symbol_table.get_pointer_type(ptr_u16_type);
		auto argv_sym = std::make_shared<Symbol>("argv", SymbolKind::SYMBOL_VAR, ptr_ptr_u16_type, m_symbol_table.get_current_scope()->level, false, &name_tok);
		m_symbol_table.add_symbol(argv_sym);
		locals_and_params_vec.push_back(argv_sym);
		parsed_param_types.push_back(ptr_ptr_u16_type);
		m_dump_log.push_back("  - Injected 'argv: ^^u16'.");

		ret_type = m_symbol_table.find_type("u8");
		m_dump_log.push_back("Forcing return type to 'u8' for 'main'.");

	} else {
		m_dump_log.push_back(std::format("Matched function name '{}'. Expecting '('.", func_name));
		expect(eToken::T_P0);
		m_dump_log.push_back("Parsing <param_list>.");

		if (peek().type != eToken::T_P1) {
			do {
				m_dump_log.push_back("Parsing a parameter. Rule: <param> ::= T_IDENTIFIER ':' <type_specifier>");
				const Token& param_name_tok = expect(eToken::T_IDENTIFIER);
				const std::string param_name = std::get<std::string>(param_name_tok.value);

				expect(eToken::T_COLON);

				std::shared_ptr<Type> final_type = parse_type_specifier();
				parsed_param_types.push_back(final_type);

				if (m_symbol_table.find_symbol(param_name)) {
					ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::SymbolNameConflict, param_name_tok.row, param_name_tok.col, "Parameter name '" + param_name + "' conflicts with an existing symbol (shadowing is not allowed).");
					throw ParseError{};
				}

				auto param_symbol = std::make_shared<Symbol>(param_name, SymbolKind::SYMBOL_VAR, final_type, m_symbol_table.get_current_scope()->level, false, &param_name_tok);
				m_symbol_table.add_symbol(param_symbol);
				locals_and_params_vec.push_back(param_symbol);
				m_dump_log.push_back(std::format("Parsed parameter '{}'.", param_name));

			} while (match(eToken::T_COMMA));
		}
		expect(eToken::T_P1);

		m_dump_log.push_back("Matched ')'. Expecting '->' for return type.");
		expect(eToken::T_RETURN);

		m_dump_log.push_back("Matched '->'. Expecting return type specifier.");
		ret_type = parse_type_specifier();
	}

	auto func_symbol = std::make_shared<Symbol>(final_func_name, SymbolKind::SYMBOL_FUNC, ret_type, m_symbol_table.get_current_scope()->parent->level, true, &name_tok);
	func_symbol->param_types = std::move(parsed_param_types);
	func_symbol->is_syscall = false;

	if (!m_symbol_table.get_current_scope()->parent->add_symbol(func_symbol)) {
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Internal, Action::Parsing, ErrorMessage::SymbolNameConflict, name_tok.row, name_tok.col, "Failed to add function symbol '" + final_func_name + "'");
		throw ParseError{};
	}

	auto func_node = std::make_unique<Node>();
	func_node->kind = NodeKind::ND_FUNCTION;
	func_node->symbol = func_symbol;
	m_current_function = func_node.get();

	m_dump_log.push_back(std::format("Created symbol for function '{}'. Expecting '{{'.", final_func_name));
	expect(eToken::T_G0);

	m_dump_log.push_back("Parsing <block>.");

	Node body_head;
	Node* current_stmt = &body_head;
	while (peek().type != eToken::T_G1 && peek().type != eToken::T_EOF) {
		try {
			auto stmt = parse_statement();
			if (stmt && stmt->kind == NodeKind::ND_VAR_DECL) {
				locals_and_params_vec.push_back(stmt->symbol);
			}
			current_stmt->next = std::move(stmt);
			if (current_stmt->next) {
				current_stmt = current_stmt->next.get();
			}
		} catch (const ParseError&) {
			m_in_panic_mode = true;
			m_dump_log.push_back("Caught ParseError in statement. Synchronizing to next ';'.");
			while(peek().type != eToken::T_EOF && peek().type != eToken::T_SEMICOLON) {
				consume();
			}
			if (match(eToken::T_SEMICOLON)) {
				m_dump_log.push_back("Synchronization complete at ';'.");
			}
			m_in_panic_mode = false;
		}
	}
	func_node->body = std::move(body_head.next);

	// --- VERIFICA FINALE JUMP/LABEL ---
	m_dump_log.push_back("Verifying all jumps have defined labels within the function.");
	for (const auto* jump_node : m_jumps) {
		// La label del salto è memorizzata in str_val
		if (m_labels.find(jump_node->str_val) == m_labels.end()) {
			// Errore: etichetta non trovata
			// TODO: Per avere la posizione corretta, il token andrebbe salvato nel nodo JUMP
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::IdentifierNotFound, 0, 0, "Jump to undefined label '" + jump_node->str_val + "' in function '" + final_func_name + "'.");
			// Non lanciamo un'eccezione qui per poter trovare tutti i label mancanti
		}
	}

	m_dump_log.push_back("Finished parsing <block>. Expecting '}'.");
	expect(eToken::T_G1);

	m_dump_log.push_back("Checking for name conflicts with function '" + final_func_name + "'.");
	for (const auto& sym : locals_and_params_vec) {
		if (sym->name == final_func_name) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::SymbolNameConflict, sym->token->row, sym->token->col, "Symbol '" + sym->name + "' has the same name as the function.");
			throw ParseError{};
		}
	}

	m_dump_log.push_back("Calculating stack offsets for function '" + final_func_name + "'.");
	int current_offset = 0;
	for (const auto& sym : locals_and_params_vec) {
		sym->offset = current_offset;
		current_offset += sym->type->size;
		m_dump_log.push_back(std::format("  - Symbol '{}', size {}, offset {}", sym->name, sym->type->size, sym->offset));
	}

	func_node->stack_size = current_offset;
	func_symbol->stack_size = current_offset; // Aggiorna la size nel simbolo per il debug

	if (current_offset > 255) {
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::StackOverflow, name_tok.row, name_tok.col, "Function '" + final_func_name + "' uses " + std::to_string(current_offset) + " bytes of stack.");
		// Non lanciamo eccezione per permettere di trovare altri errori
	}

	func_node->locals_and_params = std::move(locals_and_params_vec);
	m_dump_log.push_back(std::format("Total stack size for '{}': {}", final_func_name, func_node->stack_size));

	m_symbol_table.leave_scope();
	m_current_function = nullptr;
	m_dump_log.push_back("Matched '}' and left scope. Expecting ';'.");

	expect(eToken::T_SEMICOLON);
	m_dump_log.push_back("Matched ';'. Function parsing complete.");

	return func_node;
}

//
// Analizza una definizione di funzione 'sys'.
// BNF: <sys_function>   ::= "sys" T_IDENTIFIER "(" <sys_param_list> ")" ("->" <type_specifier>)? "{" <block> "}" ";"
//      <sys_param_list> ::= (<sys_param> ("," <sys_param>)*)?
//      <sys_param>      ::= <sys_type> ":" <sys_destination>
//      <sys_type>       ::= "byte" | "word" | "real"
//      <sys_destination>::= "a" | "x" | "y" | "ay" | "ax" | "xy" | "fac1" | "fac2" | T_IDENTIFIER
//
std::unique_ptr<Node> Parser::parse_sys_function()
{
	m_dump_log.push_back("Entering parse_sys_function().");

	const Token& name_tok = expect(eToken::T_IDENTIFIER);
	const std::string func_name = std::get<std::string>(name_tok.value);
	const std::string final_func_name = m_current_namespace.empty() ? func_name : m_current_namespace + "." + func_name;

	if (m_symbol_table.find_symbol(final_func_name)) {
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::SymbolNameConflict, name_tok.row, name_tok.col, "Symbol '" + final_func_name + "' is already defined.");
		throw ParseError{};
	}

	auto func_node = std::make_unique<Node>();
	func_node->kind = NodeKind::ND_SYS_FUNCTION;

	expect(eToken::T_P0);

	std::vector<SysParam> params;
	std::vector<std::shared_ptr<Type>> param_types_for_symbol;
	std::set<std::string> used_destinations;
	std::set<char> used_phys_regs;

	if (peek().type != eToken::T_P1) {
		do {
			const Token& type_tok = expect(eToken::T_IDENTIFIER);
			const std::string type_name = std::get<std::string>(type_tok.value);
			auto param_type = m_symbol_table.find_type(type_name);

			if (type_name != "byte" && type_name != "word" && type_name != "real") {
				ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, type_tok.row, type_tok.col, "Invalid type '" + type_name + "' for sys function. Must be 'byte', 'word', or 'real'.");
				throw ParseError{};
			}

			expect(eToken::T_COLON);

			const Token& dest_tok = expect(eToken::T_IDENTIFIER);
			const std::string dest_name = std::get<std::string>(dest_tok.value);

			if (used_destinations.count(dest_name)) {
				ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::SymbolNameConflict, dest_tok.row, dest_tok.col, "Destination '" + dest_name + "' already used in this parameter list.");
				throw ParseError{};
			}
			used_destinations.insert(dest_name);

			bool is_register = (dest_name == "a" || dest_name == "x" || dest_name == "y" ||
			                    dest_name == "ay" || dest_name == "ax" || dest_name == "xy" ||
			                    dest_name == "fac1" || dest_name == "fac2");

			// --- Validazione Registri Fisici ---
			if (is_register) {
				m_dump_log.push_back(std::format(" -> Checking register '{}'. Used physical regs: [{}].", dest_name, std::string(used_phys_regs.begin(), used_phys_regs.end())));

				try {
					if (dest_name == "a") {
						if (used_phys_regs.count('a')) throw std::runtime_error("Register 'a' is already in use.");
						used_phys_regs.insert('a');
					} else if (dest_name == "x") {
						if (used_phys_regs.count('x')) throw std::runtime_error("Register 'x' is already in use.");
						used_phys_regs.insert('x');
					} else if (dest_name == "y") {
						if (used_phys_regs.count('y')) throw std::runtime_error("Register 'y' is already in use.");
						used_phys_regs.insert('y');
					} else if (dest_name == "ay") {
						if (used_phys_regs.count('a') || used_phys_regs.count('y')) throw std::runtime_error("Component registers for 'ay' are already in use.");
						used_phys_regs.insert('a');
						used_phys_regs.insert('y');
					} else if (dest_name == "ax") {
						if (used_phys_regs.count('a') || used_phys_regs.count('x')) throw std::runtime_error("Component registers for 'ax' are already in use.");
						used_phys_regs.insert('a');
						used_phys_regs.insert('x');
					} else if (dest_name == "xy") {
						if (used_phys_regs.count('x') || used_phys_regs.count('y')) throw std::runtime_error("Component registers for 'xy' are already in use.");
						used_phys_regs.insert('x');
						used_phys_regs.insert('y');
					}
				} catch (const std::runtime_error& e) {
					ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::SymbolNameConflict, dest_tok.row, dest_tok.col, e.what());
					throw ParseError{};
				}
			}

			// --- Validazione Tipo vs Registro ---
			if (is_register) {
				if (type_name == "byte") {
					if (dest_name != "a" && dest_name != "x" && dest_name != "y") {
						ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::InvalidSysRegisterForType, dest_tok.row, dest_tok.col, "Type 'byte' can only be mapped to registers a, x, or y.");
						throw ParseError{};
					}
				} else if (type_name == "word") {
					if (dest_name != "ay" && dest_name != "ax" && dest_name != "xy") {
						ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::InvalidSysRegisterForType, dest_tok.row, dest_tok.col, "Type 'word' can only be mapped to registers ay, ax, or xy.");
						throw ParseError{};
					}
				} else if (type_name == "real") {
					if (dest_name != "fac1" && dest_name != "fac2") {
						ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::InvalidSysRegisterForType, dest_tok.row, dest_tok.col, "Type 'real' can only be mapped to registers fac1 or fac2.");
						throw ParseError{};
					}
				}
			}

			params.push_back({param_type, dest_name, is_register, &dest_tok});
			param_types_for_symbol.push_back(param_type);

		} while (match(eToken::T_COMMA));
	}
	expect(eToken::T_P1);

	// Analizza il tipo di ritorno opzionale
	std::shared_ptr<Type> ret_type;
	if (match(eToken::T_RETURN)) { // Cerca '->'
		m_dump_log.push_back("Matched '->'. Parsing return type for sys function.");
		ret_type = parse_type_specifier();
	} else {
		m_dump_log.push_back("No return type specified for sys function. Defaulting to 'u8'.");
		ret_type = m_symbol_table.find_type("u8");
	}

	auto func_symbol = std::make_shared<Symbol>(final_func_name, SymbolKind::SYMBOL_FUNC, ret_type, m_symbol_table.get_current_scope()->level, true, &name_tok);
	func_symbol->is_syscall = true;
	func_symbol->param_types = std::move(param_types_for_symbol);
	m_symbol_table.add_symbol(func_symbol);

	func_node->symbol = func_symbol;
	func_node->sys_params = std::move(params);

	expect(eToken::T_G0);
	Node body_head;
	Node* current_stmt = &body_head;
	while (peek().type != eToken::T_G1 && peek().type != eToken::T_EOF) {
		current_stmt->next = parse_statement();
		if (current_stmt->next) {
			current_stmt = current_stmt->next.get();
		}
	}
	func_node->body = std::move(body_head.next);
	expect(eToken::T_G1);
	expect(eToken::T_SEMICOLON);

	m_dump_log.push_back(std::format("Finished parsing sys function '{}'.", final_func_name));
	return func_node;
}

//
// Analizza una singola istruzione.
// BNF: <statement> ::= <if_statement> | <declaration> | <const_declaration> | <asm_statement> | <expr_statement> | <return_statement>
//                      | <label_statement> | <jump_statement>
//
std::unique_ptr<Node> Parser::parse_statement()
{
	m_dump_log.push_back(std::format("Entering parse_statement(). Rule: <statement> ::= ... | <label_statement> | <jump_statement>"));

	// Riconoscimento di una label: IDENTIFIER : ;
	if (peek().type == eToken::T_IDENTIFIER && peek_next().type == eToken::T_COLON) {
		const auto& third_token = (*m_tokens)[m_current_pos + 2];
		if (third_token.type == eToken::T_SEMICOLON) {
			return parse_label_statement();
		}
	}

	// Blocco nidificato non permesso
	if (peek().type == eToken::T_G0) {
		const Token& tok = peek();
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::NestedScopeNotAllowed, tok.row, tok.col);
		throw ParseError{};
	}

	if (match_keyword("ret")) {
		return parse_return_statement();
	}

	if (match_keyword("if")) {
		return parse_if_statement();
	}

	if (match_keyword("loop")) {
		return parse_loop_statement();
	}

	if (match_keyword("break")) {
		return parse_break_statement();
	}

	if (match_keyword("continue")) {
		return parse_continue_statement();
	}

	if (match_keyword("jmp")) {
		return parse_jump_statement();
	}

	if (peek().type == eToken::T_IDENTIFIER && peek_next().type == eToken::T_COLON) {
		const auto& third_token = (*m_tokens)[m_current_pos + 2];

		if (third_token.type == eToken::T_POINTER) {
			return parse_declaration();
		}

		if (third_token.type == eToken::T_IDENTIFIER) {
			const std::string& name = std::get<std::string>(third_token.value);
			if (m_symbol_table.find_type(name)) {
				return parse_declaration();
			}
		}
		
		return parse_const_declaration();
	}

	if (peek().type == eToken::T_ASM_BLOCK)
	{
		return parse_asm_statement();
	}

	return parse_expr_statement();
}

//
// Analizza un'istruzione 'ret'.
// BNF: <return_statement> ::= "ret" <expr> ";"
//
std::unique_ptr<Node> Parser::parse_return_statement()
{
	m_dump_log.push_back("Entering parse_return_statement(). Rule: <return_statement> ::= 'ret' <expr> ';'");

	if (!m_current_function) {
		const Token& tok = (*m_tokens)[m_current_pos - 1];
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, tok.row, tok.col, "'ret' statement outside of a function.");
		throw ParseError{};
	}

	auto expr_node = parse_expr();
	expect(eToken::T_SEMICOLON);

	auto ret_node = std::make_unique<Node>();
	ret_node->kind = NodeKind::ND_RETURN;

	auto func_ret_type = m_current_function->symbol->type;
	auto expr_type = expr_node->type;

	if (func_ret_type != expr_type) {
		if (m_symbol_table.is_castable(expr_type, func_ret_type)) {
			ret_node->lhs = create_cast_node(std::move(expr_node), func_ret_type, m_dump_log);
		} else {
			const Token& prev_token = (*m_tokens)[m_current_pos - 2];
			std::string err_msg = std::format("Cannot return type '{}' from a function with return type '{}'.",
			                                  type_to_string_for_parser(expr_type),
			                                  type_to_string_for_parser(func_ret_type));
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, prev_token.row, prev_token.col, err_msg);
			throw ParseError{};
		}
	} else {
		ret_node->lhs = std::move(expr_node);
	}

	m_dump_log.push_back("Created ND_RETURN node.");
	return ret_node;
}

//
// Analizza una definizione di etichetta.
// BNF: <label_statement> ::= T_IDENTIFIER ":" ";"
//
std::unique_ptr<Node> Parser::parse_label_statement()
{
	m_dump_log.push_back("Entering parse_label_statement(). Rule: <label_statement> ::= T_IDENTIFIER ':' ';'");

	const Token& label_tok = expect(eToken::T_IDENTIFIER);
	const std::string label_name = std::get<std::string>(label_tok.value);

	expect(eToken::T_COLON);
	expect(eToken::T_SEMICOLON);

	// Controlla se l'etichetta è già stata definita in questa funzione
	if (m_labels.count(label_name)) {
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::SymbolNameConflict, label_tok.row, label_tok.col, "Redefinition of label '" + label_name + "'.");
		throw ParseError{};
	}

	// Aggiunge l'etichetta al set di quelle definite
	m_labels.insert(label_name);
	m_dump_log.push_back("Defined and registered label '" + label_name + "'.");

	auto node = std::make_unique<Node>();
	node->kind = NodeKind::ND_LABEL;
	node->str_val = label_name;
	return node;
}

//
// Analizza un'istruzione di salto 'jmp'.
// BNF: <jump_statement> ::= "jmp" T_IDENTIFIER ";"
//
std::unique_ptr<Node> Parser::parse_jump_statement()
{
	m_dump_log.push_back("Entering parse_jump_statement(). Rule: <jump_statement> ::= 'jmp' T_IDENTIFIER ';'");

	const Token& target_tok = expect(eToken::T_IDENTIFIER);
	const std::string target_name = std::get<std::string>(target_tok.value);

	expect(eToken::T_SEMICOLON);

	auto node = std::make_unique<Node>();
	node->kind = NodeKind::ND_JUMP;
	node->str_val = target_name;

	// Aggiunge il puntatore grezzo al vettore dei salti per la verifica a fine funzione
	m_jumps.push_back(node.get());
	m_dump_log.push_back("Registered jump to label '" + target_name + "' for later verification.");

	return node;
}

//
// Analizza un'istruzione 'break'.
// BNF: <break_statement> ::= "break" ";"
//
std::unique_ptr<Node> Parser::parse_break_statement()
{
	m_dump_log.push_back("Entering parse_break_statement().");
	expect(eToken::T_SEMICOLON);
	auto node = std::make_unique<Node>();
	node->kind = NodeKind::ND_BREAK;
	return node;
}

//
// Analizza un'istruzione 'continue'.
// BNF: <continue_statement> ::= "continue" ";"
//
std::unique_ptr<Node> Parser::parse_continue_statement()
{
	m_dump_log.push_back("Entering parse_continue_statement().");
	expect(eToken::T_SEMICOLON);
	auto node = std::make_unique<Node>();
	node->kind = NodeKind::ND_CONTINUE;
	return node;
}

//
// Analizza un'istruzione 'loop'.
// BNF: <loop_statement> ::= "loop" <header_options> <body_block> <footer_options> ";"
//
std::unique_ptr<Node> Parser::parse_loop_statement()
{
	m_dump_log.push_back("Entering parse_loop_statement().");

	auto loop_node = std::make_unique<Node>();
	loop_node->kind = NodeKind::ND_LOOP;

	// --- Header Options ---
	if (match(eToken::T_P0)) { // '('
		m_dump_log.push_back("Parsing loop header.");

		if (match(eToken::T_P1)) { // ')' -> Empty header (infinite loop or do-while)
			m_dump_log.push_back("Empty loop header found.");
		} 
		else {
			// Check for declaration: id : type
			if (peek().type == eToken::T_IDENTIFIER && peek_next().type == eToken::T_COLON) {
				m_dump_log.push_back("Parsing loop init (declaration).");
				loop_node->init = parse_declaration(); // Consumes ';'
				
				m_dump_log.push_back("Parsing loop condition.");
				loop_node->lhs = parse_expr();
				expect(eToken::T_SEMICOLON);

				m_dump_log.push_back("Parsing loop step.");
				loop_node->rhs = parse_expr(); // Step is stored in rhs
			}
			else if (peek().type == eToken::T_SEMICOLON) {
				// Empty init: ; cond ; step
				consume(); // ';'
				m_dump_log.push_back("Empty loop init.");
				
				m_dump_log.push_back("Parsing loop condition.");
				loop_node->lhs = parse_expr();
				expect(eToken::T_SEMICOLON);

				m_dump_log.push_back("Parsing loop step.");
				loop_node->rhs = parse_expr();
			}
			else {
				// Expr... could be init (for) or condition (while)
				auto expr = parse_expr();
				
				if (match(eToken::T_SEMICOLON)) {
					// It was init: expr ; cond ; step
					m_dump_log.push_back("Parsed loop init (expression).");
					
					// Wrap expression in a statement if needed, or just keep as init node
					// But init needs to be executed. CodeGen expects init node.
					// ND_EXPR_STMT wrapper might be useful if CodeGen expects a statement, 
					// but here we can just assign the expr to init.
					// However, CodeGen says: generate_node(node->init.get(), out);
					// generate_node handles ND_ASSIGN etc.
					loop_node->init = std::move(expr);

					m_dump_log.push_back("Parsing loop condition.");
					loop_node->lhs = parse_expr();
					expect(eToken::T_SEMICOLON);

					m_dump_log.push_back("Parsing loop step.");
					loop_node->rhs = parse_expr();
				}
				else {
					// It was condition: while(expr)
					// No init, no step.
					loop_node->lhs = std::move(expr);
				}
			}
			expect(eToken::T_P1); // ')'
		}
	}

	// --- Body Block ---
	m_dump_log.push_back("Parsing loop body.");
	expect(eToken::T_G0); // '{'
	
	Node body_head;
	Node* current_stmt = &body_head;
	while (peek().type != eToken::T_G1 && peek().type != eToken::T_EOF) {
		try {
			current_stmt->next = parse_statement();
			if (current_stmt->next) {
				current_stmt = current_stmt->next.get();
			}
		} catch (const ParseError&) {
			m_in_panic_mode = true;
			while(peek().type != eToken::T_EOF && peek().type != eToken::T_SEMICOLON) consume();
			if (match(eToken::T_SEMICOLON)) m_in_panic_mode = false;
		}
	}
	loop_node->body = std::move(body_head.next);
	expect(eToken::T_G1); // '}'

	// --- Footer Options ---
	if (match(eToken::T_P0)) { // '(' -> Post-check (do-while)
		m_dump_log.push_back("Parsing loop footer (post-check).");
		
		if (loop_node->lhs || loop_node->init || loop_node->rhs) {
			const Token& tok = (*m_tokens)[m_current_pos - 1];
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, tok.row, tok.col, "Loop cannot have both header (init/pre-check) and footer (post-check).");
			throw ParseError{};
		}

		loop_node->lhs = parse_expr(); // Condition is stored in lhs
		loop_node->is_post_check = true;
		
		expect(eToken::T_P1); // ')'
	}

	expect(eToken::T_SEMICOLON);
	return loop_node;
}


//
// Analizza uno statement 'if'.
// BNF: <if_statement> ::= "if" "(" <expr> ")" "{" <block> "}" ( "else" "{" <block> "}" )? ";"
//
std::unique_ptr<Node> Parser::parse_if_statement()
{
	m_dump_log.push_back("Entering parse_if_statement(). Rule: <if_statement> ::= 'if' '(' <expr> ')' '{' <block> '}' ('else' '{' <block> '}')? ';'");

	expect(eToken::T_P0);
	m_dump_log.push_back("Matched '('. Parsing condition expression.");

	auto condition_node = parse_expr();
	m_dump_log.push_back("Parsed condition. Expecting ')'.");

	expect(eToken::T_P1);
	m_dump_log.push_back("Matched ')'. Expecting '{' for the 'then' block.");

	auto if_node = std::make_unique<Node>();
	if_node->kind = NodeKind::ND_IF;
	if_node->lhs = std::move(condition_node);

	expect(eToken::T_G0);
	m_dump_log.push_back("Matched '{'. Parsing 'then' block.");
	Node then_body_head;
	Node* current_then_stmt = &then_body_head;
	while (peek().type != eToken::T_G1 && peek().type != eToken::T_EOF) {
		try {
			current_then_stmt->next = parse_statement();
			if (current_then_stmt->next) {
				current_then_stmt = current_then_stmt->next.get();
			}
		} catch (const ParseError&) {
			m_in_panic_mode = true;
			m_dump_log.push_back("Caught ParseError in 'then' block. Synchronizing to next ';'.");
			while(peek().type != eToken::T_EOF && peek().type != eToken::T_SEMICOLON) {
				consume();
			}
			if (match(eToken::T_SEMICOLON)) {
				m_dump_log.push_back("Synchronization complete at ';'.");
			}
			m_in_panic_mode = false;
		}
	}
	if_node->body = std::move(then_body_head.next);
	expect(eToken::T_G1);
	m_dump_log.push_back("Matched '}'. Finished parsing 'then' block.");

	if (match_keyword("else")) {
		m_dump_log.push_back("Matched 'else'. Expecting '{' for the 'else' block.");
		expect(eToken::T_G0);
		m_dump_log.push_back("Matched '{'. Parsing 'else' block.");

		Node else_body_head;
		Node* current_else_stmt = &else_body_head;
		while (peek().type != eToken::T_G1 && peek().type != eToken::T_EOF) {
			 try {
				current_else_stmt->next = parse_statement();
				if (current_else_stmt->next) {
					current_else_stmt = current_else_stmt->next.get();
				}
			} catch (const ParseError&) {
				m_in_panic_mode = true;
				m_dump_log.push_back("Caught ParseError in 'else' block. Synchronizing to next ';'.");
				while(peek().type != eToken::T_EOF && peek().type != eToken::T_SEMICOLON) {
					consume();
				}
				if (match(eToken::T_SEMICOLON)) {
					m_dump_log.push_back("Synchronization complete at ';'.");
				}
				m_in_panic_mode = false;
			}
		}
		if_node->rhs = std::move(else_body_head.next);
		expect(eToken::T_G1);
		m_dump_log.push_back("Matched '}'. Finished parsing 'else' block.");
	}

	expect(eToken::T_SEMICOLON);
	m_dump_log.push_back("Matched ';'. 'if' statement parsing complete.");

	return if_node;
}

//
// Analizza una dichiarazione di costante.
// BNF: <const_declaration> ::= T_IDENTIFIER ":" <expr> ";"
//
std::unique_ptr<Node> Parser::parse_const_declaration()
{
    m_dump_log.push_back("Entering parse_const_declaration(). Rule: <const_declaration> ::= T_IDENTIFIER ':' <expr> ';'");

    const Token& id_token = expect(eToken::T_IDENTIFIER);
    const std::string const_name = std::get<std::string>(id_token.value);

    m_dump_log.push_back(std::format("Matched identifier '{}'. Expecting ':'.", const_name));
    expect(eToken::T_COLON);

    auto value_node = parse_expr();

    if (value_node->kind != NodeKind::ND_INTEGER_CONSTANT &&
        value_node->kind != NodeKind::ND_REAL_CONSTANT &&
        value_node->kind != NodeKind::ND_STRING_LITERAL) {
        ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::ConstantNotInitializedWithLiteral, id_token.row, id_token.col, "The expression for a constant must evaluate to a compile-time literal.");
        throw ParseError{};
    }

    expect(eToken::T_SEMICOLON);

    if (m_symbol_table.find_symbol(const_name)) {
        ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::SymbolNameConflict, id_token.row, id_token.col, "Symbol '" + const_name + "' conflicts with an existing definition.");
        throw ParseError{};
    }

    auto new_symbol = std::make_shared<Symbol>(const_name, SymbolKind::SYMBOL_CONST, value_node->type, m_symbol_table.get_current_scope()->level, m_symbol_table.get_current_scope()->level == 0, &id_token);

    // Assegna il valore in base al tipo di nodo risultante
    if (value_node->kind == NodeKind::ND_INTEGER_CONSTANT) {
        new_symbol->const_value = value_node->val;
    } else if (value_node->kind == NodeKind::ND_REAL_CONSTANT) {
        new_symbol->const_value = value_node->r_val;
    } else { // ND_STRING_LITERAL
        new_symbol->const_value = value_node->str_val;
    }

    m_symbol_table.add_symbol(new_symbol);
    m_dump_log.push_back(std::format("Created constant symbol '{}' and added to scope.", const_name));

    auto node = std::make_unique<Node>();
    node->kind = NodeKind::ND_CONST_DECL;
    node->symbol = new_symbol;
    return node;
}

//
// Analizza una dichiarazione di variabile.
// BNF: <declaration> ::= T_IDENTIFIER ":" <type_specifier> ";"
//
std::unique_ptr<Node> Parser::parse_declaration()
{
	m_dump_log.push_back("Entering parse_declaration(). Rule: <declaration> ::= T_IDENTIFIER ':' <type_specifier> ';'");

	const Token& id_token = expect(eToken::T_IDENTIFIER);
	const std::string var_name = std::get<std::string>(id_token.value);

	m_dump_log.push_back(std::format("Matched identifier '{}'. Expecting ':'.", var_name));
	expect(eToken::T_COLON);

	m_dump_log.push_back("Parsing type specifier for declaration.");
	std::shared_ptr<Type> final_type = parse_type_specifier();

	m_dump_log.push_back(std::format("Parsed type for '{}'. Expecting ';'.", var_name));
	expect(eToken::T_SEMICOLON);

	if (m_symbol_table.find_symbol(var_name)) {
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::SymbolNameConflict, id_token.row, id_token.col, "Symbol '" + var_name + "' conflicts with an existing definition (shadowing is not allowed).");
		throw ParseError{};
	}

	auto new_symbol = std::make_shared<Symbol>(var_name, SymbolKind::SYMBOL_VAR, final_type, m_symbol_table.get_current_scope()->level, m_symbol_table.get_current_scope()->level == 0, &id_token);

	m_symbol_table.add_symbol(new_symbol);

	m_dump_log.push_back(std::format("Created symbol '{}' and added to scope. Creating ND_VAR_DECL node.", var_name));

	auto node = std::make_unique<Node>();
	node->kind = NodeKind::ND_VAR_DECL;
	node->symbol = new_symbol;
	return node;
}

//
// Analizza uno specificatore di tipo.
// BNF: <type_specifier> ::= T_POINTER* T_IDENTIFIER
//
std::shared_ptr<Type> Parser::parse_type_specifier()
{
	m_dump_log.push_back("Entering parse_type_specifier(). Rule: <type_specifier> ::= T_POINTER* T_IDENTIFIER");

	int pointer_count = 0;
	while (match(eToken::T_POINTER)) {
		pointer_count++;
		m_dump_log.push_back("Matched pointer type specifier '^'.");
	}

	m_dump_log.push_back("Expecting type identifier.");
	const Token& type_id_token = expect(eToken::T_IDENTIFIER);
	const std::string type_name = std::get<std::string>(type_id_token.value);

	std::shared_ptr<Type> final_type = m_symbol_table.find_type(type_name);
	if (!final_type) {
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::IdentifierNotFound, type_id_token.row, type_id_token.col, "Unknown type name '" + type_name + "'");
		throw ParseError{};
	}

	m_dump_log.push_back(std::format("Matched type identifier '{}'.", type_name));

	for (int i = 0; i < pointer_count; ++i) {
		final_type = m_symbol_table.get_pointer_type(final_type);
	}

	return final_type;
}

//
// Analizza un blocco asm.
// BNF: <asm_statement> ::= T_ASM_BLOCK ";"
//
std::unique_ptr<Node> Parser::parse_asm_statement()
{
	m_dump_log.push_back("Entering parse_asm_statement(). Rule: <asm_statement> ::= T_ASM_BLOCK ';'");

	const Token& asm_token = consume();
	const std::string& content = std::get<std::string>(asm_token.value);

	m_dump_log.push_back("Matched T_ASM_BLOCK. Expecting ';'.");
	expect(eToken::T_SEMICOLON);
	m_dump_log.push_back("Matched ';'. Creating ND_ASM node.");

	auto node = std::make_unique<Node>();
	node->kind = NodeKind::ND_ASM;
	node->asm_str = content;
	return node;
}

//
// Analizza un'istruzione di espressione.
// BNF: <expr_statement> ::= <expr> ";"
//
std::unique_ptr<Node> Parser::parse_expr_statement()
{
	m_dump_log.push_back("Entering parse_expr_statement(). Rule: <expr_statement> ::= <expr> ';'");
	auto expr_node = parse_expr();

	m_dump_log.push_back("Parsed expression. Expecting ';'.");
	expect(eToken::T_SEMICOLON);
	m_dump_log.push_back("Matched ';'.");

	auto stmt_node = std::make_unique<Node>();
	stmt_node->kind = NodeKind::ND_EXPR_STMT;
	stmt_node->lhs = std::move(expr_node);
	m_dump_log.push_back("Wrapping expression in ND_EXPR_STMT.");
	return stmt_node;
}

//
// Punto di ingresso per le espressioni.
// BNF: <expr> ::= <assign>
//
std::unique_ptr<Node> Parser::parse_expr()
{
	m_dump_log.push_back("Entering parse_expr(), delegating to parse_assign(). Rule: <expr> ::= <assign>");
	return parse_assign();
}

//
// Analizza l'operatore di assegnazione.
// BNF: <assign> ::= <logical_or_and> (":=" <assign>)?
//
std::unique_ptr<Node> Parser::parse_assign()
{
	m_dump_log.push_back("Entering parse_assign(). Rule: <assign> ::= <logical_or_and> (':=' <assign>)?");
	auto node = parse_logical_or_and();

	if (match(eToken::T_ASSIGN)) {
		m_dump_log.push_back("Matched T_ASSIGN (:=). Creating ND_ASSIGN node.");

		if (node->kind == NodeKind::ND_VAR && node->symbol->kind == SymbolKind::SYMBOL_CONST) {
			const Token& prev_token = (*m_tokens)[m_current_pos - 2];
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::AssignmentToConstant, prev_token.row, prev_token.col, "Cannot assign to constant '" + node->symbol->name + "'.");
			throw ParseError{};
		}

		if (node->kind != NodeKind::ND_VAR && node->kind != NodeKind::ND_DEREF) {
			const Token& prev_token = (*m_tokens)[m_current_pos - 2];
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, prev_token.row, prev_token.col, "The left-hand side of an assignment must be a variable or a pointer dereference.");
			throw ParseError{};
		}

		auto assign_node = std::make_unique<Node>();
		assign_node->kind = NodeKind::ND_ASSIGN;
		assign_node->lhs = std::move(node);

		m_dump_log.push_back("Parsing right-hand side of ':='.");
		assign_node->rhs = parse_assign();

		// Gestione speciale per assegnamento puntatore := intero (es. ptr := 0)
		bool lhs_is_ptr = assign_node->lhs->type->kind == TypeKind::TYPE_POINTER;
		bool rhs_is_int_literal = assign_node->rhs->kind == NodeKind::ND_INTEGER_CONSTANT;

		if (lhs_is_ptr && rhs_is_int_literal) {
			m_dump_log.push_back(" -> Special case: assigning integer literal to a pointer.");
			// Il tipo del letterale intero viene forzato a essere un puntatore.
			// Questo permette `ptr := 0` ma anche `ptr := $C000`.
			// Il tipo base del puntatore non viene controllato, si assume che il programmatore sappia cosa sta facendo.
			assign_node->rhs->type = assign_node->lhs->type;
		}
		else if (assign_node->lhs->type != assign_node->rhs->type) {
			if (m_symbol_table.is_castable(assign_node->rhs->type, assign_node->lhs->type)) {
				assign_node->rhs = create_cast_node(std::move(assign_node->rhs), assign_node->lhs->type, m_dump_log);
			} else {
				const Token& prev_token = (*m_tokens)[m_current_pos - 2];
				std::string err_msg = std::format("Cannot assign type '{}' to type '{}'.",
				                                  type_to_string_for_parser(assign_node->rhs->type),
				                                  type_to_string_for_parser(assign_node->lhs->type));
				ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, prev_token.row, prev_token.col, err_msg);
				throw ParseError{};
			}
		}

		assign_node->type = assign_node->lhs->type;
		return assign_node;
	}

	return node;
}

//
// Analizza gli operatori logici AND e OR.
// BNF: <logical_or_and> ::= <bitwise_or> (("&&" | "||") <bitwise_or>)*
//
std::unique_ptr<Node> Parser::parse_logical_or_and()
{
	m_dump_log.push_back("Entering parse_logical_or_and(). Rule: <logical_or_and> ::= <bitwise_or> (('&&' | '||') <bitwise_or>)*");
	auto node = parse_bitwise_or();

	while (true)
	{
		NodeKind op_kind;
		if (match(eToken::T_AND)) {
			op_kind = NodeKind::ND_LOGICAL_AND;
		} else if (match(eToken::T_OR)) {
			op_kind = NodeKind::ND_LOGICAL_OR;
		} else {
			return node;
		}

		auto rhs = parse_bitwise_or();

		// --- Logica per f40 in && e || ---
		bool lhs_is_f40 = (node->type && node->type->kind == TypeKind::TYPE_F40);
		bool rhs_is_f40 = (rhs->type && rhs->type->kind == TypeKind::TYPE_F40);

		if (lhs_is_f40 || rhs_is_f40) {
			m_dump_log.push_back(" -> Logical operation with f40 operand(s). Casting to u8 for boolean evaluation.");
			auto u8_type = m_symbol_table.find_type("u8");

			if (node->type != u8_type) {
				node = create_cast_node(std::move(node), u8_type, m_dump_log);
			}
			if (rhs->type != u8_type) {
				rhs = create_cast_node(std::move(rhs), u8_type, m_dump_log);
			}
		}

		auto op_node = std::make_unique<Node>();
		op_node->kind = op_kind;
		op_node->lhs = std::move(node);
		op_node->rhs = std::move(rhs);
		op_node->type = m_symbol_table.find_type("u8"); // Il risultato è sempre un booleano
		node = std::move(op_node);
	}
}

//
// Analizza l'operatore bitwise OR.
// BNF: <bitwise_or> ::= <bitwise_xor> ("|" <bitwise_xor>)*
//
std::unique_ptr<Node> Parser::parse_bitwise_or() {
    m_dump_log.push_back("Entering parse_bitwise_or(). Rule: <bitwise_or> ::= <bitwise_xor> ('|' <bitwise_xor>)*");
    auto node = parse_bitwise_xor();

    while (match(eToken::T_BIT_OR)) {
        const Token& op_tok = (*m_tokens)[m_current_pos - 1];
        auto rhs = parse_bitwise_xor();

        if (!is_integer_type(node->type) || !is_integer_type(rhs->type)) {
            ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::BitwiseOperationOnFloat, op_tok.row, op_tok.col, "Operator '|' can only be applied to integer types.");
            throw ParseError{};
        }

        auto folded_node = try_constant_folding(node, rhs, NodeKind::ND_BIT_OR, m_symbol_table, m_dump_log, op_tok);
        if (folded_node) {
            node = std::move(folded_node);
            continue;
        }

        auto op_node = std::make_unique<Node>();
        op_node->kind = NodeKind::ND_BIT_OR;
        op_node->lhs = std::move(node);
        op_node->rhs = std::move(rhs);

        apply_arithmetic_conversions(op_node->lhs, op_node->rhs, m_symbol_table, m_dump_log);
        op_node->type = op_node->lhs->type;
        node = std::move(op_node);
    }
    return node;
}

//
// Analizza l'operatore bitwise XOR.
// BNF: <bitwise_xor> ::= <bitwise_and> ("^" <bitwise_and>)*
//
std::unique_ptr<Node> Parser::parse_bitwise_xor() {
    m_dump_log.push_back("Entering parse_bitwise_xor(). Rule: <bitwise_xor> ::= <bitwise_and> ('^' <bitwise_and>)*");
    auto node = parse_bitwise_and();

    // NOTA: '^' è tokenizzato come T_POINTER
    while (match(eToken::T_POINTER)) {
        const Token& op_tok = (*m_tokens)[m_current_pos - 1];
        auto rhs = parse_bitwise_and();

        if (!is_integer_type(node->type) || !is_integer_type(rhs->type)) {
            ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::BitwiseOperationOnFloat, op_tok.row, op_tok.col, "Operator '^' can only be applied to integer types.");
            throw ParseError{};
        }

        auto folded_node = try_constant_folding(node, rhs, NodeKind::ND_BIT_XOR, m_symbol_table, m_dump_log, op_tok);
        if (folded_node) {
            node = std::move(folded_node);
            continue;
        }

        auto op_node = std::make_unique<Node>();
        op_node->kind = NodeKind::ND_BIT_XOR;
        op_node->lhs = std::move(node);
        op_node->rhs = std::move(rhs);

        apply_arithmetic_conversions(op_node->lhs, op_node->rhs, m_symbol_table, m_dump_log);
        op_node->type = op_node->lhs->type;
        node = std::move(op_node);
    }
    return node;
}

//
// Analizza l'operatore bitwise AND.
// BNF: <bitwise_and> ::= <equality> ("&" <equality>)*
//
std::unique_ptr<Node> Parser::parse_bitwise_and() {
    m_dump_log.push_back("Entering parse_bitwise_and(). Rule: <bitwise_and> ::= <equality> ('&' <equality>)*");
    auto node = parse_equality();

    while (match(eToken::T_BIT_AND)) {
        const Token& op_tok = (*m_tokens)[m_current_pos - 1];
        auto rhs = parse_equality();

        if (!is_integer_type(node->type) || !is_integer_type(rhs->type)) {
            ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::BitwiseOperationOnFloat, op_tok.row, op_tok.col, "Operator '&' can only be applied to integer types.");
            throw ParseError{};
        }

        auto folded_node = try_constant_folding(node, rhs, NodeKind::ND_BIT_AND, m_symbol_table, m_dump_log, op_tok);
        if (folded_node) {
            node = std::move(folded_node);
            continue;
        }

        auto op_node = std::make_unique<Node>();
        op_node->kind = NodeKind::ND_BIT_AND;
        op_node->lhs = std::move(node);
        op_node->rhs = std::move(rhs);

        apply_arithmetic_conversions(op_node->lhs, op_node->rhs, m_symbol_table, m_dump_log);
        op_node->type = op_node->lhs->type;
        node = std::move(op_node);
    }
    return node;
}


//
// Analizza gli operatori di uguaglianza.
// BNF: <equality> ::= <relational> (("?=" | "!=") <relational>)*
//
std::unique_ptr<Node> Parser::parse_equality()
{
	m_dump_log.push_back("Entering parse_equality(). Rule: <equality> ::= <relational> (('?=' | '!=') <relational>)*");
	auto node = parse_relational();

	while (true)
	{
		const Token& op_tok = peek();
		NodeKind op_kind;
		if (match(eToken::T_COMPARE)) {
			op_kind = NodeKind::ND_EQ;
		} else if (match(eToken::T_NOT_EQUAL)) {
			op_kind = NodeKind::ND_NE;
		} else {
			return node;
		}

		auto rhs = parse_relational();

		auto folded_node = try_constant_folding(node, rhs, op_kind, m_symbol_table, m_dump_log, op_tok);
		if (folded_node) {
			node = std::move(folded_node);
			continue;
		}

		auto op_node = std::make_unique<Node>();
		op_node->kind = op_kind;
		op_node->lhs = std::move(node);
		op_node->rhs = std::move(rhs);

		apply_arithmetic_conversions(op_node->lhs, op_node->rhs, m_symbol_table, m_dump_log);
		op_node->type = m_symbol_table.find_type("u8"); // Il risultato C( un booleano (u8)
		node = std::move(op_node);
	}
}

//
// Analizza gli operatori relazionali.
// BNF: <relational> ::= <shift> (("<" | "<=" | ">" | ">=") <shift>)*
//
std::unique_ptr<Node> Parser::parse_relational()
{
	m_dump_log.push_back("Entering parse_relational(). Rule: <relational> ::= <shift> (('<' | '<=' | '>' | '>=') <shift>)*");
	auto node = parse_shift();

	while (true)
	{
		const Token& op_tok = peek();
		NodeKind op_kind;
		if (match(eToken::T_LESS_THAN)) {
			op_kind = NodeKind::ND_LT;
		} else if (match(eToken::T_LESS_EQUAL_THAN)) {
			op_kind = NodeKind::ND_LE;
		} else if (match(eToken::T_GREATER_THAN)) {
			op_kind = NodeKind::ND_GT;
		} else if (match(eToken::T_GREATER_EQUAL_THAN)) {
			op_kind = NodeKind::ND_GE;
		} else {
			return node;
		}

		auto rhs = parse_shift();

		auto folded_node = try_constant_folding(node, rhs, op_kind, m_symbol_table, m_dump_log, op_tok);
		if (folded_node) {
			node = std::move(folded_node);
			continue;
		}

		auto op_node = std::make_unique<Node>();
		op_node->kind = op_kind;
		op_node->lhs = std::move(node);
		op_node->rhs = std::move(rhs);

		apply_arithmetic_conversions(op_node->lhs, op_node->rhs, m_symbol_table, m_dump_log);
		op_node->type = m_symbol_table.find_type("u8"); // Il risultato C( un booleano (u8)
		node = std::move(op_node);
	}
}

//
// Analizza gli operatori di shift.
// BNF: <shift> ::= <add_sub> (("<<" | ">>") <add_sub>)*
//
std::unique_ptr<Node> Parser::parse_shift() {
    m_dump_log.push_back("Entering parse_shift(). Rule: <shift> ::= <add_sub> (('<<' | '>>') <add_sub>)*");
    auto node = parse_add_sub();

    while (true) {
        const Token& op_tok = peek();
        NodeKind op_kind;
        if (match(eToken::T_SHIFT_LEFT)) {
            op_kind = NodeKind::ND_SHIFT_LEFT;
        } else if (match(eToken::T_SHIFT_RIGHT)) {
            op_kind = NodeKind::ND_SHIFT_RIGHT;
        } else {
            return node;
        }

        auto rhs = parse_add_sub();

        if (!is_integer_type(node->type) || !is_integer_type(rhs->type)) {
            ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::BitwiseOperationOnFloat, op_tok.row, op_tok.col, "Shift operators can only be applied to integer types.");
            throw ParseError{};
        }

        auto folded_node = try_constant_folding(node, rhs, op_kind, m_symbol_table, m_dump_log, op_tok);
        if (folded_node) {
            node = std::move(folded_node);
            continue;
        }

        auto op_node = std::make_unique<Node>();
        op_node->kind = op_kind;
        op_node->lhs = std::move(node);
        op_node->rhs = std::move(rhs);

        // Il tipo del risultato di uno shift è il tipo (promosso) dell'operando sinistro.
        // Qui non facciamo promozioni complesse, quindi usiamo il tipo di lhs.
        op_node->type = op_node->lhs->type;
        node = std::move(op_node);
    }
}


//
// Analizza addizione e sottrazione, gestendo l'aritmetica dei puntatori.
// BNF: <add_sub> ::= <mul_div_mod> (("+" | "-") <mul_div_mod>)*
//
std::unique_ptr<Node> Parser::parse_add_sub()
{
	m_dump_log.push_back("Entering parse_add_sub(). Rule: <add_sub> ::= <mul_div_mod> (('+' | '-') <mul_div_mod>)*");
	auto node = parse_mul_div_mod();
	m_dump_log.push_back("Parsed initial <mul_div_mod>, now looking for '+' or '-'.");

	while (true)
	{
		const Token& op_tok = peek();
		NodeKind op_kind;

		if (match(eToken::T_PLUS)) {
			op_kind = NodeKind::ND_ADD;
		} else if (match(eToken::T_MINUS)) {
			op_kind = NodeKind::ND_SUB;
		} else {
			m_dump_log.push_back("No more '+' or '-'. Exiting parse_add_sub().");
			return node;
		}

		auto rhs = parse_mul_div_mod();
		auto& lhs = node;

		// --- NUOVA LOGICA: Constant Folding ---
		auto folded_node = try_constant_folding(lhs, rhs, op_kind, m_symbol_table, m_dump_log, op_tok);
		if (folded_node) {
			node = std::move(folded_node);
			continue;
		}

		// --- Controllo per le stringhe letterali ---
		if (lhs->kind == NodeKind::ND_STRING_LITERAL || rhs->kind == NodeKind::ND_STRING_LITERAL) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::InvalidStringOperation, op_tok.row, op_tok.col, std::string("Operator '") + (op_kind == NodeKind::ND_ADD ? "+" : "-") + "' cannot be applied to string literals.");
			throw ParseError{};
		}

		bool lhs_is_ptr = lhs->type && lhs->type->kind == TypeKind::TYPE_POINTER;
		bool rhs_is_ptr = rhs->type && rhs->type->kind == TypeKind::TYPE_POINTER;
		bool lhs_is_int = is_integer_type(lhs->type);
		bool rhs_is_int = is_integer_type(rhs->type);

		// Case: ptr - ptr
		if (lhs_is_ptr && rhs_is_ptr) {
			if (op_kind == NodeKind::ND_ADD) {
				ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::PtrArithmetic, op_tok.row, op_tok.col, "Invalid addition of two pointers.");
				throw ParseError{};
			}
			if (lhs->type->base != rhs->type->base) {
				ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::PtrArithmetic, op_tok.row, op_tok.col, "Subtracting incompatible pointer types.");
				throw ParseError{};
			}

			m_dump_log.push_back(" -> Pointer subtraction (ptr - ptr). Injecting scaling division.");
			auto sub_node = std::make_unique<Node>();
			sub_node->kind = NodeKind::ND_SUB;
			sub_node->lhs = std::move(lhs);
			sub_node->rhs = std::move(rhs);
			sub_node->type = m_symbol_table.find_type("u16"); // Byte difference

			int size = sub_node->lhs->type->base->size;
			if (size > 1) {
				auto size_node = std::make_unique<Node>();
				size_node->kind = NodeKind::ND_INTEGER_CONSTANT;
				size_node->val = size;
				size_node->type = m_symbol_table.find_type("u16");

				auto div_node = std::make_unique<Node>();
				div_node->kind = NodeKind::ND_DIV;
				div_node->lhs = std::move(sub_node);
				div_node->rhs = std::move(size_node);
				div_node->type = m_symbol_table.find_type("u16");
				node = std::move(div_node);
			} else {
				node = std::move(sub_node);
			}
			continue; // L'intero nodo C( stato sostituito, ricomincia il loop
		}

		// Case: ptr +/- int  OR  int + ptr
		if ((lhs_is_ptr && rhs_is_int) || (lhs_is_int && rhs_is_ptr)) {
			if (lhs_is_int && rhs_is_ptr) { // int + ptr
				if (op_kind == NodeKind::ND_SUB) {
					ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::PtrArithmetic, op_tok.row, op_tok.col, "Invalid subtraction of a pointer from an integer.");
					throw ParseError{};
				}
				std::swap(lhs, rhs); // Normalizza in ptr + int
			}

			// BUG #2 FIX: L'intero (rhs) deve essere promosso a u16 prima della moltiplicazione per il sizeof.
			auto u16_type = m_symbol_table.find_type("u16");
			if (rhs->type != u16_type) {
				rhs = create_cast_node(std::move(rhs), u16_type, m_dump_log);
			}

			m_dump_log.push_back(" -> Pointer arithmetic (ptr +/- int). Injecting scaling multiplication.");
			int size = lhs->type->base->size;
			if (size > 1) {
				auto size_node = std::make_unique<Node>();
				size_node->kind = NodeKind::ND_INTEGER_CONSTANT;
				size_node->val = size;
				size_node->type = u16_type;

				auto mul_node = std::make_unique<Node>();
				mul_node->kind = NodeKind::ND_MUL;
				mul_node->lhs = std::move(rhs);
				mul_node->rhs = std::move(size_node);
				mul_node->type = u16_type;
				rhs = std::move(mul_node);
			}
		}
		// Case: Standard arithmetic
		else if ((lhs_is_int && rhs_is_int) || (lhs->type->kind == TypeKind::TYPE_F40 && rhs->type->kind == TypeKind::TYPE_F40)) {
			apply_arithmetic_conversions(lhs, rhs, m_symbol_table, m_dump_log);
		}
		// Error case
		else {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::PtrArithmetic, op_tok.row, op_tok.col, "Invalid operands for '+' or '-' operator.");
			throw ParseError{};
		}

		auto op_node = std::make_unique<Node>();
		op_node->kind = op_kind;
		op_node->lhs = std::move(lhs);
		op_node->rhs = std::move(rhs);
		op_node->type = op_node->lhs->type; // Dopo la normalizzazione, il tipo di lhs (il puntatore) C( il tipo del risultato
		node = std::move(op_node);
	}
}

//
// Analizza moltiplicazione, divisione e modulo.
// BNF: <mul_div_mod> ::= <unary> (("*" | "/" | "%%") <unary>)*
//
std::unique_ptr<Node> Parser::parse_mul_div_mod()
{
	m_dump_log.push_back("Entering parse_mul_div_mod(). Rule: <mul_div_mod> ::= <unary> (('*' | '/' | '%%') <unary>)*");
	auto node = parse_unary();
	m_dump_log.push_back("Parsed initial <unary>, now looking for '*', '/' or '%%'.");

	while (true)
	{
		const Token& op_tok = peek();
		NodeKind op_kind;

		if (match(eToken::T_MUL)) {
			op_kind = NodeKind::ND_MUL;
		} else if (match(eToken::T_DIV)) {
			op_kind = NodeKind::ND_DIV;
		} else if (match(eToken::T_MOD)) {
			op_kind = NodeKind::ND_MOD;
		} else {
			m_dump_log.push_back("No more '*', '/' or '%%'. Exiting parse_mul_div_mod().");
			return node;
		}

		auto rhs = parse_unary();

		// --- NUOVA LOGICA: Constant Folding ---
		auto folded_node = try_constant_folding(node, rhs, op_kind, m_symbol_table, m_dump_log, op_tok);
		if (folded_node) {
			node = std::move(folded_node);
			continue;
		}

		// --- Controllo per le stringhe letterali ---
		if (node->kind == NodeKind::ND_STRING_LITERAL || rhs->kind == NodeKind::ND_STRING_LITERAL) {
			std::string op_str;
			if (op_kind == NodeKind::ND_MUL) op_str = "*";
			else if (op_kind == NodeKind::ND_DIV) op_str = "/";
			else op_str = "%%";
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::InvalidStringOperation, op_tok.row, op_tok.col, std::string("Operator '") + op_str + "' cannot be applied to string literals.");
			throw ParseError{};
		}

		// I puntatori non possono essere usati con *, /, %%
		if ((node->type && node->type->kind == TypeKind::TYPE_POINTER) || (rhs->type && rhs->type->kind == TypeKind::TYPE_POINTER)) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::PtrArithmetic, op_tok.row, op_tok.col, "Multiplicative operators cannot be applied to pointers.");
			throw ParseError{};
		}

		auto op_node = std::make_unique<Node>();
		op_node->kind = op_kind;
		op_node->lhs = std::move(node);
		op_node->rhs = std::move(rhs);

		apply_arithmetic_conversions(op_node->lhs, op_node->rhs, m_symbol_table, m_dump_log);
		op_node->type = op_node->lhs->type;

		node = std::move(op_node);
	}
}

//
// Analizza gli operatori unari.
// BNF: <unary> ::= ("++" | "--" | "+" | "-" | "!" | "@" | "^" | "~" | "°") <unary> | <postfix>
//
std::unique_ptr<Node> Parser::parse_unary()
{
	m_dump_log.push_back("Entering parse_unary(). Rule: <unary> ::= ('++' | '--' | '+' | '-' | '!' | '@' | '^' | '~' | '°') <unary> | <postfix>");

	// Gestione operatori prefissi
	if (match(eToken::T_INCREMENT) || match(eToken::T_DECREMENT)) {
		const Token& op_tok = (*m_tokens)[m_current_pos - 1];
		NodeKind kind = (op_tok.type == eToken::T_INCREMENT) ? NodeKind::ND_PRE_INC : NodeKind::ND_PRE_DEC;
		std::string op_str = (kind == NodeKind::ND_PRE_INC) ? "++" : "--";
		m_dump_log.push_back("Matched prefix '" + op_str + "'.");

		auto operand = parse_unary();

		// Validazione: l'operando deve essere un l-value
		if (operand->kind != NodeKind::ND_VAR && operand->kind != NodeKind::ND_DEREF) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, op_tok.row, op_tok.col, "Operand for prefix '" + op_str + "' must be a variable or a pointer dereference (l-value).");
			throw ParseError{};
		}

		// Validazione: l'operando non può essere un numero reale
		if (operand->type && operand->type->kind == TypeKind::TYPE_F40) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, op_tok.row, op_tok.col, "Prefix '" + op_str + "' cannot be applied to a real number.");
			throw ParseError{};
		}

		auto node = std::make_unique<Node>();
		node->kind = kind;
		node->lhs = std::move(operand);
		node->type = node->lhs->type; // Il tipo del risultato è lo stesso dell'operando
		return node;
	}

	if (match(eToken::T_PLUS))
	{
		const Token& op_tok = (*m_tokens)[m_current_pos - 1];
		m_dump_log.push_back("Matched unary '+'. Creating ND_POS node.");
		auto node = std::make_unique<Node>();
		node->kind = NodeKind::ND_POS;
		node->lhs = parse_unary();
		if (node->lhs->kind == NodeKind::ND_STRING_LITERAL) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::InvalidStringOperation, op_tok.row, op_tok.col, "Unary '+' cannot be applied to a string literal.");
			throw ParseError{};
		}
		node->type = node->lhs->type;
		return node;
	}

	if (match(eToken::T_MINUS))
	{
		const Token& op_tok = (*m_tokens)[m_current_pos - 1];
		m_dump_log.push_back("Matched unary '-'.");
		auto operand = parse_unary();

		// --- Constant Folding per la negazione (Interi) ---
		if (operand->kind == NodeKind::ND_INTEGER_CONSTANT) {
			m_dump_log.push_back(" -> Attempting to fold unary integer negation.");
			int64_t val = static_cast<int64_t>(operand->val);
			int64_t result = -val;

			std::shared_ptr<Type> result_type;
			if (result >= -128 && result <= 127) {
				result_type = m_symbol_table.find_type("s8");
			} else if (result >= -32768 && result <= 32767) {
				result_type = m_symbol_table.find_type("s16");
			} else {
				ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::ConstantOutOfRange, op_tok.row, op_tok.col, std::format("Value {} is out of range for any signed 16-bit integer.", result));
				throw ParseError{};
			}

			auto new_node = std::make_unique<Node>();
			new_node->kind = NodeKind::ND_INTEGER_CONSTANT;
			new_node->val = static_cast<uint64_t>(result);
			new_node->type = result_type;

			m_dump_log.push_back(std::format(" -> Folded to constant {} with inferred type '{}'.", result, type_to_string_for_parser(result_type)));
			return new_node;
		}

		// --- Constant Folding per la negazione (Float) ---
		if (operand->kind == NodeKind::ND_REAL_CONSTANT) {
			m_dump_log.push_back(" -> Attempting to fold unary real negation.");
			double val = operand->r_val;
			double result = -val;

			auto new_node = std::make_unique<Node>();
			new_node->kind = NodeKind::ND_REAL_CONSTANT;
			new_node->r_val = result;
			new_node->type = m_symbol_table.find_type("f40");

			m_dump_log.push_back(std::format(" -> Folded to real constant {}.", result));
			return new_node;
		}

		// --- Logica per operandi non costanti ---
		if (operand->kind == NodeKind::ND_STRING_LITERAL) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::InvalidStringOperation, op_tok.row, op_tok.col, "Unary '-' cannot be applied to a string literal.");
			throw ParseError{};
		}

		auto node = std::make_unique<Node>();
		node->kind = NodeKind::ND_NEG;
		node->lhs = std::move(operand);
		node->type = node->lhs->type; // Il tipo non cambia (la promozione a signed avviene in codegen)
		return node;
	}

	if (match(eToken::T_NOT))
	{
		const Token& op_tok = (*m_tokens)[m_current_pos - 1];
		m_dump_log.push_back("Matched unary '!'. Creating ND_NOT node.");
		auto node = std::make_unique<Node>();
		node->kind = NodeKind::ND_NOT;
		node->lhs = parse_unary();
		if (node->lhs->kind == NodeKind::ND_STRING_LITERAL) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::InvalidStringOperation, op_tok.row, op_tok.col, "Unary '!' cannot be applied to a string literal.");
			throw ParseError{};
		}
		node->type = node->lhs->type;
		return node;
	}

	if (match(eToken::T_BIT_NOT)) {
        const Token& op_tok = (*m_tokens)[m_current_pos - 1];
        m_dump_log.push_back("Matched unary bitwise NOT ('~' or '°').");
        auto operand = parse_unary();

        if (!is_integer_type(operand->type)) {
            ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::BitwiseOperationOnFloat, op_tok.row, op_tok.col, "Bitwise NOT can only be applied to integer types.");
            throw ParseError{};
        }

        if (operand->kind == NodeKind::ND_INTEGER_CONSTANT) {
            m_dump_log.push_back(" -> Attempting to fold unary bitwise NOT.");
            uint64_t val = operand->val;
            uint64_t result = ~val;

            if (operand->type->size == SIZEOF_BYTE) {
                result &= 0xFF;
            } else if (operand->type->size == SIZEOF_WORD) {
                result &= 0xFFFF;
            }
            
            operand->val = result;
            m_dump_log.push_back(std::format(" -> Folded to constant {}.", result));
            return operand;
        }

        auto node = std::make_unique<Node>();
        node->kind = NodeKind::ND_BIT_NOT;
        node->lhs = std::move(operand);
        node->type = node->lhs->type;
        return node;
    }

	if (match(eToken::T_ADDRESS))
	{
		m_dump_log.push_back("Matched unary '@'. Creating ND_ADDR node.");
		auto node = std::make_unique<Node>();
		node->kind = NodeKind::ND_ADDR;
		node->lhs = parse_unary(); // Right-associativity

		// L'operatore @ non puC2 essere applicato a un letterale stringa (che C( giC  un indirizzo/valore)
		if (node->lhs->kind == NodeKind::ND_STRING_LITERAL) {
			const Token& prev_token = (*m_tokens)[m_current_pos - 1];
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::InvalidStringOperation, prev_token.row, prev_token.col, "Cannot take the address of a string literal.");
			throw ParseError{};
		}
		if (node->lhs->symbol && node->lhs->symbol->kind == SymbolKind::SYMBOL_FUNC) {
			const Token& prev_token = (*m_tokens)[m_current_pos - 1];
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::AddressOfFunction, prev_token.row, prev_token.col);
			throw ParseError{};
		}
		if (node->lhs->kind != NodeKind::ND_VAR) {
			const Token& prev_token = (*m_tokens)[m_current_pos - 1]; // Heuristic to get token position
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, prev_token.row, prev_token.col, "The operand of the '@' operator must be a variable (l-value).");
			throw ParseError{};
		}

		node->type = m_symbol_table.get_pointer_type(node->lhs->type);
		m_dump_log.push_back(std::format(" -> Resulting type is '{}'.", type_to_string_for_parser(node->type)));
		return node;
	}

	if (match(eToken::T_POINTER))
	{
		m_dump_log.push_back("Matched unary '^'. Creating ND_DEREF node.");
		auto node = std::make_unique<Node>();
		node->kind = NodeKind::ND_DEREF;
		node->lhs = parse_unary(); // Right-associativity

		// C  possibile dereferenziare un letterale stringa, poichC) il suo tipo C( ^u8.
		// Il risultato C( il primo carattere.
		if (node->lhs->type->kind != TypeKind::TYPE_POINTER) {
			const Token& prev_token = (*m_tokens)[m_current_pos - 1]; // Heuristic
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, prev_token.row, prev_token.col, "The operand of the '^' (dereference) operator must be a pointer.");
			throw ParseError{};
		}

		// The result type is the base type of the pointer
		node->type = node->lhs->type->base;
		m_dump_log.push_back(std::format(" -> Resulting type is '{}'.", type_to_string_for_parser(node->type)));
		return node;
	}

	m_dump_log.push_back("No prefix unary operator found. Parsing <postfix>.");
	return parse_postfix();
}

//
// Analizza gli operatori postfissi.
// BNF: <postfix> ::= <term> ( "++" | "--" )*
//
std::unique_ptr<Node> Parser::parse_postfix()
{
	m_dump_log.push_back("Entering parse_postfix(). Rule: <postfix> ::= <term> ('++' | '--')*");
	auto node = parse_term();

	while (match(eToken::T_INCREMENT) || match(eToken::T_DECREMENT)) {
		const Token& op_tok = (*m_tokens)[m_current_pos - 1];
		NodeKind kind = (op_tok.type == eToken::T_INCREMENT) ? NodeKind::ND_POST_INC : NodeKind::ND_POST_DEC;
		std::string op_str = (kind == NodeKind::ND_POST_INC) ? "++" : "--";
		m_dump_log.push_back("Matched postfix '" + op_str + "'.");

		// Validazione: l'operando deve essere un l-value
		if (node->kind != NodeKind::ND_VAR && node->kind != NodeKind::ND_DEREF) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, op_tok.row, op_tok.col, "Operand for postfix '" + op_str + "' must be a variable or a pointer dereference (l-value).");
			throw ParseError{};
		}

		// Validazione: l'operando non può essere un numero reale
		if (node->type && node->type->kind == TypeKind::TYPE_F40) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, op_tok.row, op_tok.col, "Postfix '" + op_str + "' cannot be applied to a real number.");
			throw ParseError{};
		}

		auto op_node = std::make_unique<Node>();
		op_node->kind = kind;
		op_node->lhs = std::move(node);
		op_node->type = op_node->lhs->type; // Il tipo del risultato è lo stesso dell'operando
		node = std::move(op_node);
	}

	return node;
}

//
// Analizza un termine (numero, variabile, costante, chiamata di funzione, espressione tra parentesi).
// BNF: <term> ::= T_INTEGER | T_REAL | T_STRING | <func_call> | T_IDENTIFIER | "(" <expr> ")"
//      <func_call> ::= (T_IDENTIFIER "::")? T_IDENTIFIER "(" <arg_list> ")"
//      <arg_list>  ::= (<expr> ("," <expr>)*)?
//
std::unique_ptr<Node> Parser::parse_term()
{
	m_dump_log.push_back("Entering parse_term(). Rule: <term> ::= T_INTEGER | T_REAL | T_STRING | <func_call> | T_IDENTIFIER | '(' <expr> ')'");

	if (match(eToken::T_P0))
	{
		m_dump_log.push_back("Matched '('. Parsing sub-expression via <expr>.");
		auto node = parse_expr();
		m_dump_log.push_back("Parsed sub-expression. Expecting ')'.");
		expect(eToken::T_P1);
		m_dump_log.push_back("Matched ')'.");
		return node;
	}

	if (peek().type == eToken::T_IDENTIFIER)
	{
		// --- Logica unificata per identificatori ---
		const Token& first_tok = peek();
		std::string name_part1 = std::get<std::string>(first_tok.value);
		std::string final_name;
		const Token* error_tok = &first_tok;

		// Controlla se è un nome qualificato (es. ns::func)
		if (peek_next().type == eToken::T_SCOPE_RES) {
			consume(); // Consuma 'ns'
			consume(); // Consuma '::'
			const Token& name_part2_tok = expect(eToken::T_IDENTIFIER);
			std::string name_part2 = std::get<std::string>(name_part2_tok.value);
			final_name = name_part1 + "." + name_part2;
			error_tok = &name_part2_tok;
		} else {
			final_name = name_part1;
			consume(); // Consuma l'identificatore
		}

		std::shared_ptr<Symbol> symbol = m_symbol_table.find_symbol(final_name);
		if (!symbol) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::IdentifierNotFound, error_tok->row, error_tok->col, "Undeclared identifier '" + final_name + "'");
			throw ParseError{};
		}

		// --- NUOVA LOGICA: Gestione delle costanti ---
		if (symbol->kind == SymbolKind::SYMBOL_CONST) {
			m_dump_log.push_back("Identifier '" + final_name + "' is a constant. Replacing with its literal value.");
			auto node = std::make_unique<Node>();
			node->type = symbol->type;
			// MODIFICA: Memorizza il simbolo originale per il debugging del codegen
			node->original_symbol = symbol;
			if (std::holds_alternative<uint64_t>(symbol->const_value)) {
				node->kind = NodeKind::ND_INTEGER_CONSTANT;
				node->val = std::get<uint64_t>(symbol->const_value);
			} else if (std::holds_alternative<double>(symbol->const_value)) {
				node->kind = NodeKind::ND_REAL_CONSTANT;
				node->r_val = std::get<double>(symbol->const_value);
			} else if (std::holds_alternative<std::string>(symbol->const_value)) {
				node->kind = NodeKind::ND_STRING_LITERAL;
				node->str_val = std::get<std::string>(symbol->const_value);
			}
			return node;
		}

		// Ora, in base al tipo di simbolo e al token successivo, decidiamo cosa fare
		if (symbol->kind == SymbolKind::SYMBOL_FUNC) {
			if (match(eToken::T_P0)) { // È una chiamata di funzione
				m_dump_log.push_back("Identifier '" + final_name + "' is a function call.");
				auto node = std::make_unique<Node>();
				node->kind = NodeKind::ND_FUNCALL;
				node->symbol = symbol;
				node->type = symbol->type;

				m_dump_log.push_back("Parsing argument list for '" + final_name + "'.");
				if (peek().type != eToken::T_P1) {
					do {
						m_dump_log.push_back("Parsing an argument expression.");
						node->args.push_back(parse_expr());
					} while (match(eToken::T_COMMA));
				}
				expect(eToken::T_P1);

				if (node->args.size() != symbol->param_types.size()) {
					std::string err_msg = std::format("Expected {} arguments, but got {}.", symbol->param_types.size(), node->args.size());
					ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::WrongNumberOfArgs, error_tok->row, error_tok->col, err_msg);
					throw ParseError{};
				}

				m_dump_log.push_back("Performing type checking for arguments of '" + final_name + "'.");
				for (size_t i = 0; i < node->args.size(); ++i) {
					auto& arg_node = node->args[i];
					const auto& param_type = symbol->param_types[i];
					if (arg_node->type != param_type) {
						if (!m_symbol_table.is_castable(arg_node->type, param_type)) {
							std::string err_msg = std::format("Argument {} type mismatch: cannot convert '{}' to '{}'.", i + 1, type_to_string_for_parser(arg_node->type), type_to_string_for_parser(param_type));
							ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::TypeMismatchInArg, error_tok->row, error_tok->col, err_msg);
							throw ParseError{};
						}
						arg_node = create_cast_node(std::move(arg_node), param_type, m_dump_log);
					}
				}
				return node;
			} else { // È un nome di funzione usato come valore (indirizzo)
				m_dump_log.push_back(std::format("Matched function identifier '{}' used as an address value. Creating ND_VAR node.", final_name));
				auto node = std::make_unique<Node>();
				node->kind = NodeKind::ND_VAR;
				node->symbol = symbol;
				node->type = m_symbol_table.find_type("u16");
				return node;
			}
		}

		if (symbol->kind == SymbolKind::SYMBOL_VAR) {
			if (final_name != name_part1) { // Una variabile non può essere qualificata
				ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, first_tok.row, first_tok.col, "Variables cannot be qualified with '::'.");
				throw ParseError{};
			}
			m_dump_log.push_back(std::format("Matched T_IDENTIFIER '{}'. Creating ND_VAR node.", final_name));
			auto node = std::make_unique<Node>();
			node->kind = NodeKind::ND_VAR;
			node->symbol = symbol;
			node->type = symbol->type;
			return node;
		}

		// Qualsiasi altro tipo di simbolo (namespace, tipo) è un errore qui.
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, error_tok->row, error_tok->col, "'" + final_name + "' cannot be used as a value in an expression.");
		throw ParseError{};
	}

	if (peek().type == eToken::T_STRING)
	{
		const Token& string_token = consume();
		const std::string& value = std::get<std::string>(string_token.value);

		m_dump_log.push_back(std::format("Matched T_STRING. Value: \"{}\".", value));

		auto node = std::make_unique<Node>();
		node->kind = NodeKind::ND_STRING_LITERAL;
		node->str_val = value;

		// Il tipo di una stringa letterale C( puntatore a u8
		auto u8_type = m_symbol_table.find_type("u8");
		node->type = m_symbol_table.get_pointer_type(u8_type);

		return node;
	}

	if (peek().type == eToken::T_CHAR)
	{
		const Token& char_token = consume();
		char32_t value = std::get<char32_t>(char_token.value);

		m_dump_log.push_back(std::format("Matched T_CHAR. Value: '{}'.", (char)value));

		auto node = std::make_unique<Node>();
		node->kind = NodeKind::ND_INTEGER_CONSTANT;
		node->val = static_cast<uint64_t>(value);
		node->type = m_symbol_table.find_type("u8"); // I caratteri sono trattati come u8
		return node;
	}

	if (peek().type == eToken::T_REAL)
	{
		const Token& real_token = consume();
		double value = std::get<double>(real_token.value);
		const std::string& suffix = real_token.type_suffix;

		m_dump_log.push_back(std::format("Matched T_REAL. Value: {}.", value));

		auto node = std::make_unique<Node>();
		node->kind = NodeKind::ND_REAL_CONSTANT;
		node->r_val = value;

		std::shared_ptr<Type> assigned_type = m_symbol_table.find_type("f40");
		if (!suffix.empty() && suffix != "f40") {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Warning, Action::Parsing, ErrorMessage::UnsupportedTypeSuffix, real_token.row, real_token.col, "Type suffix '" + suffix + "' on real literal is ignored; defaulting to 'f40'.");
		}

		node->type = assigned_type;
		return node;
	}

	if (peek().type == eToken::T_INTEGER)
	{
		const Token& integer_token = consume();
		uint64_t value = std::get<uint64_t>(integer_token.value);
		const std::string& suffix = integer_token.type_suffix;

		auto node = std::make_unique<Node>();
		node->kind = NodeKind::ND_INTEGER_CONSTANT;
		node->val = value;

		std::shared_ptr<Type> assigned_type = nullptr;

		if (!suffix.empty()) {
			m_dump_log.push_back(std::format("Matched T_INTEGER with suffix '{}'. Value: {}.", suffix, value));

			if (suffix == "u32" || suffix == "s32" || suffix == "u64" || suffix == "s64") {
				ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnsupportedTypeSuffix, integer_token.row, integer_token.col, "Type suffix '" + suffix + "' is not yet supported.");
				throw ParseError{};
			}

			assigned_type = m_symbol_table.find_type(suffix);
			if (!assigned_type) {
				ErrorHandler::get().push_error(Sender::Parser, ErrorType::Internal, Action::Parsing, ErrorMessage::UnexpectedToken, integer_token.row, integer_token.col, "Unknown type suffix '" + suffix + "' passed from lexer.");
				throw ParseError{};
			}

			bool is_in_range = true;
			if (suffix == "u8" && value > 255) is_in_range = false;
			else if (suffix == "s8" && value > 127) is_in_range = false;
			else if (suffix == "u16" && value > 65535) is_in_range = false;
			else if (suffix == "s16" && value > 32767) is_in_range = false;

			if (!is_in_range) {
				ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::ConstantOutOfRange, integer_token.row, integer_token.col, std::format("Value {} is out of range for type '{}'.", value, suffix));
				throw ParseError{};
			}
		} else {
			m_dump_log.push_back(std::format("Matched T_INTEGER with no suffix. Value: {}. Inferring type.", value));
			if (value <= 255) {
				assigned_type = m_symbol_table.find_type("u8");
				m_dump_log.push_back("Inferred type: u8.");
			} else if (value <= 65535) {
				assigned_type = m_symbol_table.find_type("u16");
				m_dump_log.push_back("Inferred type: u16.");
			} else {
				ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::ConstantOutOfRange, integer_token.row, integer_token.col, std::format("Integer literal {} is too large for default types (u8, u16). Use a type suffix or a smaller value.", value));
				throw ParseError{};
			}
		}
		node->type = assigned_type;
		return node;
	}

	const Token& current_token = peek();
	m_dump_log.push_back(std::format("Error in parse_term(): Unexpected token."));
	ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, current_token.row, current_token.col, "Expected an integer, a real, a character, an identifier, or '('.");
	throw ParseError{};
}

// --- Funzioni Helper del Parser ---

const Token& Parser::peek() const
{
	return (*m_tokens)[m_current_pos];
}

const Token& Parser::peek_next() const
{
	if (m_current_pos + 1 < m_tokens->size()) {
		return (*m_tokens)[m_current_pos + 1];
	}
	return (*m_tokens)[m_tokens->size() - 1];
}

const Token& Parser::consume()
{
	if (m_current_pos < m_tokens->size())
	{
		return (*m_tokens)[m_current_pos++];
	}
	return (*m_tokens)[m_tokens->size() - 1];
}

bool Parser::match(eToken type)
{
	if (peek().type == type)
	{
		consume();
		return true;
	}
	return false;
}

bool Parser::match_keyword(const std::string& keyword)
{
	const auto& tok = peek();
	if (tok.type == eToken::T_KEYWORD && std::holds_alternative<std::string>(tok.value))
	{
		if (std::get<std::string>(tok.value) == keyword) {
			consume();
			return true;
		}
	}
	return false;
}

const Token& Parser::expect(eToken type)
{
	const auto& tok = peek();
	if (tok.type != type)
	{
		// Sopprime gli errori generici se siamo in modalitC  panico
		if (!m_in_panic_mode) {
			ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::UnexpectedToken, tok.row, tok.col, "Token inatteso.");
		}
		throw ParseError{};
	}
	return consume();
}

void Parser::dump_ast_to_file(const std::string& input_filepath) const
{
	std::string output_filename = input_filepath + ".parser.debug";
	std::ofstream out_file(output_filename);

	if (!out_file)
	{
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Coding, ErrorMessage::FileNotFound, 0, 0, "Could not open debug parser file: " + output_filename);
		return;
	}

	out_file << "--- Begin Parser Debug ---\n\n";
	out_file << "Parser execution log:\n\n";
	for (const auto& log_entry : m_dump_log)
	{
		out_file << "  - " << log_entry << "\n";
	}
	out_file << "\nGrammar implemented (with implicit casting):\n\n";
	out_file << "  <program>             ::= <top_level_statement>*\n";
	out_file << "  <top_level_statement> ::= <namespace> | <function> | <sys_function> | <declaration> | <const_declaration> | <asm_statement>\n";
	out_file << "  <namespace>           ::= 'ns' T_IDENTIFIER '{' (<function> | <sys_function>)* '}' ';'\n";
	out_file << "  <function>            ::= 'fn' T_IDENTIFIER '(' <param_list> ')' '->' <type_specifier> '{' <block> '}' ';'\n";
	out_file << "  <sys_function>        ::= 'sys' T_IDENTIFIER '(' <sys_param_list> ')' ('->' <type_specifier>)? '{' <block> '}' ';'\n";
	out_file << "  <param_list>          ::= (<param> (',' <param>)*)?\n";
	out_file << "  <param>               ::= T_IDENTIFIER ':' <type_specifier>\n";
	out_file << "  <sys_param_list>      ::= (<sys_param> (',' <sys_param>)*)?\n";
	out_file << "  <sys_param>           ::= <sys_type> ':' <sys_destination>\n";
	out_file << "  <sys_type>            ::= 'byte' | 'word' | 'real'\n";
	out_file << "  <sys_destination>     ::= 'a' | 'x' | 'y' | 'ay' | 'ax' | 'xy' | 'fac1' | 'fac2' | T_IDENTIFIER\n";
	out_file << "  <type_specifier>      ::= T_POINTER* T_IDENTIFIER\n";
	out_file << "  <block>               ::= <statement>*\n";
	out_file << "  <statement>           ::= <if_statement> | <declaration> | <const_declaration> | <asm_statement> | <expr_statement> | <return_statement> | <label_statement> | <jump_statement>\n";
	out_file << "  <return_statement>    ::= 'ret' <expr> ';'\n";
	out_file << "  <if_statement>        ::= 'if' '(' <expr> ')' '{' <block> '}' ('else' '{' <block> '}')? ';'\n";
	out_file << "  <declaration>         ::= T_IDENTIFIER ':' <type_specifier> ';'\n";
	out_file << "  <const_declaration>   ::= T_IDENTIFIER ':' <expr> ';'\n";
	out_file << "  <asm_statement>       ::= T_ASM_BLOCK ';'\n";
	out_file << "  <expr_statement>      ::= <expr> ';'\n";
	out_file << "  <expr>                ::= <assign>\n";
	out_file << "  <label_statement>     ::= T_IDENTIFIER ':' ';'\n";
	out_file << "  <jump_statement>      ::= 'jmp' T_IDENTIFIER ';'\n";
	out_file << "  <assign>              ::= <logical_or_and> (':=' <assign>)?\n";
	out_file << "  <logical_or_and>      ::= <bitwise_or> (('&&' | '||') <bitwise_or>)*\n";
	out_file << "  <bitwise_or>          ::= <bitwise_xor> ('|' <bitwise_xor>)*\n";
	out_file << "  <bitwise_xor>         ::= <bitwise_and> ('^' <bitwise_and>)*\n";
	out_file << "  <bitwise_and>         ::= <equality> ('&' <equality>)*\n";
	out_file << "  <equality>            ::= <relational> (('?=' | '!=') <relational>)*\n";
	out_file << "  <relational>          ::= <shift> (('<' | '<=' | '>' | '>=') <shift>)*\n";
	out_file << "  <shift>               ::= <add_sub> (('<<' | '>>') <add_sub>)*\n";
	out_file << "  <add_sub>             ::= <mul_div_mod> (('+' | '-') <mul_div_mod>)*\n";
	out_file << "  <mul_div_mod>         ::= <unary> (('*' | '/' | '%%') <unary>)*\n";
	out_file << "  <unary>               ::= ('++' | '--' | '+' | '-' | '!' | '@' | '^' | '~' | '°') <unary> | <postfix>\n";
	out_file << "  <postfix>             ::= <term> ('++' | '--')*\n";
	out_file << "  <term>                ::= T_INTEGER | T_REAL | T_STRING | <func_call> | T_IDENTIFIER | '(' <expr> ')'\n";
	out_file << "  <func_call>           ::= (T_IDENTIFIER '::')? T_IDENTIFIER '(' <arg_list> ')'\n";
	out_file << "  <arg_list>            ::= (<expr> (',' <expr>)*)?\n\n";


	out_file << "\n--- End Parser Debug ---\n";
}

void Parser::dump_symbol_table_to_file(const std::string& input_filepath) const
{
	if (!m_streamer) {
		return;
	}
	m_symbol_table.dump_to_file(input_filepath, *m_streamer);
}

// --- Implementazione delle Funzioni Helper Statiche ---

static std::string type_to_string_for_parser(const std::shared_ptr<Type>& type) {
	if (!type) return "unknown";
	if (type->kind == TypeKind::TYPE_POINTER) {
		return "^" + type_to_string_for_parser(type->base);
	}
	switch (type->kind) {
	case TypeKind::TYPE_U8:
		return "u8";
	case TypeKind::TYPE_S8:
		return "s8";
	case TypeKind::TYPE_U16:
		return "u16";
	case TypeKind::TYPE_S16:
		return "s16";
	case TypeKind::TYPE_F40:
		return "f40";
	default:
		return "other";
	}
}

static std::unique_ptr<Node> create_cast_node(std::unique_ptr<Node> node_to_cast, std::shared_ptr<Type> target_type, std::vector<std::string>& dump_log) {
	dump_log.push_back(std::format(" -> Inserting implicit cast from '{}' to '{}'.",
	                               type_to_string_for_parser(node_to_cast->type),
	                               type_to_string_for_parser(target_type)
	                              ));
	auto cast_node = std::make_unique<Node>();
	cast_node->kind = NodeKind::ND_CAST;
	cast_node->lhs = std::move(node_to_cast);
	cast_node->type = target_type;
	return cast_node;
}

static void apply_arithmetic_conversions(std::unique_ptr<Node>& lhs, std::unique_ptr<Node>& rhs, SymbolTable& symbol_table, std::vector<std::string>& dump_log) {
	if (!lhs->type || !rhs->type) return;
	if (lhs->type == rhs->type) return;

	// La logica dei puntatori C( ora gestita direttamente in parse_add_sub
	if (lhs->type->kind == TypeKind::TYPE_POINTER || rhs->type->kind == TypeKind::TYPE_POINTER) {
		return;
	}

	dump_log.push_back(std::format("Applying standard arithmetic conversions for types '{}' and '{}'.",
	                               type_to_string_for_parser(lhs->type),
	                               type_to_string_for_parser(rhs->type)
	                              ));

	std::shared_ptr<Type> common_type = symbol_table.get_common_type(lhs->type, rhs->type);
	if (!common_type) {
		return;
	}

	if (lhs->type != common_type) {
		lhs = create_cast_node(std::move(lhs), common_type, dump_log);
	}
	if (rhs->type != common_type) {
		rhs = create_cast_node(std::move(rhs), common_type, dump_log);
	}
}

static std::unique_ptr<Node> try_constant_folding(std::unique_ptr<Node>& lhs, std::unique_ptr<Node>& rhs, NodeKind op, SymbolTable& symbol_table, std::vector<std::string>& dump_log, const Token& op_tok) {
	bool is_lhs_const = (lhs->kind == NodeKind::ND_INTEGER_CONSTANT || lhs->kind == NodeKind::ND_REAL_CONSTANT);
	bool is_rhs_const = (rhs->kind == NodeKind::ND_INTEGER_CONSTANT || rhs->kind == NodeKind::ND_REAL_CONSTANT);

	if (!is_lhs_const || !is_rhs_const) {
		return nullptr;
	}

	dump_log.push_back("Attempting to fold constant expression.");

	bool is_real_op = (lhs->kind == NodeKind::ND_REAL_CONSTANT || rhs->kind == NodeKind::ND_REAL_CONSTANT);

	if (is_real_op) {
		dump_log.push_back(" -> Performing real or mixed-type folding. Promoting to f40.");

		double val1 = (lhs->kind == NodeKind::ND_REAL_CONSTANT)
		              ? lhs->r_val
		              : static_cast<double>(static_cast<int64_t>(lhs->val));
		double val2 = (rhs->kind == NodeKind::ND_REAL_CONSTANT)
		              ? rhs->r_val
		              : static_cast<double>(static_cast<int64_t>(rhs->val));

		switch (op) {
		case NodeKind::ND_ADD:
		case NodeKind::ND_SUB:
		case NodeKind::ND_MUL:
		case NodeKind::ND_DIV:
		case NodeKind::ND_MOD:
		{
			double result;
			if (op == NodeKind::ND_ADD) result = val1 + val2;
			else if (op == NodeKind::ND_SUB) result = val1 - val2;
			else if (op == NodeKind::ND_MUL) result = val1 * val2;
			else if (op == NodeKind::ND_DIV) {
				if (val2 == 0.0) {
					ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::DivisionByZero, op_tok.row, op_tok.col);
					throw Parser::ParseError{};
				}
				result = val1 / val2;
			} else { // MOD
				if (val2 == 0.0) {
					ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::DivisionByZero, op_tok.row, op_tok.col);
					throw Parser::ParseError{};
				}
				result = fmod(val1, val2);
			}
			auto new_node = std::make_unique<Node>();
			new_node->kind = NodeKind::ND_REAL_CONSTANT;
			new_node->r_val = result;
			new_node->type = symbol_table.find_type("f40");
			dump_log.push_back(std::format(" -> Folded to real constant {} with type 'f40'.", result));
			return new_node;
		}

		case NodeKind::ND_EQ:
		case NodeKind::ND_NE:
		case NodeKind::ND_LT:
		case NodeKind::ND_LE:
		case NodeKind::ND_GT:
		case NodeKind::ND_GE:
		{
			bool b_result;
			if (op == NodeKind::ND_EQ) b_result = (val1 == val2);
			else if (op == NodeKind::ND_NE) b_result = (val1 != val2);
			else if (op == NodeKind::ND_LT) b_result = (val1 < val2);
			else if (op == NodeKind::ND_LE) b_result = (val1 <= val2);
			else if (op == NodeKind::ND_GT) b_result = (val1 > val2);
			else b_result = (val1 >= val2); // GE

			auto new_node = std::make_unique<Node>();
			new_node->kind = NodeKind::ND_INTEGER_CONSTANT;
			new_node->val = b_result ? 1 : 0;
			new_node->type = symbol_table.find_type("u8");
			dump_log.push_back(std::format(" -> Folded real relational op to constant {} with type 'u8'.", new_node->val));
			return new_node;
		}
		default:
			return nullptr;
		}
	} else { // Operazione puramente intera
		dump_log.push_back(" -> Performing integer-only folding.");

		int64_t val1 = static_cast<int64_t>(lhs->val);
		int64_t val2 = static_cast<int64_t>(rhs->val);

		if (lhs->type->kind == TypeKind::TYPE_S8 && (val1 & 0x80)) val1 |= ~0xFFLL;
		if (lhs->type->kind == TypeKind::TYPE_S16 && (val1 & 0x8000)) val1 |= ~0xFFFFLL;
		if (rhs->type->kind == TypeKind::TYPE_S8 && (val2 & 0x80)) val2 |= ~0xFFLL;
		if (rhs->type->kind == TypeKind::TYPE_S16 && (val2 & 0x8000)) val2 |= ~0xFFFFLL;

		switch (op) {
		case NodeKind::ND_ADD:
		case NodeKind::ND_SUB:
		case NodeKind::ND_MUL:
		case NodeKind::ND_DIV:
		case NodeKind::ND_MOD:
		case NodeKind::ND_BIT_AND:
        case NodeKind::ND_BIT_OR:
        case NodeKind::ND_BIT_XOR:
        case NodeKind::ND_SHIFT_LEFT:
        case NodeKind::ND_SHIFT_RIGHT:
		{
			int64_t result;
			if (op == NodeKind::ND_ADD) result = val1 + val2;
			else if (op == NodeKind::ND_SUB) result = val1 - val2;
			else if (op == NodeKind::ND_MUL) result = val1 * val2;
			else if (op == NodeKind::ND_DIV) {
				if (val2 == 0) {
					ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::DivisionByZero, op_tok.row, op_tok.col);
					throw Parser::ParseError{};
				}
				result = val1 / val2;
			} else if (op == NodeKind::ND_MOD) {
				if (val2 == 0) {
					ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::DivisionByZero, op_tok.row, op_tok.col);
					throw Parser::ParseError{};
				}
				result = val1 % val2;
			} else if (op == NodeKind::ND_BIT_AND) result = val1 & val2;
            else if (op == NodeKind::ND_BIT_OR) result = val1 | val2;
            else if (op == NodeKind::ND_BIT_XOR) result = val1 ^ val2;
            else if (op == NodeKind::ND_SHIFT_LEFT) result = val1 << val2;
            else result = val1 >> val2; // SHIFT_RIGHT

			std::shared_ptr<Type> result_type;
			if (result >= 0) {
				if (result <= 255) result_type = symbol_table.find_type("u8");
				else if (result <= 65535) result_type = symbol_table.find_type("u16");
				else {
					ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::ConstantOutOfRange, op_tok.row, op_tok.col, std::format("Result {} of constant expression is out of range for any 16-bit unsigned integer.", result));
					throw Parser::ParseError{};
				}
			} else {
				if (result >= -128) result_type = symbol_table.find_type("s8");
				else if (result >= -32768) result_type = symbol_table.find_type("s16");
				else {
					ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Parsing, ErrorMessage::ConstantOutOfRange, op_tok.row, op_tok.col, std::format("Result {} of constant expression is out of range for any 16-bit signed integer.", result));
					throw Parser::ParseError{};
				}
			}
			auto new_node = std::make_unique<Node>();
			new_node->kind = NodeKind::ND_INTEGER_CONSTANT;
			new_node->val = static_cast<uint64_t>(result);
			new_node->type = result_type;
			dump_log.push_back(std::format(" -> Folded to integer constant {} with inferred type '{}'.", result, type_to_string_for_parser(result_type)));
			return new_node;
		}

		case NodeKind::ND_EQ:
		case NodeKind::ND_NE:
		case NodeKind::ND_LT:
		case NodeKind::ND_LE:
		case NodeKind::ND_GT:
		case NodeKind::ND_GE:
		{
			bool b_result;
			if (op == NodeKind::ND_EQ) b_result = (val1 == val2);
			else if (op == NodeKind::ND_NE) b_result = (val1 != val2);
			else if (op == NodeKind::ND_LT) b_result = (val1 < val2);
			else if (op == NodeKind::ND_LE) b_result = (val1 <= val2);
			else if (op == NodeKind::ND_GT) b_result = (val1 > val2);
			else b_result = (val1 >= val2); // GE

			auto new_node = std::make_unique<Node>();
			new_node->kind = NodeKind::ND_INTEGER_CONSTANT;
			new_node->val = b_result ? 1 : 0;
			new_node->type = symbol_table.find_type("u8");
			dump_log.push_back(std::format(" -> Folded integer relational op to constant {} with type 'u8'.", new_node->val));
			return new_node;
		}
		default:
			return nullptr;
		}
	}
}
