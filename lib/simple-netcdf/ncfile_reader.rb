class NCObject

  include NC

  def get_attributes (file_id, var_id)
    attrs = {}
    varnatts = nc_inq_varnatts(file_id, var_id)
    varnatts.times do |i|
      attname = nc_inq_attname(file_id, var_id, i)
      value   = nc_get_att(file_id, var_id, attname) 
      attrs[attname] = value
    end
    return attrs.freeze
  end

  def get_attributes! (file_id, var_id)
    attrs = {}
    varnatts = nc_inq_varnatts(file_id, var_id)
    varnatts.times do |i|
      attname = nc_inq_attname(file_id, var_id, i)
      type    = nc_inq_atttype(file_id, var_id, attname)
      case type
      when NC_CHAR
        value = nc_get_att(file_id, var_id, attname) 
      else
        len   = nc_inq_attlen(file_id, var_id, attname)
        value = CArray.new(NC.ca_type(type), [len])
        nc_get_att(file_id, var_id, attname, value)         
      end
      attrs[attname] = value
    end
    return attrs.freeze
  end
  
  def attribute (name)
    return @attributes[name]
  end

  attr_reader :attributes

end

class NCFile < NCObject

  class Var < NCObject
  
    include NC
  
    TYPENAME = {
      NC_CHAR   => "char",
      NC_BYTE   => "byte",
      NC_SHORT  => "short",
      NC_INT    => "int",
      NC_FLOAT  => "float",
      NC_DOUBLE => "double",
    }
  
    def initialize (ncfile, var_id)
      @ncfile     = ncfile
      @file_id    = ncfile.file_id
      @var_id     = var_id
      @name       = nc_inq_varname(@file_id, var_id)
      @vartype    = nc_inq_vartype(@file_id, var_id)
      @dims       = ncfile.dims.values_at(*nc_inq_vardimid(@file_id, var_id))
      @shape      = @dims.map{|d| d.len}
      @attributes = get_attributes(@file_id, var_id)
      @dims.freeze
      @shape.freeze
      @attributes.freeze
    end

    attr_reader :name, :dims, :attributes

    def declaration
      if not dims.empty?
        [TYPENAME[@vartype], name, format("(%s)", dims.map(&:name).join(", "))].join(" ")
      else
        [TYPENAME[@vartype], name].join(" ")        
      end
    end

    def definition
      atts = {}
      get_attributes!(@file_id, @var_id).each do |k,v|
        atts[k] = NCFile.convert_attribute_value(v)
      end
      return atts
    end

    def inspect
      return "#{@name}#{@dims}"
    end
  
    def is_dim? 
      begin
        nc_inq_dimlen(@file_id, @var_id)
        return true
      rescue RuntimeError
        return false
      end
    end

    def decode (value)
      if @attributes.has_key?("_FillValue")
        fill_value = @attributes["_FillValue"]
        case value
        when CArray
          value[:eq, fill_value] = UNDEF
        else
          value = UNDEF if value == fill_value
        end
      end
      if @attributes.has_key?("missing_value")
        missing_values = [@attributes["missing_value"]].flatten
        missing_values.each do |mv|
          case value
          when CArray
            value[:eq, mv] = UNDEF
          else
            if value == mv
              value = UNDEF 
              break
            end
          end
        end
      end
      if @attributes.has_key?("scale_factor")
        value *= @attributes["scale_factor"]
      end
      if @attributes.has_key?("add_offset")
        value += @attributes["add_offset"]
      end
      return value    
    end

    def to_ca
      return self[]
    end

    TIME_UNITS = {
      "day"     => 86400,
      "days"    => 86400,
      "d"       => 86400,
      "hour"    => 3600,
      "hours"   => 3600,
      "hr"      => 3600,
      "h"       => 3600,
      "minute"  => 60,
      "minutes" => 60,
      "min"     => 60,
      "second"  => 1,
      "seconds" => 1,
      "sec"     => 1,
      "s"       => 1,
    }  
  
    def to_time

      if not @attributes.has_key?("units")
        raise "variable #{@name} has no attribute 'units'"
      elsif @attributes["units"] =~ /\A(.+?)\s+since\s+(.+)\Z/
        interval = TIME_UNITS[$1]
        basetime = Time.parse($2)
        if interval
          return self[].convert(:object) {|x| basetime + interval*x }
        else
          raise "#{$1} is invalid for time units"        
        end
      else
        raise "format of #{@name}:units is not valid for time"
      end
    end

    def [] (*argv)
      return get!(*argv)
    end

    def get (*argv)
      if argv.size > 0 and argv[0].is_a?(Struct::CAIndexInfo)
        info = argv.shift
      else
        info = CArray.scan_index(@shape, argv)
      end
      out  = nil
      case info.type
      when CA_REG_ADDRESS
        addr  = info.index[0]
        index = []
        (0..@shape.size-1).reverse_each do |i|
          index[i] = addr % @shape[i]
          addr /= @shape[i]
        end
        out = get_var1(*index)
      when CA_REG_FLATTEN
        out = get_var[nil]
      when CA_REG_POINT
        out = get_var1(*info.index)
      when CA_REG_ALL
        out = get_var()
      when CA_REG_BLOCK
        use_compact = false
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
            use_compact = true
            start << idx
            count << 1
            stride << 1
          end
        end
        if stride.all?{|x| x == 1 }
          out = get_vara(start, count)
        else
          out = get_vars(start, count, stride)
        end
        if use_compact and out.is_a?(CArray)
          out = out.compact
        end
      when CA_REG_SELECT, CA_REG_GRID
        out = get_var[*argv]
      else
        raise "invalid index"
      end
      return out
    end
  
    def get! (*argv)
      info = CArray.scan_index(@shape, argv)
      case info.type
      when CA_REG_METHOD_CALL
        return decode(get_var)[*argv]
      else
        return decode(get(info, *argv))
      end
    end
   
    def get_var1 (*index)
      return nc_get_var1(@file_id, @var_id, index)
    end

    def get_var1! (*index)
      return decode(get_var1(*index))
    end

    def get_var ()
      return nc_get_var(@file_id, @var_id)
    end

    def get_var! ()
      return decode(nc_get_var(@file_id, @var_id))
    end

    def get_vara (start, count)
      return nc_get_vara(@file_id, @var_id, start, count)
    end

    def get_vara! (start, count)
      return decode(nc_get_vara(@file_id, @var_id, start, count))
    end

    def get_vars (start, count, stride)
      return nc_get_vars(@file_id, @var_id, start, count, stride)
    end

    def get_vars! (start, count, stride)
      return decode(nc_get_vars(@file_id, @var_id, start, count, stride))
    end

    def get_varm (start, count, stride, imap)
      return nc_get_varm(@file_id, @var_id, start, count, stride, imap)
    end

    def get_varm! (start, count, stride, imap)
      return decode(nc_get_varm(@file_id, @var_id, start, count, stride, imap))
    end

    def method_missing (id, *argv)
      to_ca.send(id, *argv)
    end

  end

