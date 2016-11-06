/*
 * echo.c --
 *
 *	Produce a page containing all FastCGI inputs
 *
 *
 * Copyright (c) 1996 Open Market, Inc.
 *
 * See the file "LICENSE.TERMS" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */
#ifndef lint
static const char rcsid[] = "$Id: echo.c,v 1.5 1999/07/28 00:29:37 roberts Exp $";
#endif /* not lint */

#include "fcgi_config.h"
#include "make_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h> 

#include <hiredis.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h> 
#include <time.h>

#include "redis_op.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#include "fcgi_stdio.h"

#define FCGI_TEST_MODULE "test"
#define FCGI_TEST_PROC "fcgi_test"

#define FILE_ID_LEN   4096


static void PrintEnv(char *label, char **envp)
{
	printf("%s:<br>\n<pre>\n", label);
	for ( ; *envp != NULL; envp++) {
		printf("%s\n", *envp);
	}
	printf("</pre><p>\n");
}


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



int main ()
{
	char **initialEnv = environ;
	int count = 0;
	char *full_buf = NULL;
	char *full_temp = NULL;
	
	char *file_start = NULL;
	char *file_end = NULL;
	char file_id[FILE_ID_LEN] = {0};
	
	char *file_name_start = NULL;
	char *file_name = NULL;
	
	pid_t pid;
	int pfd[2];//管道fd
	
	redisContext *redis_conn = NULL;
	//char buf[4096] = {0};
	
	int i = 0;
	int len = 0;
	int ret = 0;

	while (FCGI_Accept() >= 0) 
	{
		char *contentLength = getenv("CONTENT_LENGTH");
		
		printf("Content-type: text/html\r\n"
				"\r\n"
				"<title>FastCGI echo</title>"
				"<h1>FastCGI echo</h1>\n"
				"Request number %d,  Process ID: %d<p>\n", ++count, getpid());

		if (contentLength != NULL) 
		{
			len = strtol(contentLength, NULL, 10);
		}
		else 
		{
			len = 0;
		}

		if (len <= 0) 
		{
			printf("No data from standard input.<p>\n");
		}
		else 
		{
			int ch;

			full_buf = (char *)malloc(sizeof(char) * len);
			memset(full_buf,0,sizeof(char) * len);
			printf("Standard input:<br>\n<pre>\n");
			for (i = 0; i < len; i++) 
			{
				if ((ch = getchar()) < 0) 
				{
					printf("Error: Not enough bytes received on standard input<p>\n");
					break;
				}
				full_buf[i] = ch;
				full_temp = full_buf;
				putchar(ch);
			}
			printf("\n</pre><p>\n");
		}

		/*做用户自定义操作*/
		
		char *buf_type = getenv("CONTENT_TYPE");
		buf_type = memstr(buf_type, strlen(buf_type), "----");
		LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "CONTENT_TYPE : %s\n", buf_type);
		
		file_name_start = memstr(full_buf, strlen(full_buf), "filename");
		//LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "File Name : %s len: %d\n", file_name_start,strlen(file_name_start));
		
		file_name = memstr(file_name_start, strlen(file_name_start), "\"");
		//LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "File Name : %s\n", file_name);
		
		for(i = 1 ; i < strlen(file_name) - 1; ++i)
		{
			file_name[i - 1] = file_name[i];
			if(file_name[i] == '"')
			{
				file_name[i - 1] = '\0';
				break;
			}
		}
		LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "File Name : %s\n", file_name);
		
		
		for(i = 0 ; i < 4; ++i)
		{
			full_temp = memstr(full_buf,len,"\n\r");
			full_temp += 3;
		}
		file_start = full_temp;
		LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "file_start : %s\n", file_start);
		
		file_end = memstr(file_start, len, "------");
		LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "full_buf : %s\n", file_end);
		
		
		int file_len = (file_end - file_start);
		int fp = open(file_name,O_RDWR|O_CREAT|O_TRUNC,0777);
		write(fp,file_start,file_len);
		
		close(fp);
		
		//上传fastFDS
	
	    if (pipe(pfd) < 0) {
	        LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "pip error");
	        exit(1);
	    }
	
	    pid = fork();
	    if (pid == 0) 
	    {
	        close(pfd[0]);
	        dup2(pfd[1], STDOUT_FILENO);

	        execlp("fdfs_upload_file", "fdfs_upload_file", "./conf/client.conf", file_name, NULL);
	        LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "exec error");
	    }
	    else 
	    {
	        close(pfd[1]);
	        wait(NULL);
	        read(pfd[0], file_id, FILE_ID_LEN);
			LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "upload file_id[%s] succ!", file_id);
	    }
	    
	    
	    //保存hiredis
	
	    redis_conn = rop_connectdb_nopwd("127.0.0.1", "6379");
	    if (redis_conn == NULL) {
	        LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "conn error");
	        exit(1);
	    }
		
		
		//LIST_OF_FILE_ID
		char *Key_File_ID = "FILE_INFO_LIST";
		ret = rop_list_push(redis_conn, Key_File_ID, file_id);
		if (ret == -1) {
	        LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "LPUSH error:(%s)", file_id);
	        exit(1);
	    }
	    LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "LPUSH succ:(%s)", file_id);
	    
	    
	    //HASH_OF_FILE_ID_AND_NAME
	    char *FILEID_NAME_HASH = "FILEID_NAME_HASH";
	    ret = rop_set_hash(redis_conn, "HSET", FILEID_NAME_HASH, file_id, file_name);
	    if (ret == -1) {
	        LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "HSET error:(%s : %s)", file_id, file_name);
	        exit(1);
	    }
	    LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "HSET succ:(%s : %s)", file_id, file_name);
	    
	    
	    //HASH_OF_FILE_ID_AND_TIME
	    char *FILEID_TIME_HASH = "FILEID_TIME_HASH";
	    time_t timep;
	    time(&timep);
	    char *time = ctime(&timep);
	    ret = rop_set_hash(redis_conn, "HSET", FILEID_TIME_HASH, file_id, time);
	    if (ret == -1) {
	        LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "HSET error:(%s : %s)", file_id, time);
	        exit(1);
	    }
	    LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "HSET succ:(%s : %s)", file_id, time);
	    
	    
	    //HASH_OF_FILE_ID_AND_URL
	    char *FILEID_URL_HASH = "FILEID_URL_HASH";
	    char *buf_http = getenv("HTTP_REFERER");
	    char *url = NULL;
	    sprintf(url,"%s%s",buf_http, file_id);
	    ret = rop_set_hash(redis_conn, "HSET", FILEID_URL_HASH, file_id, url);
	    if (ret == -1) {
	        LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "HSET error:(%s : %s)", file_id, url);
	        exit(1);
	    }
	    LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "HSET succ:(%s : %s)", file_id, url);
	    
	    
	    //HASH_OF_FILE_ID_AND_USER
	    char *FILEID_USR_HASH = "FILEID_USR_HASH";
	    char *buf_user = getenv("HOME");
	    char *user_name = buf_user + 6;
	    ret = rop_set_hash(redis_conn, "HSET", FILEID_USR_HASH, file_id, user_name);
	    if (ret == -1) {
	        LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "HSET error:(%s : %s)", file_id, user_name);
	        exit(1);
	    }
	    LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "HSET succ:(%s : %s)", file_id, user_name);
	    
	    
	    
	    /*
	    ret = rop_set_string(redis_conn, file_name, buf);
	    if (ret == -1) {
	        LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "set %s  error",file_name);
	        exit(1);
	    }
	    
	    
	    ret = rop_get_string(redis_conn, file_name, buf);
	    if (ret == -1) {
	        LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "get %s  error",file_name);
	        exit(1);
	    }
	    
	    
	    LOG(FCGI_TEST_MODULE, FCGI_TEST_PROC, "get %s succ. is %s", file_name, buf);
	    */
	
	    rop_disconnect(redis_conn);
	    

		PrintEnv("Request environment", environ);
		PrintEnv("Initial environment", initialEnv);
	} /* while */

	return 0;
}
