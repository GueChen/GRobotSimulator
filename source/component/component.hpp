#ifndef _COMPONENT_HPP 
#define _COMPONENT_HPP

namespace GComponent {
class Model;

class Component {
protected:
	Model* ptr_parent_ = nullptr;

public:
// Constructor & Destructors		
	explicit		Component(Model* ptr_parent) : ptr_parent_(ptr_parent) {}
	virtual			~Component()				 { ptr_parent_ = nullptr; }		

// Getter & Setter		
	inline void		setParent(Model* ptr_parent) { ptr_parent_ = ptr_parent; }
	inline Model*	getParent() const			 { return ptr_parent_;}

// Virtual Base Member Functions
	virtual void	tick(float delta) {}
	virtual void	Derigistered()	   {}
};

}

#endif // !_COMPONENT_HPP 

