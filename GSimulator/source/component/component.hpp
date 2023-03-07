/**
 *  @file  	component.hpp
 *  @brief 	This class is a basic component class, supply a standard interfaces.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Apr xx, 2022
 **/
#ifndef _COMPONENT_HPP 
#define _COMPONENT_HPP

#include <string>
#include <vector>
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
		for (auto& del_fun : delete_functions_)
		{
			del_fun();
		}
		ptr_parent_ = nullptr;
	}

// The Public Interface to Invoke the Component
	void			tick(float delta) {
		for (auto& update_fun : update_functions_)
		{
			update_fun(delta);
		}
		tickImpl(delta);
	}

	void			RegisterDelFunction(const _DelFun& fun)		  { delete_functions_.push_back(fun); }
	void			DeregisterDelFunction() { delete_functions_.clear(); }
	
	void			RegisterUpdateFunction(const _UpdateFun& fun) { update_functions_.push_back(fun); }
	void			DeregisterUpdateFunction()					  { update_functions_.clear(); }

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
	vector<_UpdateFun> update_functions_;
	vector<_DelFun>	   delete_functions_;
};

}

#endif // !_COMPONENT_HPP 

