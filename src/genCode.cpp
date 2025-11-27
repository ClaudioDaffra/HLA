// ***********
// genCode.cpp
// ***********

#include "genCode.hpp"
#include "error.hpp"
#include "symbolTable.hpp"
#include "f40_converter.hpp"
#include "config.hpp"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <format>
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <memory>
#include <vector>
#include <iomanip>

// --- Funzioni Helper ---

// Helper per ottenere il suffisso del tipo come stringa (es. "u8", "s16")
static std::string get_type_suffix(const std::shared_ptr<Type>& type) 
{
	if (!type) return "u16"; // Fallback per sicurezza
	switch (type->kind) {
		case TypeKind::TYPE_U8:  return "u8";
		case TypeKind::TYPE_S8:  return "s8";
		case TypeKind::TYPE_U16: return "u16";
		case TypeKind::TYPE_S16: return "s16";
		case TypeKind::TYPE_F40: return "f40";
		case TypeKind::TYPE_POINTER: return "u16"; // I puntatori sono indirizzi a 16 bit
		default: return "u16"; // Fallback
	}
}

// Helper per ottenere la rappresentazione in stringa di un tipo per i nomi delle funzioni di cast
static std::string type_to_string_for_cast(const std::shared_ptr<Type>& type) {
	if (!type) return "unknown";
	switch (type->kind) {
		case TypeKind::TYPE_U8:      return "u8";
		case TypeKind::TYPE_S8:      return "s8";
		case TypeKind::TYPE_U16:     return "u16";
		case TypeKind::TYPE_S16:     return "s16";
		case TypeKind::TYPE_F40:     return "f40";
		case TypeKind::TYPE_POINTER: return "u16";
		default:                     return "unknown";
	}
}

// Helper per verificare se un NodeKind è un operatore relazionale
static bool is_relational_op(NodeKind kind) {
	switch (kind) {
		case NodeKind::ND_EQ:
		case NodeKind::ND_NE:
		case NodeKind::ND_LT:
		case NodeKind::ND_LE:
		case NodeKind::ND_GT:
		case NodeKind::ND_GE:
			return true;
		default:
			return false;
	}
}

// --- Metodi della classe CodeGenerator ---

// Gestisce l'ottenimento del valore di una variabile (globale o locale).
// Il valore viene caricato nel registro appropriato (a, ay) o in fac1.
void CodeGenerator::generate_variable_get(Node* node, std::ofstream& out)
{
	if (!node || !node->symbol) return;

	// --- NUOVA LOGICA: Ottenere l'indirizzo di una funzione ---
	if (node->symbol->kind == SymbolKind::SYMBOL_FUNC) {
		const std::string& func_name = node->symbol->name;
		out << "\t; Get address of function '" << func_name << "'\n";
		out << "    lda #<" << func_name << "\n";
		out << "    ldy #>" << func_name << "\n\n";
		return; // Fatto per questo caso
	}

	const auto& symbol = node->symbol;
	const std::string& var_name = symbol->name;

	if (symbol->is_global) 
	{
		switch (symbol->type->size) {
			case SIZEOF_BYTE:
				out << "    lda " << var_name << "\n";
				break;
			case SIZEOF_WORD:
				out << "    lda " << var_name << "+0\n";
				out << "    ldy " << var_name << "+1\n";
				break;
			case SIZEOF_REAL:
				out << "\t; Get global real '" << var_name << "' into fac1\n";
				out << "    lda #<" << var_name << "\n";
				out << "    ldy #>" << var_name << "\n";
				out << "    jsr basic.load5_fac1 " << "\n";
				break;
			default:
				out << "\t; TODO get global var '" << var_name << "' with unsupported size.\n";
				break;
		}
	} 
	else 
	{ // Variabile locale
		switch (symbol->type->kind) {
			case TypeKind::TYPE_U8:
			case TypeKind::TYPE_S8:
				out << "    .fstack_byte_get " << var_name << "\n";
				break;
			case TypeKind::TYPE_U16:
			case TypeKind::TYPE_S16:
			case TypeKind::TYPE_POINTER:
				out << "    .fstack_word_get " << var_name << "\n";
				break;
			case TypeKind::TYPE_F40:
				out << "    .fstack_fac1_get " << var_name << "\n";
				break;
			default:
				out << "\t; TODO get local var '" << var_name << "' with unsupported type.\n";
				break;
		}
	}
	out << "\n";
}

// Gestisce l'assegnazione di un valore a una variabile (globale o locale) o a un puntatore dereferenziato.
void CodeGenerator::generate_variable_set(Node* node, std::ofstream& out)
{
	// node è il nodo ND_ASSIGN
	if (!node || !node->lhs) {
		ErrorHandler::get().push_error(Sender::Emitter, ErrorType::Internal, Action::Coding, ErrorMessage::UnexpectedToken, 0, 0, "Invalid lvalue in assignment generation.");
		return;
	}

	// 1. Genera il codice per il RHS, che lascia il valore nel registro/i corretto/i
	generate_node(node->rhs.get(), out);

	Node* lvalue = node->lhs.get();

	if (lvalue->kind == NodeKind::ND_VAR) {
		// --- Logica esistente per l'assegnazione di variabili ---
		const auto& symbol = lvalue->symbol;
		const std::string& var_name = symbol->name;

		if (symbol->is_global) {
			switch (symbol->type->size) {
				case SIZEOF_BYTE:
					out << "    sta " << var_name << "\n";
					break;
				case SIZEOF_WORD:
					out << "    sta " << var_name << "+0\n";
					out << "    sty " << var_name << "+1\n";
					break;
				case SIZEOF_REAL:
					out << "\t; Set global real '" << var_name << "' from fac1\n";
					out << "    lda #<" << var_name << "\n";
					out << "    ldy #>" << var_name << "\n";
					out << "    jsr basic.store_fac1\n";
					break;
				default:
					out << "\t; TODO set global var '" << var_name << "' with unsupported size.\n";
					break;
			}
		} else { // Variabile locale
			switch (symbol->type->kind) {
				case TypeKind::TYPE_U8:
				case TypeKind::TYPE_S8:
					out << "    .fstack_byte_set " << var_name << "\n";
					break;
				case TypeKind::TYPE_U16:
				case TypeKind::TYPE_S16:
				case TypeKind::TYPE_POINTER:
					out << "    .fstack_word_set " << var_name << "\n";
					break;
				case TypeKind::TYPE_F40:
					out << "    .fstack_fac1_set " << var_name << "\n";
					break;
				default:
					out << "\t; TODO set local var '" << var_name << "' with unsupported type.\n";
					break;
			}
		}
	} else if (lvalue->kind == NodeKind::ND_DEREF) {
		// --- Nuova logica per l'assegnazione di puntatori dereferenziati ---
		out << "\t; Dereference pointer (set value)\n";
		
		// 2. Il valore da memorizzare è già in a/ay/fac1. Dobbiamo salvarlo.
		auto value_type = node->rhs->type;
		if (!value_type) {
			out << "\t; ERRORE: Impossibile determinare il tipo di valore da memorizzare tramite puntatore.\n";
			return;
		}

		switch (value_type->kind) {
			case TypeKind::TYPE_U8:
			case TypeKind::TYPE_S8:
				out << "    pha\n\n";
				break;
			case TypeKind::TYPE_U16:
			case TypeKind::TYPE_S16:
			case TypeKind::TYPE_POINTER:
				out << "    pha\n";
				out << "    tya\n";
				out << "    pha\n\n";
				break;
			case TypeKind::TYPE_F40:
				out << "    jsr fstack.push.fac1_c64\n\n";
				break;
			default:
				out << "\t; TODO: Tipo non supportato per la memorizzazione del puntatore (push).\n\n";
				break;
		}

		// 3. Ora, genera il codice per l'espressione del puntatore stessa (es. 'p' in '^p := ...')
		// Questo metterà il valore del puntatore nella coppia di registri AY.
		generate_node(lvalue->lhs.get(), out);

		// 4. Il puntatore è in AY. Ne abbiamo bisogno in zpWord0 per le routine di memorizzazione.
		out << "    sta zpWord0+0\n";
		out << "    sty zpWord0+1\n\n";

		// 5. Ora, ripristina il valore da memorizzare dallo stack nel/i registro/i corretto/i.
		switch (value_type->kind) {
			case TypeKind::TYPE_U8:
			case TypeKind::TYPE_S8:
				out << "    pla\n";
				out << "    .mem_store_byte_pointer\n";
				break;
			case TypeKind::TYPE_U16:
			case TypeKind::TYPE_S16:
			case TypeKind::TYPE_POINTER:
				out << "    pla\n";
				out << "    tay\n";
				out << "    pla\n";
				out << "    .mem_store_word_pointer\n";
				break;
			case TypeKind::TYPE_F40:
				out << "    jsr fstack.pop.fac1_c64\n";
				out << "    .mem_store_real_pointer\n";
				break;
			default:
				out << "\t; TODO: Tipo non supportato per la memorizzazione del puntatore (pop e call).\n";
				break;
		}
	} else {
		ErrorHandler::get().push_error(Sender::Emitter, ErrorType::Internal, Action::Coding, ErrorMessage::UnexpectedToken, 0, 0, "Invalid lvalue kind in assignment generation.");
	}
	out << "\n";
}

