
// *********
// compiler.cpp
// *********

#include "compiler.hpp"
#include "error.hpp"
#include "f40_converter.hpp" 
#include "config.hpp"
#include <iostream>
#include <vector>
#include <string_view>
#include <algorithm>
#include <format>
#include <charconv>
#include <string> 
#include <cctype>

// --- Inizializzazione delle architetture supportate ---
const std::vector<std::string> Compiler::SUPPORTED_ARCHITECTURES = {
	"C64", "VIC20", "VIC20_3K", "VIC20_8K", "VIC20_16K", 
	"VIC20_24K", "VIC20_32K", "C128"
};

// --- COSTRUTTORE ---
Compiler::Compiler(int argc, char** argv)
{
	parse_command_line(argc, argv);
}

// --- parse_command_line ---
void Compiler::parse_command_line(int argc, char** argv) 
{
	std::vector<std::string_view> args(argv + 1, argv + argc);

	for (size_t i = 0; i < args.size(); ++i) 
	{
		const auto& arg = args[i];

		// Funzione helper per validare e recuperare il valore di un'opzione
		auto get_value = [&](std::string_view& value) -> bool {
			if (i + 1 >= args.size()) {
				ErrorHandler::get().push_error(Sender::Compiler, ErrorType::Fatal, Action::CommandLine, ErrorMessage::MissingValueForOption, 0, 0, "Missing value for option " + std::string(arg));
				return false;
			}
			const auto& next_arg = args[i + 1];
			if (next_arg.starts_with("-")) {
				ErrorHandler::get().push_error(Sender::Compiler, ErrorType::Fatal, Action::CommandLine, ErrorMessage::MissingValueForOption, 0, 0, "Missing value for option " + std::string(arg) + ", got option '" + std::string(next_arg) + "' instead");
				return false;
			}
			value = next_arg;
			i++; // Consuma l'argomento del valore
			return true;
		};

		if (arg == "-h" || arg == "--help") {
			m_help_requested = true;
			return;
		} else if (arg == "-f64tof40" || arg == "-f32tof40") {
			std::string_view value_sv;
			if (!get_value(value_sv)) return;
			std::string value_str(value_sv);
			try {
				m_conversion_input_value = std::stod(value_str);
				m_conversion_type = (arg == "-f64tof40") ? ConversionType::F64_TO_F40 : ConversionType::F32_TO_F40;
			} catch (const std::invalid_argument&) {
				ErrorHandler::get().push_error(Sender::Compiler, ErrorType::Fatal, Action::CommandLine, ErrorMessage::MissingValueForOption, 0, 0, "Invalid number provided for " + std::string(arg) + ": " + value_str);
				return;
			} catch (const std::out_of_range&) {
				ErrorHandler::get().push_error(Sender::Compiler, ErrorType::Fatal, Action::CommandLine, ErrorMessage::MissingValueForOption, 0, 0, "Number out of range for " + std::string(arg) + ": " + value_str);
				return;
			}
		} else if (arg == "-g") {
			m_emit_debug_symbols = true;
		} else if (arg == "-O") {
			m_enable_optimizations = true;
		} else if (arg == "-mcpp") {
			m_mcpp_compatibility = true;
		} else if (arg == "-T") {
			std::string_view val_str;
			if (!get_value(val_str)) return;
			auto result = std::from_chars(val_str.data(), val_str.data() + val_str.size(), m_tab_width);
			if (result.ec != std::errc() || m_tab_width == 0) {
				ErrorHandler::get().push_error(Sender::Compiler, ErrorType::Fatal, Action::CommandLine, ErrorMessage::MissingValueForOption, 0, 0, "Invalid numeric value for -T: " + std::string(val_str));
				return;
			}
		} else if (arg == "-t") {
			std::string_view arch_sv;
			if (!get_value(arch_sv)) return;
			std::string arch(arch_sv);
			std::string upper_arch = arch;
			// Converte in maiuscolo per il confronto
			std::transform(upper_arch.begin(), upper_arch.end(), upper_arch.begin(),
				   [](unsigned char c){ return static_cast<char>(std::toupper(c)); });

			auto it = std::find(SUPPORTED_ARCHITECTURES.begin(), SUPPORTED_ARCHITECTURES.end(), upper_arch);
			
			if (it != SUPPORTED_ARCHITECTURES.end()) {
				m_target_architecture = *it; // Memorizza la versione canonica maiuscola
			} else {
				ErrorHandler::get().push_error(Sender::Compiler, ErrorType::Fatal, Action::CommandLine, ErrorMessage::InvalidArchitecture, 0, 0, "Unsupported architecture: " + arch);
				return;
			}
		}
		else if (arg == "-s") {
			std::string_view addr_sv;
			if (!get_value(addr_sv)) return;
			m_start_address = addr_sv;
		}
		else if (arg == "-o" || arg == "--output") {
			std::string_view file_sv;
			if (!get_value(file_sv)) return;
			m_output_file = file_sv;
		} else if (arg == "-i" || arg == "--input") {
			std::string_view file_sv;
			if (!get_value(file_sv)) return;
			m_input_file = file_sv;
		} else {
			if (arg.starts_with("-")) {
				ErrorHandler::get().push_error(Sender::Compiler, ErrorType::Fatal, Action::CommandLine, ErrorMessage::UnknownOption, 0, 0, std::string(arg));
				return;
			}

			if (m_input_file.empty()) {
				m_input_file = arg;
			} else {
				 ErrorHandler::get().push_error(Sender::Compiler, ErrorType::Warning, Action::CommandLine, ErrorMessage::UnexpectedToken, 0, 0, "Ignoring extra argument: " + std::string(arg));
			}
		}
	}
}

