#ifndef _COMPONENT_HPP 
#define _COMPONENT_HPP

namespace GComponent {

	class Model;

	class Component {
	protected:
		Model* ptr_parent_ = nullptr;
	public:
		Component() = default;
		explicit Component(Model* ptr_parent) : ptr_parent_(ptr_parent) {}
		virtual ~Component() { ptr_parent_ = nullptr; }
		
		void setParent(Model* ptr_parent) { ptr_parent_ = ptr_parent; }

		virtual void tick(float delta) {}
		virtual void Derigistered() {}

	};
}

#endif // !_COMPONENT_HPP 

