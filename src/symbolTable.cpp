
// *********
// symbolTable.cpp
// *********

#include "symbolTable.hpp"
#include "error.hpp" // Per ErrorHandler
#include <fstream>
#include <format>
#include <string>
#include <vector>
#include <algorithm>

// --- Funzioni Helper per la conversione in stringa ---

static std::string symbol_kind_to_string(const Symbol& sym) 
{
	if (sym.kind == SymbolKind::SYMBOL_FUNC) {
		return sym.is_syscall ? "FUNC (sys)" : "FUNC (fn)";
	}
	switch (sym.kind) {
		case SymbolKind::SYMBOL_VAR:  return "VAR";
		case SymbolKind::SYMBOL_CONST: return "CONST";
		case SymbolKind::SYMBOL_NAMESPACE: return "NS";
		case SymbolKind::SYMBOL_TYPE: return "TYPE";
		default:                      return "UNKNOWN";
	}
}

static std::string type_to_string(const Type& type) {
	switch (type.kind) {
		case TypeKind::TYPE_VOID:    return "void"; // Non dovrebbe apparire per le variabili
		case TypeKind::TYPE_U8:      return "u8";
		case TypeKind::TYPE_S8:      return "s8";
		case TypeKind::TYPE_U16:     return "u16";
		case TypeKind::TYPE_S16:     return "s16";
		case TypeKind::TYPE_F40:     return "f40";
		case TypeKind::TYPE_POINTER:
			if (type.base) {
				return "^" + type_to_string(*type.base);
			}
			return "^unknown";
		default:
			return "unknown_type";
	}
}

// --- Implementazione Scope ---

bool Scope::add_symbol(std::shared_ptr<Symbol> sym) {
	if (symbols.count(sym->name)) {
		return false; // Simbolo già esistente in questo scope
	}
	symbols[sym->name] = sym;
	return true;
}

std::shared_ptr<Symbol> Scope::find_symbol_in_current_scope(const std::string& name) {
	if (symbols.count(name)) {
		return symbols.at(name);
	}
	return nullptr;
}

// --- Implementazione SymbolTable ---

SymbolTable::SymbolTable() : m_next_scope_level(0) {
	m_global_scope = std::make_shared<Scope>(nullptr, m_next_scope_level++, "");
	m_current_scope = m_global_scope;
	m_all_scopes.push_back(m_global_scope); // Aggiungi il globale
	init_built_in_types();
}

void SymbolTable::init_built_in_types() 
{
	// Ranks: 3=f40, 2=word, 1=byte, 0=other
	
	add_type("u8",  std::make_shared<Type>(TypeKind::TYPE_U8,  SIZEOF_BYTE, 1));
	add_type("s8",  std::make_shared<Type>(TypeKind::TYPE_S8,  SIZEOF_BYTE, 1));
	add_type("u16", std::make_shared<Type>(TypeKind::TYPE_U16, SIZEOF_WORD, 2));
	add_type("s16", std::make_shared<Type>(TypeKind::TYPE_S16, SIZEOF_WORD, 2));
	add_type("f40", std::make_shared<Type>(TypeKind::TYPE_F40, SIZEOF_REAL, 3));
	add_type("void",std::make_shared<Type>(TypeKind::TYPE_VOID, 0, 0));
	
	// Tipi specifici per sys
	
	add_type("byte", find_type("u8"));
	add_type("word", find_type("u16"));
	add_type("real", find_type("f40"));
}

void SymbolTable::enter_scope(const std::string& owner_name) {
	auto new_scope = std::make_shared<Scope>(m_current_scope, m_next_scope_level++, owner_name);
	m_current_scope = new_scope;
	m_all_scopes.push_back(new_scope); // Traccia il nuovo scope
}

void SymbolTable::leave_scope() {
	if (m_current_scope->parent) {
		m_current_scope = m_current_scope->parent;
	}
}

bool SymbolTable::add_symbol(std::shared_ptr<Symbol> sym) {
	return m_current_scope->add_symbol(sym);
}

