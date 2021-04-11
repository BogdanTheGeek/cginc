#include "raylib.h"
#include <math.h>

#define DEBUG_MODE
#include "settings.h"

//prototypes
void CustomUpdateCamera(Camera *camera); 

int main(void){

	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "CGinC");

	// Define the camera to look into our 3d world
	Camera3D camera = { 0 };
	camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };  // Camera position
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
	camera.fovy = 45.0f;                                // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

	Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
	SetCameraMode(camera, CAMERA_CUSTOM); // Set a first person camera mode
	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		CustomUpdateCamera(&camera); 

		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
		DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);

		DrawGrid(10, 1.0f);

		EndMode3D();

		DEBUG_SHOW(DrawFPS(10, 10);)

			EndDrawing();
	}

	CloseWindow();        // Close window and OpenGL context

	return 0;
}


void CustomUpdateCamera(Camera *camera){

	if(IsKeyPressed(KEY_HOME)){
		camera->position = (Vector3){ 0.0f, 10.0f, 10.0f };  // Camera position
		camera->target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
		camera->up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
		return;
	}

	static Vector2 previousMousePosition = { 0.0f, 0.0f };

	// Mouse movement detection
	Vector2 mousePositionDelta = { 0.0f, 0.0f };
	Vector2 mousePosition = GetMousePosition();
	float mouseWheelMove = GetMouseWheelMove();

	mousePositionDelta.x = mousePosition.x - previousMousePosition.x;
	mousePositionDelta.y = mousePosition.y - previousMousePosition.y;

	previousMousePosition = mousePosition;

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
		targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
		if (targetDistance > CAMERA_FREE_DISTANCE_MAX_CLAMP) targetDistance = CAMERA_FREE_DISTANCE_MAX_CLAMP;
	}

	// Camera looking down
	else if ((camera->position.y > camera->target.y) && (targetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0)){
		camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
		camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
		camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
	}
	else if ((camera->position.y > camera->target.y) && (camera->target.y >= 0)){
		camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
		camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
		camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
	}
	else if ((camera->position.y > camera->target.y) && (camera->target.y < 0) && (mouseWheelMove > 0)){
		targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
		if (targetDistance < CAMERA_FREE_DISTANCE_MIN_CLAMP) targetDistance = CAMERA_FREE_DISTANCE_MIN_CLAMP;
	}


	// Camera looking up
	else if ((camera->position.y < camera->target.y) && (targetDistance == CAMERA_FREE_DISTANCE_MAX_CLAMP) && (mouseWheelMove < 0)){
		camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
		camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
		camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
	}
	else if ((camera->position.y < camera->target.y) && (camera->target.y <= 0)){
		camera->target.x += mouseWheelMove*(camera->target.x - camera->position.x)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
		camera->target.y += mouseWheelMove*(camera->target.y - camera->position.y)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
		camera->target.z += mouseWheelMove*(camera->target.z - camera->position.z)*CAMERA_MOUSE_SCROLL_SENSITIVITY/targetDistance;
	}
	else if ((camera->position.y < camera->target.y) && (camera->target.y > 0) && (mouseWheelMove > 0)){
		targetDistance -= (mouseWheelMove*CAMERA_MOUSE_SCROLL_SENSITIVITY);
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
