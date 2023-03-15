################################################################################
# TFG
# author: Francis Guindon < FIXME : add email >
#
# This meson.build controls the construction and instalation of my TFG project.
# The TFG project can be seen as a collection of tools that can be used to
# accomplish the objectives laid out in my Project Proposal.
################################################################################
project('TFG','cpp', default_options : ['c_std=c11', 'cpp_std=c++11'])

################################################################################
# Tools
#
# These tools are meant to interactively pipe with each other or
# provide small libraries with useful APIs
################################################################################

# Switches
activate_tools = true         # Activate tools directory
activate_bash_tools = false   # Activate bash tools directory
activate_cpp_tools = true     # Activate cpp tools directory
activate_python_tools = false # Activate python tools directory

################################################################################
# Ranger
#
# Build ranger project
################################################################################

# Switches
activate_ranger = true

################################################################################
# Ranger Extension
#
# This extension creates a shared library that may be used as dependency.
# It opens up the ranger API to personal tools.
# It aditionally extends the ranger functionality for TFG purposes.
#
# The Ranger build must be activated to use this option.
################################################################################

# Switches
activate_ranger_ext = true

################################################################################
# Switch Cases
#
# Some of the switches defined in this file are used here
################################################################################

# Ranger switch case
if activate_ranger
  ranger = subproject('ranger')
endif

# Ranger ext switch case
if activate_ranger and activate_ranger_ext
  ext_incdirs = ranger.get_variable('ext_incdirs')
  ext_deps = ranger.get_variable('ext_deps')
  ext_sources = ranger.get_variable('ext_sources')
endif

# Tools switch case
if activate_tools
  subdir('tools')
endif