std::shared_ptr<Symbol> SymbolTable::find_symbol(const std::string& name) {
	std::shared_ptr<Scope> scope = m_current_scope;
	while (scope) {
		auto sym = scope->find_symbol_in_current_scope(name);
		if (sym) {
			return sym;
		}
		scope = scope->parent;
	}
	return nullptr;
}

bool SymbolTable::add_type(const std::string& name, std::shared_ptr<Type> type) {
	if (m_types.count(name)) {
		return false;
	}
	m_types[name] = type;
	return true;
}

std::shared_ptr<Type> SymbolTable::find_type(const std::string& name) {
	if (m_types.count(name)) {
		return m_types.at(name);
	}
	return nullptr;
}

std::shared_ptr<Type> SymbolTable::get_pointer_type(std::shared_ptr<Type> base_type) {
	// Controlla se un tipo puntatore a questo tipo base esiste già nella cache
	if (m_pointer_cache.count(base_type.get())) {
		return m_pointer_cache.at(base_type.get());
	}
	
	// Altrimenti, crea un nuovo tipo puntatore, lo aggiunge alla cache e lo restituisce
	// La dimensione di un puntatore è sempre SIZEOF_WORD (2 byte), rank 0
	auto ptr_type = std::make_shared<Type>(TypeKind::TYPE_POINTER, SIZEOF_WORD, 0, base_type);
	m_pointer_cache[base_type.get()] = ptr_type;
	return ptr_type;
}

std::shared_ptr<Type> SymbolTable::get_common_type(std::shared_ptr<Type> t1, std::shared_ptr<Type> t2) {
	if (!t1 || !t2) return nullptr;
	if (t1->rank > t2->rank) return t1;
	if (t2->rank > t1->rank) return t2;
	
	// Se i rank sono uguali (es. u16 e s16), preferisci signed su unsigned (convenzione comune)
	if (t1->kind == TypeKind::TYPE_S16 && t2->kind == TypeKind::TYPE_U16) return t1;
	if (t2->kind == TypeKind::TYPE_S16 && t1->kind == TypeKind::TYPE_U16) return t2;
	if (t1->kind == TypeKind::TYPE_S8 && t2->kind == TypeKind::TYPE_U8) return t1;
	if (t2->kind == TypeKind::TYPE_S8 && t1->kind == TypeKind::TYPE_U8) return t2;
	
	return t1; // Altrimenti, t1 è buono come t2
}

bool SymbolTable::is_castable(std::shared_ptr<Type> from, std::shared_ptr<Type> to) {
	if (!from || !to || from == to) {
		return false; // Non si casta a se stesso o da/a tipi nulli
	}
	// Secondo la tabella fornita, tutti i tipi aritmetici sono convertibili tra loro.
	// Escludiamo i puntatori e void da questa logica (rank > 0).
	return from->rank > 0 && to->rank > 0;
}

// --- Implementazione del Dump ---

// Stampa una singola riga per un simbolo.
void SymbolTable::dump_symbol(std::ofstream& out, const Symbol& sym, const Streamer& streamer) const {
	std::string location = "N/A";
	if (sym.token) {
		const auto& tok = *sym.token;
		std::string filename = streamer.get_filename_by_index(tok.file_idx);
		
		constexpr int max_filename_len = 34;
		if (filename.length() > max_filename_len) { 
			filename = "..." + filename.substr(filename.length() - (max_filename_len - 3));
		}
		location = std::format("{}:{:03}:{:03}", filename, tok.row, tok.col);
	}

	std::string type_str = sym.type ? type_to_string(*sym.type) : "N/A";
	
	// Aggiunge le dimensioni dell'array alla stringa del tipo per il debug
	if (sym.is_array()) {
		for (int dim : sym.array_dims) {
			type_str += std::format("[{}]", dim);
		}
	}

	std::string offset_str;
	if (sym.kind == SymbolKind::SYMBOL_VAR && !sym.is_global) {
		offset_str = std::to_string(sym.offset);
	} else if (sym.kind == SymbolKind::SYMBOL_FUNC) {
		offset_str = std::to_string(sym.stack_size);
	} else {
		offset_str = "-";
	}

	// Stampa la riga di riepilogo principale per il simbolo
	out << std::format(
		"| {:<20} | {:<10} | {:<16} | {:<5} | {:<6} | {:<6} | {:<42} |\n",
		sym.name,
		symbol_kind_to_string(sym),
		type_str,
		sym.scope_level,
		sym.is_global ? "Global" : "Local",
		offset_str,
		location
	);

	// Se il tipo è un puntatore, stampa la scomposizione "a scaletta"
	if (sym.type && sym.type->kind == TypeKind::TYPE_POINTER) {
		const std::string prefix = "                                     "; // Allinea l'output
		std::string indent = "  ";
		const Type* current_type = sym.type.get();
		while(current_type && current_type->kind == TypeKind::TYPE_POINTER) {
			out << prefix << indent << "puntatore ^ a " << "\n";
			current_type = current_type->base.get();
			indent += "  ";
		}
		if (current_type) {
			out << prefix << indent << type_to_string(*current_type) << "\n";
		}
	}
}

