#include "raylib.h"
#include <math.h>
#include <string.h>
#include "raymath.h"
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"
#include "stl_loader.h"
#define DEBUG_MODE
#include "settings.h"
#include "util.h"	// this should be the last include

//structures
typedef struct Segment{
	Vector3 point;	//end point for line or center for circle
	Vector3 center;	//end point for line or center for circle
	Color color;
	float radius;	//arc radius
	float angle;	//arc angle
	float offset;	//arc quadrant offset
	float k;	//z step
	bool arc;
}Segment;

//globals 
Color main_color = {187, 35, 255, 255};
float scale = 0.10f;

//prototypes
int parse_gcode(char *gcode_file, Segment **output);
void DrawGcodePath(Segment * seg, int len);

int main(int argc, char *argv[]) {

	if(argc<2){
		printf("No file provided %d\n", argc);
		exit(-1);
	}

	char * gcode_file = NULL;
	char * model_file = NULL;

	for(int i=1; i<argc; i++){
		if(strstr(argv[i], ".stl")) model_file = argv[i];
		if(strstr(argv[i], ".nc") || strstr(argv[i], ".gc") || strstr(argv[i], ".ngc") || strstr(argv[i], ".gcode")) gcode_file = argv[i];
	}

	const int screenWidth = 800;
	const int screenHeight = 450;

	SetConfigFlags(
			FLAG_WINDOW_RESIZABLE
			//| FLAG_MSAA_4X_HINT  // Enable Multi Sampling Anti Aliasing 4x (if available)
			);

	InitWindow(screenWidth, screenHeight, "CGinC");

	// Define the camera to look into our 3d world
	Camera3D camera = { 0 };
	camera.position = (Vector3){ 0.0f, -10.0f, 10.0f };  // Camera position
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
	camera.fovy = 45.0f;                                // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

	SetCameraMode(camera, CAMERA_CUSTOM); // Set a first person camera mode
	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	//Texture texture = LoadTexture("resources/texel_checker.png");
	Image texture_image = GenImageColor(1.0f, 1.0f, main_color);
	Texture texture = LoadTextureFromImage(texture_image);

	Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/base_lighting.vs", GLSL_VERSION), 
			TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));

	// Get some shader loactions
	shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	// ambient light level
	int ambientLoc = GetShaderLocation(shader, "ambient");
	SetShaderValue(shader, ambientLoc, (float[4]){ 0.2f, 0.2f, 0.2f, 1.0f }, SHADER_UNIFORM_VEC4);

	Light light = { 0 };
	light = CreateLight(LIGHT_POINT, (Vector3){ 4, 2, 4 }, Vector3Zero(), WHITE, shader);

	//Model model = LoadModel("bunny.obj");
	//Model model = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
	Model model;
	if(model_file){
		Mesh model_mesh = load_stl(model_file);
		model = LoadModelFromMesh(model_mesh);
		//model.transform = MatrixMultiply(model.transform, MatrixRotateX(DEG2RAD*90));
		model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
		model.materials[0].shader = shader;
	}


	Segment *path;
	int path_len = parse_gcode(gcode_file, &path);


	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		CustomUpdateCamera(&camera); 

		light.position.x = camera.position.x;
		light.position.y = camera.position.y;
		light.position.z = camera.position.z;

		UpdateLightValues(shader, light);

		float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
		SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		DrawGcodePath(path, path_len);
		//free(path);
		//parse_gcode(gcode_file, &path);

		if(model_file)DrawModel(model, (Vector3){ 0.0f, 0.0f, 0.0f }, scale, GRAY);   // Draw 3d model with texture
		//DrawModelWires(model, (Vector3){ 0.0f, 0.0f, 0.0f }, scale, BLACK);   // Draw 3d model with texture

		DrawXYGrid();

		EndMode3D();

		DEBUG_SHOW(DrawFPS(10, 10);)

			EndDrawing();
	}

	free(path);
	if(model_file){
		UnloadModel(model);
		UnloadTexture(texture);
	}

	CloseWindow();        // Close window and OpenGL context

	return 0;
}

