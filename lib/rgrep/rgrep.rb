require 'rgrep/rgrep_ext'

module RGrep
  class RGrep
    def initialize(query, filename, opts = {})
      @rgrep = Ext::RGrep.new(query, filename, opts)
    end
  end
end
