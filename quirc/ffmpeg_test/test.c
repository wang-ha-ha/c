#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_ES2
#include <GLFW/glfw3.h>
#include <GL/gl.h> 
#include <EGL/egl.h> 
#include <unistd.h>
#include <KHR/khrplatform.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <math.h>

static  int WIDTH = 1920;
static  int HEIGHT = 1080;
#define PI 3.1415927



int CreateShaderProgram(const char *vertex_shader_source, const char *fragment_shader_source) {
    enum Consts {INFOLOG_LEN = 512};
    char infoLog[INFOLOG_LEN];
    int fragment_shader;
    int shader_program;
    int success;
    int vertex_shader;

    /* Vertex shader */
    // 创建一个新shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    // 加载shader源代码
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    // 编译shader
    glCompileShader(vertex_shader);
    // 获取Shader的编译情况
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    /* Fragment shader */
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    /* Link shaders */
    // 创建着色器程序
    shader_program = glCreateProgram();
     // 若程序创建成功则向程序中加入顶点着色器与片元着色器
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    // 链接程序
    glLinkProgram(shader_program);
    // 获取program的链接情况
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    // 若链接失败则报错并删除程序
    if (!success) {
        glGetProgramInfoLog(shader_program, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
    // 释放shader资源
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return shader_program;
}
// 生成纹理，编译链接着色器程序

void TextureMapSampleInit()
{
    int m_ProgramObj = 0;
    GLuint  m_SamplerLoc[2];
    GLuint   FBO;
    GLuint VAO;
    GLuint VBO[2];
    GLubyte pixels[4 * 4 * 4] =
    {
        255, 0, 0,255,  0, 255, 0,255,
        0, 0, 255, 255,  255, 255, 0,255,
        255, 0, 0,255,  0, 255, 0,255,
        0, 0, 255, 255,  255, 255, 0,255,
            255, 0, 0,255,  0, 255, 0,255,
            0, 0, 255, 255,  255, 255, 0,255,
            255, 0, 0,255,  0, 255, 0,255,
            0, 0, 255, 255,  255, 255, 0,255,
    };
    GLubyte pixels1[4 * 4 * 4] =
    {
        255, 0, 0,255,  255, 0, 0,255,
        255, 0, 0, 255,  255, 0, 0,255,
        255, 0, 0,255,  255, 0, 0,255,
        255, 0, 0, 255,  255, 0, 0,255,
        
        255, 0, 0,255,  255, 0, 0,255,
        255, 0, 0, 255,  255, 0, 0,255,
        255, 0, 0,255,  255, 0, 0,255,
        255, 0, 0, 255,  255, 0, 0,255,
    };

GLubyte outpixels[4 * 4]= {0,0};

    GLFWwindow* window;
    WIDTH = 256;
    HEIGHT = 256;
    
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(WIDTH, HEIGHT, __FILE__, NULL, NULL);
    glfwMakeContextCurrent(window);

    printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
    printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );
    // 解绑纹理
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    // 解绑 FBO

    //纹理初始化
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);               
    glClearDepth(1.0f);                                 
    glDepthFunc(GL_LEQUAL);                             
    glEnable(GL_DEPTH_TEST);                            




        GLfloat verticesCoords[] = {
            -0.5,   0.5f,  0.75,  
            -0.5f,  -0.5f,  0.75, 
            0.5f,   -0.5f,  0.75, 
            
            -0.5,    0.5f, 0.75,  
            0.5f,  -0.5f,  0.75,   
            0.5f,   0.5f,  0.75, 
            
            -0.5,   0.5f,  -0.75, 
            -0.5f,  -0.5f,  -0.75,
            0.5f,   -0.5f,  -0.75,
            -0.5,    0.5f, -0.75, 
            0.5f,  -0.5f,  -0.75,  
            0.5f,   0.5f,  -0.75,
    
            // X轴0.5处的平面
            0.5,  -0.5,    0.75f, 
            0.5,  -0.5f,  -0.75f, 
            0.5,  0.5f,   -0.75f, 
            
            0.5,  -0.5f,   0.75f,
            0.5,  0.5f,   -0.75f, 
            0.5,  0.5f,   0.75f, 
            // X轴-0.5处的平面
            -0.5,  -0.5,    0.75f,
            -0.5,  -0.5f,  -0.75f, 
            -0.5,  0.5f,   -0.75f, 
            
            -0.5,  -0.5f,   0.75f, 
            -0.5,  0.5,    -0.75f, 
            -0.5,  0.5f,    0.75f, 
    
    
            // Y轴0.5处的平面
    
            -0.5,  0.5,  0.75f, 
            -0.5f, 0.5, -0.75f, 
             0.5f, 0.5,  -0.75f, 
            
            -0.5f, 0.5,  0.75f, 
             0.5,  0.5,  -0.75f, 
             0.5f, 0.5,  0.75f, 
            
            -1.0, -0.5,   1.75f,
            -1.0f, -0.5, -1.75f,
             1.0f, -0.5,  -1.75f,
            
            -1.0f, -0.5,  1.75f,
             1.0,  -0.5,  -1.75f,
             1.0f, -0.5,   1.75f,
    
        };
    //纹理坐标
        GLfloat textureCoords[] = {
        0.0f, 0.0f,        // TexCoord 0
        0.0f,  1.0f,        // TexCoord 1
        1.0f,  1.0f,        // TexCoord 2
        
        0.0f,  0.0f,        // TexCoord 0
        1.0f,  1.0f,        // TexCoord 2
        1.0f,  0.0f,         // TexCoord 3
    
                
        0.0f, 0.0f,        // TexCoord 0
        0.0f,  1.0f,        // TexCoord 1
        1.0f,  1.0f,        // TexCoord 2
        0.0f,  0.0f,        // TexCoord 0
        1.0f,  1.0f,        // TexCoord 2
        1.0f,  0.0f,         // TexCoord 3
    
        0.0f, 0.0f,        // TexCoord 0
        0.0f,  1.0f,        // TexCoord 1
        1.0f,  1.0f,        // TexCoord 2
        0.0f,  0.0f,        // TexCoord 0
        1.0f,  1.0f,        // TexCoord 2
        1.0f,  0.0f,         // TexCoord 3
        
        0.0f, 0.0f,        // TexCoord 0
        0.0f,  1.0f,        // TexCoord 1
        1.0f,  1.0f,        // TexCoord 2
        0.0f,  0.0f,        // TexCoord 0
        1.0f,  1.0f,        // TexCoord 2
        1.0f,  0.0f,         // TexCoord 3
        
        0.0f, 0.0f,        // TexCoord 0
        0.0f,  1.0f,        // TexCoord 1
        1.0f,  1.0f,        // TexCoord 2
        0.0f,  0.0f,        // TexCoord 0
        1.0f,  1.0f,        // TexCoord 2
        1.0f,  0.0f,         // TexCoord 3
        
        0.0f, 0.0f,        // TexCoord 0
        0.0f,  1.0f,        // TexCoord 1
        1.0f,  1.0f,        // TexCoord 2
        0.0f,  0.0f,        // TexCoord 0
        1.0f,  1.0f,        // TexCoord 2
        1.0f,  0.0f,         // TexCoord 3
    
        };





    GLuint m_TextureId;
    GLuint  m_TextureId1;

    glGenBuffers(2, VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    // 1. 绑定VAO
    glBindVertexArray(VAO);
    // 2. 把顶点数组复制到缓冲中提供给OpenGL使用
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glEnableVertexAttribArray (0);
	// Load the vertex position
	glVertexAttribPointer (0, 3, GL_FLOAT,
							GL_FALSE, 3 * sizeof (GLfloat), (GLvoid * )0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glEnableVertexAttribArray (1);
	// Load the texture coordinate
	glVertexAttribPointer (1, 2, GL_FLOAT,
							GL_FALSE, 2 * sizeof (GLfloat), (GLvoid * )0);
    glBindVertexArray(GL_NONE);





    glActiveTexture(GL_TEXTURE0);
    //create RGBA texture
    //生成一个纹理，将纹理 id 赋值给 m_TextureId
    glGenTextures(1, &m_TextureId); 
    
    //将纹理 m_TextureId 绑定到类型 GL_TEXTURE_2D前激活的纹理单元 GL_TEXTURE0 纹理
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    
    //设置纹理 S 轴（横轴）的拉伸方式为截取
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    //设置纹理 T 轴（纵轴）的拉伸方式为截取
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    //设置纹理采样方式 线性纹理采样
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    //加载 RGBA 格式的图像数据
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4,4, 0, GL_RGBA, GL_UNSIGNED_BYTE,pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);


    
    glActiveTexture(GL_TEXTURE1);
    
    //生成一个纹理，将纹理 id 赋值给 m_TextureId
    glGenTextures(1, &m_TextureId1); 
    
    //将纹理 m_TextureId 绑定到类型 GL_TEXTURE_2D前激活的纹理单元 GL_TEXTURE1 纹理
    
    glBindTexture(GL_TEXTURE_2D, m_TextureId1);
    
    //设置纹理 S 轴（横轴）的拉伸方式为截取
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    //设置纹理 T 轴（纵轴）的拉伸方式为截取
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    //设置纹理采样方式 线性纹理采样
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    //加载 RGBA 格式的图像数据
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4,4, 0, GL_RGBA, GL_UNSIGNED_BYTE,pixels1);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);



    char vShaderStr[] =
            "#version 300 es                            \n"
            "layout(location = 0) in vec4 a_position;   \n"
            "layout(location = 1) in vec2 a_texCoord;   \n"
            "out vec2 v_texCoord;                       \n"
            "uniform float degree; \n"
            "uniform float degree2; \n"
            "void main()                                \n"
            "{                                          \n"
                "gl_Position = mat4( \n"
                "cos(degree),0           ,-sin(degree), 0,\n " 
                "0          ,1           ,0           , 0, \n" 
                "sin(degree),0           ,cos(degree) , 0, \n" 
                "0          ,0           ,0           , 1 \n" 
                ") * mat4( \n"
                "1          ,0           ,0           , 0, \n" 
                "0          ,cos(degree2) ,-sin(degree2), 0,\n " 
                "0          ,sin(degree2) ,cos(degree2) , 0, \n" 
                "0          ,0           ,0           , 1 \n" 
                ") *a_position;\n" 
            "   v_texCoord = a_texCoord;                \n"
            "}                                          \n";
    
    char fShaderStr[] =
            "#version 300 es                                     \n"
            "precision mediump float;                            \n"
            "in vec2 v_texCoord;                                 \n"
            "layout(location = 0) out vec4 outColor;             \n"
            "uniform sampler2D tex1;                     \n"
            "uniform sampler2D tex2;                     \n"
            "void main()                                         \n"
            "{                                                   \n"
            "   if(v_texCoord.x < 0.5) \n"
            "     outColor =  texture2D(tex1, v_texCoord);\n"
            "   else \n"
            "     outColor =  texture2D(tex2, v_texCoord);\n"
            "}                                                   \n";
    

	m_ProgramObj = CreateShaderProgram(vShaderStr, fShaderStr);
    if (!m_ProgramObj)
	{
		printf("TextureMapSample::Init create program fail");
	}

	if(m_ProgramObj == GL_NONE || m_TextureId == GL_NONE || m_TextureId1 == GL_NONE) return;
    //顶点坐标
	glUseProgram (m_ProgramObj);
    

    

    
    printf("tex1[0]  : %d\n", glGetUniformLocation(m_ProgramObj, "tex1"));
    printf("tex2[1]  : %d\n", glGetUniformLocation(m_ProgramObj, "tex2"));
    



    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glUniform1i(glGetUniformLocation(m_ProgramObj, "tex1"), 0);

    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_TextureId1);
    glUniform1i(glGetUniformLocation(m_ProgramObj, "tex2"), 1);
    glBindVertexArray(VAO);
    
    float angle = 0.0f;
    float degree =  angle * 3.1415926 / 180.0f;
   
      while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // 图形绘制
        //getchar();
        
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
        angle +=0.2;
        degree =  angle * 3.1415926 / 180.0f;
       glUniform1f(glGetUniformLocation(m_ProgramObj, "degree"), degree);
        
        glUniform1f(glGetUniformLocation(m_ProgramObj, "degree2"), 15.0* 3.1415926 / 180.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
    }
    glfwTerminate();

}