// Gestisce i nodi di dichiarazione di variabile.
void CodeGenerator::generate_var_decl(Node* node, std::ofstream& out)
{
	if (!node || !node->symbol) return;

	const auto& symbol = node->symbol;
	if (!symbol->is_global) return;

	out << std::left << std::setw(10) << symbol->name << " ";

	int size = (symbol->type->kind == TypeKind::TYPE_POINTER) ? SIZEOF_WORD : symbol->type->size;

	switch (size) {
		case SIZEOF_BYTE:
			out << "\t.byte 0\n";
			break;
		case SIZEOF_WORD:
			out << "\t.word 0\n";
			break;
		case SIZEOF_REAL:
			out << "\t.byte 0,0,0,0,0\n";
			break;
		default:
			out << "\t; Errore: dimensione tipo non supportata: " << size << "\n";
			break;
	}
}

// Gestisce i nodi di operazione binaria
void CodeGenerator::generate_binary_op(Node* node, std::ofstream& out)
{
	// --- Caso speciale per la sottrazione di puntatori (p1 - p2) ---
	if (node->kind == NodeKind::ND_SUB && 
		node->lhs->type && node->lhs->type->kind == TypeKind::TYPE_POINTER &&
		node->rhs->type && node->rhs->type->kind == TypeKind::TYPE_POINTER)
	{
		out << "\t; Pointer subtraction (p1 - p2)\n";
		generate_node(node->lhs.get(), out);
		out << "    pha\n";
		out << "    tya\n";
		out << "    pha\n\n";
		generate_node(node->rhs.get(), out);
		out << "    sty zpWord0+1\n";
		out << "    sta zpWord0+0\n";
		out << "    pla\n";
		out << "    tay\n";
		out << "    pla\n\n";
		out << "    jsr math.sub_u16\n\n";
		return;
	}

	auto type = node->type;
	if (!type) {
		type = node->lhs->type;
	}

	if (type->kind == TypeKind::TYPE_F40) {
		generate_node(node->lhs.get(), out);
		
		out << "    jsr fstack.push.fac1_c64\n\n";
		
		generate_node(node->rhs.get(), out);
		
		out << "    jsr fstack.push.fac1_c64\n\n";
		
		out << "    jsr fstack.pop.fac1_c64\n\n";
		out << "    jsr basic.store_fac1_in_fac2_round\n\n";
	
		out << "    jsr fstack.pop.fac1_c64\n\n";

		out << "\t; Performing real operation\n";
		switch (node->kind) {
			case NodeKind::ND_ADD: out << "    jsr math.add_f40\n"; break;
			case NodeKind::ND_SUB: out << "    jsr math.sub_f40\n"; break;
			case NodeKind::ND_MUL: out << "    jsr math.mul_f40\n"; break;
			case NodeKind::ND_DIV: out << "    jsr math.div_f40\n"; break;
			case NodeKind::ND_MOD: out << "    jsr math.mod_f40\n"; break;
			case NodeKind::ND_LT:  out << "    jsr math.f40_cmp_lt\n"; break;
			case NodeKind::ND_LE:  out << "    jsr math.f40_cmp_le\n"; break;
			case NodeKind::ND_GT:  out << "    jsr math.f40_cmp_gt\n"; break;
			case NodeKind::ND_GE:  out << "    jsr math.f40_cmp_ge\n"; break;
			case NodeKind::ND_EQ:  out << "    jsr math.f40_cmp_eq\n"; break;
			case NodeKind::ND_NE:  out << "    jsr math.f40_cmp_ne\n"; break;
			default: break;
		}
		out << "\n";
		return;
	}

	generate_node(node->lhs.get(), out);

	if (type->size == SIZEOF_BYTE) {
		out << "    pha\n\n";
	} else {
		out << "    pha\n";
		out << "    tya\n";
		out << "    pha\n\n";
	}

	generate_node(node->rhs.get(), out);

	if (type->size == SIZEOF_BYTE) {
		out << "    sta zpWord0+0\n";
		out << "    pla\n\n";
	} else {
		out << "    sty zpWord0+1\n";
		out << "    sta zpWord0+0\n";
		out << "    pla\n";
		out << "    tay\n";
		out << "    pla\n\n";
	}

	std::string suffix = get_type_suffix(type);
	switch (node->kind)
	{
		case NodeKind::ND_ADD:
			if (type->size == SIZEOF_BYTE) {
				out << "    clc\n";
				out << "    adc zpWord0+0\n";
			} else {
				out << "    jsr math.add_" << suffix << "\n";
			}
			break;
		case NodeKind::ND_SUB:
			if (type->size == SIZEOF_BYTE) {
				out << "    sec\n";
				out << "    sbc zpWord0+0\n";
			} else {
				out << "    jsr math.sub_" << suffix << "\n";
			}
			break;
		case NodeKind::ND_MUL:
			if ( ( suffix=="u8" ) || ( suffix=="s8" ) ) out << "    ldy zpWord0+0\n";
			out << "    jsr math.mul_" << suffix << "\n";
			break;
		case NodeKind::ND_DIV:
			if ( ( suffix=="u8" ) || ( suffix=="s8" ) ) out << "    ldy zpWord0+0\n";
			out << "    jsr math.div_" << suffix << "\n";
			break;
		case NodeKind::ND_MOD:
			if ( ( suffix=="u8" ) || ( suffix=="s8" ) ) out << "    ldy zpWord0+0\n";
			out << "    jsr math.mod_" << suffix << "\n";
			break;
		case NodeKind::ND_EQ:
			out << "    jsr math." << suffix << "_cmp_eq\n";
			break;
		case NodeKind::ND_NE:
			out << "    jsr math." << suffix << "_cmp_ne\n";
			break;
		case NodeKind::ND_LT:
			out << "    jsr math." << suffix << "_cmp_lt\n";
			break;
		case NodeKind::ND_LE:
			out << "    jsr math." << suffix << "_cmp_le\n";
			break;
		case NodeKind::ND_GT:
			out << "    jsr math." << suffix << "_cmp_gt\n";
			break;
		case NodeKind::ND_GE:
			out << "    jsr math." << suffix << "_cmp_ge\n";
			break;
		case NodeKind::ND_LOGICAL_AND:
			if (node->lhs->type->size == SIZEOF_BYTE) {
				out << "    jsr math.logical_and_u8s8\n";
			} else {
				out << "    jsr math.logical_and_u16s16\n";
			}
			break;
		case NodeKind::ND_LOGICAL_OR:
			if (node->lhs->type->size == SIZEOF_BYTE) {
				out << "    jsr math.logical_or_u8s8\n";
			} else {
				out << "    jsr math.logical_or_u16s16\n";
			}
			break;
		case NodeKind::ND_BIT_AND:
            out << "    .math_bit_and_" << (type->size == SIZEOF_BYTE ? "byte" : "word") << "\n";
            break;
        case NodeKind::ND_BIT_OR:
            out << "    .math_bit_or_" << (type->size == SIZEOF_BYTE ? "byte" : "word") << "\n";
            break;
        case NodeKind::ND_BIT_XOR:
            out << "    .math_bit_xor_" << (type->size == SIZEOF_BYTE ? "byte" : "word") << "\n";
            break;
        case NodeKind::ND_SHIFT_LEFT:
            out << "    .math_shl_" << suffix << "\n";
            break;
        case NodeKind::ND_SHIFT_RIGHT:
            out << "    .math_shr_" << suffix << "\n";
            break;
		default:
			break;
	}
	out << "\n";
}

