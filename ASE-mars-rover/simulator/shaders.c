#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* The rover vertex shader */
const char vertex_src [] = (
    "#version 120\n varying float target; varying float camera_distance;"
    "void main() {"
    /* Compute the distance to (0,0) (for vertex shader) */
      "target = sqrt(gl_Vertex.x*gl_Vertex.x+gl_Vertex.z*gl_Vertex.z);"
    /* Get coordinates in camera space */
      "vec4 cs_position = gl_ModelViewMatrix * gl_Vertex;"
    /* Compute the distance to the camera (for vertex shader)*/
      "camera_distance = -cs_position.z;"
     /* Return the vertex projection position */
      "gl_Position = gl_ProjectionMatrix * cs_position;"
    "}");

/* The rover fragment shader */
const char fragment_src [] = (
    "#version 120\n varying float target; varying float camera_distance;"
    "void main() {"
    /* Two things happen here: if the distance to origin is less than 10 meters,
       the fragments are red. This allows to see the ROVER target.
       If not, the fragment color is chosen depending on
       the distance to the point 0 meters -> white, 100 meters -> black */
    "gl_FragColor = (target< 5.0)?vec4(1.0,0.0,0.0,1.0): "
    "(camera_distance > 100) ? vec4(0.0,0.0,0.0,0.0): "
    "mix(vec4(1.0, 1.0, 1.0, 1.0), vec4(0.0, 0.0, 0.0, 1.0), camera_distance/100.0);"
    "}");

/* print_shader_info: debug problems with shaders */
static void print_shader_info(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}

/* load_shader: load a shader GLSL source */
static GLuint load_shader (const char  *shader_source, GLenum type)
{
   GLuint  shader = glCreateShader( type );
   glShaderSource  ( shader , 1 , &shader_source , NULL );
   glCompileShader ( shader );
   print_shader_info( shader );
   return shader;
}

/* init_shaders: init the rover shaders */
void init_shaders(void) {
	GLuint vertexShader = load_shader ( vertex_src , GL_VERTEX_SHADER );
	GLuint fragmentShader = load_shader ( fragment_src , GL_FRAGMENT_SHADER );
	GLuint shaderProgram  = glCreateProgram ();
	glAttachShader ( shaderProgram, fragmentShader );
	glAttachShader ( shaderProgram, vertexShader );
	glLinkProgram ( shaderProgram );
	glUseProgram  ( shaderProgram );
}