// 生成纹理，编译链接着色器程序
int TriangleSampleInit()
{
    int shader_program, vbo;
    int pos;

    GLFWwindow* window;
    WIDTH = 1000;
    HEIGHT = 1000;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(WIDTH, HEIGHT, __FILE__, NULL, NULL);
    glfwMakeContextCurrent(window);

    printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
    printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );
    /**
     * 顶点着色器
     */
    
    static const char* vertex_shader_source =
        "#version 100\n                                 \
        attribute vec3 position;\n                      \
        uniform float degree; \n                      \
         void main() {\n                                \
                vec2 centerPos = vec2(0.0,0.0);                  \n         \
                float x0 = w * (position.x - centerPos.x) ;\n         \
                float y0 = h * (position.y - centerPos.y) ;\n         \
               vec2 result = mat2(cos(degree),-sin(degree),sin(degree),cos(degree)) * vec2(x0,y0) + centerPos;\n         \
                gl_Position =vec4(result,0.0,1.0); \n         \
         }\n";
         
    /**
     * 片段着色器
     */
    static const char* fragment_shader_source =
        "#version 100\n"
        "void main() {\n"
        "   gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    static const GLfloat vertices[] = {
         0.0f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         -0.5f, -0.5f, 0.0f,
    };
    

    shader_program = CreateShaderProgram(vertex_shader_source, fragment_shader_source);
    pos = glGetAttribLocation(shader_program, "position");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, WIDTH, HEIGHT);
    // 使用shader程序
    glUseProgram(shader_program);

    /*glGenBuffers(1, &vbo);
    //glBindBuffer(GL_ARRAY_BUFFER, vbo);
   // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //如果是0的话默认用上面buffer里面的数据，需要绑定 也可以下面那种写法
    //glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);*/
   
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    
    // 顶点颜色数据传入着色器中
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)vertices);
    // 允许使用顶点坐标数组
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        // 图形绘制
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
    }
    glDeleteBuffers(1, &vbo);
    glfwTerminate();
    return EXIT_SUCCESS;
}


