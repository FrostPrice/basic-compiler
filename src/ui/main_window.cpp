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

    // Parte de cima: editor + botão + output empilhados verticalmente
    QSplitter *verticalSplitter = new QSplitter(Qt::Vertical, this);
    verticalSplitter->addWidget(editor);
    verticalSplitter->addWidget(compileButton);
    verticalSplitter->addWidget(output);

    verticalSplitter->setCollapsible(0, false);
    verticalSplitter->setCollapsible(1, false);
    verticalSplitter->setCollapsible(2, false);

    verticalSplitter->setStretchFactor(0, 8);
    verticalSplitter->setStretchFactor(1, 1);
    verticalSplitter->setStretchFactor(2, 2);

    // Horizontal: verticalSplitter | tableView
    QSplitter *horizontalSplitter = new QSplitter(Qt::Horizontal, this);
    horizontalSplitter->addWidget(verticalSplitter);
    horizontalSplitter->addWidget(tableView);

    horizontalSplitter->setStretchFactor(0, 3);
    horizontalSplitter->setStretchFactor(1, 2);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(horizontalSplitter);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);
    setCentralWidget(widget);
    widget->setStyleSheet("background-color: #1E1E1E;");

    connect(compileButton, &QPushButton::clicked, this, &MainWindow::compileCode);
}

void MainWindow::compileCode()
{
    QString code = editor->toPlainText();

    Lexical lex;
    Syntactic syn;
    Semantic sem;
    sem.warnings.clear(); // Clear previous warnings

    lex.setInput(code.toStdString().c_str());

    try
    {
        syn.parse(&lex, &sem);

        // Check if a symbol was no used
        for (const auto &symbol : sem.symbolTable.getAllSymbols())
        {
            if (!symbol.isUsed)
            {
                sem.warnings.push_back("Warning: Variable '" + symbol.id + "' declared but not used.");
            }
        }

        // Show warnings if any
        QString resultText;
        if (!sem.warnings.empty())
        {
            for (const string &warning : sem.warnings)
            {
                resultText += "<span style='color:orange;'>" + QString::fromStdString(warning) + "</span><br>";
            }
            resultText += "<br>";
        }

        resultText += "<span style='color:lightgreen;'>Compiled successfully!</span>";

        output->setHtml(resultText);
        sem.symbolTable.printTable(); // Print the symbol table for debugging
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
}
