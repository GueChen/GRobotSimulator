#ifndef GCOMPONENT_TREEITEM_H
#define GCOMPONENT_TREEITEM_H

#include <QVariant>
#include <vector>

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
_RawPtr          parent_;

/// Constructer 构造函数
public:
explicit TreeItem(const vector<QVariant> & datas, _RawPtr parent = nullptr);
~TreeItem()                 = default;
/// Non Copyable 禁止拷贝
TreeItem(_ConstRef)         = delete;
_Ref operator=(_ConstRef)   = delete;

/// Methods     成员函数
public:
void     ApeendChild(_RawPtr child);
_RawPtr  GetChild(int idx);
_RawPtr  GetParent();
QVariant GetData(int idx)           const;
int      IndexInParent()            const;
int      ChildrenSize()             const;
int      DataSize()                 const;

private:
int      GetChildIndex(_RawPtr child);

};

} // namespace GComponent

#endif // GCOMPONENT_TREEITEM_H