int parse_gcode(char *gcode_file, Segment **output){

	printf("Parsing Gcode\n");

	FILE *g = fopen(gcode_file, "r");
	if(g == NULL){
		printf("Gcode file: \"%s\" does not exist!", gcode_file);
		exit(-1);
	}

	int seg_block = 1024;
	int seg_index = 0;

	Segment * segments = (Segment *)malloc(sizeof(Segment)*seg_block);

	segments[seg_index].point.x = 0;
	segments[seg_index].point.y = 0;
	segments[seg_index].point.z = 0;
	segments[seg_index].color = BLACK;

	char *line, *line_alloc = (char *)malloc(1024);
	line = line_alloc;

	Vector3 last_position = {0,0,0};
	Vector3 l_end = {0,0,0};
	Color l_color;

	bool absolute = true;
	
	while(1){	//go through all the lines
		

		if(fgets(line, 1023, g) == NULL){
			printf("No string read!\n");
			break;	//read line
		}
		
		if(feof(g)){
			printf("EOF Reached\n");
			break;
		}

		char *end = NULL;
		end = strchr(line, ';');	//find end of gcode command
		if(end != NULL) *end = '\0';	//terminate string there

		char* c = strchr(line, 'G');	//look for g command in line
		if(c != NULL) line = c;	//start intepreting from there
		else continue;	//or abort/ nothing for us to draw


		while(line != NULL){		//interpret all gcode command in line
			int cmd = strtol(line+1, &line, 10);
		
			switch(cmd){
			case 90:	//absolute mode
				absolute = true;
				break;
			case 91:	//incremental mode
				absolute = false;
				break;
			case 0:		//rapid
				l_color = RED;	
				break;
			case 1:		//feed
				l_color = GREEN;	
				break;
			case 2:	//clockwise arc
			case 3:	//counterclockwise arc
				l_color = BLUE;
				break;
			default:
				break;
			}

			if(cmd == 91 || cmd == 90) {	//these just change the coordinate system
				char *tmp = strchr(line, 'G');
				if(tmp != NULL) line = tmp;
				continue;	//go to top of loop
			}

			float x,y,z;	//get coordinates
			char *x_pos = strchr(line, 'X');
			char *y_pos = strchr(line, 'Y');
			char *z_pos = strchr(line, 'Z');
			char *i_pos = strchr(line, 'I');
			char *j_pos = strchr(line, 'J');
			char *k_pos = strchr(line, 'K');
			char *r_pos = strchr(line, 'R');

			//check if axis gets moved
			if(x_pos != NULL) x = strtof(x_pos+1, NULL)*scale;	//check if value present and get it
			else if(cmd != 2 && cmd != 3) x = last_position.x;	//if its not and we are not in arc
			else x = 0;	//if its not and we are in arc

			if(y_pos != NULL) y = strtof(y_pos+1, NULL)*scale;
			else if(cmd != 2 && cmd != 3) y = last_position.y;
			else y = 0;

			if(z_pos != NULL) z = strtof(z_pos+1, NULL)*scale;
			else if(cmd != 2 && cmd != 3) z = last_position.z;
			else z = 0;

			if(absolute){	//this means we go to x
				l_end.x = x;
				l_end.y = y;
				l_end.z = z;
			}
			else{		//this means we go x amount from where we are
				l_end.x += x;
				l_end.y += y;
				l_end.z += z;
			}

			//allocate more segments if needed
			if(seg_index == seg_block-1){
				seg_block += 1024;
				segments = (Segment *)realloc(segments, sizeof(Segment)*seg_block);
				if(segments == NULL){
					perror("Could not allocate more space for segments!");
					exit(-1);
				}
			}

			if(cmd == 2 || cmd == 3){	//arc

				Vector3 center;

				float i,j,k;

				if(i_pos != NULL) i = strtof(i_pos+1, NULL)*scale;
				else i = 0;
				if(j_pos != NULL) j = strtof(j_pos+1, NULL)*scale;
				else j = 0;
				if(k_pos != NULL) k = strtof(k_pos+1, NULL)*scale;
				else k = 0;


				//if(l_end.z == last_position.z) u.z = 0;
				float radius;
				if(r_pos != NULL){
					radius = strtof(r_pos+1, NULL)*scale;
					//trying to implement radius mode
					//for any angle between the start of the arc and the end of it
					//the center will lie on the tangent
					float q = sqrt((l_end.x-last_position.x)*(l_end.x-last_position.x) + (l_end.y-last_position.y)*(l_end.y-last_position.y));

					float y3 = (last_position.y+l_end.y)/2;
					float x3 = (last_position.x+l_end.x)/2;

					float basex = sqrt( radius*radius - q*q/4.0 ) * (last_position.y-l_end.y)/q; //calculate once
					float basey = sqrt( radius*radius - q*q/4.0 ) * (l_end.x-last_position.x)/q; //calculate once

					if(cmd == 3){
						center.x = x3 + basex; //center x of circle 1
						center.y = y3 + basey; //center y of circle 1
					}
					else {
						center.x = x3 - basex; //center x of circle 2
						center.y = y3 - basey; //center y of circle 2
					}

					center.z = last_position.z;
				}
				else{
					if(absolute){
						center.x = i;
						center.y = j;
						center.z = k;
					}
					else{
						center.x = last_position.x + i;
						center.y = last_position.y + j;
						center.z = last_position.z + k;
					}
				}

				if(z_pos == NULL) l_end.z = last_position.z;

				//calculate the vectors from the center
				Vector3 v, u;
				v.x = last_position.x - center.x;
				v.y = last_position.y - center.y;
				v.z = last_position.z - center.z;
				//printVector3("v", v);

				u.x = l_end.x - center.x;
				u.y = l_end.y - center.y;
				u.z = l_end.z - center.z;
				//printVector3("u", u);

				if(r_pos == NULL){
					//calculate the radius
					//radius = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
					radius = 1.0f / Q_rsqrt(v.x*v.x + v.y*v.y + v.z*v.z);
				}

				float rotationAngle;	//this angle can be calculated from the definition of the vector dot product

				if(l_end.x == last_position.x && l_end.y == last_position.y){
					//if the end point is the same as the starting point assume full circle
					rotationAngle = 360;
				}
				else {
					float v_dot_u = v.x*u.x + v.y*u.y + v.z*u.z;
					//For the gcode to be valid, the magnitudes of both vectors should be equal, or close enough
					//I will check it for the user
					//float mag_v = radius;	 //the radius is the first magnitude
					float mag_v = 1.0f / Q_rsqrt(v.x*v.x + v.y*v.y + v.z*v.z);

					//float mag_u  = sqrt(u.x*u.x + u.y*u.y + u.z*u.z);
					float mag_u  = 1.0f / Q_rsqrt(u.x*u.x + u.y*u.y + u.z*u.z);

					if(fast_abs(mag_v - mag_u) > 0.01) printf("Something's fishy about that arc, check it again\n");

					//finally we can calculate the angle
					rotationAngle = RAD2DEG*acos(v_dot_u / (mag_v * mag_u));
				}

				//use the angle of the vector to x axis to offset the start of the section
				float offsetAngle = atan2(v.x, v.y)*RAD2DEG -180;

				if(cmd == 2){
					rotationAngle = -rotationAngle;	//use negative angle to rotate backwards
					offsetAngle = offsetAngle+180;	//this now needs to be also offset
				}

				//printf("X%f Y%f Z%f I%f J%f K%f R%f A%f O%f\n", l_end.x, l_end.y, l_end.z, center.x, center.y, center.z, radius, rotationAngle, offsetAngle);

				//printVector3("last_position", last_position);
				//printVector3("l_end", l_end);
				//printVector3("center", center);
				//DrawCircleSector3D(center, radius, rotationAngle, offsetAngle, u.z, l_color);
				seg_index++;
				segments[seg_index].point.x = l_end.x;
				segments[seg_index].point.y = l_end.y;
				segments[seg_index].point.z = l_end.z;
				segments[seg_index].center.x = center.x;
				segments[seg_index].center.y = center.y;
				segments[seg_index].center.z = center.z;
				segments[seg_index].color = l_color;
				segments[seg_index].radius = radius;
				segments[seg_index].angle = rotationAngle;
				segments[seg_index].offset = offsetAngle;
				segments[seg_index].k = u.z;
				segments[seg_index].arc = true;
			}
			else {
				//DrawLine3D(last_position, l_end, l_color);
			}
			//this is added for the next line too use as start
			seg_index++;
			segments[seg_index].point.x = l_end.x;
			segments[seg_index].point.y = l_end.y;
			segments[seg_index].point.z = l_end.z;
			segments[seg_index].color = l_color;
			segments[seg_index].arc = false;


			last_position.x = l_end.x;
			last_position.y = l_end.y;
			last_position.z = l_end.z;

			break;	//out of loop
		}

	}
	fclose(g);
	free(line_alloc);

	*output = segments;

	printf("Parsing Complete, %d points found!\n", seg_index);
	return seg_index;
}

void DrawGcodePath(Segment * seg, int len){
	for(int i=1; i<len; i++){
		//printf("%d: %f, %f, %f\n", i, seg[i].point.x, seg[i].point.y, seg[i].point.z);
		if(seg[i].arc) DrawCircleSector3D(seg[i].center, seg[i].radius, seg[i].angle, seg[i].offset, seg[i].k, seg[i].color);
		else DrawLine3D(seg[i-1].point, seg[i].point, seg[i].color);
	}
}
