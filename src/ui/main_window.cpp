#include "main_window.hpp"
#include "semantic_table_model.hpp"
#include "../gals/Lexical.h"
#include "../gals/Semantic.h"
#include "../gals/Syntactic.h"
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QWidget>
#include <QLabel>
#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <QScrollBar>
#include <QShortcut>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    resize(1200, 700);

    editor = new CodeEditor(this);
    editor->setMinimumHeight(40);
    editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QFont font = editor->font();
    font.setPointSize(14);
    editor->setFont(font);
    editor->setStyleSheet(R"(
        background-color: #2D2A2E;
        color: #FCFCFA;
        border: 1px solid #403E41;
        padding-left: 4px;
    )");

    QPushButton *compileButton = new QPushButton("Compilar", this);
    compileButton->setStyleSheet(R"(
        QPushButton {
            background-color: #66D9EF;
            color: #272822;
            font-weight: bold;
            padding: 6px 12px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #A1EFE4;
        }
    )");
    compileButton->setMinimumHeight(40);
    compileButton->setMaximumHeight(40);
    compileButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    output = new QTextBrowser(this);
    output->setMinimumHeight(40);
    output->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    output->setFontPointSize(14);
    output->setStyleSheet(R"(
        QTextBrowser {
            background-color: #2D2A2E;
            color: #A9DC76;
            border: 1px solid #403E41;
        }
    )");

    // criar a tabela de semântica
    tableView = new QTableView(this);
    tableModel = new SemanticTableModel(this);
    tableView->setModel(tableModel);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->verticalHeader()->setStyleSheet(R"(
        QHeaderView::section {
            color: #75715E;
        }
    )");
    tableView->setStyleSheet(R"(
        QTableView {
            background-color: #2D2A2E;
            color: rgba(255, 255, 255, 0.69);
            selection-background-color: rgb(110, 110, 110);
            gridline-color:rgb(155, 155, 155);
        }
    )");
    tableView->verticalScrollBar()->setStyleSheet(R"(
        QScrollBar:vertical {
            background: #2D2A2E;
            width: 14px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:vertical {
            background: #75715E;
            border-radius: 7px;
        }
        QScrollBar::add-line:vertical {
            border: none;
            background: transparent;
        }
        QScrollBar::sub-line:vertical {
            border: none;
            background: transparent;
        }
    )");

    tableView->horizontalScrollBar()->setStyleSheet(R"(
        QScrollBar:horizontal {
            background: #2D2A2E;
            height: 15px;
            margin: 0px 0px 0px 0px;
        }
        QScrollBar::handle:horizontal {
            background: #75715E;
            border-radius: 7px;
        }
        QScrollBar::add-line:horizontal {
            border: none;
            background: transparent;
        }
        QScrollBar::sub-line:horizontal {
            border: none;
            background: transparent;
        }
    )");

    assemblyOutput = new QTextBrowser(this);
    assemblyOutput->setMinimumHeight(40);
    assemblyOutput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    assemblyOutput->setFontPointSize(14);
    assemblyOutput->setStyleSheet(R"(
        QTextBrowser {
            background-color: #2D2A2E;
            color: #FCFCFA;
            border: 1px solid #403E41;
        }
    )");
    assemblyOutput->setText("Assembly output will be shown here.");

    // Parte de cima: editor + botão + output empilhados verticalmente
    QSplitter *leftSplitter = new QSplitter(Qt::Vertical, this);
    leftSplitter->addWidget(editor);
    leftSplitter->addWidget(compileButton);
    leftSplitter->addWidget(output);

    leftSplitter->setCollapsible(0, false);
    leftSplitter->setCollapsible(1, false);
    leftSplitter->setCollapsible(2, false);

    leftSplitter->setStretchFactor(0, 8);
    leftSplitter->setStretchFactor(1, 1);
    leftSplitter->setStretchFactor(2, 2);

    // Parte de cima: editor + botão + output empilhados verticalmente
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical, this);
    rightSplitter->addWidget(tableView);
    rightSplitter->addWidget(assemblyOutput);

    rightSplitter->setCollapsible(0, true);
    rightSplitter->setCollapsible(1, true);

    rightSplitter->setStretchFactor(0, 2);
    rightSplitter->setStretchFactor(1, 3);

    // Horizontal: verticalSplitter | tableView
    QSplitter *horizontalSplitter = new QSplitter(Qt::Horizontal, this);
    horizontalSplitter->addWidget(leftSplitter);
    horizontalSplitter->addWidget(rightSplitter);

    horizontalSplitter->setStretchFactor(0, 3);
    horizontalSplitter->setStretchFactor(1, 2);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(horizontalSplitter);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);
    setCentralWidget(widget);
    widget->setStyleSheet("background-color: #1E1E1E;");

    connect(compileButton, &QPushButton::clicked, this, &MainWindow::compileCode);

    QShortcut *compileShortcut = new QShortcut(this);
    compileShortcut->setKey(Qt::CTRL + Qt::Key_Return);

    connect(compileShortcut, &QShortcut::activated, this, &MainWindow::compileCode);

    QShortcut *symbolTableShortcut = new QShortcut(this);
    symbolTableShortcut->setKey(Qt::CTRL + Qt::Key_T);
    connect(symbolTableShortcut, &QShortcut::activated, this, [rightSplitter]()
            { rightSplitter->setVisible(!rightSplitter->isVisible()); });
}

