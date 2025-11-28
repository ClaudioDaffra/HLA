
// *********
// error.hpp
// *********

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstddef>

// --- Enum Class per la Sicurezza dei Tipi ---
enum class Sender {
	Loader,
	Lexer,
	Parser,
	AST,
	Node,
	Asm,
	Emitter,
	Scanner,
	Cpp,
	Compiler,
	Optimizer
};
enum class ErrorType {
	Info,
	Warning,
	Error,
	Critical,
	Fatal,
	Internal
};
enum class Action {
	NoAction,
	CheckFileExists,
	Init,
	Tokenizing,
	TokenizingString,
	TokenizingNumber,
	Parsing,
	Scanning,
	Coding,
	CommandLine
};
enum class ErrorMessage {
	NoError,
	FileNotFound,
	PtrArithmetic,             // per errori specifici sui puntatori
	TooFewArguments,
	TooManyArguments,
	UnexpectedToken,
	IdentifierNotFound,
	MissingInputFile,
	MissingValueForOption,
	InvalidArchitecture,
	InvalidUtf8Sequence,
	UnknownOption,
	UnclosedComment,
	InvalidNumberFormat, 		// per numeri malformati o fuori range
	UnclosedString,
	InvalidCharacterLiteral,
	InvalidIdentifierStart,
	UnclosedAsmBlock,
	MissingClosingParenthesis,
	MissingMainFunction,
	SymbolNameConflict,
	NestedScopeNotAllowed, 		// per vietare i blocchi {} nidificati
	ConstantOutOfRange,
	UnsupportedTypeSuffix,
	WrongNumberOfArgs,
	TypeMismatchInArg,
	InvalidStringOperation,
	DivisionByZero,
	InvalidSysRegisterForType,
	AddressOfFunction,
	AssignmentToConstant,
	ConstantNotInitializedWithLiteral,
	TypeMismatchInConstantDecl,
	BitwiseOperationOnFloat, 	// per operatori bitwise su float
	StackOverflow				// Local stack exceeds 255 bytes
};

// --- Struttura per un Singolo Errore ---
struct ErrorRecord {
	Sender sender;
	ErrorType type;
	Action action;
	ErrorMessage message;
	std::string filename;
	size_t line;
	size_t column;
	std::string extra_message;
};

// --- Classe Principale per la Gestione degli Errori (Singleton) ---
class ErrorHandler {
public:
	// Rimuovi costruttori di copia e assegnamento
	ErrorHandler(const ErrorHandler&) = delete;
	ErrorHandler& operator=(const ErrorHandler&) = delete;

	// Metodo statico per accedere all'istanza unica
	static ErrorHandler& get();

	void set_current_file(const std::string& filename);

	void push_error(Sender sender, ErrorType type, Action action,
					ErrorMessage message, size_t line = 0,
					size_t column = 0,
					const std::string& extra_message = "");

	void log_errors(std::ostream& out = std::cout) const;

	bool has_errors() const;
	bool has_fatal_errors() const;

	size_t get_warning_count() const;
	size_t get_error_count() const;

private:
	// Costruttore privato per il pattern Singleton
	ErrorHandler() = default;

	std::string m_current_filename;
	std::vector<ErrorRecord> m_errors;

	size_t m_warning_count = 0;
	size_t m_error_count = 0;

	// Mappe statiche per la conversione (invariate)
	static const std::map<Sender, std::string> sender_map;
	static const std::map<ErrorType, std::string> type_map;
	static const std::map<ErrorMessage, std::string> message_map;
	static const std::map<Action, std::string> action_map;
	static const std::map<ErrorType, std::string> type_prefix_map;

	static std::string get_filename_from_path(const std::string& path);
};
