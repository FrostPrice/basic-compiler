// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "code_editor.hpp"

class QTextEdit;
class QTextBrowser;
class QPushButton;

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
};

#endif // MAINWINDOW_H
