# Xclipse Vulkan Enhanced Driver (XV-Driver)

Driver Vulkan customizado para GPUs **Samsung Xclipse (Exynos 2400+)**

## Features

✔ Suporte a extensões Vulkan expandido  
✔ Sistema de perfis por aplicativo  
✔ Fallback OpenGL→Vulkan (via Zink, futuro)  
✔ Workarounds para bugs de drivers upstream  
✔ Sistema de logging e HUD

## Build

### Android
```bash
export ANDROID_NDK_HOME=/path/to/android/ndk
bash tools/build_android.sh