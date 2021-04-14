
#ifdef DEBUG_MODE
#define DEBUG_SHOW(X) X
#else
#define DEBUG_SHOW(X)
#endif

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define CAMERA_MOUSE_SCROLL_SENSITIVITY                 1.5f
#define CAMERA_FREE_MOUSE_SENSITIVITY                   0.01f
#define CAMERA_FREE_DISTANCE_MIN_CLAMP                  0.3f
#define CAMERA_FREE_DISTANCE_MAX_CLAMP                  120.0f
#define CAMERA_FREE_MIN_CLAMP                           85.0f
#define CAMERA_FREE_MAX_CLAMP                          -85.0f
#define CAMERA_FREE_PANNING_DIVIDER                     5.1f
#define PLAYER_MOVEMENT_SENSITIVITY                     20.0f
