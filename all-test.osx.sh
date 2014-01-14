type=Debug
sh osx_build.sh ${type} &&\
build/${type}/SourceStreamTest &&\
build/${type}/UnicodeIteratorAdapterTest &&\
build/${type}/ScannerTest
