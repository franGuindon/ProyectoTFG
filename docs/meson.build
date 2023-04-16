if get_option('enable-docs').enabled() and doxygen.found()

 #Configure env variables used in Doxyfile.in
 cdata = configuration_data()
 cdata.set('TOP_SRCDIR', meson.project_source_root())
 cdata.set('DESIGN_DOC_NAME', 'design.md')
 cdata.set('TOP_BUILDDIR', meson.project_build_root())
 cdata.set('PROJECT', meson.project_name())
 cdata.set('VERSION', meson.project_version())

 doxyfile = configure_file(input: 'Doxyfile.in',
                          output: 'Doxyfile',
                          configuration: cdata,
                          install: false)

 install_docs_dir = join_paths(get_option('datadir'), 'doc', 'TFG-@0@.0'.format(version_major))

 html_target = custom_target('docs',
                            input: doxyfile,
                            output: 'html',
                            command: [doxygen, doxyfile],
                            install: true,
                            install_dir: install_docs_dir)
endif
