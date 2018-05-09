This application is to sync files across storages.
Currently it only support add/remove files (no updates)

Prefix and postfix is used to make sure the file is valid and writing is done (this is important for non-atomic operation like file system)
There's no update here, so postfix is very important to make sure the data is writing is complete.


For Mysql storage, there's no need because the operation is atomic.
Remember to make sure max_allowed_packet on mysql server is bigger than the max file/data size.
To set max_allowed_packet, go to mysql.cnf file (probably it's located in /etc/mysql/) and insert
```
max_allowed_packet=32M
```
create mysql table based on create_table.sql
