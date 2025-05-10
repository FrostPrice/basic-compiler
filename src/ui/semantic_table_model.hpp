#ifndef SEMANTICTABLEMODEL_HPP
#define SEMANTICTABLEMODEL_HPP

#include <QStandardItemModel>
#include <vector>

#include "../gals/SymbolTable.hpp"
#include "../gals/SemanticTable.hpp"

class SemanticTableModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit SemanticTableModel(QObject *parent = nullptr);

    void populateModel(vector<SymbolTable::SymbolInfo> symbols);

private:
    QString dataTypeToString(SemanticTable::Types type)
    {
        switch (type)
        {
        case SemanticTable::INT:
            return "int";
        case SemanticTable::FLOAT:
            return "float";
        case SemanticTable::DOUBLE:
            return "double";
        case SemanticTable::CHAR:
            return "char";
        case SemanticTable::STRING:
            return "string";
        default:
            return "unknown";
        }
    }

    QString classificationToString(SymbolTable::SymbolClassification classification)
    {
        switch (classification)
        {
        case SymbolTable::VARIABLE:
            return "Variable";
        case SymbolTable::FUNCTION:
            return "Function";
        case SymbolTable::ARRAY:
            return "Array";
        case SymbolTable::PARAM:
            return "Parameter";
        default:
            return "Unknown";
        }
    }

    QString formatArraySize(std::vector<int> arraySize)
    {
        if (arraySize.empty())
            return "-";

        QString result;
        for (int size : arraySize)
        {
            QString strSize = size == -1 ? "exp" : QString::number(size);
            result += "[" + strSize + "]";
        }
        return result;
    }
};

#endif // SEMANTICTABLEMODEL_HPP
