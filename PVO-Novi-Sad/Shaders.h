
#ifndef SHADERS_H
#define SHADERS_H

#include <GL/glew.h>
#include <string>

// Funkcije za rad sa �ejderima
unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);

#endif // SHADERS_H