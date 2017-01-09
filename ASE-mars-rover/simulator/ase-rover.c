#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "server.h"
#include <err.h>

#include "tga.h"
#include "terrain.h"
#include "shaders.h"

#include "constants.h"

char * handle_request(const char * url);

/* This buffer is used to dump camera view */
unsigned char pixels[3*WIDTH*HEIGHT];

/* This buffer is used to answer http requests
   The largest request is a camera dump, which requires
   less than 256 bytes for the header and WIDTH*HEIGHT
   space-separated 3 digit values */
   char response[256+4*WIDTH*HEIGHT];

/* coordinates of the rover in the world*/
   float angle=RIANGLE;
   float x=RIX,z=RIZ;
   const float y=RHEIGHT;

/* terrain model */
   int terrainDL;
   int energy;
   int time;

/* We forbid window changes of size in this simulator */
   void changeSize(int w1, int h1)
   {
   	if (w1 != WIDTH || h1 != HEIGHT)
   		glutReshapeWindow( WIDTH, HEIGHT);
   }

/* renderScene: plots the terrain */
   void renderScene(void) {
   	server_run();
   	glLoadIdentity();
   	gluLookAt(x, y, z,
   		x + 10*sin(angle), y,z + 10*cos(angle),
   		0.0f,1.0f,0.0f);

   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   	glCallList(terrainDL);

   	glutSwapBuffers();
   }

/* processKeys: GLUT key callback, exits if ESC pressed */
   void processKeys(unsigned char key, int x, int y) {
   	if (key == 27) {
   		terrainDestroy();
   		exit(0);
   	}
   }

   void timer(int notused)
   {
   	time += 1;
   	energy += 50;
   	glutTimerFunc(1000,timer,0);
   	fprintf(stderr, "Time = %d s x = %f m z = %f \n m angle = %f rad Energy = %d J\n", time, x, z, angle, energy);
   }

/* prefix: returns true if pre is a prefix of str */
   bool prefix(const char *pre, const char *str)
   {
   	return strncmp(pre, str, strlen(pre)) == 0;
   }

/* forward: jumps forward of i meters and checks that no Crash occurs at the
new position */
   int forward(float i) {
   	x = x + i*sin(angle);
   	z = z + i*cos(angle);
   	float h = terrainGetHeight(x,z);

   	if (fabs(x) > XDIM/2 || fabs(z) > XDIM/2) {
        /* rover went out of the map */
   		return 1;
   	}
   	if (h > RHEIGHT) {
		/* A crash was detected */
   		return 1;
   	}
   	if (x*x + z*z < 5.0*5.0) {
		/* Target reached */
   		return 2;
   	}
   	return 0;
   }

/* handle_turn: handle /CAMERA http requests */
   void handle_camera(const char * url) {
	/* Capture GL view */
   	glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	/* Encode the camera image as a P2 PGM image */
   	char * cursor = response;
   	cursor += sprintf(cursor, "P2 %d %d 255 ", WIDTH, HEIGHT);
   	for (int j=HEIGHT-1; j>= 0; j--) {
   		for (int i=0; i< WIDTH; i++) {
   			cursor += sprintf(cursor, "%d ", pixels[3*(j*WIDTH+i)]);
   		}
   	}
   }

/* handle_turn: handle /TURN/<radians> http requests */
   void handle_turn(const char * url) {
   	float delta = 0.0f;
   	sscanf(url, "/TURN/%f", &delta);
   	energy += 50*delta;
   	angle+=delta;
   	sprintf(response, "OK");
   }

/* handle_forward: handle /FORWARD/<meters> http requests */
   void handle_forward(const char * url) {
   	const float eps = .300f;
   	float delta;
   	sscanf(url, "/FORWARD/%f", &delta);

    /* delta should always be positive */
   	delta = fabs(delta);
   	energy += 100*delta;

   	while(1) {
   		float step = eps;
   		if (delta < eps) {
   			step = delta;
   		}

   		int res = forward(step);
   		if ( res == 1 ) {
   			fprintf(stderr, "ROVER CRASHED");
   			sprintf(response, "CRASH");
   			return;
   		} else if ( res == 2 ) {
   			fprintf(stderr, "TARGET REACHED");
   			sprintf(response, "WIN");
   			return;
   		}
   		delta -= step;
   		if (delta < eps) {
   			res = forward(delta);

   			if ( res == 1 ) {
   				fprintf(stderr, "ROVER CRASHED");
   				sprintf(response, "CRASH");
   				return;
   			} else if ( res == 2 ) {
   				fprintf(stderr, "TARGET REACHED");
   				sprintf(response, "WIN");
   				return;
   			}


   			break;
   		}
   	}
   	sprintf(response, "OK");
   }

/* handle_request: handler function for incoming HTTP requests */
   char * handle_request(const char * url) {
	/* Response is set initially to ERROR. If the request is
	correctly handled, this initial value will be updated*/
   	sprintf(response, "ERROR");

   	printf("Got URL %s\n", url);

   	if (prefix("/CAMERA", url)) {
   		handle_camera(url);
   	}
   	else if (prefix("/TURN", url)) {
   		handle_turn(url);
   	}
   	else if (prefix("/FORWARD", url)) {
   		handle_forward(url);
   	}
   	else {
   		fprintf(stderr, "Received unknown command");
   	}

   	return response;
   }

/* init_scene: Loads terrain file and initializes GL perspective
projection */
   void init_scene(char * map) {
   	glEnable(GL_DEPTH_TEST);
   	glEnable(GL_CULL_FACE);
   	glCullFace(GL_BACK);
   	init_shaders();

	/* Load terrain map */
   	if (terrainLoadFromImage(map,1) != TERRAIN_OK) {
   		errx(1, "Could not load map image file");
   	}
   	terrainScale(0,10);
   	terrainDL = terrainCreateDL(0,0,0);

	/* Reset the coordinate system before modifying */
   	glMatrixMode(GL_PROJECTION);
   	glLoadIdentity();
    /* Set the viewport to be the entire window */
   	glViewport(0, 0, WIDTH, HEIGHT);
   	float ratio = 1.0f * WIDTH / HEIGHT;
    /* Set the clipping volume */
   	gluPerspective(45,ratio,0.1,100);
   	glMatrixMode(GL_MODELVIEW);
   	glLoadIdentity();
   }

/* main: the main function of the Mars Rover Simulator */
   int main(int argc, char **argv)
   {
	/* Check arguments */
   	if (argc != 2) {
   		errx(1, "usage: %s map.tga", argv[0]);
   	}

	/* Init HTTP server */
   	server_init(&handle_request);

	/* Init Glut */
   	glutInit(&argc, argv);
   	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
   	glutInitWindowPosition(100,100);
   	glutInitWindowSize(WIDTH, HEIGHT);
   	glutCreateWindow("Mars Rower Simulator");

	/* Init Glew */
   	GLenum err = glewInit();
   	if (GLEW_OK != err) {
   		errx(1, "Could not initialize Glew. Please "
   			"ensure your graphic card support GL extensions.");
   	}

	/* Set Glut Callback */
   	glutKeyboardFunc(processKeys);
   	glutDisplayFunc(renderScene);
   	glutIdleFunc(renderScene);
   	glutReshapeFunc(changeSize);
   	glutTimerFunc(1000,timer,0);

	/* Init Scene */
   	init_scene(argv[1]);

	/* Launch main loop */
   	glutMainLoop();

   	return 0;
   }
