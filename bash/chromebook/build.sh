make CC="ccache gcc -march=native " defconfig || exit
sleep 5 && sync
make CC="ccache gcc -march=native " -j 2 || exit
sleep 5 && sync
sudo make CC="ccache gcc" modules_install || exit
sleep 5 && sync
# sudo make CC="ccache gcc" install || exit
# make CC="ccache gcc" bindeb-pkg || exit
