#pragma once

#include <glm/glm.hpp>

#include <span>
#include <memory>
#include <string_view>

// Meshes aren't final either, May not nessesarily always want to use this vertex format,
// perhaps allow it to be changed in a render pipeline down the line??

namespace FoxEngine
{
	class Mesh
	{
	public:
		struct Vertex final
		{
			glm::vec3 position;
			glm::vec3 normal;
		};

		using Index = unsigned int;

		struct CreateInfo final
		{
			std::span<Vertex> vertices;
			std::span<Index> indices;
			std::string_view debugName;
		};

		static std::unique_ptr<Mesh> Create(const CreateInfo& info);
	public:
		constexpr Mesh() noexcept = default;
		virtual ~Mesh() noexcept = default;

		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		virtual void Draw() = 0;
	};
}