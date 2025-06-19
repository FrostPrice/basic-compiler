#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

#include "code_editor.hpp"
#include "semantic_table_model.hpp"
#include "../gals/Semantic.h"

class QTextEdit;
class QTextBrowser;
class QPushButton;
class QTableView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void compileCode();
    void validateSemantics(Semantic &sem);
    void validateMainFunction(Semantic &sem);
    void checkUnusedSymbols(Semantic &sem);
    void displayCompilationResults(Semantic &sem);
    QString buildWarningsText(const std::vector<std::string> warnings);

private:
    CodeEditor *editor;
    QTextBrowser *output;

    QTableView *tableView;
    QTextBrowser *assemblyOutput;

    SemanticTableModel *tableModel;
};

#endif // MAINWINDOW_H
