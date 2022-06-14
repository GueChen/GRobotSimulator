/**
 *  @file  	tracker_component.h
 *  @brief 	This class Applies Bind to a Goal and Try use Kinematic Component to Closer Goal.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 25, 2022
 **/
#ifndef _TRACKER_COMPONENT_H
#define _TRACKER_COMPONENT_H

#include "component/component.hpp"
#include "model/model.h"

#include <functional>
#include <string>
#include <list>
#include <unordered_set>
#include <unordered_map>

namespace GComponent {
using std::list;
using std::string;
using std::string_view;
using std::unordered_set;
using std::unordered_map;

// TODO: support all model trackable
class TrackerComponent: public Component {
	using _Self		= TrackerComponent;
	using _RawPtr	= TrackerComponent*;
	using _Ref		= TrackerComponent&;

public:
	enum class State {		
		Sleeping = 0,
		Tracking = 1
	};

public:
	explicit		TrackerComponent(Model* ptr_parent = nullptr, const string& goal_name = "");
	virtual			~TrackerComponent();
	
	void			SetGoal(const string & goal_name);
	inline  string	GetGoal()			const			{ return tracked_goal_ ? tracked_goal_->getName() : "NULL"; }

	inline  void	SetState(State state)				{ state_ = state; }
	inline	State   GetState()			const			{ return state_; }

	const list<string> &
					GetTracerNames()	const			{ return tracer_list_; }

	virtual const string_view&		
					GetTypeName()		const override	{ return type_name; }

	static inline	list<string>
					GetTrackableNames()					{ return list<string>(trackable_table_.begin(), trackable_table_.end()); }
protected:
	void		    RegisterTracer(const string& tracer_name);
	void			DeregisterTracer(const string& tracer_name);

	void			ClearTracers();

	virtual void	tickImpl(float delta_time) override;
	inline string	GetName()	const					{ return GetParent()->getName(); }

private:
	State			state_						= State::Sleeping;
	Model*			tracked_goal_				= nullptr;

	list<string>	tracer_list_				= {};
	unordered_map<string, list<string>::iterator> 
					tracer_node_map_			= {};

	static 
	unordered_set<string> trackable_table_;
	static constexpr 
	const string_view type_name = "TrackerComponent";	
};


}

#endif // !_TRACKER_COMPONENT_H

