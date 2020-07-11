CDL-like definition using YAML
------------------------------

### Example

```yaml
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
```

### Dimensions

```yaml
dimensions:
  lon: 1
  lat: 1
  time: 5 
```

#### UNLIMITED dimension, 

```yaml
dimensions:
  time: 0   ### unlimited
```

### Variables

#### CDL-like style definition

```yaml
variables:
  float temp (time, lat, lon): 
```

#### Attribute style definition

```yaml
variables:
  temp:
    type: float
    dim: [time, lat, lon] 
```

#### Variable attributes

```yaml
variables:
  float temp (time, lat, lon): 
    long_name: air temperature
    standard_name: air_temperature
    units: K
    _FillValue: -999.9f
```

#### Literals for numerical value

    byte:   0b -1b 255b
    short: -2s 0123s 0x7ffs
    int:    -2 1234567890L 0123 0x7ff
    float:  -2.0f 3.14159265358979f 1.0e-20f 1.f
    double: -2.0  3.141592653589793 1.0e-20  1.d

#### Hash expression for numerical value

    { byte: -1 }
    { short: 1234 }
    { int: 1234567890 }
    { float: -999.9 }
    { double: -999.9 }

### Global attributes

```yaml
  attributes:
    creator: "simple-netcdf"
```

