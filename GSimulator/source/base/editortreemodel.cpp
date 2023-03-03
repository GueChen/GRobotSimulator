#include "editortreemodel.h"

#include "function/stringprocessor.hpp"
#include "ui/treeview/glmodeltreeview.h"

#include <stack>
#include <queue>

#include <QtCore/QMimeData>

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG

constexpr size_t DELETED_PTR = 0xdddddddddddddddd;
constexpr size_t MaxDataSize = 100;

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
    _RawPtrItem ptr_parent  = ptr_child && ptr_child->DataSize() && ptr_child->DataSize() < MaxDataSize ?
                              ptr_child->GetParent() : nullptr;
    if(ptr_parent == root_.get() || !ptr_parent || !ptr_parent->ChildrenSize() || ptr_parent->DataSize() > MaxDataSize){
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
    return ptr_item && ptr_item->DataSize() && ptr_item->DataSize() < MaxDataSize? 
           ptr_item->GetData(index.column()) : QVariant();
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
    return  QAbstractItemModel::flags(index) |
        Qt::ItemIsDragEnabled |
        Qt::ItemIsDropEnabled;
}

bool EditorTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole) return false;
    _RawPtrItem ptr_item = getItem(index);
    bool result = ptr_item->SetData(index.column(), value);
    
    if (result) {
        emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    }
    
    return result;
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

    emit layoutAboutToBeChanged();
    beginInsertRows(parent, position, position + rows - 1);
    const bool result = ptr_parent->InsertChildren(position, rows, root_->DataSize());
    endInsertRows();
    emit layoutChanged();

    return result;
}

bool EditorTreeModel::insertRows(const vector<vector<QVariant>>& datas, int position, const QModelIndex& parent)
{
    _RawPtrItem ptr_parent = getItem(parent);
    if (!ptr_parent) return false;
    for (auto& data : datas) if (data.size() != root_->DataSize()) return false;    
    
    emit layoutAboutToBeChanged();
    beginInsertRows(parent, position, position + datas.size() - 1);
    const bool result = ptr_parent->InsertChildren(datas, position);
    endInsertRows();
    emit layoutChanged();

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

QModelIndex EditorTreeModel::getIndexByItem(_RawPtrItem item)
{
    return item ? createIndex(item->IndexInParent(), 0, item) : QModelIndex();;
}

QModelIndex EditorTreeModel::getIndexByName(const string& name)
{
    if (name.empty()) return QModelIndex();
    return getIndexByItem(getItemByName(name));    
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

void EditorTreeModel::removeData(const string& delete_item_name)
{
    _RawPtrItem ptr_item = getItemByName(delete_item_name);
    if (ptr_item)
    {
        _RawPtrItem ptr_parent = ptr_item->GetParent();
        if (ptr_parent)
        {
            removeRow(ptr_item->IndexInParent(), createIndex(ptr_parent->IndexInParent(), 0, ptr_parent));
        }
        emit DataDeletedNotice();
    }
}

QStringList EditorTreeModel::mimeTypes() const
{
    QStringList types;
    types << QStringLiteral("application/editor_tree_msg");
    return types;
}

QMimeData* EditorTreeModel::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.count() == 0) return nullptr;
    QStringList types = mimeTypes();
    QMimeData*  data = new QMimeData();
    QByteArray  encoded;
    QDataStream stream(&encoded, QDataStream::WriteOnly);
    for (auto& index : indexes) {
        stream << getItem(index)->GetData(0);
    }
    data->setData(types.at(0), encoded);
    return data;
}

bool EditorTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{   
    QStringList format = mimeTypes();
    if (!data->hasFormat(format.at(0))) return false;
    if (row > rowCount(parent)) row = rowCount(parent);
    if (row == -1)              row = rowCount(parent);
    if (column == -1)           column = 0;
    QByteArray encoded   = data->data(format.at(0));
    QDataStream stream   = QDataStream(&encoded, QDataStream::ReadOnly);    
    _RawPtrItem ptr_p    = getItem(parent);
    auto insert_children = [this](auto && insert, _RawPtrItem ptr, const QModelIndex& parent)->void {       
        int row = 0;
        for (auto& child : ptr->GetChildren()) {
            insertRow(child->GetDatas(), row, parent);
            insert(insert, child.get(), getIndexByItem(getItem(parent)->GetChild(row)));
            ++row;
        }        
    };
    while (!stream.atEnd()) {
        QVariant    name;
        stream  >>  name;
        _RawPtrItem ptr_item = getItemByName(name.toString().toStdString());
        insertRow({name}, row, parent);
        insert_children(insert_children, ptr_item, getIndexByItem(getItem(parent)->GetChild(row)));
        emit ParentChangeRequest(name.toString().toStdString(), 
                                 ptr_p == root_.get()? 
                                 "@__ROOT__" : ptr_p->GetData(0).toString().toStdString());
    } 
    return true;
}

Qt::DropActions EditorTreeModel::supportedDropActions() const
{
    return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
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
        TreeItem* new_node    = new TreeItem(input, cur_node);
        cur_node->ApeendChild(new_node);
        node_stack.push(new_node);     
        count_stack.push(pos + 1);     

#ifdef _DEBUG
        std::cout << std::format("Line:[{: <100}]\npos:[{: >2}] | curTop:[{: >2}]\n", data_line.toStdString(), pos, count_stack.top());
#endif // _DEBUG
    }
}
/*_________________________________________SLOT FUNCTIONS_____________________________________________*/
void EditorTreeModel::ResponseDeleteRequest(const string& name)
{
    removeData(name);
}

void EditorTreeModel::ResponseCreateRequest(const string& name, const string& parent_name)
{
    insertRow(vector<QVariant>{QString::fromStdString(name)}, 0, getIndexByName(parent_name));
}

void EditorTreeModel::ResponseParentChangeRequest(const string& name, const string& parent_name)
{
    TreeItem* item_ptr = static_cast<_RawPtrItem>(getIndexByName(name).internalPointer());
    if (!item_ptr) return;
    insertRow(item_ptr->GetDatas(), 0, getIndexByName(parent_name));
    /* BFS traversal the tree item */
    std::queue<_RawPtrItem> item_queue;
    item_queue.push(item_ptr);
    while (!item_queue.empty()) {
        _RawPtrItem cur_item = item_queue.front();
        item_queue.pop();
        string cur_name = cur_item->GetData(0).toString().toStdString();
        for (int count = 0; auto & child : cur_item->GetChildren()) {
            item_queue.push(child.get());
            insertRow(child->GetDatas(), count++, getIndexByName(cur_name));
        }
    }
    removeRow(item_ptr->IndexInParent(), getIndexByName(item_ptr->GetParent()->GetData(0).toString().toStdString()));
}


} // namespace GComponent
