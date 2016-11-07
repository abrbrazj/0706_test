
CC=gcc 
CPPFLAGS= -I./include/  -I/usr/local/include/hiredis/ -I/usr/local/include/ 
CFLAGS=-Wall -g
LIBS= -lhiredis -lfcgi -lm

#找到当前目录下所有的.c文件
src = $(wildcard *.c)

#将当前目录下所有的.c  转换成.o给obj
obj = $(patsubst %.c, %.o, $(src))


fdfs_test = ./test/fdfs_test
main = main_test
hiredis_test = ./test/hiredis_test
test_fdfs_client = ./test/fdfs_client_test
upload=upload
cJSON_test = ./test/cJSON_test
data=data

target= $(main) $(hiredis_test) $(upload) $(test_fdfs_client) $(cJSON_test) $(data)


ALL:$(target)


#生成所有的.o文件
$(obj):%.o:%.c
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) 


#fdfs_test程序
#$(fdfs_test):./test/fdfs_test.o  make_log.o cJSON.o util_cgi.o config.o
#	$(CC) $^ -o $@ $(LIBS)

#main程序
$(main):main.o make_log.o cJSON.o config.o util_cgi.o cJSON.o
	$(CC) $^ -o $@ $(LIBS)

#hiredis_test程序
$(hiredis_test):./test/hiredis_test.o make_log.o redis_op.o cJSON.o util_cgi.o config.o
	$(CC) $^ -o $@ $(LIBS)

#upload程序
$(upload):upload.o make_log.o redis_op.o cJSON.o util_cgi.o config.o
	$(CC) $^ -o $@ $(LIBS)
	
#fdfs_client_test程序
$(test_fdfs_client):./test/fdfs_client_test.o  make_log.o cJSON.o util_cgi.o config.o
	$(CC) $^ -o $@ $(LIBS)
	
#cJSON_test程序
$(cJSON_test):./test/cJSON_test.o cJSON.o util_cgi.o config.o
	$(CC) $^ -o $@ $(LIBS) 
	
#data程序
$(data):data.o make_log.o redis_op.o cJSON.o util_cgi.o config.o
	$(CC) $^ -o $@ $(LIBS)

#clean指令

clean:
	-rm -rf $(obj) $(target) ./test/*.o

distclean:
	-rm -rf $(obj) $(target) ./test/*.o

#将clean目标 改成一个虚拟符号
.PHONY: clean ALL distclean
