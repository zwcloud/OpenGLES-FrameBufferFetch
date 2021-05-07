//
// This file is used by the template to render a basic scene using GL.
//
#include "pch.h"

#include "SimpleRenderer.h"

// These are used by the shader compilation methods.
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cassert>

float VertexData[] = 
{
	// float3 position, float3 color
	-0.5, -0.5, 0,			1, 0, 0,
	0.5, -0.5, 0,			0, 1, 0,
	0, 0.5, 0,				0, 0, 1,
};

uint32_t IndexData[] = {0, 2, 1};

GLuint CompileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);

    const char *sourceArray[1] = { source.c_str() };
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    if (compileResult == 0)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, (GLsizei)infoLog.size(), NULL, infoLog.data());

        std::string errorMessage = std::string("Shader compilation failed: ");
        errorMessage += std::string(infoLog.begin(), infoLog.end()); 
        const char* msg = errorMessage.c_str();
        throw std::runtime_error(msg);
    }

    return shader;
}

GLuint CompileProgram(const std::string &vsSource, const std::string &fsSource)
{
    GLuint program = glCreateProgram();

    if (program == 0)
    {
        throw std::runtime_error("Program creation failed");
    }

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    if (vs == 0 || fs == 0)
    {
        glDeleteShader(fs);
        glDeleteShader(vs);
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == 0)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program, (GLsizei)infoLog.size(), NULL, infoLog.data());

        std::string errorMessage = std::string("Program link failed: ");
        errorMessage += std::string(infoLog.begin(), infoLog.end()); 

        throw std::runtime_error(errorMessage.c_str());
    }

    return program;
}

SimpleRenderer::SimpleRenderer() :
    mWindowWidth(0),
    mWindowHeight(0),
    mDrawCount(0)
{
    // Vertex Shader source
    const std::string vs = R"(
#version 310 es
in vec4 vPosition;
in vec4 vColor;
out vec4 pColor;
void main()
{
	gl_Position = vec4(vPosition.xy, 0.0, 1.0);
	pColor = vColor;
})";

    // Fragment Shader source
    const std::string fs = R"(
#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
precision mediump float;
in vec4 pColor;
layout(location = 0) inout vec4 fragColor;
void main()
{
#ifdef GL_EXT_shader_framebuffer_fetch
	vec4 Color = vec4(0,0.02,0,1);
    Color.g += fragColor.g;
    Color.g = mod(Color.g, 1.0f);
#else
    vec4 Color = 0.5 * pColor + 0.5 * vec4(1,0,0,1);
#endif
    fragColor = Color;
})";

    mProgram = CompileProgram(vs, fs);
    attributePos = glGetAttribLocation(mProgram, "vPosition");
    attributeColor = glGetAttribLocation(mProgram, "vColor");

	glGenBuffers(1, &vertexBuf);
	assert(vertexBuf != 0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), (GLvoid*)VertexData, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuf);
	assert(indexBuf != 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndexData), IndexData, GL_STATIC_DRAW);
}

SimpleRenderer::~SimpleRenderer()
{
    if (mProgram != 0)
    {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }

    if (vertexBuf != 0)
    {
        glDeleteBuffers(1, &vertexBuf);
        vertexBuf = 0;
    }

    if (indexBuf != 0)
    {
        glDeleteBuffers(1, &indexBuf);
        indexBuf = 0;
    }
}

void SimpleRenderer::Draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (mProgram == 0)
        return;

    glUseProgram(mProgram);
	
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);	
	glVertexAttribPointer(attributePos, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), 0);
	glVertexAttribPointer(attributeColor, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(attributePos);
	glEnableVertexAttribArray(attributeColor);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    glDrawElements(GL_TRIANGLES, sizeof(IndexData) / sizeof(uint32_t), GL_UNSIGNED_INT, 0);
	
	auto err = glGetError();

    mDrawCount += 1;
}

void SimpleRenderer::UpdateWindowSize(GLsizei width, GLsizei height)
{
    glViewport(0, 0, width, height);
    mWindowWidth = width;
    mWindowHeight = height;
}
