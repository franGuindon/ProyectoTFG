feature_extractor_tool_src = [
  'feature_extractor_tool.cpp',
]

executable('feature_extractor_tool',
  feature_extractor_tool_src,
  cpp_args : cpp_args,
  include_directories : [projectinc],
  dependencies : [libartifactdetector_dep],
  install : false,
)

trainer_tool_src = [
  'trainer_tool.cpp',
]

executable('trainer_tool',
  trainer_tool_src,
  cpp_args : cpp_args,
  include_directories : [projectinc],
  dependencies : [libartifactdetector_dep],
  install : false,
)

