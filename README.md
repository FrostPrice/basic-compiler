# Compiladores Project

This repository contains the source code and documentation for the **Basic Compiler** project, developed as part of the coursework at **Universidade do Vale do Itajaí (UNIVALI)**.

## Project Overview

The goal of this project is to implement a compiler for a custom programming language. The project involves the following stages:

- Lexical analysis
- Syntax analysis
- Semantic analysis
- Code generation
- Optimization

## 🛠️ Requirements

- **Language**: C++
- **GUI Framework**: Qt 5 (or 6)
- **Build Tool**: qmake (comes with Qt)
- **Platform**: Linux (tested on Arch Linux)

## How to Run

1. Clone the repository:

```bash
   git clone https://github.com/FrostPrice/basic-compiler.git
```

2. Build and run the compiler:

```bash
   mkdir build
   cd build
   qmake ../
   make
```

3. Run the compiler:

```bash
   ./basic-compile
```

## TODOs

### Related to Semantic Actions on Gals

TODO:

- Loops (For, While, Do While)
- If, Else, Else If
- Switch

OK:

- Functions
- Subroutines
- Declarations
- Assignments
- Operators
- Values
- Arrays
- Function calls
- Blocks
- Input and Output

### Related to C++ Code

TODO:

- Add Symbol Table
- Add all actions to the symbol table
- Validate semantic actions

### Related to GUI

TODO:

- Add semantic table to the GUI
- Show errors and warnings in the GUI

## Authors

- Mateus Barbosa
- Jonathas Meine
- Mateus José

## License

This project is licensed under the [MIT License](LICENSE).
