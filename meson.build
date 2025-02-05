project('tinyjson', 'cpp', version : '0.0', default_options : ['warning_level=3', 'cpp_std=c++23'])
add_project_arguments('-Wfatal-errors', language: 'cpp')

subdir('src')

executable('example', tinyjson_files + tinyjson_debug_files)
