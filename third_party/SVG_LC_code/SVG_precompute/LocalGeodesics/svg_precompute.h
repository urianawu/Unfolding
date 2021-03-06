#pragma once
#ifndef _SVG_PRECOMPUTE_H_
#define _SVG_PRECOMPUTE_H_
#include "stdafx.h"

#include "Utils/common.h"

void svg_precompute(const string& input_obj_name, const int fixed_k, string& svg_file_name,
	const doubles_t* verts = nullptr,
	const ui32s_t* faces = nullptr);
#endif
