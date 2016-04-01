require 'rgrep/rgrep_ext'

module RGrep
  class RGrep
    def initialize(query, filename, opts = {})
      options = default_opts.merge(opts)
      @rgrep = Ext::RGrep.new(query, filename, options)
    end

    def run(&block)
      if block_given?
        @rgrep.run(&block)
      else
        Enumerator.new do |y|
          @rgrep.run do |data|
            y << data
          end
        end
      end
    end

    private

    def default_opts
      {
        insensitive: false,
        begin_pat: '@id:',
        cols: %w(id title content),
        limit: -1
      }
    end
  end
end
