#include"internal.h"
#include<GL/gl.h>
#include<GL/glext.h>



void resize_screen_gl(ExEvent* event, struct uniform_location_t* uniform, ExShader* shader, ExTexture* ftexture){


}

void displaygraphic_gl(ExWin drawable){
	/*	draw quad.	*/
	glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(quad) / sizeof(quad[0]));

	pswapbuffer(drawable);
}
