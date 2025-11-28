
// *********
// error.cpp
// *********

#include "error.hpp"
#include <format>
#include <filesystem>

// --- Implementazione del Singleton ---
ErrorHandler& ErrorHandler::get()
{
	static ErrorHandler instance;
	return instance;
}

// --- Mappe di conversione ---
const std::map<Sender, std::string> ErrorHandler::sender_map = 
{
	{Sender::Loader, "Loader"}, 
	{Sender::Lexer, "Lexer"}, 
	{Sender::Parser, "Parser"},
	{Sender::AST, "AST"}, 
	{Sender::Node, "Node"}, 
	{Sender::Asm, "Assembler"},
	{Sender::Emitter, "Emitter"}, 
	{Sender::Scanner, "Scanner"}, 
	{Sender::Cpp, "Preprocessor"},
	{Sender::Compiler, "Compiler"}, 
	{Sender::Optimizer, "Optimizer"}
};

const std::map<ErrorType, std::string> ErrorHandler::type_map = 
{
	{ErrorType::Info, "info"}, 
	{ErrorType::Warning, "warning"}, 
	{ErrorType::Error, "error"},
	{ErrorType::Critical, "critical"}, 
	{ErrorType::Fatal, "fatal"}, 
	{ErrorType::Internal, "internal"}
};

const std::map<ErrorMessage, std::string> ErrorHandler::message_map = 
{
	{ErrorMessage::NoError, "No error"},
	{ErrorMessage::FileNotFound, "File not found"},
	{ErrorMessage::PtrArithmetic, "Invalid pointer arithmetic operation"},
	{ErrorMessage::TooFewArguments, "Too few arguments"},
	{ErrorMessage::TooManyArguments, "Too many arguments"},
	{ErrorMessage::UnexpectedToken, "Unexpected token"},
	{ErrorMessage::IdentifierNotFound, "Identifier not found"},
	{ErrorMessage::MissingInputFile, "No input file specified"},
	{ErrorMessage::MissingValueForOption, "Missing value for command line option"},
	{ErrorMessage::InvalidArchitecture, "Invalid target architecture specified"},
	{ErrorMessage::InvalidUtf8Sequence, "Invalid UTF-8 sequence encountered"},
	{ErrorMessage::UnknownOption, "Unknown command line option"},
	{ErrorMessage::UnclosedComment, "Unclosed multi-line comment"},
	{ErrorMessage::InvalidNumberFormat, "Invalid number format or value out of range"},
	{ErrorMessage::UnclosedString, "Unclosed string or character literal"},
	{ErrorMessage::InvalidCharacterLiteral, "Character literal must contain exactly one character"},
	{ErrorMessage::InvalidIdentifierStart, "Identifier cannot start with a '$' character, which is reserved for hexadecimal memory locations"},
	{ErrorMessage::MissingClosingParenthesis, "Missing closing parenthesis ')'"},
	{ErrorMessage::MissingMainFunction, "Mandatory function 'main' not defined"},
	{ErrorMessage::SymbolNameConflict, "Symbol name conflicts with an existing definition"},
	{ErrorMessage::NestedScopeNotAllowed, "Nested scopes ('{...}') are not allowed"},
	{ErrorMessage::ConstantOutOfRange, "Constant value is out of range for its type"},
	{ErrorMessage::UnsupportedTypeSuffix, "Unsupported type suffix for literal constant"},
	{ErrorMessage::WrongNumberOfArgs, "Wrong number of arguments in function call"},
	{ErrorMessage::TypeMismatchInArg, "Type mismatch for argument in function call"},
	{ErrorMessage::InvalidStringOperation, "Invalid operation on string literal"},
	{ErrorMessage::DivisionByZero, "Division by zero in constant expression"},
	{ErrorMessage::InvalidSysRegisterForType, "Invalid register used as a destination for the specified sys parameter type"},
	{ErrorMessage::AddressOfFunction, "Cannot take the address of a function name; it is already an address value"},
	{ErrorMessage::AssignmentToConstant, "Cannot assign a value to a constant"},
    {ErrorMessage::ConstantNotInitializedWithLiteral, "Constant must be initialized with a literal value"},
    {ErrorMessage::TypeMismatchInConstantDecl, "Type mismatch in constant declaration"},
	{ErrorMessage::BitwiseOperationOnFloat, "Bitwise operators cannot be applied to floating-point numbers"},
	{ErrorMessage::StackOverflow, "Stack overflow: function locals exceed 255 bytes"}
};

