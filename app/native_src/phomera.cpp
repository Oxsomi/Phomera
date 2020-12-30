#include "phomera.hpp"
#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <unordered_map>

//Managing our gstreamer thread
//Adapted and restyled from https://gstreamer.freedesktop.org/documentation/tutorials/android/a-running-pipeline.html?gi-language=c

GST_DEBUG_CATEGORY_STATIC(debugCategory);

static jmethodID onGSReady, onGSUnready;
static JavaVM *javaVm;

static struct GSInstance *instance = nullptr;

struct GSInstance {

    JNIEnv *env;
    jobject app;
    ANativeWindow *surface;
    GMainLoop *mainLoop;
    GstElement *pipeline, *videoSink;
    pthread_t thread;
    jint version;

    static void busError(GstBus *bus, GstMessage *msg, GSInstance *inst) {

        GError *err;
        gchar *debugInfo;
        gst_message_parse_error(msg, &err, &debugInfo);

        GST_ERROR("Received error from %s: %s", GST_OBJECT_NAME(msg->src), err->message);

        g_clear_error(&err);
        g_free(debugInfo);

        gst_element_set_state(inst->pipeline, GST_STATE_NULL);
    }

    static void busStateChange(GstBus *bus, GstMessage *msg, GSInstance *inst) {

        GstState oldState, newState, pendingState;
        gst_message_parse_state_changed(msg, &oldState, &newState, &pendingState);

        if(GST_MESSAGE_SRC(msg) == GST_OBJECT(inst->pipeline))
            GST_DEBUG(
               "State changed from %s to %s with pending state %s",
               gst_element_state_get_name(oldState),
               gst_element_state_get_name(newState),
               gst_element_state_get_name(pendingState)
            );
    }

    static void *threadFunction(GSInstance *inst) {

        GST_DEBUG("Creating pipeline in instance %p", inst);

        auto context = g_main_context_new();
        g_main_context_push_thread_default(context);

        //We a udp server now, the camera needs to post to us at :5000

        GError *error = nullptr;

        auto pipeline = gst_parse_launch(
           //"audiotestsrc ! audioconvert ! audioresample ! autoaudiosink",
           //"videotestsrc ! warptv ! videoconvert ! autovideosink",
           "udpsrc port=5000 ! application/x-rtp, media=video, clock-rate=90000, encoding-name=H264 ! decodebin ! autovideosink sync=false",
           &error
        );

        inst->pipeline = pipeline;

        if(error) {

            g_main_context_pop_thread_default(context);
            g_main_context_unref(context);

            GST_DEBUG("Unable to build pipeline: %s\n", error->message);
            g_clear_error(&error);

            return nullptr;
        }

        //Setup our surface

        gst_element_set_state(inst->pipeline, GST_STATE_READY);

        if (!(inst->videoSink = gst_bin_get_by_interface(GST_BIN(inst->pipeline), GST_TYPE_VIDEO_OVERLAY))) {
            GST_ERROR("Could not get video sink");
            return nullptr;
        }

        //Attach callbacks to our bus

        GstBus *bus = gst_element_get_bus(pipeline);
        GSource *busSource = gst_bus_create_watch(bus);

        g_source_set_callback(busSource, (GSourceFunc) gst_bus_async_signal_func, nullptr, nullptr);
        g_source_attach(busSource, context);

        g_signal_connect(G_OBJECT(bus), "message::error", (GCallback) busError, inst);
        g_signal_connect(G_OBJECT(bus), "message::state-changed", (GCallback) busStateChange, inst);

        g_source_unref(busSource);
        gst_object_unref(bus);

        //Make main loop

        GST_DEBUG("Starting main loop");
        inst->mainLoop = g_main_loop_new(context, FALSE);

        //Notify our UI thread that we can start taking pictures, since our camera is now valid

        JNIEnv *env;
        JavaVMAttachArgs args{ inst->version };

        javaVm->AttachCurrentThread(&env, &args);

        GST_DEBUG("Notifying UI thread for onGSReady");
        env->CallVoidMethod(inst->app, onGSReady);

        if(env->ExceptionCheck()) {

            GST_ERROR("Failed exception check after calling onGSReady");
            env->ExceptionClear();

            javaVm->DetachCurrentThread();
            g_main_context_pop_thread_default(context);
            g_main_context_unref(context);
            gst_element_set_state(pipeline, GST_STATE_NULL);
            gst_object_unref(pipeline);
            return nullptr;
        }

        //Mark as playing

        GST_DEBUG("Starting stream");
        gst_element_set_state (inst->pipeline, GST_STATE_PLAYING);

        //Run our main loop

        g_main_loop_run(inst->mainLoop);
        GST_DEBUG("Exit main loop");

        g_main_loop_unref(inst->mainLoop);

        inst->mainLoop = nullptr;

        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(inst->videoSink), (guintptr) inst->surface);

