#pragma once

// Use to implment move/copy operations
#include <typeinfo>

#include <type_traits>
#include <memory> // std::destroy_at, std::unique_ptr

namespace FoxEngine
{
	template<class T>
	constexpr T* NullOf = nullptr;

	template<class T, size_t S = sizeof(void*) * 4>
	class Poly final
	{
	public:
		Poly() noexcept = default;

		template<class U, class... Args, std::enable_if_t<std::is_base_of_v<T, U>, bool> = true>
		Poly(U*, Args&& ...args)
		{
			static_assert(sizeof(U) <= S);

			new(&mBuffer) U(std::forward<Args>(args)...);
			mDtor = [](void* ptr) { std::destroy_at(static_cast<U*>(ptr)); };
			mUnique = []() -> std::unique_ptr<T> { return std::make_unique<U>(); };
			mTypeHash = typeid(U).hash_code();
			mTypeSize = sizeof(U);
		}

		~Poly()
		{
			if (mDtor)
				mDtor((T*)mBuffer);
		}

		// Allocates heap memory, moves object into heap, invalidates the poly object, may be reassigned to another type
		// Can be undone, once turned into a shared_ptr, cannot be undone
		std::unique_ptr<T> MakeUnique()&&
		{
			std::unique_ptr<T> ptr = mUnique();

			mTypeHash = 0;
			mTypeSize = 0;
			mDtor = nullptr;
			mUnique = nullptr;

			char* p = (char*)ptr.get();

			for (size_t i = 0; i < mTypeSize; ++i)
				std::swap(p[i], ((char*)mBuffer)[i]);

			return ptr;
		}

		// Much more runtime checks are needed to safely support these operations
		Poly(const Poly&) = delete;
		Poly& operator=(const Poly&) = delete;


		// Moves are easy as we can directly swap the bytes around
		// without needing to know any implmentation details of the underlying type
		// This is potentially dangerous if the base type does unexpected operatios in their move ops
		Poly(Poly&& other) noexcept
		{
			*this = std::move(other);
		}


		Poly& operator=(Poly&& other)
		{
			for (size_t i = 0; i < S; ++i)
				std::swap(mBuffer[i], other.mBuffer[i]);

			std::swap(mTypeHash, other.mTypeHash);
			std::swap(mDtor, other.mDtor);
			std::swap(mUnique, other.mUnique);
			std::swap(mTypeSize, other.mTypeSize);

			return *this;
		}

		// Get the subtype
		T* operator->() { return (T*)mBuffer; }
		T const* operator->() const { return (const T*)mBuffer; }
		T* get() { return (T*)mBuffer; }
	public:
		size_t mTypeHash = 0;
		size_t mTypeSize = 0;
		char mBuffer[S];
		void(*mDtor)(void*) = nullptr;
		std::unique_ptr<T>(*mUnique)() = nullptr;
	};
}