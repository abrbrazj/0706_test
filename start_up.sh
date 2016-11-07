sudo /usr/bin/fdfs_trackerd ./conf/tracker.conf
sudo /usr/bin/fdfs_storaged ./conf/storage.conf

redis-server 

spawn-fcgi -a 127.0.0.1 -p 8082 -f ./upload