// Gestisce i nodi di operazione unaria
void CodeGenerator::generate_unary_op(Node* node, std::ofstream& out)
{
	generate_node(node->lhs.get(), out);

	auto type = node->lhs->type;
	if (!type) {
		out << "\t; ERRORE: tipo sconosciuto per operazione unaria\n";
		return;
	}

	if (type->kind == TypeKind::TYPE_F40) {
		switch (node->kind) {
			case NodeKind::ND_POS:
				out << "\t; Unary + (no-op for real)\n";
				break;
			case NodeKind::ND_NEG:
				out << "    .math_neg_f40\n";
				break;
			case NodeKind::ND_NOT:
				out << "    jsr math.not_f40\n";
				break;
			default: break;
		}
		out << "\n";
		return;
	}

	std::string suffix = get_type_suffix(type);
	switch (node->kind)
	{
		case NodeKind::ND_POS:
			out << "\t; Unary + (no-op for integer)\n";
			break;
		case NodeKind::ND_NEG:
			out << "    jsr math.neg_" << suffix << "\n";
			break;
		case NodeKind::ND_NOT:
			out << "    jsr math.not_" << suffix << "\n";
			break;
		case NodeKind::ND_BIT_NOT:
            out << "    .math_bit_neg_" << (type->size == SIZEOF_BYTE ? "byte" : "word") << "\n";
            break;
		default:
			break;
	}
	out << "\n";
}

// Gestisce i nodi di incremento/decremento prefisso e postfisso.
void CodeGenerator::generate_inc_dec(Node* node, std::ofstream& out)
{
	Node* operand = node->lhs.get();
	if (!operand) return;

	bool is_prefix = (node->kind == NodeKind::ND_PRE_INC || node->kind == NodeKind::ND_PRE_DEC);
	bool is_inc = (node->kind == NodeKind::ND_PRE_INC || node->kind == NodeKind::ND_POST_INC);
	std::string op_str = is_inc ? "++" : "--";
	std::string fix_str = is_prefix ? "prefix" : "postfix";

	out << "\t; --- " << fix_str << " " << op_str << " ---\n";

	// 1. Ottiene il valore dell'operando in A/AY
	generate_node(operand, out);

	// 2. Per il postfisso, salva il valore originale (il valore di ritorno)
	if (!is_prefix) 
	{
		if (operand->type->size == SIZEOF_BYTE) {
			out << "    pha\n";
		} 
		else 
		{	// Word o Pointer
			out << "    tax	;	save    AY\n";	// occorre salvare i valori di A
			out << "    pha\n";	// qui i valori vengono spinti nello stack, AY POSTFIX
			out << "    tya\n"; // ovviamente qui A cambia ma è salvato in X
			out << "    pha\n"; // qui viene messo Y nello stack
			out << "    txa	;	restore AY\n"; // q qui ripristinati i valori dato che Y non è cambiato
		}
		out << "\n";
	}

	// 3. Esegue l'operazione di incremento/decremento sul valore in A/AY
	if (operand->type->kind == TypeKind::TYPE_POINTER) {
		int size = operand->type->base ? operand->type->base->size : 1;
		if (is_inc) {
			out << "    .inc_ptr_ay " << size << "\n";
		} else {
			out << "    .dec_ptr_ay " << size << "\n";
		}
	} else if (operand->type->size == SIZEOF_BYTE) {
		if (is_inc) {
			out << "    .inc_byte_a\n";
		} else {
			out << "    .dec_byte_a\n";
		}
	} else { // Word
		if (is_inc) {
			out << "    .inc_word_ay\n";
		} else {
			out << "    .dec_word_ay\n";
		}
	}
	out << "\n";

	// 4. Memorizza il valore modificato.
	Node* lvalue = operand;
	if (lvalue->kind == NodeKind::ND_VAR) {
		const auto& symbol = lvalue->symbol;
		const std::string& var_name = symbol->name;
		out << "\t; Store result back to variable '" << var_name << "'\n";
		if (symbol->is_global) {
			if (symbol->type->size == SIZEOF_BYTE) {
				out << "    sta " << var_name << "\n";
			} else { // Word o Pointer
				out << "    sta " << var_name << "+0\n";
				out << "    sty " << var_name << "+1\n";
			}
		} else { // Locale
			if (symbol->type->size == SIZEOF_BYTE) {
				out << "    .fstack_byte_set " << var_name << "\n";
			} else { // Word o Pointer
				out << "    .fstack_word_set " << var_name << "\n";
			}
		}
	} else if (lvalue->kind == NodeKind::ND_DEREF) {
		out << "\t; Store result back to dereferenced pointer\n";
		// Salva il valore modificato
		if (lvalue->type->size == SIZEOF_BYTE) {
			out << "    pha\n\n";
		} else {
			out << "    pha\n";
			out << "    tya\n";
			out << "    pha\n\n";
		}
		// Valuta l'espressione del puntatore per ottenere l'indirizzo in AY
		generate_node(lvalue->lhs.get(), out);
		// Memorizza l'indirizzo in zp
		out << "    sta zpWord0+0\n";
		out << "    sty zpWord0+1\n\n";
		// Ripristina il valore modificato
		if (lvalue->type->size == SIZEOF_BYTE) {
			out << "    pla\n";
			out << "    .mem_store_byte_pointer\n";
		} else {
			out << "    pla\n";
			out << "    tay\n";
			out << "    pla\n";
			out << "    .mem_store_word_pointer\n";
		}
	}
	out << "\n";

	// 5. Per il postfisso, ripristina il valore originale in A/AY come risultato dell'espressione.
	// Per il prefisso, il risultato è già in A/AY.
	if (!is_prefix) {
		out << "\t; Restore original value for postfix result\n";
		if (operand->type->size == SIZEOF_BYTE) {
			out << "    pla\n";
		} else { // Word o Pointer
			out << "    pla\n";
			out << "    tay\n";
			out << "    pla\n";
		}
	}
	out << "\t; --- end " << fix_str << " " << op_str << " ---\n\n";
}

