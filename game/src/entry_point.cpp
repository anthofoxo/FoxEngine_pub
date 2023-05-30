#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include "engine/window.hpp"
#include "engine/texture.hpp"
#include "engine/shader.hpp"
#include "engine/mesh.hpp"

#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <span>


#include "engine/log.hpp"

// Move implmentation to a different file, perhaps vendor/impl.cpp ???
#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

#include <fstream>
#include <vector>
#include <optional>
#include <string_view>

#include <stdexcept>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Guidelines for the order of includes should be made

// Error prone, needs more logging ability
static std::unique_ptr<FoxEngine::Mesh> load_mesh(std::string_view resource)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(resource.data(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_ImproveCacheLocality | aiProcess_OptimizeMeshes);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		//fe_log_warn("failed to load model: {} with error: {}", resource, importer.GetErrorString());
		return nullptr;
	}

	if (scene->mNumMeshes == 0)
	{
		//fe_log_warn("scene has no meshes");
		return nullptr;
	}

	//if (scene->mNumMeshes != 1)
		//fe_log_warn("scene has more than one mesh, only the first will be processed");

	//if (scene->mRootNode->mNumChildren != 0)
		//fe_log_warn("root has children, they will not be processed");

	aiMesh* mesh = scene->mMeshes[0];

	std::vector<FoxEngine::Mesh::Vertex> vertices;
	vertices.reserve(mesh->mNumVertices);

	std::vector<FoxEngine::Mesh::Index> indices;
	indices.reserve(mesh->mNumFaces * 3);

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		// A lot of this can likely be copied via memcpy
		FoxEngine::Mesh::Vertex vertex;
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;
		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;
		// Add texture coords and tangents when needed
		// vertex.tex_coord.x = mesh->mTextureCoords[0][i].x;
		// vertex.tex_coord.y = mesh->mTextureCoords[0][i].y;
		vertices.push_back(std::move(vertex));
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];

		// We should support lines and points eventually...
		if (face.mNumIndices != 3) continue;

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
			indices.push_back(face.mIndices[j]);
	}

	return FoxEngine::Mesh::Create(
		{
			.vertices = vertices,
			.indices = indices,
			.debugName = resource
		});
}

// Put inside engine struct
static std::atomic_bool sRunning = true;

void engine_main()
{
	FoxEngine::LogInfo("Welcome to FoxEngine");

	FoxEngine::Window window = FoxEngine::WindowCreateInfo{};

	// Create an engine structure to contain this

	glfwSetWindowCloseCallback(window.Handle(), [](GLFWwindow*)
		{
			sRunning = false;
		});

	std::thread thread = std::thread([&]()
		{
			window.MakeContextCurrent();
			gladLoadGL(&FoxEngine::Window::GetProcAddress);
			FoxEngine::Window::SwapInterval(-1);

			stbi_set_flip_vertically_on_load(true);

			std::unique_ptr<FoxEngine::Shader> opaqueShader = FoxEngine::Shader::Create(
				{
					.filename = "opaque.glsl",
					.debugName = "opaque.glsl"
				});

			FoxEngine::Mesh::Vertex vertices[] =
			{
				{ glm::vec3(-1, +1, +0), glm::vec3(+0, +0, -1) },
				{ glm::vec3(-1, -1, +0), glm::vec3(+0, +0, -1) }, 
				{ glm::vec3(+1, +1, +0), glm::vec3(+0, +0, -1) },
				{ glm::vec3(+1, -1, +0), glm::vec3(+0, +0, -1) }
			};

			FoxEngine::Mesh::Index indices[] = { 0, 1, 2, 2, 1, 3 };

			std::unique_ptr<FoxEngine::Mesh> mesh = FoxEngine::Mesh::Create(
				{
					.vertices = vertices,
					.indices = indices
				});

			std::unique_ptr<FoxEngine::Mesh> dragon_mesh = load_mesh("dragon.obj");

			// More defaults need set
			glEnable(GL_DEPTH_TEST);
			glClearColor(0.7f, 0.8f, 0.9f, 1.0f);

			while (sRunning)
			{
				// Use callbacks for this, no neeed to do this every frame,
				// later on this will trigger buffer and texture reallocation
				int w, h;
				glfwGetFramebufferSize(window.Handle(), &w, &h);
				glViewport(0, 0, w, h);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// projection resize should also be bound to window resize operations
				glm::mat4 projection = glm::perspectiveFov(glm::radians(90.0f), (float)w, (float)h, 0.1f, 100.0f);
				
				// Should come from object transfroms from ecs (doesn't exist)
				glm::mat4 pos = glm::identity<glm::mat4>();
				pos = glm::translate(pos, glm::vec3{ 0.0f, 0.0f, -10.f });

				// Uniforms will not stay forever, prefer uniform buffer blocks
				opaqueShader->Bind();
				opaqueShader->UniformMat4f("uProjection", glm::value_ptr(projection));
				opaqueShader->UniformMat4f("uView", glm::value_ptr(glm::identity<glm::mat4>()));
				opaqueShader->UniformMat4f("uModel", glm::value_ptr(pos));
				dragon_mesh->Draw();

				window.SwapBuffers();
			}
			
			glfwPostEmptyEvent();
		});

	// This loop can also be used to refresh object pools in the resource loader
	while (sRunning)
		FoxEngine::Window::WaitEvents();

	thread.join();
}

int main(int argc, char* argv[])
{
	try
	{
		engine_main();
	}
	catch (const std::exception& e) // supposed to catch exceptions but doesn't???
	{
		std::cout << e.what() << '\n';
		__debugbreak(); // MSVC only, fix this
	}
}