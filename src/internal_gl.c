#include"internal.h"
#include<stdio.h>
#include<unistd.h>
#include<GL/gl.h>
#include<GL/glext.h>



void glslview_resize_screen_gl(ExEvent* event, UniformLocation* uniform, ExShader* shader, ExTexture* ftexture){
	float resolution[2] = {event->size.width, event->size.height};
	glViewport(0, 0, event->size.width, event->size.height);
	glUniform2fv(uniform->resolution, 1, &resolution[0]);
	privatefprintf("%dx%d.\n", event->size.width, event->size.height);


	if(ftexture){
		if(ExIsTexture(&fbackbuffertex)){
			ExDeleteTexture(&fbackbuffertex);
			ExCreateTexture(&fbackbuffertex, GL_TEXTURE_2D,  0, ftexinternalformat, event->size.width, event->size.height, 0, ftexformat, ftextype, NULL);
		}
	}
}


void glslview_update_shader_uniform_gl(struct uniform_location_t* uniform, ExShader* shader, int width, int height){
	float res[2];

	privatefprintf("----------- fetching uniforms index location ----------\n");
	uniform->time = glGetUniformLocation(shader->program, "time");
	uniform->deltatime = glGetUniformLocation(shader->program, "deltatime");
	uniform->resolution = glGetUniformLocation(shader->program, "resolution");
	uniform->mouse = glGetUniformLocation(shader->program, "mouse");
	uniform->offset = glGetUniformLocation(shader->program, "offset");
	uniform->stdin = glGetUniformLocation(shader->program, "stdin");
	uniform->tex0 = glGetUniformLocation(shader->program, "tex0");
	uniform->tex1 = glGetUniformLocation(shader->program, "tex1");
	uniform->tex2 = glGetUniformLocation(shader->program, "tex2");
	uniform->tex3 = glGetUniformLocation(shader->program, "tex3");
	uniform->tex4 = glGetUniformLocation(shader->program, "tex4");
	uniform->tex5 = glGetUniformLocation(shader->program, "tex5");
	uniform->tex6 = glGetUniformLocation(shader->program, "tex6");
	uniform->tex7 = glGetUniformLocation(shader->program, "tex7");
	uniform->tex8 = glGetUniformLocation(shader->program, "tex8");
	uniform->tex9 = glGetUniformLocation(shader->program, "tex9");
	uniform->tex10 = glGetUniformLocation(shader->program, "tex10");
	uniform->tex11 = glGetUniformLocation(shader->program, "tex11");
	uniform->tex12 = glGetUniformLocation(shader->program, "tex12");
	uniform->tex13 = glGetUniformLocation(shader->program, "tex13");
	uniform->tex14 = glGetUniformLocation(shader->program, "tex14");
	uniform->tex15 = glGetUniformLocation(shader->program, "tex15");
	uniform->backbuffer = glGetUniformLocation(shader->program, "backbuffer");

	debugprintf("time %d\n", uniform->time);
	debugprintf("deltatime %d\n", uniform->deltatime);
	debugprintf("resolution %d\n", uniform->resolution);
	debugprintf("mouse %d\n", uniform->mouse);
	debugprintf("offset %d\n", uniform->offset);
	debugprintf("stdin %d\n", uniform->stdin);
	debugprintf("tex0 %d\n", uniform->tex0);
	debugprintf("tex1 %d\n", uniform->tex1);
	debugprintf("tex2 %d\n", uniform->tex2);
	debugprintf("tex3 %d\n", uniform->tex3);
	debugprintf("tex4 %d\n", uniform->tex4);
	debugprintf("tex5 %d\n", uniform->tex5);
	debugprintf("tex6 %d\n", uniform->tex6);
	debugprintf("tex7 %d\n", uniform->tex7);
	debugprintf("tex8 %d\n", uniform->tex8);
	debugprintf("tex9 %d\n", uniform->tex9);
	debugprintf("tex10 %d\n", uniform->tex10);
	debugprintf("tex11 %d\n", uniform->tex11);
	debugprintf("tex12 %d\n", uniform->tex12);
	debugprintf("tex13 %d\n", uniform->tex13);
	debugprintf("tex14 %d\n", uniform->tex14);
	debugprintf("tex15 %d\n", uniform->tex15);
	debugprintf("backbuffer %d\n", uniform->backbuffer);

	glUseProgram(shader->program);

	res[0] = width;
	res[1] = height;
	glUniform2fv(uniform->resolution, 1, &res[0]);

	/**/
	privatefprintf("----------- Assigning texture sampler index ----------\n");
	glUniform1i(uniform->tex0, 0);
	glUniform1i(uniform->tex1, 1);
	glUniform1i(uniform->tex2, 2);
	glUniform1i(uniform->tex3, 3);
	glUniform1i(uniform->tex4, 4);
	glUniform1i(uniform->tex5, 5);
	glUniform1i(uniform->tex6, 6);
	glUniform1i(uniform->tex7, 7);
	glUniform1i(uniform->tex8, 8);
	glUniform1i(uniform->tex9, 9);
	glUniform1i(uniform->tex10, 10);
	glUniform1i(uniform->tex11, 11);
	glUniform1i(uniform->tex12, 12);
	glUniform1i(uniform->tex13, 13);
	glUniform1i(uniform->tex14, 14);
	glUniform1i(uniform->tex15, 15);
	glUniform1i(uniform->backbuffer, numTextures);


	/*	Create backbuffer.	*/
	if(uniform->backbuffer != -1){
		if(ExIsTexture(&fbackbuffertex)){
			ExDeleteTexture(&fbackbuffertex);
		}
		ExCreateTexture(&fbackbuffertex, GL_TEXTURE_2D,  0, ftexinternalformat, width, height, 0, ftexformat, ftextype, NULL);
		privatefprintf("Created backbuffer.	\n");
	}else{
		if(ExIsTexture(&fbackbuffertex)){
			ExDeleteTexture(&fbackbuffertex);
		}
	}
}

void glslview_displaygraphic_gl(ExWin drawable){

	if(usepolygone){
		glBindVertexArray(mesh.vao);
		glDrawElements(GL_TRIANGLES, mesh.indicescount, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);
	}
	else{
		/*	draw quad.	*/
		glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(quad) / sizeof(quad[0]));
	}
	/**/
	glslview_swapbuffer(drawable);
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




void glslview_rendergraphic(ExWin drawable, ExShader* shader, UniformLocation* uniform, float ttime, float deltatime){
	int x;

	for(x = 0; x < numShaderPass; x++){

		glUseProgram(shader[x].program);

		glslview_update_uniforms(&uniform[x], &shader[x], ttime, deltatime);
		glslview_displaygraphic(drawable);

		if(uniform[x].backbuffer != -1){
			glActiveTexture(GL_TEXTURE0 + numTextures);
			glBindTexture(fbackbuffertex.target, fbackbuffertex.texture);
			glCopyTexImage2D(fbackbuffertex.target, 0, GL_RGBA, 0, 0, fbackbuffertex.width, fbackbuffertex.height, 0);
		}
	}
	glClear(GL_COLOR_BUFFER_BIT);

}

