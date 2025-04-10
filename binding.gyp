{
  "targets": [
    {
      "target_name": "ccore",
      "sources": [ "ccore/ccore.c", "ccore/cell.c", "ccore/circalloc.c" ],
      "cflags": [
        "-fno-exceptions"
      ],
      "cflags!": [
        "-fno-exceptions"
      ],
      "cflags_cc!": [
        "-fno-exceptions"
      ],
      "conditions": [
        ['OS=="mac"', {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CC": "/usr/bin/gcc",
            "CXX": "/usr/bin/g++"
          }
        }]
      ]
    }
  ]
}
