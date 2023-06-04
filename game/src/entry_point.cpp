#include "engine/window.hpp"
#include "engine/texture.hpp"
#include "engine/shader.hpp"
#include "engine/mesh.hpp"
#include "engine/log.hpp"
#include "engine/experimental/PolyContainer.h"

#include "vendor/stb_image.h"

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <entt/entt.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>
#include <span>
#include <vector>
#include <string_view>
#include <stdexcept>

//#define IMGUI_DISABLE_OBSOLETE_KEYIO
//#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

#include "misc/cpp/imgui_stdlib.h"

// Should we use YAML or JSON for configs

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
		// Add tangents when needed
		vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
		vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
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

struct Transform final
{
	glm::vec3 translation{};
	glm::quat orientation = glm::identity<glm::quat>();
	glm::vec3 scale = glm::vec3(1.0f);

	glm::mat4 ToMatrix() const
	{
		glm::mat4 matrix = glm::identity<glm::mat4>();
		matrix = glm::translate(matrix, translation);
		matrix *= glm::toMat4(orientation);
		matrix = glm::scale(matrix, scale);
		return matrix;
	}

	glm::mat4 ToInverseMatrix() const
	{
		return glm::inverse(ToMatrix());
	}

	void FromMatrix(const glm::mat4& matrix)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(matrix, scale, orientation, translation, skew, perspective);
	}
};

struct TransformComponent final
{
	Transform transform;
	std::string name = "unnamed";
	std::string tag = "default";
};

struct MeshFilterComponent final
{
	std::shared_ptr<FoxEngine::Mesh> mesh;
	std::string resource;
};

struct MeshRendererComponent final
{
	std::shared_ptr<FoxEngine::Texture> texture;
	std::string resource;
};

namespace FoxEngine
{
	class Engine final
	{
	public:
		void Start()
		{
			mWindow = FoxEngine::WindowCreateInfo{};

			// a bindable input type should be provided, or a way to attach custom listeners
			struct Input
			{
				// IsKeyDown, CursorPos
				// or
				// AddListener
			};

			glfwSetWindowUserPointer(mWindow.Handle(), this);

			glfwSetWindowCloseCallback(mWindow.Handle(), [](GLFWwindow* window)
				{
					static_cast<Engine*>(glfwGetWindowUserPointer(window))->mDispatcher.enqueue<WindowCloseEvent>();
				});

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

			io.Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 16.0f);

			ImGui::StyleColorsDark();

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGuiStyle& style = ImGui::GetStyle();
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}

			mWindow.MakeContextCurrent();

			// Setup Platform/Renderer backends
			ImGui_ImplGlfw_InitForOpenGL(mWindow.Handle(), true);
			ImGui_ImplOpenGL3_Init("#version 330 core");

			FoxEngine::Window::LoadGLFunctions(); // may fail
			FoxEngine::Window::SwapInterval(-1);

			mDispatcher.sink<WindowCloseEvent>().connect<&Engine::OnClose>(this);

			std::shared_ptr<FoxEngine::Texture> defaultTex = FoxEngine::Texture::Create(
				{
					.width = 1,
					.height = 1,
					.debugName = "Default texture (white)"
				});

			unsigned char vals[]{(unsigned char)255,(unsigned char)255,(unsigned char)255,(unsigned char)255};

			defaultTex->Upload(
				{
					.width = 1,
					.height = 1,
					.pixels = vals
				});

			FoxEngine::Mesh::Vertex vertices[] = {
				{{ -1, 1, 0 },{ 0, 0, -1 },{ 0, 1 }},
				{{ -1, -1, 0 },{ 0, 0, -1 },{ 0, 0 }},
				{{ 1, 1, 0 },{ 0, 0, -1 },{ 1, 1 }},
				{{ 1, -1, 0 },{ 0, 0, -1 },{ 1, 0 }}
			};
			FoxEngine::Mesh::Index indices[] = {
				0,1,2, 2,1,3
			};

			std::unique_ptr<FoxEngine::Mesh> fsQuad = FoxEngine::Mesh::Create(
				{
					.vertices = vertices,
					.indices = indices
				});

			std::unique_ptr<FoxEngine::Shader> opaqueShader = FoxEngine::Shader::Create(
				{
					.filename = "opaque.glsl",
					.debugName = "opaque.glsl"
				});

