#pragma once

#include "../../util/list.cpp"
#include "canvasContext.cpp"
#include "color.cpp"
#include <emscripten/html5.h>
#include <math.h>
#include <stdio.h>
#include <webgl/webgl2.h>

GLuint compileShader(GLenum shaderType, const char *src)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE)
    {
        printf("Shader source: %s\n", src);
        printf("Compile status: %i\n", compileStatus);
        GLchar infoLog[10000];
        GLsizei infoLogLength;
        glGetShaderInfoLog(shader, 10000, &infoLogLength, infoLog);
        printf("Shader compile log: %s\n", infoLog);
    }
    return shader;
}

GLuint createProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glBindAttribLocation(program, 0, "pos");
    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("Program link status: %i\n", status);
        GLchar infoLog[10000];
        GLsizei infoLogLength;
        glGetProgramInfoLog(program, 10000, &infoLogLength, infoLog);
        printf("Program info log: %s\n", infoLog);
    }
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE)
    {
        printf("Program validate status: %i\n", status);
        GLchar infoLog[10000];
        GLsizei infoLogLength;
        glGetProgramInfoLog(program, 10000, &infoLogLength, infoLog);
        printf("Program info log: %s\n", infoLog);
    }
    return program;
}

GLuint createTexture(bool isWebGL2)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLint minFilter = isWebGL2 ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return texture;
}

void deleteTexture(GLuint texture)
{
    glDeleteTextures(1, &texture);
}

struct QuadVertex
{
    int x, y;
};

// 6 vertex quad
static QuadVertex _quadVertexValues[] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 0 }, { 0, 1 }, { 1, 1 } };
static const List<QuadVertex> quadVertices = { _quadVertexValues, 6, 6 };

void initTransformationMatrix(
    float *mat, CanvasContext *canvasCtx, float displayViewportX, float displayViewportY)
{
    float drawViewportX = displayViewportX * canvasCtx->devicePixelRatio;
    float drawViewportY = displayViewportY * canvasCtx->devicePixelRatio;

    float glViewportX = roundf(-drawViewportX) * canvasCtx->drawPixelWidth - 1;
    float glViewportY = roundf(drawViewportY) * canvasCtx->drawPixelHeight + 1;

    mat[0] = canvasCtx->displayPixelWidth;
    mat[1] = 0;
    mat[2] = 0;
    mat[3] = 0;

    mat[4] = 0;
    mat[5] = -canvasCtx->displayPixelHeight;
    mat[6] = 0;
    mat[7] = 0;

    mat[8] = 0;
    mat[9] = 0;
    mat[10] = 1;
    mat[11] = 0;

    mat[12] = glViewportX;
    mat[13] = glViewportY;
    mat[14] = 0;
    mat[15] = 1;
}

struct GlColor
{
    GLfloat r, g, b, a;

    static GlColor create(Color color)
    {
        return { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
    }
};

void glUniformColor4f(GLuint location, Color color)
{
    GlColor glColor = GlColor::create(color);
    glUniform4f(location, glColor.r, glColor.g, glColor.b, glColor.a);
}
