#pragma once

#include "stdafx.h"

namespace Detail
{
	template <class T>
	struct ReleaseDeleter
	{ // default deleter for unique_ptr
		constexpr ReleaseDeleter() noexcept = default;

		template <class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
		ReleaseDeleter(const ReleaseDeleter<U>&) noexcept
		{}

		void operator()(T* _Ptr) const noexcept
		{
			static_assert(0 < sizeof(T), "can't delete an incomplete type");
			_Ptr->Release();
		}
	};
}

template <typename T>
using ReleasedUniquePtr = std::unique_ptr<T, Detail::ReleaseDeleter<T>>;