// Gestisce la generazione del codice per uno statement 'if'.
void CodeGenerator::generate_if_statement(Node* node, std::ofstream& out)
{
	// --- OTTIMIZZAZIONE: Constant Folding ---
	Node* condition = node->lhs.get();
	if (condition->kind == NodeKind::ND_INTEGER_CONSTANT) {
		if (condition->val != 0) {
			// La condizione è una costante vera, genera solo il blocco 'then'.
			out << "\t; OPTIMIZATION: 'if' condition is a non-zero constant. Generating 'then' block only.\n\n";
			for (Node* stmt = node->body.get(); stmt; stmt = stmt->next.get()) {
				generate_node(stmt, out);
			}
			out << "\t; OPTIMIZATION: End of folded 'if'.\n\n";
		} else {
			// La condizione è una costante falsa, genera solo il blocco 'else'.
			if (node->rhs) {
				out << "\t; OPTIMIZATION: 'if' condition is zero. Generating 'else' block only.\n\n";
				for (Node* stmt = node->rhs.get(); stmt; stmt = stmt->next.get()) {
					generate_node(stmt, out);
				}
				out << "\t; OPTIMIZATION: End of folded 'if-else'.\n\n";
			} else {
				out << "\t; OPTIMIZATION: 'if' condition is zero and no 'else' block. Omitting code.\n\n";
			}
		}
		return; // Salta la generazione del codice standard per l'if
	}
	// --- Fine Ottimizzazione ---

	// Ottiene un ID univoco per questo statement if
	int label_id = m_if_label_counter++;
	
	std::string if_label_gen = std::format("{:03}", label_id);
	
	std::string endif_label = std::format(" endif{:03}", label_id);
	std::string else_label  = std::format(" else{:03}", label_id);

	out << "\t; if {" << if_label_gen << "}\n";

	// 1. Genera il codice per la condizione
	generate_node(condition, out);

	// 2. Determina quale istruzione di salto usare
	if (is_relational_op(condition->kind)) { 
		// Caso 1: Operatore relazionale. Il risultato è 0 (falso) o 1 (vero) in A.
		// BEQ salta se A è 0 (flag Z impostato), che significa che la condizione è falsa.
		out << "\t; relational condition\n";
		if (node->rhs) { // if-else
			out << "    cmp #0 " << "\n";
			out << "    beq " << else_label << "\n\n";
		} else { // if-then
			out << "    cmp #0 " << "\n";
			out << "    beq " << endif_label << "\n\n";
		}
	} else {
		// Caso 2: Espressione generica.
		// Dobbiamo chiamare una routine 'not' per convertire il risultato in un booleano.
		out << "\t; expression condition\n";
		std::string suffix = get_type_suffix(condition->type);
		out << "    jsr math.not_" << suffix << "\n";
		// BNE salta se A non è 0 (flag Z non impostato).
		// Dopo math.not_*, A è 1 se l'originale era 0 (falso).
		// Quindi, BNE salta se la condizione originale era falsa.
		if (node->rhs) { // if-else
			out << "    cmp #0 " << "\n";
			out << "    bne " << else_label << "\n\n";
		} else { // if-then
			out << "    cmp #0 " << "\n";
			out << "    bne " << endif_label << "\n\n";
		}
	}

	// 3. Genera il codice per il blocco 'then'
	out << "\t; then {"<< if_label_gen << "}\n";
	for (Node* stmt = node->body.get(); stmt; stmt = stmt->next.get()) {
		generate_node(stmt, out);
	}

	// 4. Gestisce la parte 'else'
	if (node->rhs) {
		out << "    jmp " << endif_label << "\n";
		out << else_label << ":\n";
		out << "\t; else {"<< if_label_gen << "}\n\n";
		for (Node* stmt = node->rhs.get(); stmt; stmt = stmt->next.get()) {
			generate_node(stmt, out);
		}
	}

	// 5. Genera l'etichetta finale endif
	out << endif_label << ":\n";
	out << "\t; end if {"<< if_label_gen << "}\n\n";
}

void CodeGenerator::generate_loop_statement(Node* node, std::ofstream& out)
{
	// 1. Generazione ID univoci (Incrementale)
	int label_id = m_loop_label_counter++;
	
	// Creiamo le stringhe delle etichette
	std::string start_label = std::format("loop_start_{:03}", label_id);
	std::string step_label  = std::format("loop_step_{:03}", label_id); 
	std::string end_label   = std::format("loop_end_{:03}", label_id);

	out << "\t; loop construction {" << label_id << "}\n";

	// 2. PUSH nello Stack (Gestione Scope/Nesting)
	m_loop_stack.push_back({start_label, step_label, end_label});

	// --- Init (solo per For) ---
	if (node->init) {
		out << "\t; loop init\n";
		generate_node(node->init.get(), out);
	}

	// --- Start Label ---
	out << start_label << ":\n";

	// --- Pre-Check Condition ---
	if (!node->is_post_check && node->lhs) {
		Node* condition = node->lhs.get();
		
		// --- OTTIMIZZAZIONE: Constant Folding ---
		if (condition->kind == NodeKind::ND_INTEGER_CONSTANT) {
			if (condition->val == 0) {
				out << "\t; OPTIMIZATION: Loop condition is constant 0 (false). Skipping body.\n";
				out << "\t jmp " << end_label << "\n\n";
			}
			// Se è val != 0 (es. while(1)), non generiamo codice di controllo.
		} 
		else {
			out << "\t; loop pre-condition check\n";
			generate_node(condition, out);

			if (is_relational_op(condition->kind)) {
				// Relational: A=0 (False), A=1 (True). BEQ salta se False.
				out << "    cmp #0 " << "\n";				
				out << "    beq " << end_label << "\n";
			} else {
				// Expression: Usiamo math.not per invertire logica
				std::string suffix = get_type_suffix(condition->type);
				out << "    jsr math.not_" << suffix << "\n";
				// BNE salta se A!=0 (quindi originale Falso).
				out << "    cmp #0 " << "\n";	
				out << "    bne " << end_label << "\n";
			}
			out << "\n";
		}
	}

	// --- Body ---
	out << "\t; loop body\n";
	for (Node* stmt = node->body.get(); stmt; stmt = stmt->next.get()) {
		generate_node(stmt, out);
	}

	// --- Step Label ---
	// Qui atterra il 'continue'
	out << step_label << ":\n";

	// --- Step Expression (solo per For) ---
	if (node->rhs) {
		out << "\t; loop step\n";
		generate_node(node->rhs.get(), out);
	}

	// --- Post-Check Condition / Loop Back ---
	if (node->is_post_check && node->lhs) {
		// do-while logic
		Node* condition = node->lhs.get();
		out << "\t; loop post-condition check\n";
		generate_node(condition, out);

		// Qui la logica è inversa: Saltiamo a start se è VERO.
		if (is_relational_op(condition->kind)) {
			// A=1 (True). BNE salta a start se True (Z=0).
			out << "    cmp #0\n"; 
			out << "    bne " << start_label << "\n";
		} else {
			// Expression generica. Se A!=0 (True), ripeti.
			out << "    cmp #0\n"; 
			out << "    bne " << start_label << "\n";
		}
	} 
	else {
		// Salto incondizionato (per while, for, loop infinito)
		out << "    jmp " << start_label << "\n";
	}

	// --- End Label ---
	// Qui atterra il 'break'
	out << end_label << ":\n";
	out << "\t; end loop {" << label_id << "}\n\n";

	// 3. POP dallo Stack
	m_loop_stack.pop_back();
}

