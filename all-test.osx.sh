type=${1:-Debug}
sh build.osx.sh ${type} &&\
build/${type}/SourceStreamTest --gtest_color=auto&&\
build/${type}/UnicodeIteratorAdapterTest --gtest_color=auto &&\
build/${type}/ScannerTest --gtest_color=auto &&\
build/${type}/RegionsTest --gtest_color=auto
