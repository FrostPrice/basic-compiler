# project.pro

QT += widgets
CONFIG += c++17
TARGET = basic_compiler

RESOURCES += resources.qrc

SOURCES += src/main.cpp \
           src/ui/main_window.cpp \
           src/ui/code_editor.cpp \
           src/gals/Constants.cpp \
           src/gals/Lexical.cpp \
           src/gals/Semantic.cpp \
           src/gals/Syntactic.cpp \
           src/gals/SemanticTable.cpp \
           src/ui/semantic_table_model.cpp \
           src/gals/Assembly.cpp


HEADERS += src/ui/main_window.hpp \
           src/gals/Assembly.hpp \
           src/ui/code_editor.hpp \
           src/gals/AnalysisError.h \
           src/gals/ExpressionController.hpp \
           src/gals/Constants.h \
           src/gals/Lexical.h \
           src/gals/LexicalError.h \
           src/gals/Semantic.h \
           src/gals/SemanticError.h \
           src/gals/Syntactic.h \
           src/gals/SyntacticError.h \
           src/gals/Token.h \
           src/gals/SemanticTable.hpp \
           src/gals/SymbolTable.hpp \
           src/ui/semantic_table_model.hpp