void CodeGenerator::generate_break_statement(Node* node, std::ofstream& out)
{
	if (m_loop_stack.empty()) {
		ErrorHandler::get().push_error(Sender::Emitter, ErrorType::Error, Action::Coding, ErrorMessage::UnexpectedToken, 0, 0, "'break' statement not inside a loop.");
		return;
	}

	const auto& current_loop = m_loop_stack.back();
	
	out << "\t; break\n";
	out << "    jmp " << current_loop.end_label << "\n\n";
}

void CodeGenerator::generate_continue_statement(Node* node, std::ofstream& out)
{
	if (m_loop_stack.empty()) {
		ErrorHandler::get().push_error(Sender::Emitter, ErrorType::Error, Action::Coding, ErrorMessage::UnexpectedToken, 0, 0, "'continue' statement not inside a loop.");
		return;
	}

	const auto& current_loop = m_loop_stack.back();

	out << "\t; continue\n";
	out << "    jmp " << current_loop.step_label << "\n\n";
}


// Funzione ricorsiva per attraversare l'AST e generare codice per un singolo nodo.
void CodeGenerator::generate_node(Node* node, std::ofstream& out)
{
	if (!node) return;

	switch (node->kind)
	{
		case NodeKind::ND_VAR_DECL:
			break;

		case NodeKind::ND_CONST_DECL:
			if (node->symbol && node->symbol->is_global) {
				GlobalConstantInfo info;
				info.name = node->symbol->name;

				if (std::holds_alternative<uint64_t>(node->symbol->const_value)) {
					uint64_t val = std::get<uint64_t>(node->symbol->const_value);
					info.value = std::to_string(val);
					if (node->symbol->type->size == SIZEOF_BYTE) {
						info.directive = ".byte";
					} else {
						info.directive = ".word";
					}
				} else if (std::holds_alternative<double>(node->symbol->const_value)) {
					double val = std::get<double>(node->symbol->const_value);
					auto bytes = F40Converter::convertDoubleToMBF_bytes(val);
					info.directive = ".byte";
					info.value = std::format("${:02X},${:02X},${:02X},${:02X},${:02X}",
											 bytes[0], bytes[1], bytes[2], bytes[3], bytes[4]);
					
					// Aggiungi commento formattato per i reali
					std::stringstream comment_ss;
					comment_ss << "; " << std::right << std::setw(12) << std::fixed << std::setprecision(5) << val;
					
					comment_ss << " \t[";
					for (size_t i = 0; i < bytes.size(); ++i) {
						comment_ss << std::setw(3) << static_cast<int>(bytes[i]) << (i == bytes.size() - 1 ? "" : " ");
					}
					comment_ss << "]";
					info.comment = comment_ss.str();

				} else if (std::holds_alternative<std::string>(node->symbol->const_value)) {
					info.directive = ".null";
					info.value = "\"" + std::get<std::string>(node->symbol->const_value) + "\"";
				}
				m_global_constants.push_back(info);
			}
			break;

		case NodeKind::ND_VAR:
			generate_variable_get(node, out);
			break;

		case NodeKind::ND_INTEGER_CONSTANT:
			if (!node->type) {
				out << "    lda #<" << node->val << "\n";
				out << "    ldy #>" << node->val << "\n\n";
				break;
			}
			switch (node->type->kind) {
				case TypeKind::TYPE_U8:
					out << "    lda #" << node->val << "\n";
					break;
				case TypeKind::TYPE_S8:
					// Cast a int per forzare la stampa come numero, non come carattere
					out << "    lda #" << static_cast<int>(static_cast<int8_t>(node->val)) << "\n";
					break;
				case TypeKind::TYPE_U16:
				case TypeKind::TYPE_POINTER:
					out << "    lda #<" << node->val << "\n";
					out << "    ldy #>" << node->val << "\n";
					break;
				case TypeKind::TYPE_S16:
					// Cast a int per forzare la stampa come numero
					out << "    lda #<" << static_cast<int>(static_cast<int16_t>(node->val)) << "\n";
					out << "    ldy #>" << static_cast<int>(static_cast<int16_t>(node->val)) << "\n";
					break;
				default:
					out << "\t; TODO: Unsupported integer constant type in codegen.\n";
					break;
			}
			out << "\n";
			break;

		case NodeKind::ND_REAL_CONSTANT:
		{
			double val = node->r_val;
			
			// Controlla se esiste già una costante globale con questo valore
			if (m_global_real_constant_values.count(val)) {
				std::string global_const_name = m_global_real_constant_values.at(val);
				std::string full_global_name = "kConst." + global_const_name;
				
				std::string original_name_comment;
				if (node->original_symbol && node->original_symbol->name != global_const_name) {
					original_name_comment = " ( original " + node->original_symbol->name + " )";
				}

				out << "\t; Load global real constant '" << global_const_name << "' (" << val << ") into fac1" << original_name_comment << "\n";
				
				out << "    lda #<" << full_global_name;
				if (node->original_symbol && node->original_symbol->name != global_const_name) {
					out << "\t;\t<-\t#<" << node->original_symbol->name;
				}
				out << "\n";

				out << "    ldy #>" << full_global_name;
				if (node->original_symbol && node->original_symbol->name != global_const_name) {
					out << "\t;\t<-\t#>" << node->original_symbol->name;
				}
				out << "\n";

				out << "    jsr basic.load5_fac1\n\n";
				break;
			}

			RealConstantInfo info;
			auto it = m_real_constants.find(val);
			if (it != m_real_constants.end()) {
				info = it->second;
			} else {
				info.label = std::format("Real{:03}", m_real_constant_counter++);
				
				auto bytes = F40Converter::convertDoubleToMBF_bytes(val);
				info.byte_representation = std::format("${:02X},${:02X},${:02X},${:02X},${:02X}",
													   bytes[0], bytes[1], bytes[2], bytes[3], bytes[4]);

				std::stringstream ss_orig;
				ss_orig << std::setprecision(15) << val;
				info.original_value = ss_orig.str();

				std::stringstream ss_dec;
				ss_dec << "[" << std::setw(3) << static_cast<int>(bytes[0]);
				for (size_t i = 1; i < bytes.size(); ++i) {
					ss_dec << " " << std::setw(3) << static_cast<int>(bytes[i]);
				}
				ss_dec << "]";
				info.decimal_bytes = ss_dec.str();

				m_real_constants[val] = info;
			}

			std::string full_label = "kConst." + info.label;
			out << "\t; Load real constant " << info.original_value << " into fac1\n";
			out << "    lda #<" << full_label << "\n";
			out << "    ldy #>" << full_label << "\n";
			out << "    jsr basic.load5_fac1\n\n";
			break;
		}

		case NodeKind::ND_STRING_LITERAL:
		{
			// Controlla se esiste già una costante globale con questo valore
			if (m_global_string_constant_values.count(node->str_val)) {
				std::string global_const_name = m_global_string_constant_values.at(node->str_val);
				std::string full_global_name = "kConst." + global_const_name;

				std::string original_name_comment;
				if (node->original_symbol && node->original_symbol->name != global_const_name) {
					original_name_comment = " ( original " + node->original_symbol->name + " )";
				}

				out << "\t; Load address of global string constant '" << global_const_name << "'" << original_name_comment << "\n";
				
				out << "    lda #<" << full_global_name;
				if (node->original_symbol && node->original_symbol->name != global_const_name) {
					out << "\t;\t<-\t#<" << node->original_symbol->name;
				}
				out << "\n";

				out << "    ldy #>" << full_global_name;
				if (node->original_symbol && node->original_symbol->name != global_const_name) {
					out << "\t;\t<-\t#>" << node->original_symbol->name;
				}
				out << "\n\n";
				break;
			}

			// L'etichetta viene generata una volta e memorizzata nel nodo
			if (node->label.empty()) {
				node->label = std::format("String{:03}", m_string_constant_counter++);
				m_string_constants.push_back({node->label, "\"" + node->str_val + "\""});
			}

			std::string full_label = "kConst." + node->label;
			out << "\t; Load address of string literal\n";
			out << "    lda #<" << full_label << "\n";
			out << "    ldy #>" << full_label << "\n\n";
			break;
		}

		case NodeKind::ND_ASM:
		{
			std::string content = node->asm_str;
			std::stringstream ss(content);
			std::string line;
			
			while (std::getline(ss, line))
			{
				size_t first = line.find_first_not_of(" \t\n");
				if (std::string::npos == first) {
					continue;
				}
				size_t last = line.find_last_not_of(" \t");
				std::string trimmed_line = line.substr(first, (last - first + 1));
				
				if (!trimmed_line.empty()) {
					 out << "    " << trimmed_line << "\n";
				}
			}
			out << "\n";
			break;
		}

		case NodeKind::ND_EXPR_STMT:
			generate_node(node->lhs.get(), out);
			break;

		case NodeKind::ND_ASSIGN:
			generate_variable_set(node, out);
			break;

		case NodeKind::ND_FUNCALL:
			if (node->symbol) 
			{
				if (node->symbol->is_syscall)
					out << "\t; --- SysbCall to " << node->symbol->name << " ---\n\n";
				else
					out << "\t; --- Function to " << node->symbol->name << " ---\n\n";
				
				if (!node->args.empty()) 
				{
					if (node->symbol->is_syscall) 
					{
						out << "\t; --- BEGIN SYS param push ---\n\n";
						// Per le sys call, il push è in ordine inverso rispetto alle function e usa le macro _fast
						for (int i = static_cast<int>(node->args.size()) - 1; i >= 0; i--)
						{
							const auto& arg = node->args[i];
							generate_node(arg.get(), out);
							
							if (!arg->type) {
								out << "\t; ERROR: Argument has no type info.\n";
								continue;
							}
							switch (arg->type->kind) {
								case TypeKind::TYPE_U8:
								case TypeKind::TYPE_S8:
									out << "    .fstack_push_byte_fast\n\n";
									break;
								case TypeKind::TYPE_U16:
								case TypeKind::TYPE_S16:
								case TypeKind::TYPE_POINTER:
									out << "    .fstack_push_word_fast\n\n";
									break;
								case TypeKind::TYPE_F40:
									out << "    .fstack_push_real_fast\n\n";
									break;
								default:
									out << "\t; TODO: Unsupported sys argument type for push.\n\n";
									break;
							}
						}
						out << "\t; --- END SYS param push ---\n\n";
					} 
					else 
					{
						out << "\t; --- BEGIN FN param push ---\n\n";
						out << "\t.fstack_push_sp\n\n";

						for (const auto& arg : node->args) {
							generate_node(arg.get(), out);
							
							if (!arg->type) {
								out << "\t; ERROR: Argument has no type info.\n";
								continue;
							}

							switch (arg->type->kind) {
								case TypeKind::TYPE_U8:
								case TypeKind::TYPE_S8:
									out << "    jsr fstack.push.byte\n\n";
									break;
								case TypeKind::TYPE_U16:
								case TypeKind::TYPE_S16:
								case TypeKind::TYPE_POINTER:
									out << "    jsr fstack.push.word\n\n";
									break;
								case TypeKind::TYPE_F40:
									out << "    jsr fstack.push.real\n\n";
									break;
								default:
									out << "\t; TODO: Unsupported fn argument type for push.\n\n";
									break;
							}
						}
						out << "\t.fstack_pop_sp\n\n";
						out << "\t; --- END FN param push ---\n\n";
					}
				}

				out << "    jsr " << node->symbol->name << "\n";
				out << "\t\n";
			}
			break;

		case NodeKind::ND_CAST:
		{
			generate_node(node->lhs.get(), out);

			auto from_type = node->lhs->type;
			auto to_type = node->type;

			if (from_type && to_type && from_type != to_type) {
				std::string from_str = type_to_string_for_cast(from_type);
				std::string to_str = type_to_string_for_cast(to_type);
				std::string cast_func_name = std::format(".cast_from_{}_to_{}", from_str, to_str);
				
				out << "    " << cast_func_name << "\n\n";
			}
			break;
		}

		case NodeKind::ND_RETURN:
			if (node->lhs) {
				out << "\t; return\n";
				generate_node(node->lhs.get(), out);
			}
			break;

		case NodeKind::ND_IF:
			generate_if_statement(node, out);
			break;

		case NodeKind::ND_LOOP:
			generate_loop_statement(node, out);
			break;

		case NodeKind::ND_BREAK:
			generate_break_statement(node, out);
			break;

		case NodeKind::ND_CONTINUE:
			generate_continue_statement(node, out);
			break;

		case NodeKind::ND_LABEL:
			out << node->str_val << ":\n\n";
			break;

		case NodeKind::ND_JUMP:
			out << "    jmp " << node->str_val << "\n\n";
			break;

		case NodeKind::ND_ADDR:
		{
			if (!node->lhs || node->lhs->kind != NodeKind::ND_VAR || !node->lhs->symbol) {
				ErrorHandler::get().push_error(Sender::Emitter, ErrorType::Internal, Action::Coding, ErrorMessage::UnexpectedToken, 0, 0, "Invalid operand for address-of operator generation.");
				break;
			}
			const auto& symbol = node->lhs->symbol;
			const std::string& var_name = symbol->name;

			if (symbol->is_global) {
				out << "\t; get address Global " << var_name << " @\n";
				out << "    lda #<" << var_name << "\n";
				out << "    ldy #>" << var_name << "\n\n";
			} else { // Variabile locale
				out << "\t; get address Local " << var_name << " @\n";
				out << "    lda #" << var_name << "\n";
				out << "    jsr fstack.lea_bp\n\n";
			}
			break;
		}

		case NodeKind::ND_DEREF:
		{
			// Questo è per ottenere il valore, es. x := ^p
			out << "\t; Dereference pointer (get value)\n";
			// Per prima cosa, genera il codice per ottenere il valore del puntatore in AY
			generate_node(node->lhs.get(), out);

			// Il tipo di risultato della dereferenziazione è il tipo base del puntatore
			auto base_type = node->type;
			if (!base_type) {
				out << "\t; ERRORE: Impossibile dereferenziare il puntatore con tipo base sconosciuto.\n";
				break;
			}

			switch (base_type->kind) {
				case TypeKind::TYPE_U8:
				case TypeKind::TYPE_S8:
					out << "    .mem_load_byte_pointer\n\n";
					break;
				case TypeKind::TYPE_U16:
				case TypeKind::TYPE_S16:
				case TypeKind::TYPE_POINTER: // Dereferenziazione di un puntatore a un puntatore
					out << "    .mem_load_word_pointer\n\n";
					break;
				case TypeKind::TYPE_F40:
					out << "    .mem_load_real_pointer\n\n";
					break;
				default:
					out << "\t; TODO: Dereferenziazione del puntatore non supportata per l'operazione get.\n\n";
					break;
			}
			break;
		}

		case NodeKind::ND_PRE_INC:
		case NodeKind::ND_PRE_DEC:
		case NodeKind::ND_POST_INC:
		case NodeKind::ND_POST_DEC:
			generate_inc_dec(node, out);
			break;

		case NodeKind::ND_POS:
		case NodeKind::ND_NEG:
		case NodeKind::ND_NOT:
		case NodeKind::ND_BIT_NOT:
			generate_unary_op(node, out);
			break;

		case NodeKind::ND_ADD:
		case NodeKind::ND_SUB:
		case NodeKind::ND_MUL:
		case NodeKind::ND_DIV:
		case NodeKind::ND_MOD:
		case NodeKind::ND_EQ:
		case NodeKind::ND_NE:
		case NodeKind::ND_LT:
		case NodeKind::ND_LE:
		case NodeKind::ND_GT:
		case NodeKind::ND_GE:
		case NodeKind::ND_LOGICAL_AND:
		case NodeKind::ND_LOGICAL_OR:
		case NodeKind::ND_BIT_AND:
        case NodeKind::ND_BIT_OR:
        case NodeKind::ND_BIT_XOR:
        case NodeKind::ND_SHIFT_LEFT:
        case NodeKind::ND_SHIFT_RIGHT:
			generate_binary_op(node, out);
			break;

		default:
			ErrorHandler::get().push_error(Sender::Emitter, ErrorType::Internal, Action::Coding, ErrorMessage::UnexpectedToken, 0, 0, "Unsupported AST node type in code generation.");
			break;
	}
}

