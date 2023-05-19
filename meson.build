################################################################################
# TFG
# author: Francis Guindon <fbadilla10@gmail.com>
#
# This meson.build controls the construction and instalation of my TFG project.
# The TFG project can be seen as a collection of tools that can be used to
# accomplish the objectives laid out in my Project Proposal.
################################################################################
project(
  'TFG', ['cpp'],
  version : '0.1.1',
  meson_version : '>= 0.58',
  default_options : ['werror=true',
                    'c_std=c14',
                    'cpp_std=c++14']
)

# Project name and version

project_name = meson.project_name()
project_version = meson.project_version()
version_arr = project_version.split('.')
version_major = version_arr[0].to_int()
version_minor = version_arr[1].to_int()
version_micro = version_arr[2].to_int()

# Compiler
cc = meson.get_compiler('cpp')
cpu_family = build_machine.cpu_family()
message(cpu_family)
assert(cpu_family == 'x86_64' or cpu_family == 'aarch64')

# Project dependencies
project_deps = []

gst_dep = dependency('gstreamer-1.0')
gstapp_dep = dependency('gstreamer-app-1.0')

project_deps += gst_dep
project_deps += gstapp_dep

projectinc = include_directories('.', 'artifact_detector', 'artifact_detector/include')

cpp_args = ['-DHAVE_CONFIG_H']

if cpu_family == 'x86_64'
  cpp_args += ['-mavx -O3']
elif cpu_family == 'aarch64'
  cpp_args += ['-march=armv8-a', '-mcpu=cortex-a57', '-O3']
  # cpp_args += ['-mfpu=neon -mcpu=cortex-a9 -O3']
  # cpp_args += ['-march=armv8-a-mcpu=cortex-a57 -O3']
  # cpp_args += ['-march=armv8-a-mcpu=cortex-a57 -O2 -ftree-vectorize']
  # cpp_args += ['-march=armv8-a-mcpu=cortex-a57 -O3 -ftree-vectorize']
endif

add_global_arguments(cpp_args, language : 'cpp')

developer_mode = get_option('developer-mode')
if developer_mode
  pre_commit = find_program('pre-commit', required : true, native : true)
  run_command(pre_commit, 'install', check : true)
  run_command(pre_commit, 'install', '--hook-type', 'commit-msg', check : true)
endif

if get_option('enable-tests').enabled()
  test_dep = dependency('cpputest', required: true)
endif

# Docs
if get_option('enable-docs').enabled()
  doxygen = find_program('doxygen', required: true)
  dot = find_program('dot', required : true)
  java = find_program('java', required : true)
  epstopdf = find_program('epstopdf', required : true)
  plantumlurl = 'https://github.com/plantuml/plantuml/releases/download/v1.2023.5/plantuml-1.2023.5.jar'
  plantumlpath = meson.current_build_dir() + '/plantuml.jar'
  run_command('wget', '-O', plantumlpath, plantumlurl, check : true)
  env = environment()
  env.set('PLANTUML_JAR_PATH', plantumlpath)
endif

conf_data = configuration_data()

ranger = subproject('ranger')

ext_incdirs = ranger.get_variable('ext_incdirs')
ext_deps = ranger.get_variable('ext_deps')
ext_sources = ranger.get_variable('ext_sources')

subdir('artifact_detector')
subdir('tools')
subdir('docs')
