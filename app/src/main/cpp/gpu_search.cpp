#include <jni.h>
#include <android/log.h>
#include <CL/cl.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GPU", __VA_ARGS__)

static cl_device_id device = nullptr;
static cl_context context = nullptr;
static cl_command_queue queue = nullptr;

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_puzzle20_gpu_GPUEngine_initGPU(JNIEnv* env, jobject thiz) {
    cl_int err;
    cl_platform_id platform;
    
    err = clGetPlatformIDs(1, &platform, NULL);
    if (err != CL_SUCCESS) {
        LOGI("OpenCL not available");
        return JNI_FALSE;
    }
    
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS) {
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
        if (err != CL_SUCCESS) return JNI_FALSE;
    }
    
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    queue = clCreateCommandQueue(context, device, 0, &err);
    
    char name[256];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name), name, NULL);
    LOGI("GPU: %s", name);
    
    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_puzzle20_gpu_GPUEngine_releaseGPU(JNIEnv* env, jobject thiz) {
    if (queue) clReleaseCommandQueue(queue);
    if (context) clReleaseContext(context);
    if (device) clReleaseDevice(device);
}

}
