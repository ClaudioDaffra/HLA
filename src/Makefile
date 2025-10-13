# Compilatore C++ da utilizzare (es. g++ o clang++)
CXX = g++

# Flag di compilazione:
# -std=c++23: Utilizza lo standard C++23
# -Wall:      Abilita tutti gli avvisi comuni del compilatore
# -g:         Aggiunge informazioni di debug per l'uso con GDB
CXXFLAGS = -std=c++23 -Wall -pedantic -Wall -Wextra

# Flag per il linker (es. per aggiungere librerie come -lmylib)
LDFLAGS =

# Nome dell'eseguibile finale
TARGET = hla

# Elenco di tutti i file sorgente (.cpp)
SOURCES = \
          f40_converter.cpp \
          compiler.cpp \
          error.cpp \
          streamer.cpp \
          lexer.cpp \
          parser.cpp \
          symbolTable.cpp \
          ast.cpp \
          genCode.cpp \
          optimizer.cpp	\
		  main.cpp 

		  
# Genera automaticamente l'elenco dei file oggetto (.o) dai file sorgente
OBJECTS = $(SOURCES:.cpp=.o)

# La regola 'all' è la regola predefinita ed è un target fittizio
.PHONY: all
all: $(TARGET)

# Regola per collegare i file oggetto e creare l'eseguibile finale
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

# Regola generica per compilare i file .cpp in file .o
# $< è il nome della prima dipendenza (il file .cpp)
# $@ è il nome del target (il file .o)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regola 'clean' per rimuovere i file generati (oggetti e eseguibile)
.PHONY: clean
clean:
	rm -f $(OBJECTS) $(TARGET)
