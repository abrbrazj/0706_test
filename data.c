#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fdfs_client.h"
#include "fcgi_stdio.h"
#include "fcgi_config.h"
#include "cJSON.h"
#include "util_cgi.h"
#include "redis_op.h"
#include "make_log.h"

#define DATA_LOG_MODULE          "cgi"
#define DATA_LOG_PROC            "data"



void fileinfo_to_json(int fromId, int count, char *cmd, char *username)
{
	int i = 0;
	int retn = 0;
	
    cJSON *root = NULL; 
    cJSON *array =NULL;
    
    int value_num;
    redisContext *redis_conn = NULL;
    
    char file_name[2048] = {0};
    char file_id[2048] = {0};
    int endId = fromId + count - 1;
    
    time_t timep;
    time(&timep);
    char *time = ctime(&timep);
    
    char url[2048] = {0};
    
    char user_name[2048] = {0};
    
    char *picurl = NULL;
    char *out;
    
    RVALUES fileid_list_values = NULL;
	fileid_list_values = malloc(count*VALUES_ID_SIZE);
	
	redis_conn = rop_connectdb_nopwd("127.0.0.1", "6379");
    if (redis_conn == NULL) {
        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "conn error");
        exit(1);
    }
    
    char *Key_File_ID = "FILE_INFO_LIST";
	retn = rop_range_list(redis_conn, Key_File_ID, fromId, endId, fileid_list_values, &value_num);
    if (retn < 0) {
        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "redis range error");
        rop_disconnect(redis_conn);
        return;
    }
    LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "value_num=%d\n", value_num);
    
    
    
    root = cJSON_CreateObject();
    array = cJSON_CreateArray();
    for(i = 0; i < value_num; ++i)
    {
    	cJSON* item = cJSON_CreateObject();

        //id
        cJSON_AddStringToObject(item, "id", fileid_list_values[i]);

        //kind
        cJSON_AddNumberToObject(item, "kind", 2);
    	
    	
	    //title_m(filename)
	    char *FILEID_NAME_HASH = "FILEID_NAME_HASH";
	    retn = rop_get_hash(redis_conn, FILEID_NAME_HASH, file_id, file_name);
	    if (retn == -1) {
	        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "HGET error:(%s : %s)", file_id, file_name);
	        exit(1);
	    }
	    LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "HGET succ:(%s : %s)", file_id, file_name);
	    cJSON_AddStringToObject(item, "title_m", file_name);
	    
	    //time
	    char *FILEID_TIME_HASH = "FILEID_TIME_HASH";
	    LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "time : %s", time);
	    retn = rop_get_hash(redis_conn, FILEID_TIME_HASH, file_id, time);
	    if (retn == -1) {
	        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "HGET error:(%s : %s)", file_id, time);
	        exit(1);
	    }
	    LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "HGET succ:(%s : %s)", file_id, time);
	    cJSON_AddStringToObject(item, "descrip", time);
	    
	    //url
	    char *FILEID_URL_HASH = "FILEID_URL_HASH";
	    //char *buf_http = getenv("HTTP_REFERER");
	    retn = rop_get_hash(redis_conn, FILEID_URL_HASH, file_id, url);
	    if (retn == -1) {
	        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "HGET error:(%s : %s)", file_id, url);
	        exit(1);
	    }
	    LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "HGET succ:(%s : %s)", file_id, url);
	    cJSON_AddStringToObject(item, "url", url);
	    
	    //title_s(username)
	    char *FILEID_USR_HASH = "FILEID_USR_HASH";
	    //char *buf_user = getenv("HOME");
	    retn = rop_get_hash(redis_conn, FILEID_USR_HASH, file_id, user_name);
	    if (retn == -1) {
	        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "HGET error:(%s : %s)", file_id, user_name);
	        exit(1);
	    }
	    LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "HGET succ:(%s : %s)", file_id, user_name);
	    cJSON_AddStringToObject(item, "title_s", user_name);
	    
	    //picurl_m
	    
	    strcpy(picurl,"http://172.16.0.148/static/file_png/jpg.png");
	    cJSON_AddStringToObject(item, "picurl_m", picurl);
	    
	    cJSON_AddItemToArray(array, item);
	}
	cJSON_AddItemToObject(root, "games", array);

    out = cJSON_Print(root);

    LOG(DATA_LOG_MODULE, DATA_LOG_PROC,"%s", out);
    printf("%s\n", out);

    free(fileid_list_values);
    free(out);

    rop_disconnect(redis_conn);
}

int main (void)
{
	char fromId[5];
    char count[5];
    char cmd[20];
    char user[2048];
    char fileId[2048];
    

    while (FCGI_Accept() >= 0) {
        char *query = getenv("QUERY_STRING");
        memset(fromId, 0, 5);
        memset(count, 0, 5);
        memset(cmd, 0, 20);
        memset(user, 0, 2048);
        memset(fileId, 0, 2048);

        query_parse_key_value(query, "cmd", cmd, NULL);

        if (strcmp(cmd, "newFile") == 0) {
            //请求私有文件列表命令

            query_parse_key_value(query, "fromId", fromId, NULL);
            query_parse_key_value(query, "count", count, NULL);
            query_parse_key_value(query, "user", user, NULL);
            LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "=== fromId:%s, count:%s, cmd:%s, user:%s", fromId, count, cmd, user);
            cgi_init();

            printf("Content-type: text/html\r\n");
            printf("\r\n");

            fileinfo_to_json(atoi(fromId), atoi(count), cmd, user);
        }
  
    }
    return 0;

}