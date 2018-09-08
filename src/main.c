#include <stdbool.h>
#include <switch.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util.h"

#include "mp3.h"

#define HEAP_SIZE 0x000540000

u32 __nx_applet_type = AppletType_None;

char fake_heap[HEAP_SIZE];

void __libnx_initheap(void){
    extern char *fake_heap_start;
    extern char *fake_heap_end;

    fake_heap_start = fake_heap;
    fake_heap_end = fake_heap + HEAP_SIZE;
}

void __appInit(void){
    Result rc;
    svcSleepThread(10000000000L);
    rc = smInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = fsInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = fsdevMountSdmc();
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = timeInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = hidInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);
}

void __appExit(void)
{
    fsdevUnmountAll();
    fsExit();
    smExit();
    audoutExit();
    timeExit();
}

void inputPoller(){
    while (appletMainLoop()){
        svcSleepThread(1e+8L);
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

        if ((kDown & KEY_PLUS || kDown & KEY_MINUS || kDown & KEY_Y) && (kHeld & KEY_PLUS && kHeld & KEY_MINUS && kHeld & KEY_Y)){
            setAlbum(!isAlbum());
        }
    }
}

int main(){
    DIR* dir = opendir("/album-switched/");
    if(dir == NULL){
        mkdir("/album-switched", 0700);
    }
    albumInit();
    mp3MutInit();
    Thread pauseThread;
    Result rc = threadCreate(&pauseThread, inputPoller, NULL, 0x4000, 49, 2);
    if (R_FAILED(rc)){
        fatalLater(rc);
    }
    rc = threadStart(&pauseThread);
    if (R_FAILED(rc)){
        fatalLater(rc);
    }
    while(appletMainLoop()){
        svcSleepThread(1e+8L);
    }
}