#include <stdlib.h>
#include <stdio.h>

int * getSequence (int start);

int main() {


    for (int ps_i = 0; ps_i < 10; ps_i++) {
        int * sequence;
        int pid;

        sequence = getSequence(ps_i*100);
        pid = fork();

        if (pid == 0) {
            /* Потомок */
            for (int i = 0; i < 10; i++) {
                printf("[%d]    %d) %d\n", ps_i, i, sequence[i]);
            }

            sleep(5);
            free(sequence);
            printf("[%d] Bye\n", ps_i);
            exit(0);
        }
    }

    sleep(10);
    printf("ByeBye\n");
    return 0;
}

int * getSequence (int start) {
    int * sequence;

    sequence = (int *) malloc(10*sizeof(int));

    for (int i = 0; i < 10; i++) {
        sequence[i] = start + i;
    }

    return sequence;
}
