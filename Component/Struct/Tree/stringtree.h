#ifndef STRINGTREE_H
#define STRINGTREE_H

#include <QAbstractItemModel>
#include <QModelIndex>


namespace GComponent {
    using std::string;

    class TreeItem;

    class StringTree: public QAbstractItemModel
    {
    public:
        explicit StringTree();
        ~StringTree();
        QVariant data(const QModelIndex & index, int role) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &index) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        void appendChild(TreeItem * data);
    private:
        TreeItem * _root;
    };
}


#endif // STRINGTREE_H