			std::unique_ptr<FoxEngine::Shader> radialBlurShader = FoxEngine::Shader::Create(
				{
					.filename = "radial_blur.glsl",
					.debugName = "radial_blur.glsl"
				});

			Transform cameraTransform;

			{
				entt::handle entity = { mRegistry, mRegistry.create() };
				TransformComponent& transform = entity.emplace<TransformComponent>();
				MeshFilterComponent& meshFilter = entity.emplace<MeshFilterComponent>();
				MeshRendererComponent& meshRenderer = entity.emplace<MeshRendererComponent>();
				meshFilter.resource = "dragon.obj";
				meshFilter.mesh = load_mesh(meshFilter.resource);

				meshRenderer.resource = "#";
				meshRenderer.texture = defaultTex;

				transform.name = "dergon";
				transform.transform.translation.z = -10;
			}

			entt::handle foxEntity;

			{
				entt::handle entity = { mRegistry, mRegistry.create() };
				TransformComponent& transform = entity.emplace<TransformComponent>();
				MeshFilterComponent& meshFilter = entity.emplace<MeshFilterComponent>();
				meshFilter.resource = "fox.obj";
				meshFilter.mesh = load_mesh(meshFilter.resource);
				transform.name = "foxo";
				transform.tag = "__icon";
				transform.transform.translation.z = -4;

				MeshRendererComponent& meshRenderer = entity.emplace<MeshRendererComponent>();
				meshRenderer.resource = "fox.png";
				meshRenderer.texture = FoxEngine::Texture::Create(meshRenderer.resource);

				transform.transform.orientation = glm::rotate(transform.transform.orientation, glm::radians(180.f), glm::vec3(1, 0, 0));

				foxEntity = entity;
			}

			glClearColor(0, 0, 0, 0);
			glClearDepth(1);
			glDepthFunc(GL_LEQUAL);
			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			glFrontFace(GL_CCW);
			glCullFace(GL_BACK);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDisable(GL_MULTISAMPLE);

			unsigned int fbo = 0;
			unsigned int fboTex = 0;
			unsigned int fboTexBlack = 0;
			unsigned int fboDep = 0;
			int vpw = 0, vph = 0;

			unsigned int iconFbo;
			unsigned int iconTex;
			unsigned int iconDep;
			int size = 64;

			glGenTextures(1, &iconTex);
			glBindTexture(GL_TEXTURE_2D, iconTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glGenRenderbuffers(1, &iconDep);
			glBindRenderbuffer(GL_RENDERBUFFER, iconDep);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size, size);

			glGenFramebuffers(1, &iconFbo);
			glBindFramebuffer(GL_FRAMEBUFFER, iconFbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, iconTex, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, iconDep);

			std::unique_ptr<char[]> pixels = std::make_unique<char[]>(size * size * 4);
			glm::mat4 projection;

			double lastTime = glfwGetTime();
			double currentTime;
			double deltaTime = 1.;

			bool showDemoWindow = false;
			bool showViewport = true;
			bool showHierarchy = true;
			bool showProperties = true;
			bool showGpuInfo = false;

