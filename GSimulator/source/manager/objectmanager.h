/**
 *  @file  	worldmanager.h
 *  @brief 	create all the model and responsibility the initialize.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 14, 2022
 **/
#ifndef _WORLDMANAGER_H
#define _WORLDMANAGER_H

#include "base/singleton.h"

//#include <glm/glm.hpp>
#include <Eigen/Dense>

#include <QtCore/QObject>

#include <unordered_map>
#include <string>

namespace GComponent
{
using Mat4 = Eigen::Matrix4f;
using std::unordered_map;
using std::string;
using std::pair;

class ObjectManager:public QObject
{
	Q_OBJECT

	NonCopyable(ObjectManager)
public:
	static
	ObjectManager& getInstance();

	void	Initialize();

	bool	RegisterObject(const string& obj_name, const string & mesh_name,
						   const string& mesh_asset_path   = "");
	bool	DerigisterObject(const string& obj_name);

	bool	CreateInstanceWithModelMat(const string& obj_name, const Mat4& model_mat);
	void	CreateInstance(const string& obj_name);
	virtual ~ObjectManager() = default;

protected:
	ObjectManager() = default;

private:
	unordered_map<string, size_t>					obj_lists_count_table_;
	unordered_map<string, string>					obj_properties_table_;
};
}

#endif // !_WORLDMANAGER_H
