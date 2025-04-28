#include "semantic_table_model.hpp"
#include <vector>
#include <QStringList>

SemanticTableModel::SemanticTableModel(QObject *parent)
    : QStandardItemModel(parent)
{
    populateModel();
}

QStandardItem* createTableItem(const QString& text) { // criar os itens da tabela
    QStandardItem* item = new QStandardItem(text);
    item->setTextAlignment(Qt::AlignCenter);  // cenrtaliza o texto
    return item;
}

void SemanticTableModel::populateModel() {
    QStringList headers = { 
        "Token", "Tipo", "Valor", "Marca", "Modelo"
    };  // colunas da tabela
    setHorizontalHeaderLabels(headers);

    for (int col = 0; col < headers.size(); ++col) {
        QStandardItem* headerItem = horizontalHeaderItem(col);  // pega o item do cabeçalho
        if (headerItem) {
            headerItem->setForeground(QBrush(QColor(255, 255, 255))); // cor branca
        }
    }

    // dados de exemplo por enquanto
    std::vector<QStringList> data = {
        {"token1", "INT", "10", "1", "1"},
        {"token2", "FLOAT", "20", "2", "5"},
        {"token3", "DOUBLE", "30.5", "3", "10"},
        {"token4", "NUMBER", "40", "4", "15"},
        {"token5", "FLOAT", "50.75", "5", "20"},
        {"token6", "INT", "60", "6", "25"},
        {"token7", "DOUBLE", "70.25", "7", "30"},
        {"token8", "NUMBER", "80", "8", "35"},
        {"token9", "FLOAT", "90.5", "9", "40"},
        {"token10", "INT", "100", "10", "45"}
    };

    // cria as linhas da tabela
    for (const auto& row : data) {
        QList<QStandardItem*> rowItems;
        
        for (const auto& cellData : row) {
            rowItems.append(createTableItem(cellData));
        }

        appendRow(rowItems);
    }
}
