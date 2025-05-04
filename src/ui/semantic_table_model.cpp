#include "semantic_table_model.hpp"
#include <vector>
#include <QStringList>

SemanticTableModel::SemanticTableModel(QObject *parent)
    : QStandardItemModel(parent)
{
}

QStandardItem *createTableItem(const QString &text)
{ // criar os itens da tabela
    QStandardItem *item = new QStandardItem(text);
    item->setTextAlignment(Qt::AlignCenter); // cenrtaliza o texto
    return item;
}

void SemanticTableModel::populateModel(vector<SymbolTable::SymbolInfo> symbols)
{
    QStringList headers = {
        "Id", "Type", "Scope", "Classification", "Is Initialized", "Is Used", "Array Size"}; // colunas da tabela
    setHorizontalHeaderLabels(headers);

    for (int col = 0; col < headers.size(); ++col)
    {
        QStandardItem *headerItem = horizontalHeaderItem(col); // pega o item do cabeçalho
        if (headerItem)
        {
            headerItem->setForeground(QBrush(QColor(255, 255, 255))); // cor branca
        }
    }

    for (const auto &symbol : symbols)
    {
        QList<QStandardItem *> rowItems;

        rowItems.append(createTableItem(QString::fromStdString(symbol.id)));
        rowItems.append(createTableItem(dataTypeToString(symbol.dataType)));
        rowItems.append(createTableItem(QString::number(symbol.scope)));
        rowItems.append(createTableItem(classificationToString(symbol.symbolClassification)));
        rowItems.append(createTableItem(symbol.isInitialized ? "true" : "false"));
        rowItems.append(createTableItem(symbol.isUsed ? "true" : "false"));
        rowItems.append(createTableItem(formatArraySize(symbol.arraySize)));

        appendRow(rowItems);
    }
}
