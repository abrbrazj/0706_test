sudo /usr/bin/fdfs_trackerd ./conf/tracker.conf
sudo /usr/bin/fdfs_storaged ./conf/storage.conf

redis-server /./conf/redis.conf

spawn-fcgi -a 127.0.0.1 -p 8012 -f ./upload