// Stampa tutti i simboli di un dato scope.
void SymbolTable::dump_scope_symbols(std::ofstream& out, const Scope& scope, const Streamer& streamer) const {
	for (const auto& pair : scope.symbols) {
		dump_symbol(out, *pair.second, streamer);
	}
}

void SymbolTable::dump_to_file(const std::string& input_filepath, const Streamer& streamer) const {
	std::string output_filename = input_filepath + ".symtable.debug";
	std::ofstream out_file(output_filename);

	if (!out_file) {
		ErrorHandler::get().push_error(Sender::Parser, ErrorType::Error, Action::Coding, ErrorMessage::FileNotFound, 0, 0, "Could not open debug symbol table file: " + output_filename);
		return;
	}

	const std::string line_separator(127, '-');
	const std::string entry_separator = "| " + std::string(123, '-') + " |";

	out_file << "--- Begin Symbol Table Dump ---\n\n";
	out_file << line_separator << "\n";
	out_file << std::format(
		"| {:<20} | {:<10} | {:<16} | {:<5} | {:<6} | {:<6} | {:<42} |\n",
		"Identifier", "Kind", "Type", "Scope", "Status", "Off/Sz", "Location"
	);
	out_file << line_separator << "\n";

	// 1. Separa i simboli globali in variabili e funzioni
	std::vector<std::shared_ptr<Symbol>> global_vars;
	std::vector<std::shared_ptr<Symbol>> global_funcs;
	if (!m_all_scopes.empty()) {
		const auto& global_scope = m_all_scopes[0];
		for (const auto& pair : global_scope->symbols) {
			if (pair.second->kind == SymbolKind::SYMBOL_FUNC) {
				global_funcs.push_back(pair.second);
			} else {
				global_vars.push_back(pair.second);
			}
		}
	}

	// 2. Stampa le variabili globali, ognuna seguita da un separatore
	for (const auto& sym : global_vars) {
		dump_symbol(out_file, *sym, streamer);
		out_file << entry_separator << "\n";
	}

	// 3. Stampa i blocchi funzione, ogni blocco seguito da un separatore
	for (const auto& func_sym : global_funcs) {
		// Stampa il simbolo della funzione stessa
		dump_symbol(out_file, *func_sym, streamer);

		// Trova e stampa i suoi simboli locali
		for (const auto& scope_ptr : m_all_scopes) {
			if (scope_ptr->owner_name == func_sym->name) {
				if (!scope_ptr->symbols.empty()) {
					dump_scope_symbols(out_file, *scope_ptr, streamer);
				}
				break; 
			}
		}
		
		// Stampa il separatore dopo l'intero blocco funzione
		out_file << entry_separator << "\n";
	}

	// Sostituisce l'ultimo separatore di voce con quello completo
	// per una chiusura pulita, se sono stati stampati simboli.
	if (!global_vars.empty() || !global_funcs.empty()) {
		out_file.seekp(-static_cast<std::streamoff>(entry_separator.length()) - 1, std::ios_base::cur);
	}
	
	out_file << line_separator << "\n\n";
	out_file << "--- End Symbol Table Dump ---\n";
}
