
// *********
// symbolTable.hpp
// *********

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include "lexer.hpp" 
#include "streamer.hpp" 

// Forward declaration

class Type;
class Symbol;
class Scope;

// --- Tipi di base e costanti ---

constexpr int SIZEOF_BYTE = 1;
constexpr int SIZEOF_WORD = 2;
constexpr int SIZEOF_REAL = 5;

enum class TypeKind {
	TYPE_VOID,
	TYPE_U8,
	TYPE_S8,
	TYPE_U16,
	TYPE_S16,
	TYPE_F40,
	TYPE_POINTER
};

enum class SymbolKind {
	SYMBOL_VAR,
	SYMBOL_CONST,
	SYMBOL_FUNC,
	SYMBOL_NAMESPACE,
	SYMBOL_TYPE
};

// --- Struttura per i Tipi ---
class Type {
public:
	TypeKind kind;
	int size;
	int rank; // Aggiunto per la promozione dei tipi (es. f40 > u16 > u8)
	std::shared_ptr<Type> base; // Usato per i puntatori (tipo base)

	Type(TypeKind k, int s, int r, std::shared_ptr<Type> b = nullptr)
		: kind(k), size(s), rank(r), base(b) {}
};

// --- Struttura per i Simboli (Variabili, Funzioni, etc.) ---
class Symbol {
public:
	std::string name;
	SymbolKind kind;
	std::shared_ptr<Type> type;
	int scope_level;
	bool is_global;
	int offset; // Per variabili locali/parametri in futuro
	int stack_size = 0; // Totale stack utilizzato dalla funzione (solo per SYMBOL_FUNC)
	const Token* token; // Per la posizione nel codice sorgente (error reporting)

	// Usato se kind è SYMBOL_CONST
	std::variant<std::monostate, uint64_t, double, std::string> const_value;

	// Usato se kind è SYMBOL_FUNC
	std::vector<std::shared_ptr<Type>> param_types;
	bool is_syscall = false; // Flag per distinguere 'fn' da 'sys'

	Symbol(const std::string& n, SymbolKind k, std::shared_ptr<Type> t, int level, bool global, const Token* tok = nullptr)
		: name(n), kind(k), type(t), scope_level(level), is_global(global), offset(0), token(tok) {}
};

// --- Struttura per lo Scope ---
class Scope {
public:
	std::shared_ptr<Scope> parent;
	std::map<std::string, std::shared_ptr<Symbol>> symbols;
	int level;
	std::string owner_name; // Nome della funzione che possiede questo scope

	Scope(std::shared_ptr<Scope> p, int l, const std::string& owner = "") : parent(p), level(l), owner_name(owner) {}

	bool add_symbol(std::shared_ptr<Symbol> sym);
	std::shared_ptr<Symbol> find_symbol_in_current_scope(const std::string& name);
};

// --- Classe principale della Symbol Table ---
class SymbolTable {
public:
	SymbolTable();

	void enter_scope(const std::string& owner_name);
	void leave_scope();

	bool add_symbol(std::shared_ptr<Symbol> sym);
	std::shared_ptr<Symbol> find_symbol(const std::string& name);

	bool add_type(const std::string& name, std::shared_ptr<Type> type);
	std::shared_ptr<Type> find_type(const std::string& name);
	
	std::shared_ptr<Type> get_pointer_type(std::shared_ptr<Type> base_type);

	// Funzioni helper per la gestione dei cast
	std::shared_ptr<Type> get_common_type(std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
	bool is_castable(std::shared_ptr<Type> from, std::shared_ptr<Type> to);

	std::shared_ptr<Scope> get_current_scope() const { return m_current_scope; }

	// Metodo per il dump su file
	void dump_to_file(const std::string& input_filepath, const Streamer& streamer) const;

private:
	void init_built_in_types();

	// Metodo helper per il dump
	void dump_symbol(std::ofstream& out, const Symbol& symbol, const Streamer& streamer) const;
	void dump_scope_symbols(std::ofstream& out, const Scope& scope, const Streamer& streamer) const;

	std::shared_ptr<Scope> m_global_scope;
	std::shared_ptr<Scope> m_current_scope;
	int m_next_scope_level;

	// Mantiene tutti gli scope creati per il dump
	std::vector<std::shared_ptr<Scope>> m_all_scopes;

	// Mappa globale per i tipi, per evitare duplicazioni
	std::map<std::string, std::shared_ptr<Type>> m_types;
	
	// Cache per i tipi puntatore, per evitare di creare più volte lo stesso tipo
	std::map<Type*, std::shared_ptr<Type>> m_pointer_cache;
};
