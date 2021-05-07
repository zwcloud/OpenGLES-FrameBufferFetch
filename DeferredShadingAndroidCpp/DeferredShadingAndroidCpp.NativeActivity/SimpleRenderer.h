#pragma once

#include <string.h>

#ifdef __APPLE__
#include <unistd.h>
#include <sys/resource.h>

#include <OpenGLES/ES2/gl.h>
#else // __ANDROID__ or _WIN32
#include <GLES2/gl2.h>
#endif

class SimpleRenderer
{
public:
    SimpleRenderer();
    ~SimpleRenderer();
    void Draw();
    void UpdateWindowSize(GLsizei width, GLsizei height);

private:
    GLuint mProgram;
    GLsizei mWindowWidth;
    GLsizei mWindowHeight;

    GLint attributePos;
    GLint attributeColor;

    GLuint vertexBuf;
    GLuint indexBuf;

    int mDrawCount;
};
