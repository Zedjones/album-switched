#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <math.h>
#include <dirent.h>

#include <switch.h>

#include "mp3.h"

#define ALBUM_TID 0x010000000000100D
#define ALBUM_TITLE_DIR "/atmosphere/titles/010000000000100D"
#define RENAMED_DIR "/album-switched/010000000000100D"
#define BASE_DIR "/album-switched"

static Mutex albumMutex;

void fatalLater(Result err)
{
    Handle srv;

    while (R_FAILED(smGetServiceOriginal(&srv, smEncodeName("fatal:u"))))
    {
        // wait one sec and retry
        svcSleepThread(1000000000L);
    }

    // fatal is here time, fatal like a boss
    IpcCommand c;
    ipcInitialize(&c);
    ipcSendPid(&c);
    struct
    {
        u64 magic;
        u64 cmd_id;
        u64 result;
        u64 unknown;
    } * raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 1;
    raw->result = err;
    raw->unknown = 0;

    ipcDispatch(srv);
    svcCloseHandle(srv);
}

void albumInit(){
    mutexInit(&albumMutex);
}

bool isAlbum(){
    mutexLock(&albumMutex);
    bool empty = true;
    struct dirent *de;
    DIR *dr = opendir(BASE_DIR);
    while ((de = readdir(dr)) != NULL){
        empty = false;
        break;
    }
    mutexUnlock(&albumMutex);
    return empty;
}

void setAlbum(bool newAlbum){
    mutexLock(&albumMutex);
    if(newAlbum){
        rename(RENAMED_DIR, ALBUM_TITLE_DIR);
        playMp3("/ftpd/pauseon.mp3");
        FILE* new_album = fopen("/newAlbum.txt", "w");
        fclose(new_album);
    }
    else{
        rename(ALBUM_TITLE_DIR, RENAMED_DIR);
        playMp3("/ftpd/pauseoff.mp3");
        FILE* not_new_album = fopen("/notNewAlbum.txt", "w");
        fclose(not_new_album);
    }
    mutexUnlock(&albumMutex);
}