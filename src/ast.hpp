
// ********
// ast.hpp
// ********

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Forward declaration per evitare dipendenze circolari
class Symbol;
class Type;
struct Token;

// AST node kind, ispirato a chibicc.h
enum class NodeKind {
  ND_PROGRAM,            // Nodo radice per l'intero programma
  ND_NAMESPACE,          // Definizione di namespace
  ND_FUNCTION,           // Definizione di funzione 'fn'
  ND_SYS_FUNCTION,       // Definizione di funzione 'sys'
  ND_ADD,                // + 	(binary)
  ND_SUB,                // - 	(binary)
  ND_MUL,                // *
  ND_DIV,                // /
  ND_MOD,                // %%
  ND_NEG,                // - 	(unary)
  ND_EQ,                 // ?=
  ND_NE,                 // !=
  ND_LT,                 // <
  ND_LE,                 // <=
  ND_GT,                 // >
  ND_GE,                 // >=
  ND_LOGICAL_AND,        // &&
  ND_LOGICAL_OR,         // ||
  ND_POS,                // + 	(unary)
  ND_NOT,                // !   (unary)
  ND_ADDR,               // @ (get address)
  ND_DEREF,              // ^ (dereference)
  ND_PRE_INC,            // ++var (prefix increment)
  ND_PRE_DEC,            // --var (prefix decrement)
  ND_POST_INC,           // var++ (postfix increment)
  ND_POST_DEC,           // var-- (postfix decrement)
  ND_ASSIGN,             // :=
  ND_INTEGER_CONSTANT,   // Integer
  ND_REAL_CONSTANT,      // Real
  ND_STRING_LITERAL,     // "..."
  ND_ASM,                // "asm"
  ND_BLOCK,              // { ... }
  ND_EXPR_STMT,          // Expression statement
  ND_VAR_DECL,           // Dichiarazione di variabile
  ND_CONST_DECL,         // Dichiarazione di costante
  ND_VAR,                // Utilizzo di una variabile in un'espressione
  ND_FUNCALL,            // Chiamata di funzione
  ND_CAST,               // Cast di tipo implicito
  ND_RETURN,             // "ret" statement
  ND_IF,                 // "if" statement
  ND_BIT_AND,            // %&
  ND_BIT_OR,             // %|
  ND_BIT_XOR,            // %^
  ND_BIT_NOT,            // %~, %-
  ND_SHIFT_LEFT,         // <<
  ND_SHIFT_RIGHT,        // >>
  ND_LABEL,              // mylabel: ;
  ND_JUMP,               // jmp mylabel;
  ND_LOOP,               // loop statement
  ND_BREAK,              // break;
  ND_CONTINUE,           // continue;
};

// Struttura per i parametri di una funzione 'sys'
struct SysParam {
	std::shared_ptr<Type> type;
	std::string dest_name;
	bool is_register;
	const Token* token;
};

// AST node type, ispirato a chibicc.h
class Node
{
public:
  // Il distruttore virtuale è fondamentale per la corretta deallocazione
  // quando si usano puntatori alla classe base (come con unique_ptr).
  virtual ~Node() = default;

  NodeKind kind; // Tipo del nodo
  std::unique_ptr<Node> next; // Next node
  std::shared_ptr<Type> type; // Tipo semantico del nodo

  // Puntatori per gli operatori binari o per nodi "wrapper"
  // Per ND_IF: lhs=condizione, rhs=blocco else
  std::unique_ptr<Node> lhs; // Left-hand side
  std::unique_ptr<Node> rhs; // Right-hand side

  // Block o Program
  // Per ND_IF: body=blocco then
  // Per ND_LOOP: body=blocco loop
  std::unique_ptr<Node> body;

  // Usato per ND_LOOP (for init)
  std::unique_ptr<Node> init;

  // Usato per ND_LOOP
  bool is_post_check = false;

  // Usato se kind è ND_INTEGER_CONSTANT
  uint64_t val;

  // Usato se kind è ND_REAL_CONSTANT
  double r_val;

  // Usato se kind è ND_STRING_LITERAL, ND_LABEL, ND_JUMP
  std::string str_val;
  std::string label; // Usato anche da genCode per le stringhe

  // Usato se kind è ND_ASM
  std::string asm_str;

  // Usato se kind è ND_VAR_DECL, ND_CONST_DECL, ND_VAR, ND_FUNCTION o ND_FUNCALL
  std::shared_ptr<Symbol> symbol;

  // Usato per il constant folding per ricordare il nome della costante originale
  std::shared_ptr<Symbol> original_symbol;

  // Usato se kind è ND_FUNCTION
  int stack_size;
  std::vector<std::shared_ptr<Symbol>> locals_and_params;

  // Usato se kind è ND_SYS_FUNCTION
  std::vector<SysParam> sys_params;

  // Usato se kind è ND_FUNCALL
  std::vector<std::unique_ptr<Node>> args;

  // Usato per propagare le dimensioni degli array nelle espressioni (es. array multidimensionali)
  std::vector<int> array_dims;
};
