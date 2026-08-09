#include <FlexLexer.h> // Automatically bundled header with your flex installation
#include <fstream>
#include <sstream>
#include <iostream>

#include "lexer.h"


void run(std::istream &stream, std::ostream &out) {
  FlexLexer *lexer = new yyFlexLexer(&stream);
  int token;
  while ((token = lexer->yylex()) != TOKEN_EOF) {
    out << "Token ID: " << token << " | Matched: [" << lexer->YYText() << "]"
        << " | Length: " << lexer->YYLeng() << "\n";
  }
}

void run_file(char *path) {
  std::ifstream stream(path);
  if (!stream.is_open()) {
    std::cerr << "Error: Could not open" << path << " !" << std::endl;
    return;
  }
  run(stream, std::cout);
  stream.close();
}

void run_prompt() {
  std::cout << "Type something (press Ctrl+D on Linux/Mac or Ctrl+Z on Windows "
               "to stop):\n";
  std::cout << "> ";

  std::string line;
  while (std::getline(std::cin, line)) {
    std::istringstream stream(line);
    run(stream, std::cout);
    std::cout << "> ";
  }
  return;
}

int main(int argc, char *argv[]) {
  if (argc > 2) {
    std::cerr << "Usage: llox [script]";
    std::exit(64);
  } else if (argc == 2) {
    run_file(argv[1]);
  } else {
    run_prompt();
  }
}
