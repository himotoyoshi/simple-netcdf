simple-netcdf
=============

A library to create or load NetCDF-3 file simply.

Installation
------------

    gem install simple-netcdf
    
### Requirement

* gem carray
* gem netcdflib

Usage
------

### File creation

```ruby
require "simple-netcdf"

# Dummy data
temp = CA_FLOAT([13.7,12.9,-999.9,20.1,19.8])

# NetCDF creation
nc = NCFileWriter.create("test.nc")

nc.define netcdf %{ 
  dimensions:
    lon: 1
    lat: 1
    time: 5
  variables:
    float temp (time, lat, lon):
      long_name: air temperature
      standard_name: air_temperature
      units: K
      _FillValue: -999.9f
  attributes:
    Conventions: "CF-1.7"
}

nc["temp"] = temp

nc.close
```
