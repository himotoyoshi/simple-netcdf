require "yaml"

class NCFileWriter
  
  class Dim

    include NC
    
    def initialize (ncfile, name, len, define_mode = true)
      @ncfile     = ncfile
      @file_id    = ncfile.file_id
      @name       = name
      @len        = len
      if define_mode
        @dim_id   = nc_def_dim(@file_id, @name, @len)
      else
        @dim_id   = nc_inq_dimid(@file_id, @name)        
      end
    end
    
    attr_reader :name, :len
    
    def to_i
      return @dim_id
    end
    
  end
  
end


class NCFileWriter
    
  class Var

    include NC

    TYPEMAP = {
      "char"   => NC_CHAR,
      "byte"   => NC_BYTE,
      "short"  => NC_SHORT,
      "int"    => NC_INT,
      "float"  => NC_FLOAT,
      "double" => NC_DOUBLE,
    }

    def initialize (ncfile, name, definition, compression = 0, define_mode = true)
      @ncfile  = ncfile
      @file_id = ncfile.file_id
      @name    = name
      @type    = definition["type"] || NC_FLOAT
      if @type.is_a?(String)
        if TYPEMAP.include?(@type)
          @type = TYPEMAP[@type]
        else
          raise "invalid variable data type"
        end
      end
      @dims    = definition["dim"]
      if @dims
        @shape   = @dims.map{|key| @ncfile.dim(key).len }
        dim_ids  = @dims.map{|key| @ncfile.dim(key).to_i }
      else
        dim_ids  = []
      end
      if define_mode
        @var_id  = nc_def_var(@file_id, @name, @type, dim_ids)
        if compression != 0
          nc_def_var_deflate(@file_id, @var_id, 1, 1, compression)
        end
      else
        @var_id  = nc_inq_varid(@file_id, @name)        
      end
      @attributes = definition.dup
      @attributes.delete("type")
      @attributes.delete("dim")
      if define_mode
        @attributes.each do |name, value|
          value = NCFileWriter.convert_attribute_value(value)
          begin
            nc_put_att(@file_id, @var_id, name, value)
          rescue => e
            raise(e.class, e.message + " {#{name}: #{value}}")
          end
        end
      end
    end
    
    attr_reader :name, :type, :attributes, :shape
    
    def to_i
      return @var_id
    end

    def template (*argv, &block)
      obj = CArray.new(NC.ca_type(@type), @shape)
      unless argv.empty?
        obj = obj.template(*argv, &block)
      end
      obj.attribute.update(@attributes.clone)
      return obj
    end

    def update_attribute (key, value)
      value0 = @attributes[key]
      if value0 and value0 == value
        return
      end
      @ncfile.__transaction__ do
        value = NCFileWriter.convert_attribute_value(value)
        @attributes[key] = value
        begin
          nc_put_att(@file_id, @var_id, key, value)
        rescue => e
          raise(e.class, e.message + " {#{key}: #{value}}")
        end
      end
    end

    def []= (*argv)
      put(*argv)      
    end

    def put (*argv)
      value = argv.pop
      if value.has_attribute?
        value.attribute.each do |k, v|
          k = k.to_s
          update_attribute(k, v)
        end
      end
      info = CArray.scan_index(@shape, argv)
      case info.type
      when CA_REG_ADDRESS
        addr  = info.index[0]
        index = []
        (0..@shape.size-1).reverse_each do |i|
          index[i] = addr % @shape[i]
          addr /= @shape[i]
        end
        put_var1(index, value)
      when CA_REG_FLATTEN
        put_var(value)
      when CA_REG_POINT
        put_var1(info.index, value)
      when CA_REG_ALL
        if @shape.size == value.rank
          count = value.dim
          start = count.map {0}
          put_vara(start, count, value)
        else
          put_var(value)          
        end
      when CA_REG_BLOCK
        start = []
        count = []
        stride = []
        info.index.each do |idx|
          case idx
          when Array
            start << idx[0]
            count << idx[1]
            stride << idx[2]
          else
            start << idx
            count << 1
            stride << 1
          end
        end
        if stride.all?{|x| x == 1 }
          put_vara(start, count, value)
        else
          put_vars(start, count, stride, value)
        end
      else
        raise "invalid index"
      end
    end

    def put_var1 (index, value)
      return nc_put_var1(@file_id, @var_id, index, value)
    end

    def put_var (value)
      return nc_put_var(@file_id, @var_id, value)
    end

    def put_vara (start, count, value)
      return nc_put_vara(@file_id, @var_id, start, count, value)
    end

    def put_vars (start, count, stride, value)
      return nc_put_vars(@file_id, @var_id, start, count, stride, value)
    end

    def get_varm (start, count, stride, imap, value)
      return nc_put_varm(@file_id, @var_id, start, count, stride, imap, value)
    end

  end
  
