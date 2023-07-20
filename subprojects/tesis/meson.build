project('thesis')

thesis_builder = files('build_thesis.sh')

source_files = files([
  'solucion.tex',
  'main.tex',
  'intro.tex',
  'teoria.tex',
  'titlepage_licel_en.tex',
  'notation.tex',
  'resultados.tex',
  'resumen.tex',
  'glosario.tex',
  'literatura.bib',
  'licencia.tex',
  'macros.tex',
  'apendice.tex',
  'config.tex',
  'conclusiones.tex',
  'agradecimientos.tex',
  'code_blocks.tex',
  'dedicatoria.tex',
  'disclaimer.tex',
  'titlepage_licel_es.tex',
  'acta_aprob_licel.tex',
  'acta_aprob_mscel.tex',
  'acta_eval_licel.tex',
  'debug.tex',
  'titlepage_msc_en.tex',
  'titlepage_msc_es.tex',
  'acta_eval_mscel.tex',
  'titlepage_licce_en.tex',
  'titlepage_licce_es.tex',
])

subdir('fig')

custom_target('thesis',
              input: source_files,
              output: 'main.pdf',
              command: [thesis_builder,
                '@SOURCE_ROOT@/subprojects/tesis',
                '@BUILD_ROOT@/subprojects/tesis'],
              build_always: true)
