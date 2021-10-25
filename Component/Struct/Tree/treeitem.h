#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>

namespace GComponent {
    using std::string;
    class TreeItem
    {
    public:
        TreeItem(const QList<string> &data, TreeItem * parent = nullptr);
        ~TreeItem();

        void appendChild(TreeItem* child);

        int childCount() const;
        int columnCount() const;
        string data(int column) const;
        int row() const;
        TreeItem * GetParent();
        TreeItem * GetChild(int row);
    private:
        QList<TreeItem *> _children;
        QList<string> _data;
        TreeItem * _parent;

    };
}


#endif // TREEITEM_H
