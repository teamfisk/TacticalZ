#ifndef ShaderProgram_h__
#define ShaderProgram_h__

#include "../Common.h"
#include "../OpenGL.h"
#include "../Core/ResourceManager.h"
#include <fstream>

class Shader
{
public:
	static GLuint CompileShader(GLenum shaderType, std::string fileName);

	Shader(GLenum shaderType, std::string fileName);

	virtual ~Shader();

	GLuint Compile();

	GLenum GetType() const;
	std::string GetFileName() const;
	GLuint GetHandle() const;
	bool IsCompiled() const;
protected:
	GLenum m_ShaderType;
	std::string m_FileName;
	GLint m_ShaderHandle;
};

template <int SHADERTYPE>
class ShaderType : public Shader
{
public:
	ShaderType(std::string fileName)
		: Shader(SHADERTYPE, fileName) { }
};

class VertexShader : public ShaderType<GL_VERTEX_SHADER>
{
public:
	VertexShader(std::string fileName)
		: ShaderType(fileName) { }
};

class FragmentShader : public ShaderType<GL_FRAGMENT_SHADER>
{
public:
	FragmentShader(std::string fileName)
		: ShaderType(fileName) { }
};

class GeometryShader : public ShaderType<GL_GEOMETRY_SHADER>
{
public:
	GeometryShader(std::string fileName)
		: ShaderType(fileName) { }
};

class ComputeShader : public ShaderType<GL_COMPUTE_SHADER>
{
public:
	ComputeShader(std::string fileName)
		: ShaderType(fileName) { }
};

class ShaderProgram : public Resource
{
    friend class ResourceManager;
private:
	ShaderProgram(std::string)
		: m_ShaderProgramHandle(0) { }
public:
	~ShaderProgram();

	void AddShader(std::shared_ptr<Shader> shader);
	void Compile();
	GLuint Link();
	GLuint GetHandle();
	void Bind();
	void Unbind();
    void BindFragDataLocation(int index, std::string name);

private:
	GLuint m_ShaderProgramHandle;
	std::vector<std::shared_ptr<Shader>> m_Shaders;
};

#endif