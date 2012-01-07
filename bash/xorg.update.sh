#!/bin/sh
#use: sh xorg.update.sh 'http://cgit.freedesktop.org/xorg/util/modular/plain/module-list.txt'
wget -c -v "$1" -O list.txt
cat list.txt | rev  | sed -e 's/-/ /' | rev | sort -u > xorg_list
rm list.txt
#edit all list
#echo "#!/bin/sh" > lvu_edit.sh
#cat xorg_list | awk '{print "lvu edit " $1}' >> lvu_edit.sh
#show all exist versions
echo "#!/bin/sh" > lvu_version.sh
cat xorg_list | awk '{print "echo -n \"" $1 " \" && lvu version " $1 "&& echo"}' >> lvu_version.sh
sh ./lvu_version.sh |sort -u > moonbase_list
rm lvu_version.sh
#show installed
echo "#!/bin/sh" > lvu_installed_version.sh
cat xorg_list | awk '{print "echo -n \"" $1 " \" && lvu installed " $1 "&& echo"}' >> lvu_installed_version.sh
sh ./lvu_installed_version.sh |sort -u |grep -v 'is not installed' > installed_list
rm lvu_installed_version.sh
