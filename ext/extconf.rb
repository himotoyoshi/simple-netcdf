require "mkmf"
require "carray/mkmf"

$CFLAGS += " -Wall"

dir_config("netcdf", possible_includes("netcdf","netcdf3","netcdf-3"), possible_libs)

if have_carray() and have_header("netcdf.h") and have_library("netcdf")
  create_makefile("simple_netcdf")
end

