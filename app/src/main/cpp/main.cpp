#include <pthread.h>
#include <jni.h>
#include <memory.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>

#include "Includes/Logger.h"
#include "Substrate/CydiaSubstrate.h"
#import "Includes/Utils.h"

bool radar = false;
bool PlayerUpdateHookInitialized = false;
const char* libName = "libil2cpp.so";

void(*old_GameManager_LateUpdate)(void *instance);
void GameManager_LateUpdate(void *instance) {
    //Check if instance is NULL to prevent crashes!  If the instance object is NULL,
    //this is what the call to update would look like in C++:
    //NULL.Update(); and dat doesnt make sense right?
    if(instance != NULL) {
        if(!PlayerUpdateHookInitialized){
            //Check if this hook initialized. If so log
            PlayerUpdateHookInitialized = true;
            LOGI("Player_Update hooked");
        }
        //Your code here
    }
    old_GameManager_LateUpdate(instance);
}

void(*old_MinimapItem_Show)(void* instance, bool idk);
void MinimapItem_Show(void* instance, bool idk){
    if(instance != NULL){
        if(radar){
            return;
        }
    }
    old_MinimapItem_Show(instance, idk);
}

// we will run our patches in a new thread so our while loop doesn't block process main thread
void* hack_thread(void*) {
    LOGI("I have been loaded. Mwuahahahaha");
    // loop until our target library is found
    do {
        sleep(1);
    } while (!isLibraryLoaded(libName));
    LOGI("I found the il2cpp lib. Address is: %lu", findLibrary(libName));
    LOGI("Hooking Player_Update");
    MSHookFunction((void*)getAbsoluteAddress(libName, 0x7000DCCD0), (void*)GameManager_LateUpdate, (void**)&old_GameManager_LateUpdate);
    MSHookFunction((void*)getAbsoluteAddress(libName, 0x15AFAA46F8), (void*)MinimapItem_Show, (void**)old_MinimapItem_Show);

    return NULL;
}

extern "C"
JNIEXPORT jobjectArray JNICALL Java_com_dark_force_NativeLibrary_getListFT(JNIEnv *env, jclass jobj){
    jobjectArray ret;
    int i;
    int Total_Feature = 1;
    const char *features[]= {"Minimap Hack"};

    ret= (jobjectArray)env->NewObjectArray(Total_Feature,
                                           env->FindClass("java/lang/String"),
                                           env->NewStringUTF(""));

    for(i=0;i<Total_Feature;i++) {
        env->SetObjectArrayElement(
                ret,i,env->NewStringUTF(features[i]));
    }
    return(ret);
}


extern "C"
JNIEXPORT void JNICALL Java_com_dark_force_NativeLibrary_changeToggle(JNIEnv *env, jclass thisObj, jint number) {
    int i = (int) number;
    switch (i) {
        case 0:
            radar = !radar;
            break;
        default:
            break;
    }
    return;
}

extern "C"
{
JNIEXPORT void JNICALL Java_com_dark_force_NativeLibrary_init(JNIEnv * env, jclass obj){
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}
};
