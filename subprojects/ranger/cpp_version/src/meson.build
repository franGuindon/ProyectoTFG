################################################################################
# Ranger library
#
# This library can create, train, and use to predict multithreaded RDFs
################################################################################

# General tools
cc = meson.get_compiler('cpp')

# Includes and dependencies
ranger_incdir = include_directories('.','Forest','Tree', 'utility')
ranger_deps = cc.find_library('pthread')

subdir('utility')
subdir('Forest')
subdir('Tree')

sources = []
sources += utility_sources
sources += forest_sources
sources += tree_sources

libranger = shared_library('ranger',
  sources,
  include_directories : ranger_incdir,
  dependencies : [ranger_deps]
)

ranger_dep = declare_dependency(link_with: libranger)

################################################################################
# Ranger tool
#
# This tool can train from a dat file. 
################################################################################

executable('ranger',
           sources             : ['main.cpp'],
           include_directories : ranger_incdir,
           dependencies        : [ranger_dep])

################################################################################
# Extension
#
# These variables are extracted in TFG root and used to build extended ranger
################################################################################

ext_incdirs = include_directories('extension')
ext_deps = ranger_deps

ext_sources = []
ext_sources += ext_utility_sources
ext_sources += ext_forest_sources
ext_sources += ext_tree_sources
