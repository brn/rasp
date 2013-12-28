{
  'includes': ['common.gypi'],
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
    {
      'target_name': 'rasp',
      'product_name': 'rasp',
      'type': 'executable',
      'include_dirs': [
        '../include',
      ],
      'sources': [
        '../src/parser/scanner.cc',
      ],
      'xcode_settings': {
        'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES', # '-fvisibility-inlines-hidden'
        'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # '-fvisibility=hidden'
      },
    },
    {
      'target_name': 'scanner_test',
      'product_name': 'ScannerTest',
      'type': 'executable',
      'include_dirs': [
        '../include',
      ],
      'sources': [
        '../test/scanner-test.cc',
      ],
      'xcode_settings': {
      },
    },
  ] # targets
}
