#! /bin/sh

for build_mode in Debug Release
do
    for cxx_compiler in clang++ g++
    do
        if [ $cxx_compiler = clang++ ]; then c_compiler="clang"; else c_compiler="gcc"; fi
        build_dir="build_tests_${c_compiler}_${build_mode}"
        mkdir -p $build_dir && cd $build_dir || return 1
        cmake -D CMAKE_BUILD_TYPE=$build_mode -DCMAKE_C_COMPILER=$c_compiler -DCMAKE_CXX_COMPILER=$cxx_compiler -S .. -B . \
            && make all --jobs "$(nproc)" \
            && make test
        cd ..
    done
done
