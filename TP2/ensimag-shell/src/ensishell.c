/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */
#if USE_GUILE == 1
#include <libguile.h>

struct job {
    int id;
    int pid;
    char * commandLine;
    struct job* nextElem;
};

struct liste_jobs {
    struct job *tete;
};

struct liste_jobs *jobs;

int question6_executer(char *line)
{
    /* Question 6: Insert your code to execute the command line
     * identically to the standard execution scheme:
     * parsecmd, then fork+execvp, for a single command.
     * pipe and i/o redirection are not required.
     */
    printf("Not implemented yet: can not execute %s\n", line);

    /* Remove this line when using parsecmd as it will free it */
    free(line);

    return 0;
}

SCM executer_wrapper(SCM x)
{
    return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif

void commandJobs(){
    struct job * cur = jobs->tete->nextElem;
    while (cur != NULL){
        int status;
        if(waitpid(cur->pid, &status, WNOHANG) == 0){
            printf("%d   |   ", cur->pid);
            printf("%s ", cur->commandLine);
            printf("   |   %d\n", cur->id);
        }
        cur = cur->nextElem;
    }
}

void customExec(char **cmd){
    if (strcmp(cmd[0], "jobs") == 0){
        commandJobs();
    } else {
        execvp(cmd[0], cmd);
    }
}

void terminate(char *line) {
#if USE_GNU_READLINE == 1
    /* rl_clear_history() does not exist yet in centOS 6 */
    clear_history();
#endif
    if (line)
        free(line);
    printf("exit\n");
    exit(0);
}


int main() {
    jobs = malloc(sizeof(struct liste_jobs));
    struct job * teteElem = malloc(sizeof(struct job));
    teteElem->id = 0;
    teteElem->pid = 0;
    teteElem->commandLine = NULL;
    teteElem->nextElem = NULL;
    jobs->tete = teteElem;

    printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
    scm_init_guile();
    /* register "executer" function in scheme */
    scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

    while (1) {
        struct cmdline *l;
        char *line=0;
        int i, j;
        char *prompt = "ensishell>";

        /* Readline use some internal memory structure that
           can not be cleaned at the end of the program. Thus
           one memory leak per command seems unavoidable yet */
        line = readline(prompt);
        if (line == 0 || ! strncmp(line,"exit", 4)) {
            terminate(line);
        }

#if USE_GNU_READLINE == 1
        add_history(line);
#endif


#if USE_GUILE == 1
        /* The line is a scheme command */
        if (line[0] == '(') {
            char catchligne[strlen(line) + 256];
            sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
            scm_eval_string(scm_from_locale_string(catchligne));
            free(line);
            continue;
        }
#endif

        /* parsecmd free line and set it up to 0 */
        l = parsecmd( & line);

        /* If input stream closed, normal termination */
        if (!l) {

            terminate(0);
        }


        if (l->err) {
            /* Syntax error, read another command */
            printf("error: %s\n", l->err);
            continue;
        }

        if (l->in) {
            printf("in: %s\n", l->in);
        }
        if (l->out){
            printf("out: %s\n", l->out);
        }

        pid_t pid = fork();
        if(pid == -1) {
            printf("\nfork error, exiting\n");
            exit(1);
        }
        int status;

        if (l->bg){
            if(pid != 0){
                struct job* cur = jobs->tete;
                while(cur->nextElem != NULL) {
                    cur = cur->nextElem;
                }
                int idElem = cur->id + 1;
                struct job* elem = malloc(sizeof(struct job));
                elem->id= idElem;
                elem->pid = pid;
                elem->commandLine = l->seq[0][0];
                elem->nextElem = NULL;
                cur->nextElem = elem;
                cur = cur->nextElem;

                continue;
            } else {
                printf("background (&)\n");
            }
        } else {
            if (pid != 0){
                waitpid(pid, &status, WUNTRACED);
            }
        }

        /* Display each command of the pipe */
        if (pid == 0){
            for (i=0; l->seq[i]!=0; i++) {
                char **cmd = l->seq[i];
                printf("seq[%d]: ", i);
                for (j=0; cmd[j]!=0; j++) {
                    printf("'%s' ", cmd[j]);
                }
                printf("\n");
            }
        }

        if(l->seq[1]){
            // gestion du pipe
            int fd[2];
            pipe(fd);
            pid_t c1, c2;

            c1 = fork();
            if(c1 < 0){
                perror("fork");
                exit(1);
            }

            if (c1 == 0){
                // Child1 process
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);

                execvp(l->seq[0][0], l->seq[0]);
                return 0;
            } else {
                // parent process
                c2 = fork();

                if(c2 == 0) {

                    close(fd[1]);
                    dup2(fd[0], STDIN_FILENO);
                    close(fd[0]);

                    execvp(l->seq[1][0], l->seq[1]);
                    return 1;
                } else {
                    waitpid(c2, NULL, WCONTINUED | WUNTRACED | WNOHANG);
                    waitpid(c1, NULL, WCONTINUED | WUNTRACED);
                }
            }

        } else {
            // Cas d'une commande simple
            if (strcmp(l->seq[0][0], "jobs") == 0 && pid != 0){
                // Gestion de la commande jobs
                commandJobs();
            } else {
                if(pid == 0){
                    execvp(l->seq[0][0], l->seq[0]);
                    exit(0);
                }
            }
        }

        if (pid == 0){
            exit(0);
        }
    }
}
