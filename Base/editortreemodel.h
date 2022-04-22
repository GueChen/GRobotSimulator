#ifndef GCOMPONENT_EDITORTREEMODEL_H
#define GCOMPONENT_EDITORTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>

namespace GComponent {

class TreeItem;

class EditorTreeModel : public QAbstractItemModel
{
/// TypeAlias   类型别名
using _PtrItem      = std::unique_ptr<TreeItem>;
using _RawPtrItem   = TreeItem*;
using _Item         = TreeItem;
using _Ref          = EditorTreeModel &;
using _ConstRef     = const EditorTreeModel &;
Q_OBJECT;

/// Fields      数据域
private:
_PtrItem root_;

/// Constructor 构造函数
public:
explicit EditorTreeModel(const QString& data, QObject *parent = nullptr);
~EditorTreeModel()          = default;
/// NonCopyable 禁止拷贝
EditorTreeModel(_ConstRef)  = delete;
EditorTreeModel(_Ref)       = delete;

/// Methods     成员函数
// QAbstractItemModel interface
public:
QModelIndex
    index(int idx_child, int idx_data, const QModelIndex &parent) const override;
QModelIndex
    parent(const QModelIndex &child)                      const override;
int
    rowCount(const QModelIndex &parent)                   const override;
int
    columnCount(const QModelIndex &parent)                const override;
QVariant
    data(const QModelIndex &index, int role)              const override;
QVariant
    headerData(int section, Qt::Orientation orientation,
               int role)                                  const override;

private:
void
    setupModelData(const QString & data);
};

} // namespace GComponent

#endif // GCOMPONENT_EDITORTREEMODEL_H
