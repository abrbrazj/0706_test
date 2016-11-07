#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "cJSON.h"

/*
   {
        "name" :"jr",
        "age":26,
        "id":0001,
        "language":[
            "C",
            "C++",
            "python"
        ]
   } 
 
 */

void test_create_json(char *str_out)
{
	cJSON *info;
	cJSON *language;
	
	info = cJSON_CreateObject();
	cJSON_AddStringToObject(info, "name", "jr");
	cJSON_AddNumberToObject(info, "age", 26);
	cJSON_AddNumberToObject(info, "id", 0001);
	
	language = cJSON_CreateArray();
	cJSON_AddItemToArray(language, cJSON_CreateString("C"));
	cJSON_AddItemToArray(language, cJSON_CreateString("C++"));
	cJSON_AddItemToArray(language, cJSON_CreateString("python"));
	
	cJSON_AddItemToObject(info, "language", language);
	
	char *out = cJSON_Print(info);
	
	strcpy(str_out, out);
	cJSON_Delete(info);

	free(out);
	
    return ;
}

void test_parse_json(const char *str)
{
	cJSON *info = NULL;
	info = cJSON_Parse(str);
	
	cJSON *name = NULL;
	name = cJSON_GetObjectItem(info, "name");
	printf("%s = %s \n",name->string, name->valuestring);
	
	cJSON *age = NULL;
	age = cJSON_GetObjectItem(info, "age");
	printf("%s = %d \n",age->string, age->valueint);
	
	cJSON *id = NULL;
	id = cJSON_GetObjectItem(info, "id");
	printf("%s = %d \n",id->string, id->valueint);
	
	cJSON *language = NULL;
	language = cJSON_GetObjectItem(info, "language");
	int language_len = cJSON_GetArraySize(language);
	int i = 0;
	for(i = 0; i < language_len; ++i)
	{
		cJSON *tmp = cJSON_GetArrayItem(language, i);
		printf("language[%d] = %s \n",i, tmp->valuestring);
	}
	
	cJSON_Delete(info);
	
}

int main(int argc, char *argv[])
{
    char str[4096] = {0};

    test_create_json(str);

    printf("%s\n", str);

    
    printf("===================\n"); 

    test_parse_json(str);

    return 0;
}	
