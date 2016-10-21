#include"internal.h"
#include<stdio.h>
#include<unistd.h>
#include<GL/gl.h>
#include<GL/glext.h>



void resize_screen_gl(ExEvent* event, struct uniform_location_t* uniform, ExShader* shader, ExTexture* ftexture){


}


void update_shader_uniform_gl(struct uniform_location_t* uniform, ExShader* shader, int width, int height){

}

void displaygraphic_gl(ExWin drawable){
	/*	draw quad.	*/
	glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(quad) / sizeof(quad[0]));

	/**/
	pswapbuffer(drawable);
}





void glslview_update_uniforms_gl(UniformLocation* uniform, ExShader* shader, float ttime, long int deltatime){

	if(uniform->time != -1){
		glUniform1fv(uniform->time, 1, &ttime);
	}
	if(uniform->deltatime != -1){
		glUniform1f(uniform->deltatime, (float)((float)deltatime / (float)1E9));
	}

	/*	*/
	if(use_stdin_as_buffer){
		int buffer;
		if(read(STDIN_FILENO, (void*)&buffer, stdin_buffer_size) > 0){
			glUniform1iv(uniform->stdin, sizeof(GLint), (const GLint*)&buffer);
		}
	}

}
