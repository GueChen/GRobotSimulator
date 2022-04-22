#include "modelmanagerview.h"

ModelManagerView::ModelManagerView(QWidget *parent) :
    QTreeView(parent)
{
    // setModel(new StringTree{});
    // ((StringTree *)model())->appendChild(new TreeItem(QList<string>({"one", "two"})));
}
