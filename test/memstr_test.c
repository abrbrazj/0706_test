#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* memstr(char* full_data, int full_data_len, char* substr)
{
    if (full_data == NULL || full_data_len <= 0 || substr == NULL) {
        return NULL;
    }

    if (*substr == '\0') {
        return NULL;
    }

    int sublen = strlen(substr);

    int i;
    char* cur = full_data;
    int last_possible = full_data_len - sublen + 1;
    for (i = 0; i < last_possible; i++) {
        if (*cur == *substr) {
            //assert(full_data_len - i >= sublen);
            if (memcmp(cur, substr, sublen) == 0) {
                //found
                return cur;
            }
        }
        cur++;
    }
    return NULL;
}

int main()
{
	char *full_data = "abcdefghijklmnopqrstuvwxyz";
	int full_data_len = strlen(full_data);
	char *substr = "efg";
	
	char *ret = memstr(full_data, full_data_len, substr);
	
	printf("%s\n",ret);
	return 0;
}