			while (mRunning)
			{
				FoxEngine::Window::PollEvents();
				mDispatcher.update();

				currentTime = glfwGetTime();
				deltaTime = currentTime - lastTime;
				lastTime = currentTime;

				bool cameraEdit = glfwGetMouseButton(mWindow.Handle(), 1) != GLFW_RELEASE;

				if (cameraEdit)
				{
					glfwSetInputMode(mWindow.Handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
					ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
				}
				else
				{
					glfwSetInputMode(mWindow.Handle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
					ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
					ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
				}

				static glm::vec2 last_mouse_pos{};

				double xpos, ypos;
				glfwGetCursorPos(mWindow.Handle(), &xpos, &ypos);

				glm::vec2 mouse_pos(xpos, ypos);

				glm::vec2 mouse_delta = mouse_pos - last_mouse_pos;

				last_mouse_pos = mouse_pos;

				if (cameraEdit)
				{
					

					if (glm::length2(mouse_delta) > 1)
					{
						glm::vec4 axis = cameraTransform.ToInverseMatrix() * glm::vec4(0, 1, 0, 0);

						cameraTransform.orientation = glm::rotate(cameraTransform.orientation, glm::radians(mouse_delta.x * -0.3f), glm::vec3(axis));
						cameraTransform.orientation = glm::rotate(cameraTransform.orientation, glm::radians(mouse_delta.y * -0.3f), glm::vec3(1, 0, 0));
					}

					glm::vec3 direction{};

					if (glfwGetKey(mWindow.Handle(), GLFW_KEY_A) != GLFW_RELEASE)
						--direction.x;
					if (glfwGetKey(mWindow.Handle(), GLFW_KEY_D) != GLFW_RELEASE)
						++direction.x;
					if (glfwGetKey(mWindow.Handle(), GLFW_KEY_W) != GLFW_RELEASE)
						--direction.z;
					if (glfwGetKey(mWindow.Handle(), GLFW_KEY_S) != GLFW_RELEASE)
						++direction.z;
					if (glfwGetKey(mWindow.Handle(), GLFW_KEY_LEFT_SHIFT) != GLFW_RELEASE)
						--direction.y;
					if (glfwGetKey(mWindow.Handle(), GLFW_KEY_SPACE) != GLFW_RELEASE)
						++direction.y;

					if (glm::length2(direction) > 0)
					{
						direction = glm::normalize(direction) * (float)deltaTime * 10.0f;

						cameraTransform.FromMatrix(glm::translate(cameraTransform.ToMatrix(), direction));
					}
						
				}

				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				ImGui::DockSpaceOverViewport();

				if (ImGui::BeginMainMenuBar())
				{
					if (ImGui::BeginMenu("File"))
					{
						if (ImGui::MenuItem("Quit")) mRunning = false;
						
						ImGui::EndMenu();
					}

					if (ImGui::BeginMenu("View"))
					{
						ImGui::MenuItem("Viewport", nullptr, &showViewport);
						ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
						ImGui::MenuItem("Properties", nullptr, &showProperties);
						ImGui::MenuItem("GPU Info", nullptr, &showGpuInfo);		
						ImGui::Separator();
						ImGui::MenuItem("ImGui Demo Window", nullptr, &showDemoWindow);

						ImGui::EndMenu();
					}

					ImGui::EndMainMenuBar();
				}

				if (showDemoWindow)
					ImGui::ShowDemoWindow(&showDemoWindow);

				if (showViewport)
				{
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });

					ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

					if (ImGui::Begin("Viewport", &showViewport))
					{
						ImVec2 size = ImGui::GetContentRegionAvail();

						if (size.x != 0 && size.y != 0)
						{
							// if size changed, resize is required
							if (vpw != size.x || vph != size.y)
							{
								vpw = size.x;
								vph = size.y;

								if (fbo) glDeleteFramebuffers(1, &fbo);
								if (fboTex) glDeleteTextures(1, &fboTex);
								if (fboTexBlack) glDeleteTextures(1, &fboTexBlack);
								if (fboDep) glDeleteRenderbuffers(1, &fboDep);

								glGenTextures(1, &fboTex);
								glBindTexture(GL_TEXTURE_2D, fboTex);
								glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, vpw, vph, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

								glGenTextures(1, &fboTexBlack);
								glBindTexture(GL_TEXTURE_2D, fboTexBlack);
								glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, vpw, vph, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

								glGenRenderbuffers(1, &fboDep);
								glBindRenderbuffer(GL_RENDERBUFFER, fboDep);
								glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, vpw, vph);

								glGenFramebuffers(1, &fbo);
								glBindFramebuffer(GL_FRAMEBUFFER, fbo);
								glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
								glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fboTexBlack, 0);
								glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fboDep);

								unsigned int vals[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
								glDrawBuffers(2, vals);

							}

							// Perform rendering
							{
								glBindFramebuffer(GL_FRAMEBUFFER, fbo);
								glViewport(0, 0, vpw, vph);

								glClearColor(0, 0, 0, 0);
								glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

								// projection resize should also be bound to window resize operations
								projection = glm::perspectiveFov(glm::radians(90.0f), (float)vpw, (float)vph, 0.1f, 1000.0f);

								// Uniforms will not stay forever, prefer uniform buffer blocks
								opaqueShader->Bind();
								opaqueShader->UniformMat4f("uProjection", glm::value_ptr(projection));
								opaqueShader->UniformMat4f("uView", glm::value_ptr(cameraTransform.ToInverseMatrix()));

								auto view = mRegistry.view<TransformComponent, MeshFilterComponent, MeshRendererComponent>();

								for (auto entity : view)
								{
									auto [transform, meshFilter, meshRenderer] = view.get(entity);

									if (transform.tag == "__icon") continue;

									opaqueShader->UniformMat4f("uModel", glm::value_ptr(transform.transform.ToMatrix()));
									meshRenderer.texture->Bind();
									meshFilter.mesh->Draw();
								}

								glEnable(GL_BLEND);
								glBlendFunc(GL_ONE, GL_ONE);
								glDisable(GL_DEPTH_TEST);
								glDepthMask(GL_FALSE);

								// do radial blur
								radialBlurShader->Bind();
								radialBlurShader->Uniform2f("uResolution", vpw, vph);
								glBindTexture(GL_TEXTURE_2D, fboTexBlack);

								unsigned int bufs[] = { GL_COLOR_ATTACHMENT0 };
								glDrawBuffers(1, bufs);

								// render meshg
								fsQuad->Draw();

								unsigned int bufs2[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
								glDrawBuffers(2, bufs2);
									
								glDisable(GL_BLEND);
								glEnable(GL_DEPTH_TEST);
								glDepthMask(GL_TRUE);
							}

							ImGui::Image((ImTextureID)(intptr_t)fboTex, { (float)vpw, (float)vph }, { 0, 1 }, { 1, 0 });
						}


					}
					ImGui::End();

					ImGui::PopStyleVar();
				}