// 生成纹理，编译链接着色器程序
int VboEboSampleInit()
{
    int shader_program, vbo,m_VboIds[2],m_VaoId;
    int pos;

    GLFWwindow* window;
    WIDTH = 1000;
    HEIGHT = 1000;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(WIDTH, HEIGHT, __FILE__, NULL, NULL);
    glfwMakeContextCurrent(window);

    printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
    printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );
    /**
     * 顶点着色器
     */
    
    static const char* vertex_shader_source =
        "#version 300 es                    \n        \
layout(location = 0) in vec3 a_position;  \n  \
layout(location = 1) in vec3 a_color;     \n  \
out vec3 v_color;                        \n   \
void main()                              \n  \
{                                        \n  \
    v_color = a_color;                    \n \
    gl_Position = vec4(a_position, 1.0);            \n  \
};\n";
         
    /**
     * 片段着色器
     */
    static const char* fragment_shader_source =
        "#version 300 es\n                 \
        precision mediump float;\n \
        in vec3 v_color;\n \
        out vec4 o_fragColor;\n \
        void main()\n                                          \
        {\n \
            o_fragColor = vec4(v_color, 1.0);\n \
        }\n";
    // 4 vertices, with(x,y,z) ,(r, g, b, a) per-vertex
    GLfloat vertices[] =
            {
                    -0.5f,  0.5f, 0.0f,       // v0
                    1.0f,  0.0f, 0.0f,        // c0
                    -0.5f, -0.5f, 0.0f,       // v1
                    0.0f,  1.0f, 0.0f,        // c1
                    0.5f, -0.5f, 0.0f,        // v2
                    0.0f,  0.0f, 1.0f,        // c2
                    0.5f,  0.5f, 0.0f,        // v3
                    0.5f,  1.0f, 1.0f,        // c3
            };
    // Index buffer data
    GLushort indices[6] = { 0, 1, 2, 0, 2, 3};

    

    shader_program = CreateShaderProgram(vertex_shader_source, fragment_shader_source);
    pos = glGetAttribLocation(shader_program, "position");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, WIDTH, HEIGHT);
    // 使用shader程序
    glUseProgram(shader_program);


    
