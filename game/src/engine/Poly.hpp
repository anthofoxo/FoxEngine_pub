#pragma once

// Use to implment move/copy operations
// #include <typeinfo>

#include <type_traits>
#include <memory> // std::destroy_at, std::unique_ptr

namespace FoxEngine
{
	template<class T>
	constexpr T* NullOf = nullptr;

	template<class T, size_t S = sizeof(void*) * 32> // used to be 4, add some templated usings for different sizes
	class Poly final
	{
	public:
		constexpr Poly() noexcept = default;

		template<class U, class... Args, std::enable_if_t<std::is_base_of_v<T, U>, bool> = true>
		Poly(U*, Args&& ...args)
		{
			static_assert(sizeof(U) <= S, "Poly container isn't large enough, make S larger");
			static_assert(std::is_default_constructible_v<U>, "U must be default constructible");
			static_assert(std::is_base_of_v<T, U>, "U must be a derived type of T");

			new(&mBuffer) U(std::forward<Args>(args)...);
			mDtor = [](void* ptr) { std::destroy_at(static_cast<U*>(ptr)); };
			mUnique = [](void* ptr) -> std::unique_ptr<T> { return std::make_unique<U>(std::move(*(U*)ptr)); };
		}

		~Poly()
		{
			if (mDtor)
				mDtor((T*)mBuffer);
		}

		// Allocates heap memory, moves object into heap, invalidates the poly object, may be reassigned to another type
		std::unique_ptr<T> MakeUnique()&&
		{
			auto unique = mUnique(mBuffer);

			mDtor = nullptr;
			mUnique = nullptr;

			return unique;
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

			std::swap(mDtor, other.mDtor);
			std::swap(mUnique, other.mUnique);

			return *this;
		}

		// Get the subtype
		T* operator->() { return (T*)mBuffer; }
		T const* operator->() const { return (const T*)mBuffer; }
		T* get() { return (T*)mBuffer; }
	public:
		char mBuffer[S]{};
		void(*mDtor)(void*) = nullptr;
		std::unique_ptr<T>(*mUnique)(void*) = nullptr;
	};
}