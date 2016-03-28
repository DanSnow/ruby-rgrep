require 'mkmf-rice'
require 'pathname'


LIBRGREP_PATH = Pathname('../../../../librgrep/src/').expand_path.to_s

$CFLAGS << ' -O3 -std=gnu++14'
$LDFLAGS << " -Wl,-rpath,#{LIBRGREP_PATH}"

dir_config('rgrep', [LIBRGREP_PATH], [LIBRGREP_PATH])
have_library('rgrep')

create_makefile('rgrep_ext')
