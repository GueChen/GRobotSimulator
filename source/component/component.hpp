#ifndef _COMPONENT_HPP 
#define _COMPONENT_HPP

#include <string>

namespace GComponent {
using std::string_view;

class Model;

class Component {
protected:
	Model* ptr_parent_ = nullptr;

public:
// Constructor & Destructors		
	explicit		Component(Model* ptr_parent) : ptr_parent_(ptr_parent) {}
	virtual			~Component()				 { ptr_parent_ = nullptr; }		

// Getter & Setter		
	inline void		SetParent(Model* ptr_parent) { ptr_parent_ = ptr_parent; }
	inline Model*	GetParent() const			 { return ptr_parent_;}

// Virtual Base Member Functions
	virtual const string_view&
					GetTypeName() const = 0;
	virtual void	tick(float delta)  {}
	virtual void	Derigistered()	   {}
};

}

#endif // !_COMPONENT_HPP 