// --- Funzioni per la Gestione delle Funzioni ---

void CodeGenerator::generate_function_frame(Node* node, std::ofstream& out)
{
	if (!node || node->kind != NodeKind::ND_FUNCTION || !node->symbol || node->stack_size == 0) return;

	const std::string& func_name = node->symbol->name;
	
	//const std::string struct_name = "fstack_local_" + func_name; XXX
	// questo name space rimane uguale per tutte le funzioni
	const std::string struct_name = "fstack_local";
	
	constexpr int ALIGN_WIDTH = 15;

	out << "\t; --- Stack Frame for function " << func_name << " ---\n";
	
	out << "\t" << struct_name << " .struct\n";
	for (const auto& sym : node->locals_and_params) {
		std::string padding = " ";
		if (sym->name.length() < ALIGN_WIDTH) {
			padding = std::string(ALIGN_WIDTH - sym->name.length(), ' ');
		}

		out << "\t\t" << sym->name << padding;

		switch (sym->type->size) {
			case SIZEOF_BYTE: out << ".byte 0\n"; break;
			case SIZEOF_WORD: out << ".word 0\n"; break;
			case SIZEOF_REAL: out << ".byte 0,0,0,0,0\n"; break;
			default: out << "; unsupported size " << sym->type->size << "\n"; break;
		}
	}
	out << "\t.endstruct\n\n";

	out << "\t.cerror(size(" << struct_name << ") > 255)\t; check fstack size\n\n";

	out << "\t.weak\t; offset names\n";
	for (const auto& sym : node->locals_and_params) {
		std::string padding = " ";
		if (sym->name.length() < ALIGN_WIDTH) {
			padding = std::string(ALIGN_WIDTH - sym->name.length(), ' ');
		}
		out << "\t\t" << sym->name << padding << "= (size(" << struct_name << ")-(" << struct_name << "." << sym->name << "))\n";
	}
	out << "\t.endweak\n\n";
}

