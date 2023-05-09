/**
 *  @file  	component_ui_factory.h
 *  @brief 	This Class use to Extraction the Component data to UI.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 25, 2022
 **/
#ifndef _COMPONENT_UI_FACTORY_H
#define _COMPONENT_UI_FACTORY_H

#include "pattern/widgetbuilder.h"

#include <GComponent/GNumerical.hpp>

#include <string>
#include <unordered_map>
#include <functional>

#ifdef _DEBUG
#include <iostream>
#endif

namespace GComponent {

class Component;

class ComponentUIFactory {
public:
	ComponentUIFactory() = delete;
	// FIXME: 工厂负责生产不负责销毁，若无接收值发生内存泄漏
	[[nodiscard]]
	static QWidget* Create(Component& component); 

private:
	/*_______________________________The UI Table____________________________________________________*/
	//TODO: as below
	// std::unoredered_map<std::string, unique_ptr<UI_Builder>> ui_table;

	/*_________________________Material Component UI Create Methods__________________________________*/
public:
	static const std::string	kTag;
	static constexpr const int  kEleMiniHeight = 20;
	static std::unordered_map<std::string, std::function<QLayout*(std::string, ShaderProperty::Var&)>> build_map;
};
} 

#endif // !_COMPONENT_UI_FACTORY_H
