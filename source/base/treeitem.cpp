#include "base/treeitem.h"

using std::make_unique;

namespace GComponent {

TreeItem::TreeItem(const vector<QVariant> &datas, _RawPtr parent):
    datas_(datas), parent_(parent)
{}

TreeItem::~TreeItem() {
    datas_.clear();
    ClearChildren();
}

void TreeItem::ApeendChild(_RawPtr child)
{
    child->parent_ = this;
    children_.emplace_back(_Ptr(child));
}

void TreeItem::ApeendChild(const vector<QVariant>& datas)
{
    if (datas.size() != datas_.size()) return;
    ApeendChild(new _Self(datas, this));
}

void TreeItem::EraseChild(int idx)
{
    if (children_.empty() || idx >= children_.size() || idx < 0) return;
    children_.erase(children_.begin() + idx);
}

void TreeItem::ClearChildren()
{
    for (auto& child : children_) {
        child->parent_ = nullptr;
    }
    children_.clear();
}

bool TreeItem::SetData(int idx, const QVariant& value)
{
    if (idx < 0 || idx >= datas_.size()) return false;
    datas_[idx] = value;
    return true;
}

bool TreeItem::InsertDataTypes(int type_pos, int type_count)
{
    if (type_pos < 0 || type_pos > datas_.size()) return false;
    assert(type_count > 0);
    datas_.insert(datas_.end(), type_count, QVariant());
    
    for (auto& child : children_) {
        child->InsertDataTypes(type_pos, type_count);
    }

    return true;
}

bool TreeItem::InsertChildren(int child_pos, int child_count, int type_count)
{
    if (child_pos < 0 || child_pos > children_.size()) return false;
    
    for (int i = 0; i < child_count; ++i) {
        children_.insert(children_.begin() + child_pos, make_unique<_Self>(vector<QVariant>(type_count), this));
    }
    return true;
}

bool TreeItem::InsertChildren(const vector<vector<QVariant>>& children_datas, int child_pos)
{
    if (child_pos < 0 || child_pos > children_.size()) return false;    
    for (auto& child_datas : children_datas)
    {
        children_.insert(children_.begin() + child_pos, make_unique<_Self>(child_datas, this));
        ++child_pos;
    }
    return true;
}

bool TreeItem::RemoveDataTypes(int type_pos, int type_count)
{
    if (type_pos < 0 || type_pos + type_count > datas_.size()) return false;
    datas_.erase(datas_.begin() + type_pos, datas_.begin() + type_pos + type_count);
    for (auto& child : children_) {
        child->RemoveDataTypes(type_pos, type_count);
    }
    return true;
}

bool TreeItem::RemoveChildren(int child_pos, int child_count)
{
    if (child_pos  < 0 || child_pos + child_count > children_.size()) return false;
    children_.erase(children_.begin() + child_pos, children_.begin() + child_pos + child_count);
    return true;
}

TreeItem::_RawPtr TreeItem::SearchItemByData(const QVariant& data)
{
    // TODO: 使用树结构的 map 改善 traversal 性能
    if (datas_.front() == data) return this;
    _RawPtr ret_ptr = nullptr;
    for (auto& child : children_) 
    {
        ret_ptr = child->SearchItemByData(data);
        if(ret_ptr)
        {
            return ret_ptr;
        }
    }
    return ret_ptr;
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
