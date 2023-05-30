#include "mesh.hpp"

#include <glad/gl.h>

#include <cstddef> // offsetof

namespace FoxEngine
{
	class MeshOGL33 final : public Mesh
	{
	public:
		MeshOGL33(const Mesh::CreateInfo& info)
		{
			mCount = info.indices.size();

			glGenVertexArrays(1, &mVao);
			glBindVertexArray(mVao);

			// Apply to buffer objects too with changes to the name
			if (GLAD_GL_KHR_debug && !info.debugName.empty())
				glObjectLabel(GL_VERTEX_ARRAY, mVao, info.debugName.size(), info.debugName.data());

			glGenBuffers(1, &mVbo);
			glBindBuffer(GL_ARRAY_BUFFER, mVbo);
			glBufferData(GL_ARRAY_BUFFER, info.vertices.size_bytes(), info.vertices.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

			glGenBuffers(1, &mEbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, info.indices.size_bytes(), info.indices.data(), GL_STATIC_DRAW);

			glBindVertexArray(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		virtual ~MeshOGL33() noexcept
		{
			if (mVao)
				glDeleteVertexArrays(1, &mVao);

			if (mVbo)
				glDeleteBuffers(1, &mVbo);

			if (mEbo)
				glDeleteBuffers(1, &mEbo);
		}

		void Draw() override
		{
			glBindVertexArray(mVao);
			glDrawElements(GL_TRIANGLES, mCount, GL_UNSIGNED_INT, nullptr);
		}
	private:
		unsigned int mVao;
		unsigned int mVbo;
		unsigned int mEbo;
		int mCount;
	};

	std::unique_ptr<Mesh> Mesh::Create(const Mesh::CreateInfo& info)
	{
		return std::make_unique<MeshOGL33>(info);
	}
}