project(
  'pulse-fft', 'cpp',
  default_options : [
    'warning_level=3',
    'cpp_std=c++17',
  ],
)

pulse_dep = dependency('libpulse-simple')
fftw_dep = dependency('fftw3')

executable(
  'pulse-fft',
  'main.cpp',
  dependencies : [pulse_dep, fftw_dep]
)
