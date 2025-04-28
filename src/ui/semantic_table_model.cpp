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
      QList<QStandardItem*> rowItems;
      rowItems.append(new QStandardItem(QString::fromStdString(entry.token)));
      rowItems.append(new QStandardItem(QString::fromStdString(entry.tipo)));
      rowItems.append(new QStandardItem(QString::number(entry.valor)));

      appendRow(rowItems);
  }
}

