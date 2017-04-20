# glslview #
----
*glslview* is an OpenGL shading language viewer program. Allowing fragment shader to be rendered to display with help of predefined uniform variables.

## Installation ##
----
The program can be easily installed with invoking the following command.
```bash
make
make install
```


## Usage ##
--------------
The *glslview* requires at least one argument in order to run. The following command is the basic way of running the program.
```bash
glslview -f fragment.glsl
```
In order to add texture to fragment shader, then the option argument *-t* is needed. See following.
```bash
glslview -f fragment.glsl -t img.png
```


## Dependencies ##
----------------
In order to compile the program, the following Debian packages has to be installed prior.
```bash
apt-get install libfreeimage-dev libsdl2-dev
```