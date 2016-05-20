#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <cstdlib>
#include <cstdint>
#include <cmath>
#include <assert.h>
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <memory>
#include <limits>
#include <math.h>

using namespace std;

//#if !defined(QT_NO_DEBUG) && !defined(QT_DEBUG)
#if !defined(NDEBUG) && !defined(_DEBUG)
#	define _DEBUG
#endif

using floats_t = vector<float>;
using doubles_t = vector<double>;
using ui8s_t = vector<uint8_t>;
using ui16s_t = vector<uint16_t>;
using ui32s_t = vector<uint32_t>;
using int8s_t = vector<int8_t>;
using int16s_t = vector<int16_t>;
using int32s_t = vector<int32_t>;


typedef std::unordered_map<string, double> confMap;

#endif // COMMON_H
