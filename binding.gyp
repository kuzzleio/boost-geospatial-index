{
  "targets": [
    {
      "target_name": "BoostSpatialIndex",
      "sources": [
        "src/shape.cc",
        "src/spatialIndex.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "include"
      ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }]
      ]
    }
  ]
}
