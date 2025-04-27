#include "semantic_table_model.hpp"
#include "semantic_table.h"

#include <vector>

struct SemanticEntry {
    std::string token;
    std::string tipo;
    int valor;
};

std::vector<SemanticEntry> semanticTable; // declara a tabela


SemanticTableModel::SemanticTableModel(QObject *parent)
    : QStandardItemModel(parent)
{
    populateModel();
}

void SemanticTableModel::populateModel() {
  semanticTable.push_back({"token1", "INT", 10}); // exemplos pra tabela
  semanticTable.push_back({"token2", "FLOAT", 20});

  setHorizontalHeaderLabels({ "Token", "Tipo", "Valor" }); // colunas

  for (const auto& entry : semanticTable) {
      QList<QStandardItem*> rowItems;
      rowItems.append(new QStandardItem(QString::fromStdString(entry.token)));
      rowItems.append(new QStandardItem(QString::fromStdString(entry.tipo)));
      rowItems.append(new QStandardItem(QString::number(entry.valor)));

      appendRow(rowItems);
  }
}

