#include "code_editor.hpp"
#include <QPainter>
#include <QTextBlock>
#include <QTextCursor>
#include <QShortcut>
#include <QKeyEvent>

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(static_cast<QWidget *>(editor)), codeEditor(editor) {}

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    setViewportMargins(4, 2, 2, 2); // Set margins for the text area

    lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    QShortcut *commentShortcut = new QShortcut(this);
    commentShortcut->setKey(Qt::CTRL + Qt::Key_Slash);

    connect(commentShortcut, &QShortcut::activated, this, &CodeEditor::toggleLineComment);
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

    int space = 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor("#3E3D32"); // Dim background for current line

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor("#2D2A2E"));

    QFont font = this->font();
    font.setPointSize(14);
    painter.setFont(font);
    painter.setPen(QColor("#75715E"));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.drawText(0, top, lineNumberArea->width(), static_cast<int>(blockBoundingRect(block).height()),
                             Qt::AlignCenter, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::toggleLineComment()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    QTextCursor startCursor = cursor;
    startCursor.setPosition(cursor.selectionStart());
    QTextBlock startBlock = startCursor.block();

    QTextCursor endCursor = cursor;
    endCursor.setPosition(cursor.selectionEnd());
    QTextBlock endBlock = endCursor.block();

    for (QTextBlock block = startBlock; block != endBlock.next(); block = block.next())
    {
        QString lineText = block.text();

        if (lineText.isEmpty())
            continue;

        int position = block.position();
        cursor.setPosition(position);

        if (lineText.startsWith(" ") || lineText.startsWith("\t"))
        {
            cursor.movePosition(QTextCursor::NextWord);
            position = cursor.position();
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            lineText = cursor.selectedText();
        }

        if (lineText.startsWith("//"))
        {
            lineText.remove(0, 2);
            if (lineText.startsWith(" ") || lineText.startsWith("\t"))
                lineText.remove(0, 1);
        }
        else
        {
            lineText.prepend("// ");
        }

        cursor.setPosition(position);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.insertText(lineText);
    }

    cursor.endEditBlock();
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    codeEditor->lineNumberAreaPaintEvent(event);
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == (Qt::AltModifier | Qt::ShiftModifier))
    {
        if (event->key() == Qt::Key_Up)
        {
            duplicateLineOrSelection(-1);
            return;
        }
        else if (event->key() == Qt::Key_Down)
        {
            duplicateLineOrSelection(1);
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::duplicateLineOrSelection(int direction)
{
    QTextCursor cursor = textCursor();
    QString textToDuplicate;

    if (cursor.hasSelection())
    {
        int start = cursor.selectionStart();
        int end = cursor.selectionEnd();

        cursor.setPosition(start);
        cursor.movePosition(QTextCursor::StartOfBlock);
        int firstPos = cursor.position();

        cursor.setPosition(end, QTextCursor::KeepAnchor);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        int lastPos = cursor.position();

        textToDuplicate = cursor.selectedText();
        textToDuplicate.replace(QChar::ParagraphSeparator, "\n");

        cursor.beginEditBlock();

        if (direction > 0)
        {
            cursor.setPosition(lastPos);
            cursor.movePosition(QTextCursor::EndOfBlock);
            cursor.insertBlock();
            cursor.insertText(textToDuplicate);
        }
        else
        {
            cursor.setPosition(firstPos);
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.insertText(textToDuplicate + "\n");
        }

        cursor.endEditBlock();
    }
    else
    {
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        textToDuplicate = cursor.selectedText();

        cursor.beginEditBlock();

        if (direction > 0)
        {
            cursor.movePosition(QTextCursor::EndOfBlock);
            cursor.insertBlock();
            cursor.insertText(textToDuplicate);
        }
        else
        {
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.insertText(textToDuplicate + "\n");
        }

        cursor.endEditBlock();
    }
}