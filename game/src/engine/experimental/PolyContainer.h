#pragma once

// Use to implment move/copy operations
//#include <typeinfo>

#include <type_traits>
#include <memory> // std::destroy_at

namespace FoxEngine::Experimental
{
	template<class T>
	constexpr T* NullOf = nullptr;

	template<class T, size_t S = sizeof(void*) * 16>
	class PolyContainer
	{
	public:
		template<class U, class... Args, std::enable_if_t<std::is_base_of_v<T, U>, bool> = true>
		PolyContainer(U*, Args&& ...args)
		{
			static_assert(sizeof(U) <= S);
			static_assert(std::is_base_of_v<T, U>);

			new(&mBuffer) U(std::forward<Args>(args)...);
			mDtor = [](void* ptr) { std::destroy_at(static_cast<U*>(ptr)); };
		}

		~PolyContainer()
		{
			mDtor((T*)mBuffer);
		}

		// Much more runtime checks are needed to safely support these operations
		PolyContainer(const PolyContainer&) = delete;
		PolyContainer& operator=(const PolyContainer&) = delete;
		PolyContainer(PolyContainer&&) noexcept = delete;
		PolyContainer& operator=(PolyContainer&&) = delete;

		// Have templated function to attempt to get derived type

		// Get the subtype
		T* operator->() { return (T*)mBuffer; }
		T const* operator->() const { return (const T*)mBuffer; }
		T* get() { return (T*)mBuffer; }
	public:
		char mBuffer[S];
		void(*mDtor)(void*);
	};
}