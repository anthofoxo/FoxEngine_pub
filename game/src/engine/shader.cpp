#include "Shader.hpp"

#include "ogl/ShaderOGL.hpp"

namespace FoxEngine
{
	Poly<Shader> Shader::Create(const Shader::CreateInfo& info)
	{
		return Poly<Shader>(NullOf<ShaderOGL33>, info);
	}
}