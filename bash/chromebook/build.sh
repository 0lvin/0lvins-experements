make CC="ccache gcc" bob_defconfig || exit
make CC="ccache gcc" -j 2 Image.lzma || exit
make CC="ccache gcc" -j 2 || exit
sleep 5 && sync
sudo make CC="ccache gcc" modules_install || exit
sleep 5 && sync
# make bindeb-pkg || exit
