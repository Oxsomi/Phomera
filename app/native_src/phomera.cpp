#include "phomera.hpp"
#include <string.h>
#include <jni.h>

extern "C" JNIEXPORT jstring JNICALL Java_net_osomi_phomera_FullscreenActivity_getGStreamerVersion(JNIEnv* env, jobject thiz) {
    return env->NewStringUTF("Yeeeeehaww");
}