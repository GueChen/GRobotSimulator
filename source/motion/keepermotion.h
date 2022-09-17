#ifndef __KEEPER_MOTION_H
#define __KEEPER_MOTION_H

#include "motion/gmotionbase.h"

namespace GComponent {

class KeeperMotion : public CMotionBase
{
public:
	explicit KeeperMotion(const SE3f& goal, float execution_time);
	~KeeperMotion() = default;

protected:
	// Í¨¹ý CMotionBase ¼Ì³Ð
	virtual PathFunc PathFuncImpl(const SE3f& mat_ini, const SE3f& mat_end)		override;
	virtual float ExecutionTimeImpl(const SE3f& mat_ini, const SE3f& mat_end)	override;

};

}


#endif // !__KEEPER_MOTION



