/**
 *  @file  	editortreemodel.h
 *  @brief 	A Tree Model use TreeItem class.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 9th, 2022
 **/
#ifndef GCOMPONENT_EDITORTREEMODEL_H
#define GCOMPONENT_EDITORTREEMODEL_H

#include "base/treeitem.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QModelIndex>
#include <QtCore/QObject>

#include <string>

namespace GComponent {
using std::string;

class EditorTreeModel : public QAbstractItemModel
{
/// TypeAlias   类型别名
using _PtrItem      = std::unique_ptr<TreeItem>;
using _RawPtrItem   = TreeItem*;
using _Item         = TreeItem;
using _Ref          = EditorTreeModel &;
using _ConstRef     = const EditorTreeModel &;
Q_OBJECT;
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
/// Getter Methods 
QModelIndex
    index(int idx_child, int idx_data, const QModelIndex &parent) const override;
QModelIndex
    parent(const QModelIndex &child)                              const override;
int
    rowCount(const QModelIndex &parent)                           const override;
int
    columnCount(const QModelIndex &parent)                        const override;
QVariant
    data(const QModelIndex &index, int role)                      const override;
QVariant
    headerData(int section, Qt::Orientation orientation,
               int role)                                          const override;
Qt::ItemFlags 
    flags(const QModelIndex& index)                               const override;

/// Setter Methods
bool
    setData(const QModelIndex & index, const QVariant & value, 
            int role = Qt::EditRole)                                    override;
bool
    setHeaderData(int section, Qt::Orientation orientation,
                  const QVariant & value, int role = Qt::EditRole)      override;
bool 
    insertColumns(int position, int columns, 
                  const QModelIndex & parent = QModelIndex())           override;
bool
    removeColumns(int position, int columns,
                  const QModelIndex & parent = QModelIndex())           override;
bool 
    insertRows(int position, int rows,
               const QModelIndex & parent = QModelIndex())              override;
bool 
    insertRows(const vector<vector<QVariant>>& datas,
               int position,
               const QModelIndex & parent = QModelIndex());
inline bool
    insertRow(const vector<QVariant>& data, int position,
              const QModelIndex& parent = QModelIndex()) { return insertRows({data}, position, parent); }
bool
    removeRows(int position, int rows,
               const QModelIndex & parent = QModelIndex())              override;

QModelIndex
    getIndexByName(const string& name);
void 
    removeData(const string& delete_item_name);

protected:
_RawPtrItem
    getItem(const QModelIndex & index) const;
_RawPtrItem
    getItemByName(const string& name)  const;

void
    setupModelData(const QString & data);

public slots:
    void ResponseDeleteRequest(const string& name);
    void ResponseCreateRequest(const string& name, const string& parent_name);
    void ResponseParentChangeRequest(const string& name, const string& parent_name);

signals:
    void DataDeletedNotice();

/// Fields      数据域
private:
    _PtrItem root_ = nullptr;
};

} // namespace GComponent

#endif // GCOMPONENT_EDITORTREEMODEL_H
