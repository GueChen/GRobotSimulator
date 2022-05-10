#include "editortreemodel.h"

#include "function/stringprocessor.hpp"

#include <stack>

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG

constexpr size_t DELETED_PTR = 0xdddddddddddddddd;

namespace GComponent {

using std::stack;
using std::move;
using std::make_unique;

EditorTreeModel::EditorTreeModel(const QString &data, QObject * parent):
    QAbstractItemModel(parent)
{
    root_ = make_unique<_Item>(vector<QVariant>{tr("Name")});
    setupModelData(data);
}

QModelIndex EditorTreeModel::index(int idx_child, int idx_data, const QModelIndex &parent) const
{
    // 无指定数据
    if(!hasIndex(idx_child, idx_data, parent)){
        return QModelIndex();
    }

    // 处理树结构
    _RawPtrItem ptr_parent = getItem(parent);
    if (!ptr_parent) return QModelIndex();

    _RawPtrItem ptr_child = ptr_parent->GetChild(idx_child);
    if(ptr_child){
        return createIndex(idx_child, idx_data, ptr_child);
    }

    return QModelIndex();
}

QModelIndex EditorTreeModel::parent(const QModelIndex &child) const
{
    if(!child.isValid()){
        return QModelIndex();
    }

    _RawPtrItem ptr_child   = getItem(child);
    _RawPtrItem ptr_parent  = ptr_child && ptr_child->DataSize() ? 
                              ptr_child->GetParent() : nullptr;
    if(ptr_parent == root_.get() || !ptr_parent || !ptr_parent->ChildrenSize()){
        return QModelIndex();
    }

    return createIndex(ptr_parent->IndexInParent(), 0, ptr_parent);
}

int EditorTreeModel::rowCount(const QModelIndex &parent) const
{
    if(parent.column() > 0){
        return 0;
    }
    _RawPtrItem ptr_parent = getItem(parent);
    
    return ptr_parent? ptr_parent->ChildrenSize() : 0;
}

int EditorTreeModel::columnCount(const QModelIndex &parent) const
{
    _RawPtrItem ptr_parent = getItem(parent);    
    return ptr_parent? ptr_parent->DataSize(): root_->DataSize();
}

QVariant EditorTreeModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole)){
        return QVariant();
    }
    _RawPtrItem ptr_item = getItem(index);
    return ptr_item->GetData(index.column());
}

QVariant EditorTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole){
        return root_->GetData(section);
    }
    return QVariant();
}

Qt::ItemFlags EditorTreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return  Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

bool EditorTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole) return false;
    _RawPtrItem ptr_item = getItem(index);
    bool result = ptr_item->SetData(index.column(), value);
    
    if (result) {
        emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    }
    
    return false;
}

bool EditorTreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal) return false;    
    
    const bool result = root_->SetData(section, value);
    if (result) emit headerDataChanged(orientation, section, section);

    return result;
}

bool EditorTreeModel::insertColumns(int position, int columns, const QModelIndex& parent)
{
    beginInsertColumns(parent, position, position + columns - 1);
    const bool result = root_->InsertDataTypes(position, columns);
    endInsertColumns();

    return result;
}

bool EditorTreeModel::removeColumns(int position, int columns, const QModelIndex& parent)
{
    if (position < 0 || position + columns > root_->DataSize()) return false;
    beginRemoveColumns(parent, position, position + columns - 1);
    const bool result = root_->RemoveDataTypes(position, columns);
    endRemoveColumns();

    return result;
}

bool EditorTreeModel::insertRows(int position, int rows, const QModelIndex& parent)
{
    _RawPtrItem ptr_parent = getItem(parent);
    if (!ptr_parent) return false;

    beginInsertRows(parent, position, position + rows - 1);
    const bool result = ptr_parent->InsertChildren(position, rows, root_->DataSize());
    endInsertRows();

    return result;
}

bool EditorTreeModel::removeRows(int position, int rows, const QModelIndex& parent)
{
    _RawPtrItem ptr_parent = getItem(parent);
    if (!ptr_parent) return false;
    if (position < 0 || position + rows > ptr_parent->ChildrenSize()) return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool result = ptr_parent->RemoveChildren(position, rows);
    endRemoveRows();

    return result;
}

EditorTreeModel::_RawPtrItem EditorTreeModel::getItem(const QModelIndex& index) const
{
    if (index.isValid()) 
    {
        _RawPtrItem ptr_item = static_cast<_RawPtrItem>(index.internalPointer());
        if (ptr_item) return ptr_item;
    }

    return root_.get();
}

EditorTreeModel::_RawPtrItem EditorTreeModel::getItemByName(const string& name) const
{
    _RawPtrItem ptr_item = root_->SearchItemByData(QVariant(QString::fromStdString(name)));
    return ptr_item;
}

void EditorTreeModel::setupModelData(const QString& data)
{
    stack<_RawPtrItem> node_stack;
    stack<int>         count_stack;

    if (root_->ChildrenSize())
    {
        beginRemoveRows(QModelIndex(), 0, 0 + root_->ChildrenSize());
        root_->ClearChildren();
        endRemoveRows();
    }
    node_stack.push(root_.get());
    count_stack.push(0);

    QStringList datas = data.split("\n", Qt::SkipEmptyParts);

    for(auto & data_line: datas){
        // Process Tree Structure
        size_t pos = data_line.toStdString().find_first_not_of('\t');     
        while (pos + 1 <= count_stack.top() && !node_stack.empty()) {
            node_stack.pop();
            count_stack.pop();
        }

        // Insert Datas
        _RawPtrItem cur_node  = node_stack.top();        
        QStringList vals      = data_line.mid(pos).trimmed().split('\t', Qt::SkipEmptyParts);
        vector<QVariant> input(vals.begin(), vals.end());
        TreeItem* new_node = new TreeItem(input, cur_node);
        cur_node->ApeendChild(new_node);
        node_stack.push(new_node);     
        count_stack.push(pos + 1);     

#ifdef _DEBUG
        std::cout << std::format("Line:[{: <100}]\npos:[{: >2}] | curTop:[{: >2}]\n", data_line.toStdString(), pos, count_stack.top());
#endif // _DEBUG
    }
}

void EditorTreeModel::ResponseDeleteRequest(const string& delete_item_name)
{
    _RawPtrItem ptr_item = getItemByName(delete_item_name);
    if (ptr_item) 
    {
        _RawPtrItem ptr_parent = ptr_item->GetParent();
        if (ptr_parent)
        {
            removeRow(ptr_item->IndexInParent(), createIndex(ptr_parent->IndexInParent(), 0, ptr_parent));
        }        
    }
#ifdef _DEBUG
    std::cout << "The Tree Model Receive the Delete Request!\n";
#endif // _DEBUG
}

} // namespace GComponent
