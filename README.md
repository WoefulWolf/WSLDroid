# Android ( Waydroid ) in WSL 2
> Ide awal proyek ini berdasarkan dari [gits](https://gist.github.com/onomatopellan/c5220c0efddaff69aaff77cca80b7b8e) komuntas

## Konfigurasi Kernel WSL

Bagian ini menjelaskan konfigurasi tambahan pada kernel WSL yang diperlukan agar Waydroid dapat berjalan dengan baik.

Tambahkan parameter konfigurasi berikut pada kernel WSL:

```config
CONFIG_ANDROID_BINDER_IPC=y
CONFIG_ANDROID_BINDER_DEVICES="binder,hwbinder,vndbinder"
```

Keterangan singkat:

- `CONFIG_ANDROID_BINDER_IPC=y` mengaktifkan dukungan Android Binder IPC pada kernel.
- `CONFIG_ANDROID_BINDER_DEVICES="binder,hwbinder,vndbinder"` mendefinisikan perangkat Binder yang dibutuhkan oleh komponen Android.


# WSL CONFIG

Bagian ini menjelaskan cara menggunakan **custom kernel** untuk WSL 2 agar kompatibel dengan kebutuhan Waydroid.

Tambahkan konfigurasi berikut ke file `%UserProfile%\\.wslconfig`:

```ini
[wsl2]
kernel=D:\\mykernel\\bzImage-6.6.87.ANDROID
kernelModules=D:\\mykernel\\modules.vhdx
```

> [!WARNING]
> Pengaturan `kernel` dan `kernelModules` pada `.wslconfig` akan **menggantikan kernel WSL default** untuk seluruh distro WSL pada mesin ini.
> Pastikan path file valid dan kernel yang digunakan memang sudah dikonfigurasi dengan dukungan yang dibutuhkan (misalnya Binder IPC) sebelum menerapkannya.

Setelah mengubah `.wslconfig`, jalankan perintah berikut di PowerShell agar konfigurasi dimuat ulang:

```powershell
wsl --shutdown
```


# Related Project

- [AUR (Arch) linux-wsl2-waydroid](https://aur.archlinux.org/packages/linux-wsl2-waydroid)
- [Automated Build Scripts](https://gist.github.com/PEMessage/a527ac647ea44514dbe3fbcbe433c3b3)