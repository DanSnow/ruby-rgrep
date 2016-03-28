require 'rgrep/rgrep_ext'

module RGrep
  class RGrep
    def initialize(query, filename, opts = {})
      options = default_opts.merge(opts)
      @rgrep = Ext::RGrep.new(query, filename, options)
    end

    def run(&block)
      @rgrep.run(&block)
    end

    private

    def default_opts
      {
        insensitive: false,
        begin_pat: '@id:',
        cols: ['title', 'content']
      }
    end
  end
end
