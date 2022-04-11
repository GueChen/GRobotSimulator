#ifndef GCOMPONENT_EDITORTREEMODEL_H
#define GCOMPONENT_EDITORTREEMODEL_H

#include <QAbstractItemModel>
#include <QObject>

namespace GComponent {

class EditorTreeModel : public QAbstractItemModel
{
public:
    explicit EditorTreeModel(QObject *parent = nullptr);
};

} // namespace GComponent

#endif // GCOMPONENT_EDITORTREEMODEL_H