void Compiler::print_help() const 
{
	// MODIFICA: Usa le costanti da config.hpp
	std::cout << APP_NAME << "\n";
	std::cout << COPYRIGHT_NOTICE << "\n\n";
	std::cout << "Usage: HLA [options] [file...]\n";
	std::cout << "Options:\n";
	std::cout << "  -i, --input <file>     Specify the input file.\n";
	std::cout << "  -o, --output <file>    Specify the output file (default: a.out).\n";
	std::cout << "  -g                     Emit debug symbols and streamer file.\n";
	std::cout << "  -O                     Enable optimizations.\n";
	std::cout << "  -T <width>             Set tab width (default: 4).\n";
	std::cout << "  -t <arch>              Specify the target architecture (default: C64).\n";
	std::cout << "                         Supported: C64, VIC20, VIC20_3K, VIC20_8K, VIC20_16K,\n";
	std::cout << "                         VIC20_24K, VIC20_32K, C128.\n";
	std::cout << "  -s <address>           Specify a custom start address (e.g., 49152 or '$c000').\n";
	std::cout << "  -mcpp                  Enable compatibility with mcpp (disables apostrophe as number separator).\n";
	std::cout << "  -f32tof40 <number>     Convert a 32-bit float  to 5-byte MBF and print it.\n";
	std::cout << "  -f64tof40 <number>     Convert a 64-bit double to 5-byte MBF and print it.\n";
	std::cout << "  -h, --help             Display this information.\n";
}

void Compiler::perform_conversion() const
{
	std::string original_type_str;
	std::string mbf_representation;
	std::string operation_str;

	switch (m_conversion_type) {
		case ConversionType::F32_TO_F40: {
			original_type_str = "float (32-bit)";
			float input_value = static_cast<float>(m_conversion_input_value);
			mbf_representation = F40Converter::convertFloatToMBF_string(input_value);
			operation_str = std::format("convertFloatToMBF_string({})", input_value);
			break;
		}
		case ConversionType::F64_TO_F40: {
			original_type_str = "double (64-bit)";
			mbf_representation = F40Converter::convertDoubleToMBF_string(m_conversion_input_value);
			operation_str = std::format("convertDoubleToMBF_string({})", m_conversion_input_value);
			break;
		}
		case ConversionType::None:
			return; 
	}

	std::cout << "--- IEEE to 5-Byte MBF (F40) Conversion ---\n";
	std::cout << "Original type:         " << original_type_str << "\n";
	std::cout << "Original value:        " << std::format("{:.10f}", m_conversion_input_value) << "\n";
	std::cout << "Conversion operation:  " << operation_str << "\n";
	std::cout << "Converted MBF-5 format:  " << mbf_representation << "\n";
	std::cout << "-------------------------------------------\n";
}

