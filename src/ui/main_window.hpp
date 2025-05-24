#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

#include "code_editor.hpp"
#include "semantic_table_model.hpp"

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

private:
    CodeEditor *editor;
    QTextBrowser *output;

    QTableView *tableView;
    QTextBrowser *assemblyOutput;

    SemanticTableModel *tableModel;
};

#endif // MAINWINDOW_H