				static entt::entity selected = entt::null;

				if (showHierarchy)
				{
					if (ImGui::Begin("Hierarchy", &showHierarchy))
					{
						auto view = mRegistry.view<TransformComponent>();

						for (auto entity : view)
						{
							auto [transform] = view.get(entity);
							 
							ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;

							if (entity == selected) flags |= ImGuiTreeNodeFlags_Selected;

							ImGui::PushID((int)entity);

							bool expanded = ImGui::TreeNodeEx(transform.name.c_str(), flags);


							if (ImGui::IsItemClicked())
								selected = entity;

							if (expanded)
							{
								ImGui::TreePop();
							}

							ImGui::PopID();
						}
					}
					ImGui::End();
				}

				if (showProperties)
				{
					if (ImGui::Begin("Properties", &showProperties))
					{
						if (selected != entt::null)
						{
							entt::handle handle = { mRegistry, selected };
							TransformComponent& transform = handle.get<TransformComponent>();
				
							ImGui::InputText("Name", &transform.name);

							if (ImGui::CollapsingHeader("Transform"))
							{
								ImGui::InputText("Tag", &transform.tag);
								ImGui::Separator();
								ImGui::DragFloat3("Translation", glm::value_ptr(transform.transform.translation));
								
								glm::vec3 oldEuler = glm::degrees(glm::eulerAngles(transform.transform.orientation));
								glm::vec3 euler = oldEuler;
								bool changed = ImGui::DragFloat3("Orientation", glm::value_ptr(euler));
								if (changed)
								{
									glm::vec3 delta = glm::radians(euler - oldEuler);
									transform.transform.orientation = glm::rotate(transform.transform.orientation, delta.x, glm::vec3(1, 0, 0));
									transform.transform.orientation = glm::rotate(transform.transform.orientation, delta.y, glm::vec3(0, 1, 0));
									transform.transform.orientation = glm::rotate(transform.transform.orientation, delta.z, glm::vec3(0, 0, 1));
								}

								ImGui::DragFloat3("Scale", glm::value_ptr(transform.transform.scale));
								if (ImGui::Button("Reset"))
									transform.transform = Transform{};
							}

							if (auto* component = handle.try_get<MeshFilterComponent>())
							{
								if (ImGui::CollapsingHeader("Mesh filter"))
								{
									ImGui::InputText("Mesh", &component->resource);

									ImGui::PushID(component);
									if(ImGui::Button("Load"))
										component->mesh = load_mesh(component->resource);
									ImGui::PopID();
								}
							}

							if (auto* component = handle.try_get<MeshRendererComponent>())
							{
								if (ImGui::CollapsingHeader("Mesh renderer"))
								{
									ImGui::InputText("Texture", &component->resource);
									ImGui::PushID(component);
									if (ImGui::Button("Load"))
										component->texture = FoxEngine::Texture::Create(component->resource);
									ImGui::PopID();
								}
							}
							
						}
						else
						{
							ImGui::TextUnformatted("No entity selected");
						}
					}
					ImGui::End();
				}