void Compiler::execute_pipeline() 
{
	auto& errorHandler = ErrorHandler::get();
	errorHandler.set_current_file(m_input_file);

	std::cout << ">>> Starting compilation for: " << m_input_file << " <<<\n";
	std::cout << "    Target Architecture: " << m_target_architecture << "\n";
	std::cout << "    Debug Symbols: " << (m_emit_debug_symbols ? "Enabled" : "Disabled") << "\n";
	std::cout << "    Optimizations: " << (m_enable_optimizations ? "Enabled" : "Disabled") << "\n";
	std::cout << "    Tab Width: " << m_tab_width << "\n";
	std::cout << "    mcpp Compatibility: " << (m_mcpp_compatibility ? "Enabled" : "Disabled") << "\n\n";

	// --- FASE 1: Loader / Streamer ---
	std::cout << "Phase: Loading and Streaming Source File...\n";
	m_streamer.set_tab_width(m_tab_width);
	m_streamer.stream_file(m_input_file);
	
	if (errorHandler.has_errors()) return;

	std::cout << "    " << m_streamer.get_char_stream().size() << " characters streamed.\n";
	if (m_emit_debug_symbols) {
		std::cout << "    Dumping debug stream to " << m_input_file << ".streamer.debug\n";
		m_streamer.dump_to_file(m_input_file);
	}

	// --- FASE 2: Lexer ---
	std::cout << "Phase: Lexical Analysis...\n";
	m_lexer.tokenize(m_streamer.get_char_stream(), m_mcpp_compatibility);

	if (errorHandler.has_errors()) return;

	std::cout << "    " << m_lexer.get_token_stream().size() << " tokens generated.\n";
	if (m_emit_debug_symbols) {
		std::cout << "    Dumping debug lexer to " << m_input_file << ".lexer.debug\n";
		m_lexer.dump_to_file(m_input_file, m_streamer);
	}
	
	// --- FASE 3: Parser (Costruzione AST) ---
	std::cout << "Phase: Parsing and AST Construction...\n";
	m_ast_root = m_parser.parse(m_lexer.get_token_stream(), m_streamer);

	if (errorHandler.has_errors()) {
		// Anche se ci sono errori, il dump può essere utile per il debug
		if (m_emit_debug_symbols) {
			std::cout << "    Dumping debug parser log (due to errors) to " << m_input_file << ".parser.debug\n";
			m_parser.dump_ast_to_file(m_input_file);
		}
		return;
	}

	if (m_ast_root)
	{
		std::cout << "    AST constructed successfully.\n";
		// Aggiunta del dump del parser e della symbol table in caso di successo
		if (m_emit_debug_symbols) {
			std::cout << "    Dumping debug parser to " << m_input_file << ".parser.debug\n";
			m_parser.dump_ast_to_file(m_input_file);
			std::cout << "    Dumping debug symbol table to " << m_input_file << ".symtable.debug\n";
			m_parser.dump_symbol_table_to_file(m_input_file);
		}
	}
	else
	{
		std::cout << "    AST construction failed.\n";
		// Aggiunta del dump del parser anche in caso di fallimento senza errori registrati
		if (m_emit_debug_symbols) {
			std::cout << "    Dumping debug parser log (on failure) to " << m_input_file << ".parser.debug\n";
			m_parser.dump_ast_to_file(m_input_file);
		}
		return; // Non continuare se l'AST non è valido
	}

	// --- FASE 4: Generazione Codice ---
	std::cout << "Phase: Code Generation...\n";
	m_code_generator.generate(m_ast_root.get(), m_output_file, m_target_architecture, m_start_address);

	if (errorHandler.has_errors()) return;

	std::cout << "\n>>> Compilation finished. <<<\n";
}

int Compiler::run() 
{
	auto& errorHandler = ErrorHandler::get();

	if (m_help_requested) {
		print_help();
		return 0;
	}

	if (m_conversion_type != ConversionType::None) {
		if (errorHandler.has_fatal_errors()) {
			errorHandler.log_errors(std::cerr);
			return 1;
		}
		perform_conversion();
		return 0;
	}

	if (errorHandler.has_fatal_errors()) {
		errorHandler.log_errors(std::cerr);
		return 1;
	}

	if (m_input_file.empty()) {
		errorHandler.push_error(Sender::Compiler, ErrorType::Fatal, Action::CommandLine, ErrorMessage::MissingInputFile);
	}

	if (errorHandler.has_errors()) {
		errorHandler.log_errors(std::cerr);
		if (m_input_file.empty()) print_help();
		return 1;
	}

	execute_pipeline();

	errorHandler.log_errors(std::cout);

	return errorHandler.has_errors() ? 1 : 0;
}
