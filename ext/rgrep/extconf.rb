require 'mkmf-rice'
require 'pathname'

LIBRGREP_PATH = Pathname('./librgrep').expand_path.to_s

$CFLAGS << ' -O3 -std=gnu++14'
$LDFLAGS << " -Wl,-rpath,#{LIBRGREP_PATH}"

Dir.chdir('librgrep') do
  system 'scons'
end

dir_config('rgrep', [LIBRGREP_PATH], [LIBRGREP_PATH])
have_library('rgrep')

create_makefile('rgrep_ext')
