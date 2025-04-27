#ifndef SEMANTICTABLEMODEL_HPP
#define SEMANTICTABLEMODEL_HPP

#include <QStandardItemModel>

class SemanticTableModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit SemanticTableModel(QObject *parent = nullptr);

private:
    void populateModel();
};

#endif // SEMANTICTABLEMODEL_HPP
