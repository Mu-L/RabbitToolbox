if exist build (
  cd build
) else (
  mkdir build && cd build
)
conan remote add iteale http://conan.iteale.com:19479
conan install .. -g visual_studio_multi -s arch=x86_64 -s build_type=Debug
conan install .. -g visual_studio_multi -s arch=x86_64 -s build_type=Release
cmake .. -G "Visual Studio 16"
cmake --build . --config Release
cd ..