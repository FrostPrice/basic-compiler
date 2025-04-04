#include "main_window.h"
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextBrowser>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    editor = new QTextEdit(this);
    editor->setFontPointSize(14);

    QPushButton *compileButton = new QPushButton("Compilar", this);

    output = new QTextBrowser(this);
    output->setFontPointSize(14);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(editor);
    layout->addWidget(compileButton);
    layout->addWidget(output);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);
    setCentralWidget(widget);

    // Connect the button to the slot
    connect(compileButton, &QPushButton::clicked, this, &MainWindow::compileCode);
}
void MainWindow::compileCode()
{
    // Placeholder implementation
    output->setText("Compilando... (função conectada com sucesso!)");
}
