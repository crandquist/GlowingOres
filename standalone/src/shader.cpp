#include "shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    // Print current working directory and check if files exist
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    std::cout << "Checking if vertex shader exists: " << std::filesystem::exists(vertexPath) << std::endl;
    std::cout << "Checking if fragment shader exists: " << std::filesystem::exists(fragmentPath) << std::endl;
    
    // 1. Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    
    // Ensure ifstream objects can throw exceptions
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try {
        // Open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        
        // Read file's buffer contents into streams
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        
        // Close file handlers
        vShaderFile.close();
        fShaderFile.close();
        
        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        
        std::cout << "Successfully loaded shader files:" << std::endl;
        std::cout << "Vertex shader: " << vertexPath << std::endl;
        std::cout << "Fragment shader: " << fragmentPath << std::endl;
        std::cout << "Vertex shader code length: " << vertexCode.length() << std::endl;
        std::cout << "Fragment shader code length: " << fragmentCode.length() << std::endl;
    } catch(std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        std::cerr << "Vertex path: " << vertexPath << std::endl;
        std::cerr << "Fragment path: " << fragmentPath << std::endl;
        throw;
    }
    
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    
    // 2. Compile shaders
    unsigned int vertex, fragment;
    
    // Vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    checkGLError("vertex shader compilation");
    
    // Fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    checkGLError("fragment shader compilation");
    
    // Shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    checkGLError("shader program linking");
    
    // Delete shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    std::cout << "Shader program created with ID: " << ID << std::endl;
}

void Shader::use() {
    glUseProgram(ID);
    checkGLError("using shader program");
}

void Shader::setBool(const std::string &name, bool value) const {
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;  // Skip setting the uniform
    }
    glUniform1i(location, (int)value);
    checkGLError("setting bool uniform");
}

void Shader::setInt(const std::string &name, int value) const {
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;  // Skip setting the uniform
    }
    glUniform1i(location, value);
    checkGLError("setting int uniform");
}

void Shader::setFloat(const std::string &name, float value) const {
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;  // Skip setting the uniform
    }
    glUniform1f(location, value);
    checkGLError("setting float uniform");
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;  // Skip setting the uniform
    }
    glUniform3fv(location, 1, glm::value_ptr(value));
    checkGLError("setting vec3 uniform");
}

void Shader::setMat4(const std::string &name, const glm::mat4 &value) const {
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;  // Skip setting the uniform
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    checkGLError("setting mat4 uniform");
}

void Shader::checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" 
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            
            // Added: Print the shader source for debugging
            int shaderLen;
            glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &shaderLen);
            char* sourceStr = new char[shaderLen];
            glGetShaderSource(shader, shaderLen, NULL, sourceStr);
            std::cerr << "Shader source:\n" << sourceStr << std::endl;
            delete[] sourceStr;
            
            throw std::runtime_error("Shader compilation failed");
        } else {
            std::cout << "Shader compilation successful: " << type << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" 
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            throw std::runtime_error("Shader program linking failed");
        } else {
            std::cout << "Program linking successful" << std::endl;
        }
    }
}

void Shader::checkGLError(const char* operation) const {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after " << operation << ": " 
                  << error << " (0x" << std::hex << error << std::dec << ")" << std::endl;
    }
}