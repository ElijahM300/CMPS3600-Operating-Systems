/*Elijah Morris
 *
 * dict program
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
const char fname[] = "/usr/share/dict/cracklib-small";
#define MAX_WORD_LENGTH 100
const int CARRIAGE_RETURN = 13;

int main(void) 
{
    int count;
    char line[MAX_WORD_LENGTH];
	FILE *fpi;
	fpi = fopen(fname, "r");
	if (!fpi) {
        perror("ERROR: opening file for input\n");
        exit(0);
		return 0;
	}
	count = 0;
    while(fgets(line, MAX_WORD_LENGTH, fpi) != NULL){
        ++count;
    }
    printf("count: %i\n", count);
    /* Open a scope for testing the C language */
    {
        char **arr = (char **)malloc(count * sizeof(char *));
        int n = 0;
        int i;
        /* go back to top of file */
        fseek(fpi, 0, SEEK_SET);
        while (fgets(line, MAX_WORD_LENGTH, fpi) != NULL) {
            /* remove newline character from string. */
            int slen;
            char *p = line;
            while(*p){
                if(*p == '\n' || *p == CARRIAGE_RETURN){
                    *p = '\0';
                    break;
                }
                ++p;
            }
            slen = strlen(line);
            arr[n] = (char *)malloc(slen + 1); /*add 1 for null terminator */
            strcpy(arr[n++], line);
        }
        /*------------------------*/
        for (i=0; i<count; i++){
            if(i)
                printf(",");
            printf("%s", arr[i]);
        }
        /*-----------------------*/
        printf("\n");
        fclose(fpi);
        printf("\n");
    }
	return 0;
}


















