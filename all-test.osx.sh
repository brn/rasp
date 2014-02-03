type=${1:-Debug}
sh osx_build.sh ${type} &&\
build/${type}/SourceStreamTest --gtest_color=auto&&\
build/${type}/UnicodeIteratorAdapterTest --gtest_color=auto &&\
build/${type}/ScannerTest --gtest_color=auto &&\
build/${type}/MemoryPoolTest --gtest_color=auto
