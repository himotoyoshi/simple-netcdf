
Gem::Specification::new do |s|
  version = "1.0.0"
  files = Dir.glob("**/*") - [ 
                               Dir.glob("simple-netcdf-*.gem"), 
                               Dir.glob("doc/**/*"), 
                             ].flatten

  s.platform    = Gem::Platform::RUBY
  s.name        = "simple-netcdf"
  s.summary     = "Simple NetCDF Read/Write library"
  s.description = <<-HERE
    A Library for reading and writing NetCDF file in Ruby.
  HERE
  s.version     = version
  s.licenses    = ['MIT']
  s.author      = "Hiroki Motoyoshi"
  s.email       = ""
  s.homepage    = "https://github.com/himotoyoshi/simple-netcdf"
  s.files       = files
  s.extensions  = [ "ext/extconf.rb" ]
  s.required_ruby_version = ">= 2.1.0"
  s.add_runtime_dependency 'carray', '~> 1.0', '>= 1.0.0'
end

