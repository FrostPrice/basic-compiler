#include "MainWindow.h"
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    auto *editor = new QTextEdit(this);
    editor->setFontPointSize(14);

    auto *compileButton = new QPushButton("Compilar", this);

    auto *output = new QTextBrowser(this);
    output->setFontPointSize(14);

    auto *layout = new QVBoxLayout;
    layout->addWidget(editor);
    layout->addWidget(compileButton);
    layout->addWidget(output);

    auto *widget = new QWidget(this);
    widget->setLayout(layout);
    setCentralWidget(widget);
}

void MainWindow::compileCode()
{
    // Placeholder implementation
    output->setText("Compilando... (função conectada com sucesso!)");
}