const std::map<Action, std::string> ErrorHandler::action_map = 
{
	{Action::NoAction, ""}, 
	{Action::CheckFileExists, "checking file existence"},
	{Action::Init, "initializing"}, 
	{Action::Tokenizing, "tokenizing"},
	{Action::TokenizingString, "tokenizing string"}, 
	{Action::TokenizingNumber, "tokenizing number"},
	{Action::Parsing, "parsing"}, 
	{Action::Scanning, "scanning"},
	{Action::Coding, "generating code"}, 
	{Action::CommandLine, "parsing command line"}
};

const std::map<ErrorType, std::string> ErrorHandler::type_prefix_map = 
{
	{ErrorType::Info, ".."}, 
	{ErrorType::Warning, "!? "}, 
	{ErrorType::Error, "?! "},
	{ErrorType::Critical, "?! "}, 
	{ErrorType::Fatal, "?? "}, 
	{ErrorType::Internal, "!! "}
};

// --- Implementazione dei Metodi ---

void ErrorHandler::set_current_file(const std::string& filename)
{
	m_current_filename = filename;
}

void ErrorHandler::push_error(Sender sender, ErrorType type, Action action, ErrorMessage message, size_t line, size_t column, const std::string& extra_message)
{
	if (type == ErrorType::Warning) {
		m_warning_count++;
	} else if (type >= ErrorType::Error) {
		m_error_count++;
	}

	m_errors.push_back({sender, type, action, message, m_current_filename, line, column, extra_message});
}

bool ErrorHandler::has_errors() const
{
	return m_error_count > 0;
}

bool ErrorHandler::has_fatal_errors() const
{
	for (const auto& err : m_errors) {
		if (err.type == ErrorType::Fatal || err.type == ErrorType::Internal) {
			return true;
		}
	}
	return false;
}

size_t ErrorHandler::get_warning_count() const
{
	return m_warning_count;
}

size_t ErrorHandler::get_error_count() const
{
	return m_error_count;
}

std::string ErrorHandler::get_filename_from_path(const std::string& path) 
{
	if (path.empty()) return "";
	try {
		return std::filesystem::path(path).filename().string();
	} catch (...) {
		auto pos = path.find_last_of("/\\");
		return (pos != std::string::npos) ? path.substr(pos + 1) : path;
	}
}

void ErrorHandler::log_errors(std::ostream& out) const
{
	if (m_errors.empty()) {
		return;
	}
	out << "\n";
	for (const auto& err : m_errors) {
		std::string prefix = type_prefix_map.at(err.type);
		std::string short_filename = get_filename_from_path(err.filename);
		std::string sender_str = sender_map.at(err.sender);
		std::string type_str = type_map.at(err.type);
		std::string action_str = action_map.at(err.action);
		std::string message_str = message_map.at(err.message);

		std::string location_str="";
		if ((err.line!=0) && (err.column!=0))
			location_str = (err.line > 0) ? std::format("RC[{:03}/{:03}]", err.line, err.column) : "---/---";
		
		std::string output_line = "" ;
		if (!short_filename.empty()) 
			output_line = std::format(
				"{} {:<10} [{:>20}] {} := {} -> {}",
				prefix, sender_str, short_filename, location_str, action_str, message_str
			);
		else
			output_line = std::format(
				"{} {:<10} {} := {} -> {}",
				prefix, sender_str, location_str, action_str, message_str
			);

		if (!err.extra_message.empty()) {
			output_line += std::format(" ({})", err.extra_message);
		}

		out << output_line << '\n';
	}

	size_t warnings = get_warning_count();
	size_t errors = get_error_count();

	if (warnings > 0 || errors > 0) {
		out << "\n--- Compilation Summary ---\n";
		if (warnings > 0) out << "Warnings: " << warnings << "\n";
		if (errors > 0) out << "Errors: " << errors << "\n";
		out << "-----------------------------\n";
	}
}
