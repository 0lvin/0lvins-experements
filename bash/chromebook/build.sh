make CC="ccache gcc" defconfig || exit
make CC="ccache gcc" -j 2 || exit
sudo make CC="ccache gcc" modules_install || exit
# sudo make CC="ccache gcc" install || exit
# make CC="ccache gcc" bindeb-pkg || exit
