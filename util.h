//inlcude general utilities for code that is not important enough to be in main.c
#include <stdint.h>

//types
typedef struct {
	bool show_origin;
	bool show_grid;
	bool show_model;
	bool camera_ortho;
	bool dark_mode;
} Settings_t;

//quake inverse square root, credit goes to ID Software I guess
float Q_rsqrt( float number )
{	
	const float x2 = number * 0.5F;
	const float threehalfs = 1.5F;

	union {
		float f;
		uint32_t i;
	} conv  = { .f = number };
	conv.i  = 0x5f3759df - ( conv.i >> 1 );
	conv.f  *= threehalfs - ( x2 * conv.f * conv.f );
	conv.f  *= threehalfs - ( x2 * conv.f * conv.f );
	return conv.f;
}

float fast_abs(float f){
	int a=( (*(int *)&f ) & 0x7fffffff );
	return ( *(float *)&a );
}

//these are some modified raylib.h function
//this makes the camera work like in normal CAD programs
void CustomUpdateCamera(Camera *camera, Settings_t *s){

	if(IsKeyPressed(KEY_HOME)){
		camera->position = (Vector3){ 0.0f, -10.0f, 10.0f };  // Camera position
		camera->target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
		camera->up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
		return;
	}

	double scroll_sensitivity = CAMERA_MOUSE_SCROLL_SENSITIVITY;
	if(s->camera_ortho) scroll_sensitivity = 0.0f;

	static Vector2 previousMousePosition = { 0.0f, 0.0f };

	// Mouse movement detection
	Vector2 mousePositionDelta = { 0.0f, 0.0f };
	Vector2 mousePosition = GetMousePosition();
	float mouseWheelMove = GetMouseWheelMove();

	mousePositionDelta.x = mousePosition.x - previousMousePosition.x;
	mousePositionDelta.y = mousePosition.y - previousMousePosition.y;

	previousMousePosition = mousePosition;

	if(s->camera_ortho ){
		static float zoom = 0;
		zoom = mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY_ORTHO;
		if(camera->fovy+zoom > 0) camera->fovy += zoom;
	}

	float dx = camera->target.x - camera->position.x;
	float dy = camera->target.y - camera->position.y;
	float dz = camera->target.z - camera->position.z;

	float targetDistance = sqrtf(dx*dx + dy*dy + dz*dz);   // Distance to target

	// Camera angle calculation
	Vector2 angle;
	angle.x = atan2f(dx, dz);                        // Camera angle in plane XZ (0 aligned with Z, move positive CCW)
	angle.y = atan2f(dy, sqrtf(dx*dx + dz*dz));      // Camera angle in plane XY (0 aligned with X, move positive CW)

	float playerEyesPosition = camera->position.y;          // Init player eyes position to camera Y position

	// Camera zoom
	if ((targetDistance < CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0))
	{
		targetDistance -= (mouseWheelMove*scroll_sensitivity);
		if (targetDistance > CAMERA_FREE_DISTANCE_MAX_CLAMP) targetDistance = CAMERA_FREE_DISTANCE_MAX_CLAMP;
	}

	// Camera looking down
	else if ((camera->position.y > camera->target.y) && (targetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0)){
		camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*scroll_sensitivity/targetDistance;
		camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*scroll_sensitivity/targetDistance;
		camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*scroll_sensitivity/targetDistance;
	}
	else if ((camera->position.y > camera->target.y) && (camera->target.y >= 0)){
		camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*scroll_sensitivity/targetDistance;
		camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*scroll_sensitivity/targetDistance;
		camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*scroll_sensitivity/targetDistance;
	}
	else if ((camera->position.y > camera->target.y) && (camera->target.y < 0) && (mouseWheelMove > 0)){
		targetDistance -= (mouseWheelMove*scroll_sensitivity);
		if (targetDistance < CAMERA_FREE_DISTANCE_MIN_CLAMP) targetDistance = CAMERA_FREE_DISTANCE_MIN_CLAMP;
	}


	// Camera looking up
	else if ((camera->position.y < camera->target.y) && (targetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0)){
		camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*scroll_sensitivity/targetDistance;
		camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*scroll_sensitivity/targetDistance;
		camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*scroll_sensitivity/targetDistance;
	}
	else if ((camera->position.y < camera->target.y) && (camera->target.y <= 0)){
		camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*scroll_sensitivity/targetDistance;
		camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*scroll_sensitivity/targetDistance;
		camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*scroll_sensitivity/targetDistance;
	}
	else if ((camera->position.y < camera->target.y) && (camera->target.y > 0) && (mouseWheelMove > 0)){
		targetDistance -= (mouseWheelMove*scroll_sensitivity);
		if (targetDistance < CAMERA_FREE_DISTANCE_MIN_CLAMP) targetDistance = CAMERA_FREE_DISTANCE_MIN_CLAMP;
	}


	// Input keys checks
	if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
	{
			// Camera panning
			camera->target.x += ((mousePositionDelta.x*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(angle.x) + (mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*sinf(angle.x)*sinf(angle.y))*(targetDistance/CAMERA_FREE_PANNING_DIVIDER);
			camera->target.y += ((mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(angle.y))*(targetDistance/CAMERA_FREE_PANNING_DIVIDER);
			camera->target.z += ((mousePositionDelta.x*-CAMERA_FREE_MOUSE_SENSITIVITY)*sinf(angle.x) + (mousePositionDelta.y*CAMERA_FREE_MOUSE_SENSITIVITY)*cosf(angle.x)*sinf(angle.y))*(targetDistance/CAMERA_FREE_PANNING_DIVIDER);
	}
	else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))     // Alternative key behaviour
	{
			// Camera rotation
			angle.x += mousePositionDelta.x*-CAMERA_FREE_MOUSE_SENSITIVITY;
			angle.y += mousePositionDelta.y*-CAMERA_FREE_MOUSE_SENSITIVITY;

			// Angle clamp
			if (angle.y > CAMERA_FREE_MIN_CLAMP*DEG2RAD) angle.y = CAMERA_FREE_MIN_CLAMP*DEG2RAD;
			else if (angle.y < CAMERA_FREE_MAX_CLAMP*DEG2RAD) angle.y = CAMERA_FREE_MAX_CLAMP*DEG2RAD;

	}

	// Update camera position with changes
	camera->position.x = -sinf(angle.x)*targetDistance*cosf(angle.y) + camera->target.x;
	camera->position.y = -sinf(angle.y)*targetDistance + camera->target.y;
	camera->position.z = -cosf(angle.x)*targetDistance*cosf(angle.y) + camera->target.z;

}

void CheckInputs(Settings_t *s)
{
	if(IsKeyPressed(KEY_O)) s->show_origin = !s->show_origin;
	if(IsKeyPressed(KEY_G)) s->show_grid = !s->show_grid;
	if(IsKeyPressed(KEY_M)) s->show_model = !s->show_model;
	if(IsKeyPressed(KEY_C)) s->camera_ortho = !s->camera_ortho;
	if(IsKeyPressed(KEY_D)) s->dark_mode = !s->dark_mode;
}

//this draws the grid in the xy plane
void DrawXYGrid(Settings_t *s){
    int half = 100;
		float minor_shade = 0.5f;
		float major_shade = 0.9f;

		if(s->dark_mode){
			minor_shade = 0.5;
			major_shade = 0.2;
		}

    rlCheckRenderBatchLimit((half + 2)*4);

    rlBegin(RL_LINES);
        for (int i = -half; i <= half; i++)
        {
            if (i % 10 == 0)
            {
                rlColor3f(minor_shade, minor_shade, minor_shade);
                rlColor3f(minor_shade, minor_shade, minor_shade);
                rlColor3f(minor_shade, minor_shade, minor_shade);
                rlColor3f(minor_shade, minor_shade, minor_shade);
            }
            else
            {
                rlColor3f(major_shade, major_shade, major_shade);
                rlColor3f(major_shade, major_shade, major_shade);
                rlColor3f(major_shade, major_shade, major_shade);
                rlColor3f(major_shade, major_shade, major_shade);
            }

						//vertical
            rlVertex3f((float)i*0.1f, (float)-half*0.1f, 0.0f);
            rlVertex3f((float)i*0.1f, (float)half*0.1f, 0.0f);
						//horizontal
            rlVertex3f((float)-half*0.1f, (float)i*0.1f, 0.0f);
            rlVertex3f((float)half*0.1f, (float)i*0.1f, 0.0f);
        }
    rlEnd();
}
// Draw a circle in 3D world space
void DrawCircleSector3D(Vector3 center, float radius, float rotationAngle, float offsetAngle, float k, Color color)
{
    rlCheckRenderBatchLimit(2*36);

    rlPushMatrix();
        rlTranslatef(center.x, center.y, center.z);
        //rlRotatef(DEG2RAD*45.0f, rotationAxis.x, rotationAxis.y, rotationAxis.z);

        rlBegin(RL_LINES);
					if(rotationAngle >= 0){
            for (int i = -offsetAngle; i < rotationAngle-offsetAngle; i += 5)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);

                rlVertex3f(sinf(DEG2RAD*i)*radius, -cosf(DEG2RAD*i)*radius, (i + offsetAngle)*k/360.0);
                rlVertex3f(sinf(DEG2RAD*(i+5))*radius, -cosf(DEG2RAD*(i+5))*radius, (i+5 + offsetAngle)*k/360.0);
            }
					}
					else{
            for (int i = offsetAngle; i > rotationAngle+offsetAngle; i -= 5)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);

                rlVertex3f(sinf(DEG2RAD*(i-5))*radius, -cosf(DEG2RAD*(i-5))*radius, (i-5 + offsetAngle)*k/360.0);
                rlVertex3f(sinf(DEG2RAD*i)*radius, -cosf(DEG2RAD*i)*radius, (i + offsetAngle)*k/360.0);
            }
					}
        rlEnd();
    rlPopMatrix();
}


void printVector3(char* name, Vector3 v){
	printf("Vecotor3 %s = {\n  .x =%f;\n  .y=%f;\n .z=%f;\n}\n", name, v.x, v.y, v.z);

}

void DrawOrigin(){
		const Vector3 origin = {0.0f, 0.0f, 0.0f};
		const Vector3 x = {1.0f, 0.0f, 0.0f};
		const Vector3 y = {0.0f, 1.0f, 0.0f};
		const Vector3 z = {0.0f, 0.0f, 1.0f};
		DrawLine3D(origin, x, RED);
		DrawLine3D(origin, y, GREEN);
		DrawLine3D(origin, z, BLUE);
}