				if (showGpuInfo)
				{
					if(ImGui::Begin("GPU Debug info"))
					{
						ImGui::LabelText("Renderer", "%s", glGetString(GL_RENDERER));
						ImGui::LabelText("Vendor", "%s", glGetString(GL_VENDOR));
						ImGui::LabelText("Version", "%s", glGetString(GL_VERSION));

						if (ImGui::CollapsingHeader("Supported extensions"))
						{
							int numExts;
							glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);

							for (int i = 0; i < numExts; ++i)
							{
								ImGui::TextUnformatted((char*)glGetStringi(GL_EXTENSIONS, i));
							}
						}
						
					}
					ImGui::End();
				}
				
				// Use callbacks for this, no neeed to do this every frame,
				// later on this will trigger buffer and texture reallocation
				int w, h;
				glfwGetFramebufferSize(mWindow.Handle(), &w, &h);


				if (w != 0 && h != 0)
				{
					static double rotateDelta = 0.0;
					rotateDelta += deltaTime;

					static double timer = 0.0;
					timer += deltaTime;

					if (timer > 1.0 / 8.0)
					{
						timer = 0.0;

						// Rotate foxo
						Transform& t = foxEntity.get<TransformComponent>().transform;
						t.orientation = glm::rotate(t.orientation, (float)glm::radians(45.0 * rotateDelta), glm::vec3(0, 1, 0));
						rotateDelta = 0.0;

						glBindFramebuffer(GL_FRAMEBUFFER, iconFbo);
						glViewport(0, 0, size, size);

						glClearColor(0, 0, 0, 0);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						// projection resize should also be bound to window resize operations
						projection = glm::perspectiveFov(glm::radians(60.0f), (float)size, (float)size, 0.01f, 10.0f);

						// Uniforms will not stay forever, prefer uniform buffer blocks
						opaqueShader->Bind();
						opaqueShader->UniformMat4f("uProjection", glm::value_ptr(projection));
						opaqueShader->UniformMat4f("uView", glm::value_ptr(glm::identity<glm::mat4>()));

						auto view = mRegistry.view<TransformComponent, MeshFilterComponent, MeshRendererComponent>();

						for (auto entity : view)
						{
							auto [transform, meshFilter, meshRenderer] = view.get(entity);

							if (transform.tag != "__icon") continue;

							opaqueShader->UniformMat4f("uModel", glm::value_ptr(transform.transform.ToMatrix()));
							meshRenderer.texture->Bind();
							meshFilter.mesh->Draw();
						}

						glBindTexture(GL_TEXTURE_2D, iconTex);
						glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());

						GLFWimage image;
						image.width = size;
						image.height = size;
						image.pixels = (unsigned char*)pixels.get();

						glfwSetWindowIcon(mWindow.Handle(), 1, &image);

					}
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glBindTexture(GL_TEXTURE_2D, 0);

				ImGui::Render();
				int display_w, display_h;
				glfwGetFramebufferSize(mWindow.Handle(), &display_w, &display_h);
				glViewport(0, 0, display_w, display_h);
				glClearColor(0, 0, 0, 1);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

				// Update and Render additional Platform Windows
				// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
				//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
				if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					GLFWwindow* backup_current_context = glfwGetCurrentContext();
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
					glfwMakeContextCurrent(backup_current_context);
				}

				mWindow.SwapBuffers();
			}

			mDispatcher.disconnect(this);
			mRegistry.clear();

			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}
	private:

		// Create a structure within the window to hold events and a dispatcher for window events
		// These must be heap allocated so they can be moved without reallocation
		struct WindowCloseEvent {};

		void OnClose(const WindowCloseEvent& e)
		{
			mRunning = false;
		}	
	public:
		bool mRunning = true;
		FoxEngine::Window mWindow;
		entt::registry mRegistry;

		entt::dispatcher mDispatcher{};
	};
}

int main(int argc, char* argv[])
{
	FoxEngine::LogInfo("Welcome to FoxEngine");
	stbi_set_flip_vertically_on_load(true);

	FoxEngine::Engine engine;
	engine.Start();
}