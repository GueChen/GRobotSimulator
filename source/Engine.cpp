#include "Engine.h"

#include "manager/modelmanager.h"
#include "manager/scenemanager.h"

#include <iostream>
#include <format>

void GComponent::Engine::run()
{
	while (!need_quit_) {
		auto time_span = GetSpanTime<std::milli>();
		LogicTick(time_span.count());
		RenderTick();
	}
}

template<class _TimeScale>
std::chrono::duration<float, _TimeScale> GComponent::Engine::GetSpanTime() {
	
	steady_clock::time_point tick_now = steady_clock::now();
	duration<float, _TimeScale> time_span = duration_cast<microseconds>(tick_now - last_time_point_);
	last_time_point_ = tick_now;
	return time_span;
}

void GComponent::Engine::LogicTick(float delta_time)
{

}

void GComponent::Engine::RenderTick()
{

}
