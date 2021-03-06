require 'open3'

GCC = "/usr/bin/gcc"
AVRGCC = "/usr/bin/avr-gcc"
GCCARGS = "-lm -o"

PATTERNDIR = Rails.root + "patterns"
PATTERNTEMPLATE = PATTERNDIR + "c_pattern_template.erb"

ERBITEMPAIR = /%=\s*(?:defined\?\()?(\w*)(?:\))?\s*\?.*:\s*(\S+)\s*%>/
ERBITEMNAME = /%=\s*(?:defined\?\()?(\w*)(?:\))?\s*\?.*:\s*\S+\s*%>/
ERBITEMDEFINED = /defined\?\((\w*)\)/
ERBITEMCALLED = /\#\{(\w*)\}/

class Component < ActiveRecord::Base
  
  #has_many :options
  has_many :sketches, :through => :options
  #has_many :patterns, :inverse_of => :component
  has_many :options, :inverse_of => :component

  # after_validation :build_pattern, if: :is_pattern?

  scope :patterns, lambda { where(:category => "pattern") }

  def is_pattern?
    category.match("^pattern$")
  end

  def test_pattern(opts = {}, num_steps = self.period)
    return if !is_pattern?

    # Set up default values for pattern, substituting optional supplied vals
    defaults = variables
    defaults.each do |i|
      max = i[:max]
      min = i[:min]
      item = i[:name]
      default_value = i[:default]
      if opts[item].nil? 
        opts[item] = default_value 
      elsif (opts[item] > max) || (opts[item] < min)
        opts[item] = default_value 
      end
    end
  

    # Replace given pattern name with generic name
    # Setup build dir
    new_pat = global.gsub(/int\s+\w+\s*\(int seq\)/, "int pattern(int seq)")
    pattern_build_dir = PATTERNDIR + name
    c_prog = Tempfile.new(name) 
    c_file = Tempfile.new([name, ".c"])
    if (!Dir.exists?(pattern_build_dir))
      Dir.mkdir pattern_build_dir
    end

    # Substitute ERB variables and write to file
    pattern_template = Erubis::Eruby.new(File.read(PATTERNTEMPLATE)).result(new_pat.send(:binding))
    c_pattern = Erubis::Eruby.new(pattern_template).result(opts)
    File.open c_file, "w" do |file|
      file << c_pattern
    end
    c_file.close
    c_prog.close

    # Compile first with avr-gcc to prevent non-AVR functions (setuid()...) from slipping past us. If that's fine, then compile with gcc for local exec. Probably some way around this..
    steps = Hash.new
    stdout, stderr, status = Open3.capture3("#{AVRGCC} #{c_file.path} #{GCCARGS} #{c_prog.path}")
    if status.success?
      stdout, stderr, status = Open3.capture3("#{GCC} #{c_file.path} #{GCCARGS} #{c_prog.path}")
      if status.success?
        stdout, stderr, status = Open3.capture3("#{c_prog.path} #{num_steps}")
        if status.success?
          steps = stdout
        else 
          steps['error'] = stderr.empty? ? "Unable to run your pattern: #{status}" : stderr
        end
      else
        steps['error'] = stderr.empty? ? "Unable to compile your pattern: #{status}" : stderr
      end
    else
      steps['error'] = stderr.empty? ? "Unable to verify your pattern: #{status}" : stderr
    end
    c_file.unlink
    c_prog.unlink
    steps.class == Hash ? steps : JSON.parse(steps)
  end

  # returns variable name and default value
  def variables
    opts_with_vals = Array.new
    # global portion of pattern is where the action is. no substitutions
    # should be happening in setup or loop.
    options = [global,setup,loop].join('\n').scan(ERBITEMPAIR)

    # Each 'o' is a [variable name, default value] array
    options.each do |o|
      v = Variable.where(:name => o[0]).first_or_create
      if o[0].match(/\d/)
        opts_with_vals.push({
          name: o[0], 
          default: o[1], 
          min: v.min,
          max: v.max,
          description: v.description
        })
      else 
        opts_with_vals.push({
          name: o[0], 
          default: o[1], 
          description: v.description
        })
      end
    end
    opts_with_vals.uniq
  end

  # Fetch all ERB variable substitutions, but ignore those also 
  # defined within the component itself
  #def variables
  #  joined = [global,setup,loop].join("\n")
  #  defined = joined.scan(ERBITEMNAME).flatten.uniq.reject { |i|
  #    !joined.scan(/<%\s*(#{i})\s*=/).empty?
  #  }
  #  called = joined.scan(ERBITEMCALLED).flatten.uniq.reject { |i|
  #    !joined.scan(/<%\s*(#{i})\s*=/).empty?
  #  }
  #  defined.push(called).flatten
  #end

  def variable_objs
    objs = Array.new
    variables.each do |v|
      objs.push(Variable.where("name = ?", v).first_or_create do |var|
        var.name = v
      end)
    end
    objs.empty? ? nil : objs
    #Variable.where.any_of(*names.each_with_index { |v,i|
    #  names[i] = {"name" => v}
    #})
  end

end
