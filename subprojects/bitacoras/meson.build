project('worklogs')

thesis_builder = files('build_worklogs.sh')

targets = [
  'semana_1',
  'semana_2',
  'semana_3',
  'semana_4',
  'semana_5',
  'semana_6',
  'semana_7',
  'semana_8',
  'semana_9',
  'semana_10',
  'semana_11',
  'semana_12',
  'semana_13',
]

figs = files()
subdir('fig')

foreach target : targets
  source_file = files(target + '.tex')
  output_file = target + '.pdf'
  custom_target(target,
                input: [source_file, figs],
                output: output_file,
                command: [thesis_builder,
                  '@SOURCE_ROOT@/subprojects/bitacoras',
                  '@BUILD_ROOT@/subprojects/bitacoras',
                  target],
                build_always: true)
endforeach
