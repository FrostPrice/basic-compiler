#include "main_window.hpp"
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // Set the window size
    resize(1000, 700); // Width: 1000 px, Height: 700 px

    editor = new CodeEditor(this);
    editor->setMinimumHeight(40);
    editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QFont font = editor->font();
    font.setPointSize(14);
    editor->setFont(font);
    editor->setStyleSheet(R"(
        background-color: #2D2A2E;
        color: #FCFCFA;
        font-family: 'Fira Code', 'Courier New', monospace;
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
            font-family: 'Fira Code', 'Courier New', monospace;
            border: 1px solid #403E41;
        }
    )");
    // output->setFixedHeight(120);

    QSplitter *verticalSplitter = new QSplitter(Qt::Vertical, this);
    verticalSplitter->setStyleSheet(R"(
        QSplitter::handle {
            background-color: #403E41;
            height: 4px;
        }
    )");

    verticalSplitter->addWidget(editor);
    verticalSplitter->addWidget(compileButton);
    verticalSplitter->addWidget(output);

    // Prevent collapsing
    verticalSplitter->setCollapsible(0, false); // Editor
    verticalSplitter->setCollapsible(1, false); // Button
    verticalSplitter->setCollapsible(2, false); // Output

    // Control initial proportions
    verticalSplitter->setStretchFactor(0, 8); // Editor
    verticalSplitter->setStretchFactor(1, 1); // Button
    verticalSplitter->setStretchFactor(2, 2); // Output

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(verticalSplitter);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);
    setCentralWidget(widget);
    widget->setStyleSheet("background-color: #1E1E1E;");

    editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    compileButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    output->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // Connect the button to the slot
    connect(compileButton, &QPushButton::clicked, this, &MainWindow::compileCode);
}

void MainWindow::compileCode()
{
    // Get the code from the editor
    QString code = editor->toPlainText();

    Lexical lex;
    Syntactic syn;
    Semantic sem;

    lex.setInput(code.toStdString().c_str());

    try
    {
        syn.parse(&lex, &sem);
        output->setText("Compiled successfully!");
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
