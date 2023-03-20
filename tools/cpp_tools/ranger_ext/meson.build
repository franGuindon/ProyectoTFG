ranger_ext_incdirs = []
ranger_ext_incdirs += [ include_directories('.') ]
ranger_ext_incdirs += ext_incdirs

ranger_ext_deps = []
ranger_ext_deps += ext_deps

ranger_ext_headers = [
  'Data.h',
  'Forest.h',
  'ForestClassification.h',
  'ArgumentHandler.h']

ranger_ext_sources = [
  'Data.cpp',
  'Forest.cpp',
  'ForestClassification.cpp',
  'ArgumentHandler.cpp']
ranger_ext_sources += ext_sources

################################################################################
# Ranger Extension
#
# This extension creates a shared library that may be used as dependency.
# It opens up the ranger API to personal tools.
# It aditionally extends the ranger functionality for TFG purposes.
#
# The Ranger build must be activated to use this option.
################################################################################

librangerx = shared_library('rangerx',
  ranger_ext_sources,
  include_directories : ranger_ext_incdirs,
  dependencies : ranger_ext_deps
)

rangerx_dep = declare_dependency(link_with: librangerx)

################################################################################
# Ranger Extension Tool
#
# This tools allows the testing of the rangerx library
################################################################################

executable('rangerx',
          sources             : [ 'main.cpp' ],
          include_directories : ranger_ext_incdirs,
          dependencies        : rangerx_dep)