void CodeGenerator::generate_function_prologue(Node* node, std::ofstream& out)
{
	// dato che usiamo lo stesso nome per lo stack locale non ci serve più uno nome diverso
	// const std::string& func_name = node->symbol->name;
	out << "\t; salva bp e sp\n";
	out << "\t.fstack_push_sp_bp\n\n";

	if (node->stack_size > 0) {
		out << "\t; alloca le dimensioni dello stack\n";
		
		//out << "\tlda #size(fstack_local_" << func_name << ")\n"; XXX
		// qui mettiamo solo fstack_local , così evitiamo anche di dover aggiungere codice per il namespace
		out << "\tlda #size(fstack_local)\n";
		
		out << "\tjsr fstack.alloc\n\n";
	}

	out << "\t; -------------------------- BEGIN\n\n";
}

void CodeGenerator::generate_function_epilogue(std::ofstream& out)
{
	out << "\n\t; -------------------------- END\n\n";
	out << "\t; ripristina bp e sp\n";
	out << "\t.fstack_pop_sp_bp\n";
}

void CodeGenerator::generate_function(Node* node, std::ofstream& out)
{
	if (!node || node->kind != NodeKind::ND_FUNCTION || !node->symbol) return;

	std::string full_name = node->symbol->name;
	std::string proc_label = full_name;

	// Se il nome della funzione è qualificato (es. "std.print"),
	// l'etichetta del .proc deve essere solo la parte locale ("print")
	// perché il blocco .namespace si occupa dello scoping.
	size_t dot_pos = full_name.find_last_of('.');
	if (dot_pos != std::string::npos) {
		proc_label = full_name.substr(dot_pos + 1);
	}

	m_current_function = node;
	out << "\n" << proc_label << " .proc\n\n";

	generate_function_frame(node, out);
	generate_function_prologue(node, out);

	for (Node* stmt_node = node->body.get(); stmt_node; stmt_node = stmt_node->next.get()) {
		generate_node(stmt_node, out);
	}

	generate_function_epilogue(out);

	out << "\n    rts\n\n";
	out << ".endproc\n";
	m_current_function = nullptr;
}

void CodeGenerator::generate_sys_function(Node* node, std::ofstream& out)
{
	if (!node || node->kind != NodeKind::ND_SYS_FUNCTION || !node->symbol) return;

	std::string full_name = node->symbol->name;
	std::string proc_label = full_name;
	size_t dot_pos = full_name.find_last_of('.');
	if (dot_pos != std::string::npos) {
		proc_label = full_name.substr(dot_pos + 1);
	}

	m_current_function = node;
	out << "\n" << proc_label << " .proc\n\n";

	bool has_params = !node->sys_params.empty();

	if (has_params) {
		out << "    pla\n";
		out << "    sta retA\n";
		out << "    pla\n";
		out << "    sta retY\n\n";
	}

	out << "\t; -------------------------- BEGIN\n\n";

	// Genera il codice per il pop dei parametri in ordine di dichiarazione
	for (const auto& param : node->sys_params)
	{
		if (param.is_register) {
			if (param.dest_name == "a") {
				out << "    .fstack_pop_byte_fast\n";
			} else if (param.dest_name == "x") {
				out << "    .fstack_pop_byte_fast\n";
				out << "    tax\n";
			} else if (param.dest_name == "y") {
				out << "    .fstack_pop_byte_fast\n";
				out << "    tay\n";
			} else if (param.dest_name == "ay") {
				out << "    .fstack_pop_word_fast\n";
			} else if (param.dest_name == "ax") {
				out << "    .fstack_pop_word_fast\n";
				out << "    pha\n";
				out << "    tya\n";
				out << "    tax\n";
				out << "    pla\n";
			} else if (param.dest_name == "xy") {
				out << "    .fstack_pop_word_fast\n";
				out << "    tax\n";
			} else if (param.dest_name == "fac1") {
				out << "    .fstack_pop_real_fast\n";
			} else if (param.dest_name == "fac2") {
				out << "    .fstack_pop_real_fast_in_fac2\n";
			}
		} else { // Variabile di memoria
			if (param.type->size == SIZEOF_BYTE) {
				out << "    .fstack_pop_byte_fast\n";
				out << "    sta " << param.dest_name << "+0\n";
			} else if (param.type->size == SIZEOF_WORD) {
				out << "    .fstack_pop_word_fast\n";
				out << "    sty " << param.dest_name << "+1\n";
				out << "    sta " << param.dest_name << "+0\n";
			} else if (param.type->size == SIZEOF_REAL) {
				out << "    .fstack_pop_real_fast\n";
				out << "    lda #<" << param.dest_name << "\n";
				out << "    ldy #>" << param.dest_name << "\n";
				out << "    jsr basic.store_fac1\n";
			}
		}
		out << "\n";
	}

	for (Node* stmt_node = node->body.get(); stmt_node; stmt_node = stmt_node->next.get()) {
		generate_node(stmt_node, out);
	}

	out << "\n\t; -------------------------- END\n\n";

	if (has_params) {
		out << "    lda retY\n";
		out << "    pha\n";
		out << "    lda retA\n";
		out << "    pha\n\n";
	}

	out << "    rts\n\n";

	if (has_params) {
		out << " retY .byte 0\n";
		out << " retA .byte 0\n\n";
	}

	out << ".endproc\n";
	m_current_function = nullptr;
}

