#include "stringtree.h"
#include "Component/Struct/Tree/treeitem.h"

using namespace GComponent;

StringTree::StringTree()
{

}

StringTree::~StringTree()
{
    delete _root;
}

int StringTree::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    return _root->columnCount();
}

QVariant StringTree::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(role != Qt::DisplayRole)
        return QVariant();

    TreeItem * item = static_cast<TreeItem*>(index.internalPointer());
    // TODO: 思考这里的 Variant 到底要是什么数据
    return QVariant(QString::fromStdString(item->data(index.column())));
}

Qt::ItemFlags StringTree::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant StringTree::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QVariant(QString::fromStdString(_root->data(section)));
    return QVariant();
}

QModelIndex StringTree::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem * parentItem;

    if(!parent.isValid())
        parentItem = _root;
    else
        parentItem = static_cast<TreeItem *>(parent.internalPointer());

    TreeItem * childItem = parentItem->GetChild(row);
    if(childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex StringTree::parent(const QModelIndex & index) const
{
    if(!index.isValid())
        return QModelIndex();

    TreeItem * child = static_cast<TreeItem*>(index.internalPointer());
    TreeItem * parent= child->GetParent();

    if(parent == _root)
        return QModelIndex();

    return createIndex(parent->row(), 0, parent);
}

int StringTree::rowCount(const QModelIndex & _parent) const
{
    TreeItem * tempParent;
    if(_parent.column() > 0)
        return 0;

    if(!_parent.isValid())
        tempParent = _root;
    else
        tempParent = static_cast<TreeItem*>(_parent.internalPointer());

    return tempParent->childCount();
}

void StringTree::appendChild(TreeItem *data)
{
    _root->appendChild(data);
}
