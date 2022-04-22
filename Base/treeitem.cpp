#include "treeitem.h"

namespace GComponent {

TreeItem::TreeItem(const vector<QVariant> &datas, _RawPtr parent):
    datas_(datas), parent_(parent)
{}

TreeItem::~TreeItem() = default;

void TreeItem::ApeendChild(_RawPtr child)
{
    children_.emplace_back(_Ptr(child));
}

TreeItem::_RawPtr TreeItem::GetChild(int idx)
{
    return children_[idx].get();
}

TreeItem::_RawPtr TreeItem::GetParent()
{
    return parent_;
}

QVariant TreeItem::GetData(int count) const
{
    return datas_[count];
}

int TreeItem::IndexInParent() const
{
    if(parent_){
        return parent_->GetChildIndex(const_cast<_RawPtr>(this));
    }
    return -1;
}

int TreeItem::ChildrenSize() const
{
    return children_.size();
}

int TreeItem::DataSize() const
{
    return datas_.size();
}

int TreeItem::GetChildIndex(_RawPtr child)
{
    for(int idx = 0; auto & ptr_child : children_){
        if(ptr_child.get() == child){
            return idx;
        }
        ++idx;
    }
    return -1;
}

} // namespace GComponent
