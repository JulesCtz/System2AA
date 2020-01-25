#include "synchro.h"
#include "ensitheora.h"

#include <pthread.h>

bool fini;

/* les variables pour la synchro, ici */
pthread_cond_t tailleFenetreEnvoi, lecture, ecriture, fenetreTexturePretes;

pthread_mutex_t mut;

int nbAAfficher = 0;
bool textureRecue = false, ecritureEnCours = false, tailleRecue = false;

/* l'implantation des fonctions de synchro ici */
void envoiTailleFenetre(th_ycbcr_buffer buffer) {
    pthread_mutex_lock(&mut);
    windowsx = buffer[0].width;
    windowsy = buffer[0].height;
    tailleRecue = 1;
    pthread_cond_signal(&tailleFenetreEnvoi);
    pthread_mutex_unlock(&mut);
}

void attendreTailleFenetre() {
    pthread_mutex_lock(&mut);
    while(!tailleRecue){
        pthread_cond_wait(&tailleFenetreEnvoi, &mut);
    }
    pthread_mutex_unlock(&mut);
}

void signalerFenetreEtTexturePrete() {
    pthread_mutex_lock(&mut);
    textureRecue = true;
    pthread_cond_signal(&fenetreTexturePretes);
    pthread_mutex_unlock(&mut);
}

void attendreFenetreTexture() {
    pthread_mutex_lock(&mut);
    while(!textureRecue) {
        pthread_cond_wait(&fenetreTexturePretes, &mut);
    }
    pthread_mutex_unlock(&mut);
}

void debutConsommerTexture() {
    pthread_mutex_lock(&mut);
    while(nbAAfficher == 0 || ecritureEnCours) {
        pthread_cond_wait(&lecture, &mut);
    }
    pthread_mutex_unlock(&mut);
}

void finConsommerTexture() {
    pthread_mutex_lock(&mut);
    nbAAfficher--;
    if(nbAAfficher < NBTEX){
        pthread_cond_signal(&ecriture);
    }
    pthread_mutex_unlock(&mut);
}


void debutDeposerTexture() {
    pthread_mutex_lock(&mut);
    while (ecritureEnCours || nbAAfficher == NBTEX) {
        pthread_cond_wait(&ecriture, &mut);
    }

    ecritureEnCours = true;
    pthread_mutex_unlock(&mut);
}

void finDeposerTexture() {
    pthread_mutex_lock(&mut);
    nbAAfficher++;
    ecritureEnCours = false;

    if (nbAAfficher < NBTEX){
        pthread_cond_signal(&ecriture);
    } else {
        pthread_cond_broadcast(&lecture);
    }
    pthread_mutex_unlock(&mut);
}