#if 1
// 创建 2 个 VBO（EBO 实际上跟 VBO 一样，只是按照用途的另一种称呼）
glGenBuffers(2, m_VboIds);

/*
//不使用 VBO 的绘制
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), vertices);

glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), (vertices + 3));

glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);*/

//使用 VBO 的绘制

glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), (const void *)0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), (const void *)(3 *sizeof(GLfloat)));
// 绑定第二个 VBO（EBO），拷贝图元索引数据到显存
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1]);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


#else
glGenBuffers(2, m_VboIds);

// 创建并绑定 VAO
//VAO（Vertex Array Object）是指顶点数组对象，VAO 的主要作用是用于管理 VBO 或 EBO ，减少 glBindBuffer 、glEnableVertexAttribArray、 glVertexAttribPointer 这些调用操作，高效地实现在顶点数组配置之间切换。

glGenVertexArrays(1, &m_VaoId);
glBindVertexArray(m_VaoId);

// 在绑定 VAO 之后，操作 VBO ，当前 VAO 会记录 VBO 的操作
glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), (const void *)0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (3+3)*sizeof(GLfloat), (const void *)(3 *sizeof(GLfloat)));

// 绑定第二个 VBO（EBO），拷贝图元索引数据到显存
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[1]);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
glBindVertexArray(GL_NONE);



// 是不是精简了很多？
glUseProgram(shader_program);

glBindVertexArray(m_VaoId);


#endif


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        // 图形绘制
        
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
        glfwSwapBuffers(window);
    }
    glDeleteBuffers(2, m_VboIds);
    glfwTerminate();
    return EXIT_SUCCESS;
}



