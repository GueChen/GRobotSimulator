﻿/**
 *  @file  	component.hpp
 *  @brief 	This class is a basic component class, supply a standard interfaces.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Apr xx, 2022
 **/
#ifndef _COMPONENT_HPP 
#define _COMPONENT_HPP

#include <string>
#include <map>
#include <functional>

namespace GComponent {

using std::vector;
using std::function;
using std::string_view;

class Model;

class Component {
public:
using _UpdateFun = function<void(float)>;
using _DelFun	 = function<void(void)>;

/// Methods 成员函数
public:
// Constructor & Destructors		
	explicit		Component(Model* ptr_parent) : ptr_parent_(ptr_parent) {}
	virtual			~Component() {
		// derived class 
		Derigistered();
		// external class
		for (auto& [key, func] : delete_functions_) {
			func();
		}
		ptr_parent_ = nullptr;
	}

// The Public Interface to Invoke the Component
	void			tick(float delta) {
		for (auto& [key, func] : update_functions_) {
			func(delta);
		}
		tickImpl(delta);
	}

	void			RegisterDelFunction(const std::string& key, const _DelFun& fun) 
					{ delete_functions_.emplace(key, fun); }
	void			ClearDelFunction()												
					{ delete_functions_.clear(); }
	void			DeregisterDelFunction(const std::string& key)					
					{ delete_functions_.erase(key); }
	
	void			RegisterUpdateFunction(const std::string& key, const _UpdateFun& fun) 
					{ update_functions_.emplace(key, fun); }
	void			ClearUpdateFunction()
					{ update_functions_.clear(); }
	void			DeregisterUpdateFunction(const std::string& key)					  
					{ update_functions_.erase(key); }

// Getter & Setter		
	inline void		SetParent(Model* ptr_parent)				  { ptr_parent_ = ptr_parent; }
	inline Model*	GetParent() const							  { return ptr_parent_;}

// Virtual Base Member Functions
	virtual const string_view&
					GetTypeName() const = 0;	
	virtual void	Derigistered()		  {}

protected:
	virtual void	tickImpl(float delta) {}

/// Fields 数据域
protected:
	Model* ptr_parent_ = nullptr;

private:
	std::map<std::string, _UpdateFun> update_functions_;
	std::map<std::string, _DelFun>    delete_functions_;

};

}

#endif // !_COMPONENT_HPP 