        //Notify our UI thread that we can't take pictures anymore, our camera preview has stopped

        GST_DEBUG("Notifying UI thread for onGSUnready");
        env->CallVoidMethod(inst->app, onGSUnready);

        if(env->ExceptionCheck()) {

            GST_ERROR("Failed exception check after calling onGSUnready");
            env->ExceptionClear();

            javaVm->DetachCurrentThread();
            g_main_context_pop_thread_default(context);
            g_main_context_unref(context);
            gst_element_set_state(pipeline, GST_STATE_NULL);
            gst_object_unref(pipeline);
            return nullptr;
        }

        //Free resources

        javaVm->DetachCurrentThread();
        g_main_context_pop_thread_default(context);
        g_main_context_unref(context);
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);

        inst->pipeline = nullptr;
        return nullptr;
    }

    using PThreadFunc = void *(*)(void*);

    static GSInstance *make(JNIEnv *env, jobject thiz) {

        gst_debug_set_threshold_for_name("basesrc", GST_LEVEL_DEBUG);

        GST_DEBUG_CATEGORY_INIT(debugCategory, "phomera", 0, "Phomera");
        gst_debug_set_threshold_for_name("phomera", GST_LEVEL_DEBUG);

        auto app = env->NewGlobalRef(thiz);

        GST_DEBUG("Created GlobalRef for app object at %p", app);

        GSInstance *inst = new GSInstance{ env, app };
        inst->version = env->GetVersion();

        pthread_create(&inst->thread, nullptr, (PThreadFunc) &threadFunction, inst);

        return inst;
    }

    void free() {

        if(!mainLoop)
            return;

        GST_DEBUG("Quitting main loop");
        g_main_loop_quit(mainLoop);

        GST_DEBUG("Waiting for thread to finish");
        pthread_join(thread, nullptr);

        GST_DEBUG("Deleting GlobalRef for app object at %p", app);
        env->DeleteGlobalRef(app);

        delete this;
    }
};

//Functions for interfacing with Java

extern "C" {

    JNIEXPORT jstring Java_net_osomi_phomera_MainActivity_getGStreamerVersion(JNIEnv *env, jobject thiz) {
        char *version = gst_version_string();
        auto str = env->NewStringUTF(version);
        g_free(version);
        return str;
    }

    JNIEXPORT void Java_net_osomi_phomera_MainActivity_finalizeGS(JNIEnv *env, jobject thiz) {

        if(instance)
            instance->free();

        instance = {};
    }

    JNIEXPORT void Java_net_osomi_phomera_MainActivity_initGS(JNIEnv *env, jobject thiz) {

        if(instance)
            instance->free();

        instance = GSInstance::make(env, thiz);
    }

    JNIEXPORT jboolean Java_net_osomi_phomera_MainActivity_initNative(JNIEnv *env, jclass klass) {

        env->GetJavaVM(&javaVm);

        onGSReady = env->GetMethodID(klass, "onGSReady", "()V");
        onGSUnready = env->GetMethodID(klass, "onGSUnready", "()V");

        if(!onGSReady || !onGSUnready) {
            __android_log_print(ANDROID_LOG_ERROR, "net.osomi.phomera", "Couldn't init native; onGSReady not found");
            return JNI_FALSE;
        }

        return JNI_TRUE;
    }

    JNIEXPORT void Java_net_osomi_phomera_MainActivity_initSurface(JNIEnv *env, jobject thiz, jobject surface) {

        if(!instance)
            return;

        ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
        GST_DEBUG("Performing surface update %p (native window %p)", surface, window);

        if(instance->surface) {

            ANativeWindow_release(instance->surface);

            if(instance->surface == window && instance->videoSink) {
                gst_video_overlay_expose(GST_VIDEO_OVERLAY(instance->videoSink));
                gst_video_overlay_expose(GST_VIDEO_OVERLAY(instance->videoSink));       //Call it twice because of GLES double buffering
                return;
            }
        }

        instance->surface = window;

        if(instance->mainLoop) {
            gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(instance->videoSink), (guintptr) instance->surface);
            gst_element_set_state(instance->pipeline, GST_STATE_PLAYING);
        }
    }

    JNIEXPORT void Java_net_osomi_phomera_MainActivity_finalizeSurface(JNIEnv *env, jobject thiz) {

        if(!instance)
            return;

        GST_DEBUG("Releasing native widndow %p", instance->surface);

        if (instance->videoSink) {
            gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(instance->videoSink), (guintptr) nullptr);
            gst_element_set_state(instance->pipeline, GST_STATE_READY);
        }

        ANativeWindow_release(instance->surface);
        instance->surface = nullptr;

    }
}