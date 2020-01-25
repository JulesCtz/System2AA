#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <pthread.h>

#include "stream_common.h"
#include "oggstream.h"


int main(int argc, char *argv[]) {
    int res;

    if (argc != 2) {
	fprintf(stderr, "Usage: %s FILE", argv[0]);
	exit(EXIT_FAILURE);
    }
    assert(argc == 2);


    // Initialisation de la SDL
    res = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_EVENTS);
    atexit(SDL_Quit);
    assert(res == 0);
    
    // start the two stream readers

    if(pthread_create(&t1, NULL, theoraStreamReader, (void *) argv[1])!=0){
        perror("pthread_create");
        exit(1);
    }
    pthread_create(&t2, NULL, vorbisStreamReader, (void *) argv[1]);

    // wait audio thread
    pthread_join(t2, NULL);
//    if (status == ((void *) 123456789L)) {
//        printf("Thread_%lx_completed_ok. \n", t2);
//    }

    // 1 seconde de garde pour le son,

    sleep(1);


    // tuer les deux threads videos si ils sont bloqués
    pthread_detach(t1);
    pthread_detach(theora2sdlthread);

    // attendre les 2 threads videos
   pthread_join(t1, NULL);
//   if(status == ((void *) 123456789L)) {
//       printf("Thread_%lx_completed_ok. \n", t1);
//   }

   pthread_join(theora2sdlthread, NULL);
//   if(status == ((void *) 123456789L)) {
//       printf("Thread_%lx_completed_ok. \n", theora2sdlthread);
//   }

    
    exit(EXIT_SUCCESS);    
}
