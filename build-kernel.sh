sudo apt install build-essential flex bison dwarves libssl-dev libelf-dev cpio qemu-utils wget
wget https://github.com/microsoft/WSL2-Linux-Kernel/archive/refs/tags/linux-msft-wsl-6.18.35.2.zip
unzip -x linux-msft-wsl-6.18.35.2.zip
cd linux-msft-wsl-6.18.35.2

make -j$(nproc) KCONFIG_CONFIG=../CONFIG_KERNEL_WSL_ANDROID && make INSTALL_MOD_PATH="$PWD/modules" modules_install

# Strips Modul

find ./modules/lib/modules/$(make -s kernelrelease) -name '*.ko' -exec strip --strip-unneeded {} \;

sudo ./Microsoft/scripts/gen_modules_vhdx.sh "$PWD/modules" $(make -s kernelrelease) modules.vhdx
mv arch/x86/boot/bzImage Andrioid.kernel