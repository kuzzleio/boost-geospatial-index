{
  "targets": [
    {
      "target_name": "BoostSpatialIndex",
      "sources": [
        "src/shape.cc",
        "src/spatialIndex.cc"
      ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include_dir\")",
        "include"
      ],
      "defines": [
        "NAPI_VERSION=8",
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      'cflags': [ '-Wno-misleading-indentation' ],
      # The C++ standard is pinned, not inherited from the Node headers
      # (gnu++17 up to Node 22, gnu++20 from Node 24): boost::geometry's R*
      # tree does not return the same results under both, so an unpinned
      # build behaves differently depending on the Node version that
      # compiled it. gnu++17 is what 1.4.0 compiled to on Node 20 and 22.
      'cflags_cc!': [ '-fno-exceptions', '-std=gnu++20' ],
      'cflags_cc': [ '-std=gnu++17' ],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'CLANG_CXX_LANGUAGE_STANDARD': 'gnu++17'
          }
        }]
      ]
    }
  ]
}