// 离屏渲染
void offScreenSampleInit()
{
    // 创建并初始化 FBO 纹理
    
        GLubyte pixels[2 * 4 * 4] =
    {
        255, 0, 0,255,  0, 255, 0,255,
        0, 0, 255,255,  255, 255, 0,255,
        255, 0, 0,255,  0, 255, 0,255,
        0, 0, 255,255,  255, 255, 0,255,
    
    };
      GLubyte  outpixels [2 * 4 * 4]={0};
         GLFWwindow* window;
    WIDTH = 1000;
    HEIGHT = 500;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(WIDTH, HEIGHT, __FILE__, NULL, NULL);
    glfwMakeContextCurrent(window);

    GLuint m_ProgramObj ,m_FboProgramObj,m_SamplerLoc,m_FboSamplerLoc;
    GLuint m_FboTextureId,m_ImageTextureId,m_FboId,m_VboIds[4],m_VaoIds[2];
    //纹理初始化
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC_COLOR);




	//create RGBA texture
    //生成一个纹理，将纹理 id 赋值给 m_FboTextureId
    glGenTextures(1, &m_FboTextureId);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    // 创建并初始化 FBO
    glGenFramebuffers(1, &m_FboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboTextureId, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE) {
        printf("FBOSample::CreateFrameBufferObj glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
        return ;
    }
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    
        //顶点坐标
        GLfloat vVertices[] = {
                -1.0f, -1.0f, 0.0f,
                 1.0f, -1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                 1.0f,  1.0f, 0.0f,
        };
    
        //正常纹理坐标
        GLfloat vTexCoors[] = {
                0.0f, 1.0f,
                1.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f,
        };
    
        //fbo 纹理坐标与正常纹理方向不同，原点位于左下角
        GLfloat vFboTexCoors[] = {
                0.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f,
        };
    
        GLushort indices[] = { 0, 1, 2, 1, 3, 2 };
    
        char vShaderStr[] =
                "#version 300 es                            \n"
                "layout(location = 0) in vec4 a_position;   \n"
                "layout(location = 1) in vec2 a_texCoord;   \n"
                "out vec2 v_texCoord;                       \n"
                "vec2 a_texCoord11[10000];  \n"
                "void main()                                \n"
                "{                                          \n"
                "   gl_Position = a_position;               \n"
                "   v_texCoord = a_texCoord;                \n"
                "}                                          \n";
    
        // 用于普通渲染的片段着色器脚本，简单纹理映射
        char fShaderStr[] =
                "#version 300 es\n"
                "precision mediump float;\n"
                "in vec2 v_texCoord;\n"
                "layout(location = 0) out vec4 outColor;\n"
                "uniform sampler2D s_TextureMap;\n"
                "void main()\n"
                "{\n"
                "    outColor = texture(s_TextureMap, v_texCoord);\n"
                "}";
    
        // 用于离屏渲染的片段着色器脚本，取每个像素的灰度值
        char fFboShaderStr[] =
                "#version 300 es\n"
                "precision mediump float;\n"
                "in vec2 v_texCoord;\n"
                "layout(location = 0) out vec4 outColor;\n"
                "uniform sampler2D s_TextureMap;\n"
                "void main()\n"
                "{\n"
                "    vec4 tempColor = texture(s_TextureMap, v_texCoord);\n"
                "    float luminance = tempColor.r * 0.299 + tempColor.g * 0.587 + tempColor.b * 0.114;\n"
                "    outColor = tempColor;\n"
                "}"; // 输出灰度图
    
        // 编译链接用于普通渲染的着色器程序
        m_ProgramObj = CreateShaderProgram(vShaderStr, fShaderStr);
    
        // 编译链接用于离屏渲染的着色器程序
        m_FboProgramObj = CreateShaderProgram(vShaderStr, fFboShaderStr);
    
        if (m_ProgramObj == GL_NONE || m_FboProgramObj == GL_NONE)
        {
            printf("FBOSample::Init m_ProgramObj == GL_NONE");
            return;
        }
        m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
        m_FboSamplerLoc = glGetUniformLocation(m_FboProgramObj, "s_TextureMap");
    
        // 生成 VBO ，加载顶点数据和索引数据
        // Generate VBO Ids and load the VBOs with data
        glGenBuffers(4, m_VboIds);
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vTexCoors), vTexCoors, GL_STATIC_DRAW);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vFboTexCoors), vFboTexCoors, GL_STATIC_DRAW);
    
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
        //GO_CHECK_GL_ERROR();
    
        // 生成 2 个 VAO，一个用于普通渲染，另一个用于离屏渲染
        // Generate VAO Ids
        glGenVertexArrays(2, m_VaoIds);
        // 初始化用于普通渲染的 VAO
        // Normal rendering VAO
        glBindVertexArray(m_VaoIds[0]);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
        //GO_CHECK_GL_ERROR();
        glBindVertexArray(GL_NONE);
    
        // 初始化用于离屏渲染的 VAO
        // FBO off screen rendering VAO
        glBindVertexArray(m_VaoIds[1]);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
        //GO_CHECK_GL_ERROR();
        glBindVertexArray(GL_NONE);
    
        // 创建并初始化图像纹理
        glGenTextures(1, &m_ImageTextureId);
        glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2,4, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
       // GO_CHECK_GL_ERROR();
    
       // if (!CreateFrameBufferObj())
        {
           // printf("FBOSample::Init CreateFrameBufferObj fail");
           // return;
        }




        
        // 离屏渲染
        glPixelStorei(GL_UNPACK_ALIGNMENT,1);  //把像素从规则的状态转变到一堆X的状态（
        //把花盆里的泥倒出来，把货柜中的货物卸载到盐田港，或者解压压缩包，等等） 感觉没啥用好像是设置为1对齐
        glViewport(0, 0, 2, 4);
    
        // Do FBO off screen rendering
        glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
        glUseProgram(m_FboProgramObj);
        glBindVertexArray(m_VaoIds[1]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
        glUniform1i(m_FboSamplerLoc, 0);
        //GO_CHECK_GL_ERROR();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
      //  GO_CHECK_GL_ERROR();
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    

        //先读完数据再解
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glReadPixels(0,0, 2, 4, GL_RGBA, GL_UNSIGNED_BYTE, outpixels);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    printf("outpixels : %d\n", outpixels[0]);
    printf("outpixels : %d\n", outpixels[1]);
    printf("outpixels : %d\n", outpixels[2]);
    
        // 普通渲染
        // Do normal rendering
        glViewport(0, 0, WIDTH, HEIGHT);
        glUseProgram(m_ProgramObj);
        //GO_CHECK_GL_ERROR();
        glBindVertexArray(m_VaoIds[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
        glUniform1i(m_SamplerLoc, 0);
        //GO_CHECK_GL_ERROR();
       // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
       // GO_CHECK_GL_ERROR();
    
        while (!glfwWindowShouldClose(window)) {
          glfwPollEvents();
          glClear(GL_COLOR_BUFFER_BIT);
          // 图形绘制
          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
          glfwSwapBuffers(window);
        }
        
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
        glBindVertexArray(GL_NONE);
    return ;

}



// 生贝塞尔曲线
int BeiSaiErQuXianSampleInit()
{
    // 创建并初始化 FBO 纹理
    int img_width = 2624;
    int img_height = 1472;
    FILE *fp = NULL;
    fp = fopen("2624x1472.yuv", "r");
    if (NULL == fp) {
        printf("<openbmp>Fail to open file 1280x720_video0!" );
    }
    int offset = 0;
    unsigned char *buf = NULL;

    //get watermark picture size        
    buf = (unsigned char *)malloc(img_width * img_height * 3/2);
    if (NULL == buf) {
        
    printf("malloc Fail to open file 1280x720_video0!" );
        fclose(fp);
    }
    fread(buf, (img_width * img_height * 3/2) , 1, fp);
    fclose(fp);

    
    unsigned char  *outpixels = (unsigned char *) malloc(img_width* img_height * 4);
    GLFWwindow* window;
    WIDTH = img_width;
    HEIGHT = img_height;
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(WIDTH, HEIGHT, __FILE__, NULL, NULL);
    glfwMakeContextCurrent(window);

    GLuint m_ProgramObj ,m_FboProgramObj,m_SamplerLoc,m_FboSamplerLoc;
    GLuint m_FboTextureId,m_ImageTextureId,m_FboId,m_VboIds[4],m_VaoIds[2];
    //纹理初始化
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC_COLOR);

    //create RGBA texture
    //生成一个纹理，将纹理 id 赋值给 m_FboTextureId
    glGenTextures(1, &m_FboTextureId);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    // 创建并初始化 FBO
    glGenFramebuffers(1, &m_FboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
    glBindTexture(GL_TEXTURE_2D, m_FboTextureId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FboTextureId, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_width, img_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE) {
        printf("FBOSample::CreateFrameBufferObj glCheckFramebufferStatus status != GL_FRAMEBUFFER_COMPLETE");
        return ;
    }
    glEnable(GL_MULTISAMPLE);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    
        //顶点坐标
        GLfloat vVertices[] = {
                -1.0f, -1.0f, 0.0f,
                 1.0f, -1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                 1.0f,  1.0f, 0.0f,
        };
    
        //正常纹理坐标
        GLfloat vTexCoors[] = {
                0.0f, 1.0f,
                1.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f,
        };
    
        //fbo 纹理坐标与正常纹理方向不同，原点位于左下角
        GLfloat vFboTexCoors[] = {
                0.0f, 0.0f,
                1.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f,
        };
    
        GLushort indices[] = { 0, 1, 2, 1, 3, 2 };
    
        char vShaderStr[] =
            "#version 300 es                            \n"
            "layout(location = 0) in vec4 aPosition;   \n"
            "layout(location = 1) in vec2 aTexCoord;   \n"
            "out vec2 v_texCoord;                       \n"
            "uniform float crop_x;\n"
            "uniform float crop_y;\n"
            "uniform float crop_scaler_x;\n"
            "uniform float crop_scaler_y;\n"
            "uniform float w;\n"
            "uniform float h;\n"
            "uniform float degree;           \n"
            "uniform float xscaler;       \n"
            "uniform float yscaler;       \n"
            "uniform float maxscale;       \n"
            "void main()                                \n"
            "{                                          \n"
            "    vec2 centerPos = vec2(0.0,0.0);                  \n"
            "    float x0 = w * (aPosition.x - centerPos.x) / maxscale;\n"
            "    float y0 = h * (aPosition.y - centerPos.y) / maxscale;\n"
            "    vec2 result = mat2(cos(degree),-sin(degree),sin(degree),cos(degree)) * vec2(x0,y0) + centerPos;\n"
            "    result = vec2((result.x / w + crop_x)*crop_scaler_x ,(result.y / h + crop_y)* crop_scaler_y);\n"
            "    gl_Position =vec4(result,0.0,1.0);               \n"
            "    v_texCoord = aTexCoord;                \n"
            "}                                          \n";
        // 用于普通渲染的片段着色器脚本，简单纹理映射
        char fShaderStr[] =
            "#version 300 es                                                                             \n"               
            "precision highp float;                                                                      \n"           
            "layout(location = 0) out vec4 outColor;                                                     \n"               
            "uniform sampler2D s_TextureMap;                                                             \n"           
            "in vec2 v_texCoord;                                                                         \n"                   
            "highp vec2 CenterPos;\n "
            "uniform float k1 ;                                                    \n"
            "uniform float k2;                                                    \n"       
            "vec2 u_ImgSize =  vec2(1.0,0.5625);//图片分辨率                                                         \n"              
            "vec2 warpEyes(vec2 centerPos, vec2 curPos)                  \n"           
            "{                                                                                           \n"       
            "    float scaler = 1.05;                                                                   \n"   
            "    vec2 imgCurPos = curPos * u_ImgSize;                                                    \n"       
            "    float d = distance(imgCurPos, centerPos) * scaler;                                               \n"   
            "    float x1 = (imgCurPos.x - centerPos.x) * (1.0 - (k1 * pow(d, 1.0) + k2 * pow(d, 2.0)));\n"
            "    float y1 = (imgCurPos.y - centerPos.y) * (1.0 - (k1 * pow(d, 1.0) + k2 * pow(d, 2.0)));\n"
            "    vec2 result = vec2(x1,y1) * scaler + centerPos;     \n"
            "    result = result / u_ImgSize;                                                        \n"       
            "    return result;                                                                          \n"           
            "}                                                                                           \n"       
            "void main()                                                                                  \n"           
            "{                                                                                            \n"       
            
            "    CenterPos= u_ImgSize / vec2(2,2);// 左眼中心点                                         \n"              
            "    vec2 newTexCoord = warpEyes(CenterPos, v_texCoord);     \n"       
            "    outColor = texture(s_TextureMap, newTexCoord);                                          \n"           
            " }";
        // 用于离屏渲染的片段着色器脚本，取每个像素的灰度值\n"
        char fFboShaderStr[] =
            "#version 300 es                                                                             \n"               
            "precision highp float;                                                                      \n"           
            "layout(location = 0) out vec4 outColor;                                                     \n"               
            "uniform sampler2D s_TextureMap;                                                             \n"           
            "in vec2 v_texCoord;                                                                         \n"                   
            "highp vec2 CenterPos;\n "
            "uniform float k1 ;                                                    \n"
            "uniform float k2;                                                    \n"       
            "vec2 u_ImgSize =  vec2(1.0,0.5625);//图片分辨率                                                         \n"              
            "vec2 warpEyes(vec2 centerPos, vec2 curPos)                  \n"           
            "{                                                                                           \n"       
            "    float scaler = 1.05;                                                                   \n"   
            "    vec2 imgCurPos = curPos * u_ImgSize;                                                    \n"       
            "    float d = distance(imgCurPos, centerPos) * scaler;                                               \n"   
            "    float x1 = (imgCurPos.x - centerPos.x) * (1.0 - (k1 * pow(d, 1.0) + k2 * pow(d, 2.0)));\n"
            "    float y1 = (imgCurPos.y - centerPos.y) * (1.0 - (k1 * pow(d, 1.0) + k2 * pow(d, 2.0)));\n"
            "    vec2 result = vec2(x1,y1) * scaler + centerPos;     \n"
            "    result = result / u_ImgSize;                                                        \n"       
            "    return result;                                                                          \n"           
            "}                                                                                           \n"       
            "void main()                                                                                  \n"           
            "{                                                                                            \n"       
            
            "    CenterPos= u_ImgSize / vec2(2,2);// 左眼中心点                                         \n"              
            "    vec2 newTexCoord = warpEyes(CenterPos, v_texCoord);     \n"       
            "    outColor = texture(s_TextureMap, newTexCoord);                                          \n"           
            " }";
    
        // 编译链接用于普通渲染的着色器程序
        m_ProgramObj = CreateShaderProgram(vShaderStr, fShaderStr);
    
        // 编译链接用于离屏渲染的着色器程序
        m_FboProgramObj = CreateShaderProgram(vShaderStr, fFboShaderStr);
    
        if (m_ProgramObj == GL_NONE || m_FboProgramObj == GL_NONE)
        {
            printf("FBOSample::Init m_ProgramObj == GL_NONE");
            return;
        }
        m_SamplerLoc = glGetUniformLocation(m_ProgramObj, "s_TextureMap");
        m_FboSamplerLoc = glGetUniformLocation(m_FboProgramObj, "s_TextureMap");
        // 生成 VBO ，加载顶点数据和索引数据
        // Generate VBO Ids and load the VBOs with data
        glGenBuffers(4, m_VboIds);
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vTexCoors), vTexCoors, GL_STATIC_DRAW);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vFboTexCoors), vFboTexCoors, GL_STATIC_DRAW);
    
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
        //GO_CHECK_GL_ERROR();
    
        // 生成 2 个 VAO，一个用于普通渲染，另一个用于离屏渲染
        // Generate VAO Ids
        glGenVertexArrays(2, m_VaoIds);
        // 初始化用于普通渲染的 VAO
        // Normal rendering VAO
        glBindVertexArray(m_VaoIds[0]);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
        //GO_CHECK_GL_ERROR();
        glBindVertexArray(GL_NONE);
    
        // 初始化用于离屏渲染的 VAO
        // FBO off screen rendering VAO
        glBindVertexArray(m_VaoIds[1]);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
        glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
        //GO_CHECK_GL_ERROR();
        glBindVertexArray(GL_NONE);
    
        // 创建并初始化图像纹理
        glGenTextures(1, &m_ImageTextureId);
        glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, img_width,img_height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buf);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
       // GO_CHECK_GL_ERROR();
    
       // if (!CreateFrameBufferObj())
        {
           // printf("FBOSample::Init CreateFrameBufferObj fail");
           // return;
        }

        // 离屏渲染
        glPixelStorei(GL_UNPACK_ALIGNMENT,1);  //把像素从规则的状态转变到一堆X的状态（
        //把花盆里的泥倒出来，把货柜中的货物卸载到盐田港，或者解压压缩包，等等） 感觉没啥用好像是设置为1对齐
        glViewport(0, 0, WIDTH, HEIGHT);//和图像宽高一样输出宽高
    
        // Do FBO off screen rendering
        glBindFramebuffer(GL_FRAMEBUFFER, m_FboId);
        glUseProgram(m_FboProgramObj);
        
        glBindVertexArray(m_VaoIds[1]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
        glUniform1i(m_FboSamplerLoc, 0);

           int crop_x = 0; 
        int crop_w = WIDTH; 

        
        int crop_y = 0; 
        int crop_h = HEIGHT; 
float k1=0.555f;
        float k2=0.0002f;

        //glUniform1f(glGetUniformLocation(m_ProgramObj, "k1"), 0.23f);
        glUniform1f(glGetUniformLocation(m_FboProgramObj, "k1"), k1);
        glUniform1f(glGetUniformLocation(m_FboProgramObj, "k2"), k2);

        glUniform1f(glGetUniformLocation(m_FboProgramObj, "crop_x"), (1.0 - ((float)crop_w + 2 * (float)crop_x) / (float)WIDTH));
        glUniform1f(glGetUniformLocation(m_FboProgramObj, "crop_y"), (((float)crop_h + 2 * (float)crop_y) / (float)HEIGHT - 1.0));
        glUniform1f(glGetUniformLocation(m_FboProgramObj, "crop_scaler_x"), (float)WIDTH / (float)crop_w);
        glUniform1f(glGetUniformLocation(m_FboProgramObj, "crop_scaler_y"), (float)HEIGHT / (float)crop_h);
        
        glUniform1f(glGetUniformLocation(m_FboProgramObj, "w"), (float)WIDTH);
#if !defined(max)
#define max(A,B) ( (A) > (B) ? (A):(B))
#endif
        glUniform1f(glGetUniformLocation(m_FboProgramObj, "h"), (float)HEIGHT);
        float angle = 0.0f;
        float degree =  angle * 3.1415926 / 180.0f;
        glUniform1f(glGetUniformLocation(m_FboProgramObj, "degree"), degree);
        float xscaler = (abs((float)WIDTH * cos(degree)) + abs((float)HEIGHT * sin(degree))) / (float)WIDTH;       
        float yscaler = (abs((float)HEIGHT * cos(degree)) + abs((float)WIDTH * sin(degree))) / (float)HEIGHT;       
        float maxscale =  max(xscaler,yscaler);       
        glUniform1f(glGetUniformLocation(m_FboProgramObj, "xscaler"), xscaler);
        glUniform1f(glGetUniformLocation(m_FboProgramObj, "yscaler"), yscaler);
        glUniform1f(glGetUniformLocation(m_FboProgramObj, "maxscale"), maxscale);
        
        //GO_CHECK_GL_ERROR();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
    glReadPixels(0,0, img_width, img_height, GL_RGBA, GL_UNSIGNED_BYTE, outpixels);
    
    unsigned char  *outpixels11 = (unsigned char *) malloc(img_width*img_height*3/2);
    int i,j;
    memset (outpixels11,128,(img_width*img_height*3/2));

for(i=0;i<img_height;i++)
{
    for(j=0;j<img_width;j++)
    {

        outpixels11[i * img_width + j] = outpixels[(i * img_width + j)*4];

    }


}
      FILE *writefp = NULL;
         writefp = fopen("2624x1472_OUT.yuv", "w");
         if (NULL == writefp) {
             printf("<openbmp>Fail to open file 1280x720_video0!" );
         }
         fwrite(outpixels11, 1, (img_width*img_height*3/2), writefp);
sync();
         fclose(writefp);

      //  GO_CHECK_GL_ERROR();
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        //先读完数据再解
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
        // 普通渲染
        // Do normal rendering
        glViewport(0, 0, WIDTH, HEIGHT);
        glUseProgram(m_ProgramObj);
        glPushMatrix();
        glRotatef(50.0f,1,0,0); 
        glPopMatrix();
        //GO_CHECK_GL_ERROR();
        glBindVertexArray(m_VaoIds[0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_ImageTextureId);
        glUniform1i(m_SamplerLoc, 0);

        //glUniform1f(glGetUniformLocation(m_ProgramObj, "k1"), 0.23f);
        glUniform1f(glGetUniformLocation(m_ProgramObj, "k1"), k1);
        glUniform1f(glGetUniformLocation(m_ProgramObj, "k2"), k2);

        glUniform1f(glGetUniformLocation(m_ProgramObj, "crop_x"), (1.0 - ((float)crop_w + 2 * (float)crop_x) / (float)WIDTH));
        glUniform1f(glGetUniformLocation(m_ProgramObj, "crop_y"), (((float)crop_h + 2 * (float)crop_y) / (float)HEIGHT - 1.0));
        glUniform1f(glGetUniformLocation(m_ProgramObj, "crop_scaler_x"), (float)WIDTH / (float)crop_w);
        glUniform1f(glGetUniformLocation(m_ProgramObj, "crop_scaler_y"), (float)HEIGHT / (float)crop_h);
        
        glUniform1f(glGetUniformLocation(m_ProgramObj, "w"), (float)WIDTH);
        glUniform1f(glGetUniformLocation(m_ProgramObj, "h"), (float)HEIGHT);
        glUniform1f(glGetUniformLocation(m_ProgramObj, "degree"), degree);
        glUniform1f(glGetUniformLocation(m_ProgramObj, "xscaler"), xscaler);
        glUniform1f(glGetUniformLocation(m_ProgramObj, "yscaler"), yscaler);
        glUniform1f(glGetUniformLocation(m_ProgramObj, "maxscale"), maxscale);
        
        //GO_CHECK_GL_ERROR();
       // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
       // GO_CHECK_GL_ERROR();
    
        while (!glfwWindowShouldClose(window)) {
          glfwPollEvents();
          glClear(GL_COLOR_BUFFER_BIT);
          // 图形绘制
          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
          glfwSwapBuffers(window);
        }
        
//printf("degree %f", a );
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
        glBindVertexArray(GL_NONE);
        glfwTerminate();
    return ;

}


static EGLint const config_attribute_list[] = {
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_ALPHA_SIZE, 8,
	EGL_BUFFER_SIZE, 32,

	EGL_STENCIL_SIZE, 0,
	EGL_DEPTH_SIZE, 0,

	EGL_SAMPLES, 4,

	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,

#ifdef ON_SCREEN
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
#else
	EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
#endif
	EGL_NONE
};


static EGLint const attribute_list[] = {
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_NONE
};
int EglSampleInit111()
{
        EGLDisplay display;
        EGLConfig config;
        EGLContext context;
        EGLSurface surface;
        NativeWindowType native_window;
        
        //native_window.width = 400;
        //native_window.height = 400;
        EGLint num_config;
        /* get an EGL display connection */
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        /* initialize the EGL display connection */
        eglInitialize(display, NULL, NULL);
        /* get an appropriate EGL frame buffer configuration */
        eglChooseConfig(display, attribute_list, &config, 1, &num_config);
        /* create an EGL rendering context */
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
        /* create a native window */
        /* create an EGL window surface */
        surface = eglCreateWindowSurface(display, config, native_window, NULL);
        /* connect the context to the surface */
        eglMakeCurrent(display, surface, surface, context);
        /* clear the color buffer */
        glClearColor(1.0, 1.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glFlush();
        eglSwapBuffers(display, surface);
        sleep(10);
        return EXIT_SUCCESS;
}

// EGL例子
int EglSampleInit()
{  
  EGLDisplay display = eglGetDisplay (EGL_DEFAULT_DISPLAY); //获取显示器
  eglInitialize(display , 0, 0);                  //初始化显示器

  EGLConfig config;
  EGLint num_config;
  eglChooseConfig(display , config_attribute_list, & config, 1, &num_config );  //初始化配置
  EGLNativeWindowType ANativeWindow;

  EGLSurface surface = eglCreateWindowSurface(display, config, ANativeWindow , NULL);  //创建surface
  EGLContext  context = eglCreateContext (display, config, NULL , NULL);//创建场景
  eglMakeCurrent (display, surface, surface , context);//绑定线程

  while(1){
    //opengl绘制
    //glxx();
    
    eglSwapBuffers(display , surface );
  }

 eglDestroyContext( display , context );//销毁surface
  eglDestroySurface(display , surface );//销毁场景
  eglTerminate(display );
}


int main(void) {
    BeiSaiErQuXianSampleInit();
    //TextureMapSampleInit();
    //TextureMapSampleInit();
    return EXIT_SUCCESS;
}