end


class NCFileWriter
  
  include NC
  
  TYPEMAP = {
    "char"   => NC_CHAR,
    "byte"   => NC_BYTE,
    "short"  => NC_SHORT,
    "int"    => NC_INT,
    "float"  => NC_FLOAT,
    "double" => NC_DOUBLE,
  }
  
  def self.create (file, mode = 0, cdl: nil, compression: 0)
    return new(file, mode, cdl: cdl, compression: compression, create: true)
  end

  def self.open (file, mode = 0, compression: 0)
    return new(file, mode, cdl: nil, compression: compression, create: false)
  end
  
  def initialize (file, mode = 0, cdl: nil, compression: 0, create: true)
    # file info
    @compression = compression

    # dimensions
    @dims     = []
    @name2dim = {}

    # variables
    @vars     = []
    @name2var = {}

    # attributes
    @attributes  = {}

    # define_modes
    @enddef      = false 

    if cdl
      IO.popen("ncgen -o #{file}", "w") { |io|
        io.write(cdl)
      }
      create = false
    end

    # opening netcdf file
    if create
      if compression > 0
        @file_id  = nc_create(file, mode|NC_NETCDF4)
      else
        @file_id  = nc_create(file, mode)
      end
    else
      if compression > 0
        @file_id  = nc_open(file, mode|NC_WRITE|NC_SHARE|NC_NETCDF4)
      else
        @file_id  = nc_open(file, mode|NC_WRITE|NC_SHARE)
      end
      NCFile.open(file) {|nc|
        define_with_hash nc.definition, define_mode: false
      }
      @enddef = true
    end
  end
  
  attr_reader :file_id
  
  # key of Hash in definition should be String 
  def normalize_definition (definition)
    out = {}
    definition.each do |key, value|
      if value.is_a?(Hash)
        value = normalize_definition(value)
      end
      out[key.to_s] = value
    end
    return out
  end

  def check_definition (definition)

    if definition.has_key?("variables")

      if definition.has_key?("dimensions")
        dim_keys = definition["dimensions"].keys
      else
        dim_keys = []
      end
      dim_keys += @name2dim.keys

      definition["variables"].each do |name, var_def|
        if name =~ /\A(#{PATTERN_TYPES})\s+(.+?)(?:\s*\((.*)\))?\z/
          var_dim = $3.split(/\s*,\s*/)
          var_dim.each do |dim|
            unless dim_keys.include?(dim)
              raise "invalid dimension '#{dim}' for NetCDF Variable '#{name}'"
            end
          end
          next
        end
        unless var_def.has_key?("type")
          warn "NCFileWriter: Variable '#{name}' does not have 'type' entry in definition"
        else
          unless ["char","byte","short","int","float","double"].include?(var_def["type"])
            raise "invalid type of NetCDF Variable '#{name}'"
          end          
        end 
        if var_def.has_key?("dim") 
          unless var_def["dim"].is_a?(Array)
            raise "'dim' of NetCDF Variable '#{name}' should be an Array"
          else
            var_def["dim"].each do |dim|
              unless dim_keys.include?(dim)
                raise "invalid dimension '#{dim}' for NetCDF Variable '#{name}'"
              end
            end
          end
        end 
        var_def.each do |k, v|
          case v
          when Hash
            if v.size > 1 or not ["byte","short","int","float","double"].include?(v.keys[0])
              raise "invalid specification in attribute '#{k}' for NetCDF Variable '#{name}'"
            end
          end
        end
      end
    end

  end

  def self.convert_attribute_value (value)
    case value
    when String
      value = value.strip
      if value =~ /\A([\+\-]?\d{1,3})b\z/i
        value = CA_INT8($1.to_i)
      elsif value =~ /\A([\+\-]?(0x?)?\d{1,5})s\z/i
        value = CA_SHORT(eval($1))
      elsif value =~ /\A([\+\-]?(0x?)?\d+)l?\z/i
        value = CA_INT(eval($1))
      elsif value =~ /\A([\+\-]?\d*(?:\.\d*)?(?:e[\+\-]?\d+)?)f\z/i
        value = CA_FLOAT($1.to_f)
      elsif value =~ /\A([\+\-]?\d*(?:\.\d*)?(?:e[\+\-]?\d+)?)d?\z/i
        value = CA_DOUBLE($1.to_f)
      end
    when Hash
      type, value = *value.first
      case type.intern
      when :char
        value = CA_UINT8(value)
      when :byte
        value = CA_INT8(value)
      when :short
        value = CA_SHORT(value)
      when :int
        value = CA_INT(value)
      when :float
        value = CA_FLOAT(value)
      when :double
        value = CA_DOUBLE(value)
      end
    end
    return value
  end

  def __transaction__ (define_mode: true)
    if define_mode
      begin
        if @enddef
          nc_redef(@file_id)
        end
        yield
      ensure
        nc_enddef(@file_id)
        @enddef = true
      end
    else
      yield
    end
  end

  PATTERN_TYPES='(?:char|byte|short|int|float|double)'

  def define (yaml: )
    define_with_hash(YAML.load(yaml))
  end
  
  def define_with_hash (definition, define_mode: true)
    __transaction__(define_mode: define_mode) do
      definition = normalize_definition(definition)
      check_definition(definition)
      if definition.has_key?("dimensions") and definition["dimensions"].is_a?(Hash)
        definition["dimensions"].each do |name, len|
          if len == "unlimited"
            len = 0
          end
          dim = Dim.new(self, name, len.to_i, define_mode)
          @dims.push dim
          @name2dim[name] = dim
        end
      end
      if definition.has_key?("variables") and definition["variables"].is_a?(Hash)
        definition["variables"].each do |name, info|
          if name =~ /\A(#{PATTERN_TYPES})\s+(.+?)(?:\s*\((.*)\))?\z/
            if info.nil?
              info = {}
            end
            info["type"] = $1
            name = $2
            if $3
              info["dim"] = $3.split(/\s*,\s*/)
            end
          end
          var = Var.new(self, name, info, @compression, define_mode)
          @vars.push var
          @name2var[name] = var
        end
      end
      if definition.has_key?("attributes") and definition["attributes"].is_a?(Hash)
        if define_mode
          definition["attributes"].each do |name, value|
            value = NCFileWriter.convert_attribute_value(value)
            @attributes[name] = value
            begin
              nc_put_att(@file_id, NC_GLOBAL, name, value)
            rescue => e
              raise(e.class, e.error_message + " {#{name}: #{value}}")
            end
          end
        end
      end
    end
  end
  
  def dim (name)
    return @name2dim[name]
  end

  def new_dimension (name, len)
    __transaction__ do
      name = name.to_s
      if len == "unlimited"
        len = 0
      end
      dim = Dim.new(self, name, len.to_i)
      @dims.push dim
      @name2dim[name] = dim
    end
  end

  def new_variable (name)
    __transaction__ do
      if name =~ /\A(#{PATTERN_TYPES})\s+(.+?)(?:\s*\((.*)\))?\z/
        info = {}
        info["type"] = $1
        name = $2
        if $3
          info["dim"] = $3.split(/\s*,\s*/)
        end
        var = Var.new(self, name, info, @compression)
        @vars.push var
        @name2var[name] = var
      else
        return
      end
    end
  end

  def new_attribute (name, value)
    value = NCFileWriter.convert_attribute_value(value)
    case name
    when /:/
      name, attrname = *name.split(/:/)
      return @name2var[name].update_attribute(attrname, value)
    else
      if name =~ /\A:/
        name = name[1..-1]
      end
      @attributes[name] = value
      __transaction__ do
        begin
          nc_put_att(@file_id, NC_GLOBAL, name, value)
        rescue => e
          raise(e.class, e.error_message + " {#{name}: #{value}}")
        end
      end
      return value
    end
  end

  def [] (name)
    case name
    when /\A:/
      name = name[1..-1]
      return @attributes[name]
    when /:/
      name, attrname = *name.split(/:/)
      return @name2var[name].attributes[attrname]
    else
      return @name2var[name]
    end
  end

  def []= (name, value)
    case name
    when /:/
      return new_attribute(name, value)
    else
      return @name2var[name].put(value)      
    end
  end
  
  def close
    nc_close(@file_id)
  end

end

class CArray
  
  def to_scaled_integer (type)
    min, max = self.min, self.max
    case type
    when :byte
      byte_max = (0x7f).to_f
      fillvalue = CA_UINT8(0x80)[0]
      scale_factor = (max - min)/byte_max
      add_offset = min # + short_max*scale_factor
      packed = ((self - add_offset)/scale_factor).uint8
    when :short
      short_max = (0x7fff).to_f
      fillvalue = CA_SHORT(0x8000)[0]
      scale_factor = (max - min)/short_max
      add_offset = min # + short_max*scale_factor
      packed = ((self - add_offset)/scale_factor).short
    when :int
      int_max = (0x7fffffff).to_f
      fillvalue = CA_INT(0x80000000)[0]
      scale_factor = (max - min)/int_max
      add_offset = min # + int_max*scale_factor
      packed = ((self - add_offset)/scale_factor).int
    else
      raise
    end
    packed.attribute["scale_factor"] = scale_factor
    packed.attribute["add_offset"] = add_offset
    if self.has_mask?
      packed[:is_masked] = fillvalue
      packed.attribute["_FillValue"] = {type => fillvalue}
    end
    return packed
  end
  
end

