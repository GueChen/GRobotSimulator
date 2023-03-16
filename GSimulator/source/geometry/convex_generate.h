/**
 *  @file  	convex_generate.h
 *  @brief 	this header supply some convex hull generate helper function.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 16th, 2023
 **/
#ifndef __CONVEX_GENERATE_H
#define __CONVEX_GENERATE_H

#include "geometry/geometry_datastructure.hpp"

#include <vector>

namespace GComponent {
/// <summary>
/// using VHACD library to help make convex hulls decomposition from a triangle mesh
/// <para>
/// ʹ�� VHACD ��ʵ�������������͹���ֽ�
/// </para>
/// </summary>
/// <param name="vertices">				cref	{vector{vertex}}		���㼯��				</param>
/// <param name="triangles">			cref	{vector{triangle}}		����������������		</param>
/// <param name="max_convex_hulls">		val		{uint32_t}				���ֽ�͹����			</param>
/// <param name="max_vertices_per_ch">	val		{uint32_t}				����͹��������������	</param>
/// <param name="use_update_callback">	val		{bool}					�Ƿ�ʹ�ø��»ص�		</param>
/// <returns>							val		{vector{RawConvex}}		͹�����鼯��			</returns>
std::vector<RawConvex> GenerateConvexHull(const		std::vector<Vertex>  & vertices, 
										  const		std::vector<Triangle>& triangles,
										  uint32_t	max_convex_hulls, 
										  uint32_t	max_vertices_per_ch, 
										  bool		use_update_callback = false);

}

#endif // !__CONVEX_GENERATE_H
