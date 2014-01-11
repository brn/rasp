{
  'includes': ['commons.gypi'],
  'make_global_settings': [
    ['CXX','/usr/bin/clang++'],
    ['LINK','/usr/bin/clang++'],
  ],
  'target_defaults': {
    'msvs_settings': {
      'VCCLCompilerTool': {
        'WarningLevel': '4', # /W4
      },
    },
    'xcode_settings': {
      'GCC_VERSION': 'com.apple.compilers.llvm.clang.1_0',
      'CLANG_CXX_LANGUAGE_STANDARD': 'c++0x',
      'MACOSX_DEPLOYMENT_TARGET': '10.8', # OS X Deployment Target: 10.8
      'CLANG_CXX_LIBRARY': 'libc++', # libc++ requires OS X 10.7 or later
    },
  },
  'targets': [
    #{
    #  'target_name': 'rasp',
    #  'product_name': 'rasp',
    #  'type': 'executable',
    #  'sources': [
    #    '../src/parser/scanner.cc',
    #  ],
    #  'xcode_settings': {
    #    'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES', # '-fvisibility-inlines-hidden'
    #    'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # '-fvisibility=hidden'
    #  },
    #},
    {
      'target_name': 'scanner_test',
      'product_name': 'ScannerTest',
      'type': 'executable',
      'include_dirs' : ['./lib', '<(additional_include)'],
      'defines' : ['GTEST_HAS_RTTI=0', 'UNIT_TEST=1'],
      'sources': [
        './src/utils/os.h',
        './src/utils/os.cc',
        './src/parser/token.h',
        './src/parser/token.cc',
        './src/parser/scanner.h',
        './src/parser/scanner.cc',
        './src/parser/source.h',
        './src/parser/source.cc',
        './lib/gtest/gtest.h',
        './lib/gtest/gtest-all.cc',
        './test/parser/scanner-test-prelude.h',
        './test/parser/scanner-keyword-scan-test.cc',
        './test/parser/scanner-operator-scan-test.cc',
        './test/parser/scanner-test.cc',
        './test/test-main.cc',
      ],
      'xcode_settings': {
      },
    },
    {
      'target_name': 'unicode_iterator_adapter_test',
      'product_name': 'UnicodeIteratorAdapterTest',
      'type': 'executable',
      'include_dirs' : ['./lib', '<(additional_include)'],
      'defines' : ['GTEST_HAS_RTTI=0', 'UNIT_TEST=1'],
      'sources': [
        './src/utils/os.h',
        './src/utils/os.cc',
        './src/parser/unicode-iterator-adapter.h',
        './src/parser/unicode-iterator-adapter-inl.h',
        './src/parser/uchar.h',
        './lib/gtest/gtest.h',
        './lib/gtest/gtest-all.cc',
        './test/parser/unicode-iterator-adapter-test.cc',
        './test/test-main.cc',
      ],
      'xcode_settings': {
      },
    },
  ] # targets
}
