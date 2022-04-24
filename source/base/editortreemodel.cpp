#include "editortreemodel.h"

#include "function/stringprocessor.hpp"
#include "base/treeitem.h"

#include <iostream>
#include <format>
#include <stack>

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
    _RawPtrItem ptr_parent;
    if(!parent.isValid()){
        ptr_parent = root_.get();
    }
    else{
        ptr_parent = static_cast<_RawPtrItem>(parent.internalPointer());
    }

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

    _RawPtrItem ptr_child   = static_cast<_RawPtrItem>(child.internalPointer());
    _RawPtrItem ptr_parent  = ptr_child->GetParent();

    if(ptr_parent == root_.get()){
        return QModelIndex();
    }

    return createIndex(ptr_parent->IndexInParent(), 0, ptr_parent);
}

int EditorTreeModel::rowCount(const QModelIndex &parent) const
{
    if(parent.column() > 0){
        return 0;
    }
    _RawPtrItem ptr_parent;
    if(!parent.isValid()){
        ptr_parent = root_.get();
    }
    else{
        ptr_parent = static_cast<_RawPtrItem>(parent.internalPointer());
    }

    return ptr_parent->ChildrenSize();
}

int EditorTreeModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid()){
        return static_cast<_RawPtrItem>(parent.internalPointer())->DataSize();
    }
    return root_->DataSize();
}

QVariant EditorTreeModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || role != Qt::DisplayRole){
        return QVariant();
    }

    _RawPtrItem ptr_item = static_cast<_RawPtrItem>(index.internalPointer());
    return ptr_item->GetData(index.column());
}

QVariant EditorTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole){
        return root_->GetData(section);
    }
    return QVariant();
}

void EditorTreeModel::setupModelData(const QString &data)
{
    stack<_RawPtrItem> node_stack;
    stack<int>         count_stack;

    node_stack.push(root_.get());
    count_stack.push(0);

    QStringList datas = data.split("\n", Qt::SkipEmptyParts);

    for(auto & data_line: datas){
        // Process Tree Structure
        size_t      pos      = data_line.toStdString().find_first_not_of(' ');

        _RawPtrItem cur_node = node_stack.top();
        if(pos > count_stack.top()){
            if(int child_size = cur_node->ChildrenSize(); child_size > 0){
                node_stack.push(cur_node->GetChild(child_size - 1));
                cur_node = node_stack.top();
                count_stack.push(pos);
            }
            else{
                while(pos < count_stack.top() && !node_stack.empty()){
                    node_stack.pop();
                    count_stack.pop();
                }
            }
        }
        // Insert Datas
        QStringList      vals = data_line.mid(pos).trimmed().split('\t', Qt::SkipEmptyParts);
        vector<QVariant> input(vals.begin(), vals.end());
        cur_node->ApeendChild(new TreeItem(input, cur_node));
    }
}

} // namespace GComponent
