#include "treeitem.h"

using namespace GComponent;

TreeItem::TreeItem(const QList<string> &data, TreeItem * parent):
    _data(data), _parent(parent)
{}

TreeItem::~TreeItem()
{

}

void TreeItem::appendChild(TreeItem *child)
{
    _children.append(child);
}

TreeItem * TreeItem::GetChild(int row)
{
    if(row < 0 || row >= _children.size())
        return nullptr;
    return _children.at(row);
}

int TreeItem::childCount() const
{
    return _children.count();
}

int TreeItem::columnCount() const
{
    return _data.count();
}

string TreeItem::data(int column) const
{
    if(column < 0 || column >= _data.size())
        return string("Null");
    return _data.at(column);
}

TreeItem * TreeItem::GetParent()
{
    return _parent;
}

int TreeItem::row() const
{
    if(_parent)
        return _parent->_children.indexOf(const_cast<TreeItem*>(this));
    return 0;
}
