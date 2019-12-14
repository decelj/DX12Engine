#pragma once

#include "stdafx.h"

#include <chrono>
#include <sstream>
#include <iomanip>

template<size_t Alignment, 
	typename T,
	typename = std::enable_if_t<std::is_integral<T>::value>>
T AlignTo(const T value)
{
	static_assert((Alignment & (Alignment - 1)) == 0);
	return (value + (Alignment - 1)) & ~(Alignment - 1);
}


template<typename ClockT>
class _Timer
{
public:
    using TimePointT = typename ClockT::time_point;
    using DurationT = typename ClockT::duration;

    _Timer() = default;

    void Start();
    DurationT Elapsed() const;
    std::string ElapsedAsString(DurationT elapsed);

private:
    TimePointT m_StartTime;
};

template<typename ClockT>
inline void _Timer<ClockT>::Start()
{
    m_StartTime = ClockT::now();
}

template<typename ClockT>
inline typename _Timer<ClockT>::DurationT _Timer<ClockT>::Elapsed() const
{
    return ClockT::now() - m_StartTime;
}

template<typename ClockT>
inline std::string _Timer<ClockT>::ElapsedAsString(typename _Timer<ClockT>::DurationT elapsed)
{
    using namespace std::chrono;
    using namespace std;

    ostringstream ss;
    ss  << setw(2) << setfill('0') << duration_cast<hours>(elapsed).count() << ":"
        << setw(2) << setfill('0') << duration_cast<minutes>(elapsed % hours(1)).count() << ":"
        << setw(2) << setfill('0') << duration_cast<seconds>(elapsed % minutes(1)).count() << "."
        << setw(3) << setfill('0') << duration_cast<milliseconds>(elapsed % seconds(1)).count();

    return ss.str();
}

using Timer = _Timer<std::chrono::system_clock>;
using HighResTimer = _Timer<std::chrono::high_resolution_clock>;
