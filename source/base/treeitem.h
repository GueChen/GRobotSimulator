/**
 *  @file  	treeitem.h
 *  @brief 	Reference from editabletree item, a basic tree node class.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 9th, 2022
 **/
#ifndef GCOMPONENT_TREEITEM_H
#define GCOMPONENT_TREEITEM_H

#include <QtCore/QVariant>

#include <vector>
#include <memory>

namespace GComponent {

using std::vector;

class TreeItem
{
/// TypeAlias   类型别名
using _Self     = TreeItem;
using _Ref      = TreeItem &;
using _RawPtr   = TreeItem *;
using _Ptr      = std::unique_ptr<TreeItem>;
using _ConstRef = const TreeItem &;

/// Fields      数据域
private:
    vector<QVariant> datas_;
    vector<_Ptr>     children_;
    _RawPtr          parent_    = nullptr;

/// Constructer 构造函数
public:
    explicit TreeItem(const vector<QVariant> & datas, _RawPtr parent = nullptr);
    ~TreeItem();
/// Non Copyable 禁止拷贝
    TreeItem(_ConstRef)         = delete;
    _Ref operator=(_ConstRef)   = delete;

/// Methods     成员函数
public:
    void     ApeendChild(_RawPtr child);
    void     EraseChild(int idx);
    void     ClearChildren();
    bool     SetData(int idx, const QVariant& value);
    bool     InsertDataTypes(int type_pos, int type_count);
    bool     InsertChildren(int child_pos, int child_count, int type_count);
    bool     RemoveDataTypes(int type_pos, int type_count);
    bool     RemoveChildren(int child_pos, int child_count);

    _RawPtr  SearchItemByData(const QVariant& data);

    inline _RawPtr  GetChild(int idx)   const { return idx <= children_.size() && idx >= 0? children_[idx].get(): nullptr; }
    inline _RawPtr  GetParent()         const { return parent_; }
    inline QVariant GetData(int idx)    const { return idx <= datas_.size() && idx >= 0? datas_[idx]: QVariant(); }
    inline int      IndexInParent()     const { return parent_ ? parent_->GetChildIndex(const_cast<_RawPtr>(this)) : -1; }
    inline int      ChildrenSize()      const { return children_.size(); }
    inline int      DataSize()          const { return datas_.size(); }

private:
    int      GetChildIndex(_RawPtr child);

};

} // namespace GComponent

#endif // GCOMPONENT_TREEITEM_H
