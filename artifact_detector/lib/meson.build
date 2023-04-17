lib_src = [
  'feature_extractor.cpp',
  'videofilesink.cpp',
  'videofilesource.cpp',
  'trainer.cpp',
]

libartifactdetector = shared_library('artifact_detector',
  lib_src,
  cpp_args : cpp_args,
  version : project_version,
  include_directories : [projectinc],
  install : true,
  dependencies : [project_deps]
)

libartifactdetector_dep = declare_dependency(link_with: libartifactdetector)
