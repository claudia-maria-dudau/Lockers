#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "synclibcm.h"
#include <string.h>
#include <errno.h>

#define NTHRS 7
rwlockcm rw;

FILE *input, *output;
char text[] = "Maecenas et purus in risus consectetur vestibulum at sit amet leo. Praesent lacinia nunc vitae sagittis interdum. Vivamus molestie, elit bibendum iaculis posuere, odio ligula rutrum ligula, consequat dictum urna elit a diam. Nam cursus luctus mauris eu rhoncus. Mauris non quam at magna tempus iaculis quis vel tortor. Aenean mattis malesuada dui sit amet lobortis. Etiam quis tempus massa. In ut viverra ligula. Nam porta erat vel mauris tristique, gravida sodales quam ullamcorper. Quisque a ex ut augue faucibus efficitur.";

void* Read(void* argument)
{
	int* index = (int *) argument;
	input = fopen("text.txt", "r");
	char* buff = malloc(sizeof(char) * 20);
	
    rwlockcm_readbegin(&rw);

    //citire din fisier
	while(fscanf(input, "%s", buff) > 0)
	{
		rwlockcm_readend(&rw);

        printf("Reader %d sees %s\n", (*index), buff);
		
        rwlockcm_readbegin(&rw);
	}

	rwlockcm_readend(&rw);
}	

void* Write(void* argument)
{
	int* index = (int *) argument;
	output = fopen("text.txt", "a");
    char* rest = text;
	char* buff = strtok_r(rest, " ", &rest);

    //scriere in fisier cuvant cu cuvant a string-ului din text
	while(buff != NULL)
	{
		rwlockcm_writebegin(&rw);

		printf("Writer %d writes %s\n", (*index), buff); 
		fprintf(output, "%s ", buff);
		
        rwlockcm_writeend(&rw);

        buff = strtok_r(rest, " ", &rest);
	}	
}		


int main()
{
    printf("NTHRS=%d\n", NTHRS);
    
	rwlockcm_init(&rw);

    //creare thread-uri
    pthread_t *thrs = malloc(NTHRS * sizeof(pthread_t));
    if (thrs == NULL){
        perror(NULL);
        return errno;
    }

    int ids[NTHRS]; //vector cu id-urile thread-urilor
	for(int i = 0; i < NTHRS; i++)
	{
		ids[i] = i;
		if (i % 2 == 0)
			pthread_create(&thrs[i], NULL,  Read, &ids[i]);
		else 
            pthread_create(&thrs[i], NULL,  Write, &ids[i]);
	}

    //reunire thread-uri la final
	for(int i = 0; i < NTHRS; i++)
		pthread_join(thrs[i], NULL);

    //distrugere rwlock
	rwlockcm_destroy(&rw);

	fclose(input);
	fclose(output);
	return 0;
}