end

class NCFile < NCObject

  class Dim < NCObject
  
    include NC
  
    def initialize (ncfile, dim_id)
      @ncfile      = ncfile
      @file_id     = ncfile.file_id
      @dim_id      = dim_id
      @name        = nc_inq_dimname(@file_id, @dim_id)
      @len         = nc_inq_dimlen(@file_id, @dim_id)
    end

    attr_reader :name, :len

    def definition
      return @len
    end

    def inspect
      return "#{@name}=#{@len}"
    end
  
    def to_i
      return @dim_id
    end

    def to_ca
      return self[]
    end

    def [] (*argv)
      return @ncfile[name][*argv]
    end

  end

end

class NCFile < NCObject

  include NC

  def self.open (filename)
    file_id = NC.open(filename)
    if block_given?
      begin
        nc = NCFile.new(file_id)    
        yield(nc)
      ensure
        nc.close
      end
    else
      return NCFile.new(file_id)    
    end
  end

  def initialize (file_id)
    @file_id  = file_id
    @dims     = []
    @vars     = []
    @name2dim = {}
    @name2var = {}
    @attributes = get_attributes(@file_id, NC_GLOBAL)
    @attributes.freeze
    parse_metadata()
  end

  attr_reader :file_id, :dims, :vars, :attributes

  def close ()
    nc_close(@file_id)
  end

  def parse_metadata ()
    ndims = nc_inq_ndims(@file_id)
    ndims.times do |i|
      dim = Dim.new(self, i)
      @dims[i] = dim
      @name2dim[dim.name] = dim
    end
    @dims.freeze
    @name2dim.freeze
    nvars = nc_inq_nvars(@file_id)
    nvars.times do |i|
      var = Var.new(self, i)
      @vars[i] = var
      @name2var[var.name] = var
    end
    @vars.freeze
    @name2var.freeze
  end

  def self.convert_attribute_value (value)
    case value
    when CArray
      if value.length == 1
        case value.data_type
        when CA_UINT8
          value = { "char" => value[0] }
        when CA_INT8
          value = { "byte" => value[0] }
        when CA_SHORT
          value = { "short" => value[0] }
        when CA_INT
          value = { "int" => value[0] }
        when CA_FLOAT
          value = { "float" => value[0] }
        when CA_DOUBLE
          value = { "double" => value[0] }
        end        
      else
        case value.data_type
        when CA_UINT8
          value = { "char" => value.to_a }
        when CA_INT8
          value = { "byte" => value.to_a }
        when CA_SHORT
          value = { "short" => value.to_a }
        when CA_INT
          value = { "int" => value.to_a }
        when CA_FLOAT
          value = { "float" => value.to_a }
        when CA_DOUBLE
          value = { "double" => value.to_a }
        end
      end
    end
    return value
  end

  def definition
    atts = {}
    get_attributes!(@file_id, NC_GLOBAL).each do |k,v|
      atts[k] = NCFile.convert_attribute_value(v)
    end
    {
      "dimensions" => @dims.map{|x| [x.name, x.definition] }.to_h,
      "variables" => @vars.map{|x| [x.declaration, x.definition] }.to_h,
      "attributes" => atts
    }
  end

  def [] (name)
    return @name2var[name]
  end
  
  def dim (name)
    return @name2dim[name]
  end

  def has_dim?(name)
    return @name2dim.has_key?(name)
  end

  def has_var?(name)
    return @name2var.has_key?(name)
  end

end