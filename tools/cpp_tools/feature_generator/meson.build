gstdeps = [dependency('gstreamer-1.0'), dependency('gstreamer-app-1.0')]

executable('feature_generator',
           sources:
          ['feature_generator.cpp','pipeline.cpp','buffer_utils.cpp'],
          cpp_args: ['-mavx'],
          include_directories : [projectinc],
          dependencies : [gstdeps, libartifactdetector_dep])
