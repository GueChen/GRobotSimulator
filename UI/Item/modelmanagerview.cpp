#include "modelmanagerview.h"
#include "Component/Struct/Tree/stringtree.h"
#include "Component/Struct/Tree/treeitem.h"

using namespace GComponent;

ModelManagerView::ModelManagerView(QWidget *parent) :
    QTreeView(parent)
{
    // setModel(new StringTree{});
    // ((StringTree *)model())->appendChild(new TreeItem(QList<string>({"one", "two"})));
}
