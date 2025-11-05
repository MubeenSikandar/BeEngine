#ifdef _WIN32
#ifdef BE_BUILD_DLL
#define BE_API __declspec(dllexport)
#else
#define BE_API __declspec(dllimport)
#endif
#else
#define BE_API __attribute__((visibility("default")))
#endif
