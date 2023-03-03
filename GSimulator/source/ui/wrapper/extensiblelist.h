/**
 *  @file  	extensiblelist.h
 *  @brief 	This is a ui Class for extensible item in a vertical direction.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 30, 2022
 **/
#ifndef _EXTENSIBLE_LIST_H
#define _EXTENSIBLE_LIST_H

#include "base/global/global_qss.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>

#include <optional>
#include <list>

namespace GComponent
{

class ExtensibleList : public QWidget
{
public:
	explicit ExtensibleList(QWidget* parent = nullptr);
	~ExtensibleList() = default;

	void	 SetProtoTypeName(const QString& proto_name);

public slots:
	bool	 AddItem();
	bool	 RemoveItem();

private:
	std::list<QWidget*>			items_ = {};
	std::optional<QWidget>		prototype_ = std::nullopt;
};

}
#endif // !_EXTENSIBLE_H