void MainWindow::compileCode()
{
    tableModel->clear(); // Reset the table model

    QString code = editor->toPlainText();

    Lexical lex;
    Syntactic syn;
    Semantic sem;

    lex.setInput(code.toStdString().c_str());

    try
    {
        syn.parse(&lex, &sem);

        validateSemantics(sem);
        displayCompilationResults(sem);
    }
    catch (LexicalError err)
    {
        output->setTextColor(Qt::red);
        output->setText("Lexical error: " + QString::fromStdString(err.getMessage()));
    }
    catch (SyntacticError err)
    {
        output->setTextColor(Qt::red);
        output->setText("Syntactic error: " + QString::fromStdString(err.getMessage()));
    }
    catch (SemanticError err)
    {
        output->setTextColor(Qt::red);
        output->setText("Semantic error: " + QString::fromStdString(err.getMessage()));
    }

    sem.clearSymbolPointer();
    sem.warnings.clear(); // Clear previous warnings
}

void MainWindow::validateSemantics(Semantic &sem)
{
    validateMainFunction(sem);
    checkUnusedSymbols(sem);
}

void MainWindow::validateMainFunction(Semantic &sem)
{
    vector<SymbolTable::SymbolInfo> symbols = sem.symbolTable.getAllSymbols();
    SymbolTable::SymbolInfo *mainSymbol = nullptr;
    int mainIndex = -1;

    for (int i = 0; i < symbols.size(); ++i)
    {
        if (symbols[i].id == "main" && symbols[i].symbolClassification == SymbolTable::SymbolClassification::FUNCTION)
        {
            if (symbols[i].dataType != SemanticTable::Types::__NULL)
            {
                throw SemanticError("Semantic error: 'main' function must return 'void'.");
            }
            else if (symbols[i].functionParams.size() > 0)
            {
                throw SemanticError("Semantic error: 'main' function must not have parameters.");
            }

            // Add the first jump to the main function
            // Add in the beginning of the assembly code
            string label = sem.assembly.generateAssemblyLabel("main", symbols[i].scope);
            sem.assembly.text.insert(sem.assembly.text.begin(), "\tJMP FUNC_" + label);

            // Check if inside main has any return statement
            // Changes them to a Bip instruction: HLT 0
            bool insideMainFunction = false;
            for (auto &instruction : sem.assembly.text)
            {
                // Check if we're entering the main function
                if (instruction.find("FUNC_" + label + ":") != std::string::npos)
                {
                    insideMainFunction = true;
                }
                // Check if we're exiting the main function
                else if (insideMainFunction && instruction.find("# End of function " + label) != std::string::npos)
                {
                    break; // Exit the loop, we found the end of main function
                }
                // Replace RETURN with HLT 0 if we're inside main function
                else if (insideMainFunction && instruction.find("RETURN") != std::string::npos)
                {
                    instruction = "\tHLT 0";
                }
            }

            mainSymbol = &symbols[i];
            mainIndex = i;
            break;
        }
    }

    if (!mainSymbol)
    {
        throw SemanticError("Semantic error: 'main' function not declared.");
    }

    // Check if there are symbols after the main function, after it has been declared
    for (int i = mainIndex + 1; i < symbols.size(); ++i)
    {
        cout << "Symbol: " << symbols[i].id << ", Scope: " << symbols[i].scope << endl;
        if (symbols[i].scope == mainSymbol->scope || symbols[i].scope > sem.mainInnerScope + 1) // Ignore the symbols in the main function scope, thats why we add 1
        {
            sem.warnings.push_back("Warning: Symbol '" + symbols[i].id + "' declared after 'main' function.");
        }
    }
}

void MainWindow::checkUnusedSymbols(Semantic &sem)
{
    for (auto &symbol : sem.symbolTable.getAllSymbols())
    {
        if (symbol.id == "main")
        {
            continue; // Skip main function
        }

        if (!symbol.isUsed)
        {
            sem.warnings.push_back("Warning: Symbol '" + symbol.id + "' declared but not used.");
        }
    }
}

void MainWindow::displayCompilationResults(Semantic &sem)
{
    QString resultText = buildWarningsText(sem.warnings);
    resultText += "<span style='color:lightgreen;'>Compiled successfully!</span>";

    tableModel->populateModel(sem.symbolTable.getAllSymbols());
    output->setHtml(resultText);

    assemblyOutput->setText(QString::fromStdString(sem.assembly.generateAssembly()));
}

QString MainWindow::buildWarningsText(std::vector<std::string> warnings)
{
    QString warningsText;

    if (!warnings.empty())
    {
        for (std::string &warning : warnings)
        {
            warningsText += "<span style='color:orange;'>" + QString::fromStdString(warning) + "</span><br>";
        }
        warningsText += "<br>";
    }

    return warningsText;
}