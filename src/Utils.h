#pragma once

#include "stdafx.h"


template<size_t Alignment, 
	typename T,
	typename = std::enable_if_t<std::is_integral<T>::value>>
T AlignTo(const T value)
{
	static_assert((Alignment & (Alignment - 1)) == 0);
	return (value + (Alignment - 1)) & ~(Alignment - 1);
}
