Path2Cookies = /dev/shm/Kookies
Path2semaphore = /dev/shm/*m_semaphore*
#RH
#ConfPATH = /etc/httpd/conf.d/ 
#LFS
ConfPATH = /usr/etc/extra/
#RH
#ConfPATHApache = /etc/httpd/conf.d/ 
#LFS
ConfPATHApache = /usr/etc/

Path2APXS = /usr/sbin/apxs

all:
	echo "please use install or uninstall (Please use functions uninstall very cautiously) "
	echo "Before use stop the Web-server and check up all paths, after installation start it"

clean:
	rm -f -r .libs
	rm -f *.la
	rm -f *.lo
	rm -f *.o
	rm -f *.slo
	rm -f *~

uninstall: clean
	echo "delete configuration"
	rm -f $(ConfPATH)mod_prx.conf
	echo "please delete  'LOADMODULE mod_prx' from http.conf"
	echo "please delete  'mod_prx.so' from /usr/lib/modules"
	echo "I try delete module !!!!!"
	rm -f /etc/httpd/modules/mod_prx.so
	mv -f $(ConfPATHApache)httpd.conf $(ConfPATHApache)httpd.conf.del
	cat $(ConfPATHApache)httpd.conf.del|sed -e '/^LoadModule prx_module/d' > $(ConfPATHApache)httpd.conf

install: clean
	rm -f $(Path2Cookies)
	rm -f $(Path2semaphore)
	echo "compile module"
	/usr/sbin/apxs -i -a -c mod_prx.c || echo "Error! Please check all params and Path's!"
	echo "update configuration"
	cp -f mod_prx.conf $(ConfPATH)mod_prx.conf
	res=`ps aux|grep httpd |awk "{i++};END{print i-1 }"` && if [ $$res -ge  1 ]; then echo "reload please"; fi


check:
	echo "check compile module"
	g++ *.c -Wall -pedantic  -Wextra -Wshadow -Werror -I/usr/include/httpd/ -I/usr/include/apr-0/ -c
	g++ -shared  -Wall -fPIC `/usr/sbin/apxs -q CFLAGS`    `/usr/sbin/apxs -q CFLAGS_SHLIB`     -o mod_prx.so *.o