void CodeGenerator::generate_header(std::ofstream& out, const std::string& target_arch, const std::string& start_address)
{
	auto now = std::chrono::system_clock::now();
	auto year_month_day = std::chrono::year_month_day{ std::chrono::floor<std::chrono::days>(now) };
	std::string current_year_str = std::format("{}", static_cast<int>(year_month_day.year()));

	std::string include_target = target_arch;
	std::transform(include_target.begin(), include_target.end(), include_target.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

	if (include_target.rfind("vic20", 0) == 0) {
		include_target = "vic20";
	}

	out << "\n";
	out << "; " << APP_NAME << "\n";
	out << "; " << COPYRIGHT_NOTICE << "\n";
	out << "\n";
	if (!start_address.empty())
	{
		out << "* = " << start_address << "\n\n";
	}
	out << ".include \"../nMOSLib/lib/libCD.asm\"";

	if (start_address.empty())
	{
		out << std::format("\n\nPROGRAM TARGET_{}, PROGRAM_ADDRESS_{} , {}", target_arch, target_arch, current_year_str);
	}
	out << "\n\n";
	out << std::format(".include \"../nMOSLib/lib/libCD_{}.asm\"", include_target);
	out << "\n\n";
}


void CodeGenerator::generate_footer(std::ofstream& out)
{
	if (!m_global_constants.empty() || !m_real_constants.empty() || !m_string_constants.empty()) {
		out << "\nkConst .namespace\n";

		if (!m_global_constants.empty()) {
			out << "\n\t; --- Global constants ---\n\n";
			for (const auto& info : m_global_constants) {
				std::string value_part = std::format("{} {}", info.directive, info.value);
				out << "\t" << std::left << std::setw(10) << info.name 
					<< "\t" << std::left << std::setw(25) << value_part
					<< "\t\t" << info.comment << "\n";
			}
		}

		if (!m_real_constants.empty() || !m_string_constants.empty()) {
			out << "\n\t; --- Locals constants ---\n";

			if (!m_real_constants.empty()) {
				out << "\n";
				std::vector<std::pair<double, RealConstantInfo>> sorted_constants;
				for(const auto& pair : m_real_constants) {
					sorted_constants.push_back(pair);
				}
				
				std::sort(sorted_constants.begin(), sorted_constants.end(), [](const auto& a, const auto& b){
					return a.second.label < b.second.label;
				});

				for (const auto& pair : sorted_constants) {
					const RealConstantInfo& info = pair.second;
					std::stringstream line;
					line << "\t" << std::left << std::setw(10) << info.label 
						 << "\t.byte " << std::left << std::setw(25) << info.byte_representation
						 << "\t; " << std::right << std::setw(12) << info.original_value
						 << " \t" << info.decimal_bytes;
					out << line.str() << "\n";
				}
			}

			if (!m_string_constants.empty()) {
				out << "\n";
				for (const auto& info : m_string_constants) {
					out << "\t" << std::left << std::setw(10) << info.label << "\t.null " << info.value << "\n";
				}
			}
		}

		out << "\n.endnamespace\n";
	}
	out << "\n";
	out << "hla_heap:";
	out << "\n\n";
	out << ";;;\n";
	out << ";;\n";
	out << ";\n";
}

void CodeGenerator::generate(Node* root, const std::string& output_filename, const std::string& target_arch, const std::string& start_address)
{
	if (!root || root->kind != NodeKind::ND_PROGRAM)
	{
		return;
	}

	m_global_constants.clear();
	m_global_real_constant_values.clear();
	m_global_string_constant_values.clear();
	m_real_constants.clear();
	m_real_constant_counter = 0;
	m_string_constants.clear();
	m_string_constant_counter = 0;
	m_if_label_counter = 0;
	m_current_function = nullptr;

	std::ofstream out_file(output_filename);
	if (!out_file)
	{
		ErrorHandler::get().push_error(Sender::Emitter, ErrorType::Fatal, Action::Coding, ErrorMessage::FileNotFound, 0, 0, "Could not open output file: " + output_filename);
		return;
	}

	generate_header(out_file, target_arch, start_address);

	// Primo passaggio: raccogli le costanti globali e popola le mappe di ricerca
	for (Node* n = root->body.get(); n; n = n->next.get()) {
		if (n->kind == NodeKind::ND_CONST_DECL) {
			generate_node(n, out_file);
			if (n->symbol) {
				if (std::holds_alternative<double>(n->symbol->const_value)) {
					m_global_real_constant_values[std::get<double>(n->symbol->const_value)] = n->symbol->name;
				} else if (std::holds_alternative<std::string>(n->symbol->const_value)) {
					m_global_string_constant_values[std::get<std::string>(n->symbol->const_value)] = n->symbol->name;
				}
			}
		}
	}

	out_file << "; --- Global Variable Declarations ---\n\n";
	bool has_data = false;
	for (Node* n = root->body.get(); n; n = n->next.get()) {
		if (n->kind == NodeKind::ND_VAR_DECL) {
			generate_var_decl(n, out_file);
			has_data = true;
		}
	}
	if (!has_data) {
		out_file << "; (No global variables)\n";
	}
	out_file << "\n";

	out_file << "; --- Code Section ---\n";

	for (Node* n = root->body.get(); n; n = n->next.get())
	{
		if (n->kind == NodeKind::ND_FUNCTION)
		{
			generate_function(n, out_file);
		}
		else if (n->kind == NodeKind::ND_SYS_FUNCTION)
		{
			generate_sys_function(n, out_file);
		}
		else if (n->kind == NodeKind::ND_NAMESPACE)
		{
			out_file << "\n" << n->symbol->name << " .namespace\n";
			for (Node* func_node = n->body.get(); func_node; func_node = func_node->next.get()) {
				if (func_node->kind == NodeKind::ND_FUNCTION) {
					generate_function(func_node, out_file);
				} else if (func_node->kind == NodeKind::ND_SYS_FUNCTION) {
					generate_sys_function(func_node, out_file);
				}
			}
			out_file << "\n.endnamespace ; " << n->symbol->name << "\n";
		}
		else if (n->kind == NodeKind::ND_ASM)
		{
			generate_node(n, out_file);
		}
	}

	generate_footer(out_file);

	std::filesystem::path out_path(output_filename);
	out_path.replace_extension(".prg");
	std::string prg_filename = out_path.string();

	std::cout << "    Assembly code generated in: " << output_filename << "\n";
	std::cout << "    To compile: 64tass -C -a -B -i " << output_filename << " -o " << prg_filename << "\n";
}
