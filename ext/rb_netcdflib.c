#include "ruby.h"
#include "carray.h"
#include <netcdf.h>

#define CHECK_ARGC(n) \
  if ( argc != n ) \
    rb_raise(rb_eRuntimeError, "invalid # of argumnet (%i for %i)", argc, n)

#define CHECK_TYPE_STRING(val) \
  if ( TYPE(val) != T_STRING ) \
    rb_raise(rb_eRuntimeError, "string type arg required")

#define CHECK_TYPE_STRING_OR_NIL(val) \
  if ( TYPE(val) != T_STRING && !NIL_P(val)) \
    rb_raise(rb_eRuntimeError, "string type arg required")

#define CHECK_TYPE_ARRAY(val) \
  if ( TYPE(val) != T_ARRAY ) \
    rb_raise(rb_eRuntimeError, "array type arg required")

#define CHECK_TYPE_ARRAY_OR_NIL(val) \
  if ( TYPE(val) != T_ARRAY && ! NIL_P(val) )   \
    rb_raise(rb_eRuntimeError, "array type arg or nil required")

#define CHECK_TYPE_INT(val) \
  if ( ! rb_obj_is_kind_of(val, rb_cInteger) ) \
    rb_raise(rb_eRuntimeError, "int type arg required")

#define CHECK_TYPE_NUMERIC(val) \
  if ( ! rb_obj_is_kind_of(val, rb_cNumeric) ) \
    rb_raise(rb_eRuntimeError, "int type arg required")

#define CHECK_TYPE_ID(val) \
  if ( ! rb_obj_is_kind_of(val, rb_cInteger) ) \
    rb_raise(rb_eRuntimeError, "id must be an integer")

#define CHECK_TYPE_DATA(val) \
  if ( ! rb_obj_is_kind_of(val, rb_cCArray) ) \
    rb_raise(rb_eRuntimeError, "CArray type arg required")

#define CHECK_STATUS(status) \
    if ( (status) != NC_NOERR )	\
      rb_raise(rb_eRuntimeError, "%s", nc_strerror(status))

static VALUE mNetCDF;

static int
rb_nc_typemap (nc_type nc_type)
{
  switch ( nc_type ) {
  case NC_CHAR:
    return CA_UINT8;
  case NC_BYTE: 
    return CA_INT8;
  case NC_UBYTE: 
    return CA_UINT8;
  case NC_SHORT:
    return CA_INT16;
  case NC_USHORT:
    return CA_UINT16;
  case NC_INT:
    return CA_INT32;
  case NC_UINT:
    return CA_UINT32;
  case NC_INT64:
    return CA_INT64;
  case NC_UINT64:
    return CA_UINT64;
  case NC_FLOAT:
    return CA_FLOAT32;
  case NC_DOUBLE:
    return CA_FLOAT64;
  default:
    rb_raise(rb_eRuntimeError, "invalid NC_TYPE");
  }
}

static nc_type
rb_nc_rtypemap (int ca_type)
{
  switch ( ca_type ) {
  case CA_INT8:
    return NC_BYTE;
  case CA_UINT8:
    return NC_UBYTE;
  case CA_INT16:
    return NC_SHORT;
  case CA_UINT16:
    return NC_USHORT;
  case CA_INT32:
    return NC_INT;
  case CA_UINT32:
    return NC_UINT;
  case CA_INT64:
    return NC_INT64;
  case CA_UINT64:
    return NC_UINT64;
  case CA_FLOAT32:
    return NC_FLOAT;
  case CA_FLOAT64:
    return NC_DOUBLE;
  case CA_FIXLEN:
    return NC_STRING;
  default:
    rb_raise(rb_eRuntimeError, "invalid CA_TYPE");
  }
}

static VALUE
rb_nc_ca_type (int argc, VALUE *argv, VALUE mod)
{
  int type;

  CHECK_ARGC(1);
  CHECK_TYPE_INT(argv[0]);
  
  type = rb_nc_typemap(NUM2INT(argv[0]));

  return LONG2NUM(type);
}

static VALUE
rb_nc_nc_type (int argc, VALUE *argv, VALUE mod)
{
  nc_type type;

  CHECK_ARGC(1);
  CHECK_TYPE_INT(argv[0]);
  
  type = rb_nc_rtypemap(NUM2INT(argv[0]));

  return LONG2NUM(type);
}

static VALUE
rb_nc_create (int argc, VALUE *argv, VALUE mod)
{
  int status, nc_id;

  if ( argc < 1 ) {
    rb_raise(rb_eArgError, "invalid # of arguments");
  }

  CHECK_TYPE_STRING(argv[0]);

  if ( argc == 1 ) {
    status = nc_create(StringValuePtr(argv[0]), NC_CLOBBER, &nc_id);
  }
  else if ( argc == 2 ) {
    CHECK_TYPE_INT(argv[1]);
    status = nc_create(StringValuePtr(argv[0]), NUM2INT(argv[1]), &nc_id);
  }
  else {
    rb_raise(rb_eArgError, "too many argument");
  }

  CHECK_STATUS(status);

  return LONG2NUM(nc_id);
}

/*
static VALUE
rb_nc_create_mem (int argc, VALUE *argv, VALUE mod)
{
  int status, nc_id;
  if ( argc < 1 ) {
    rb_raise(rb_eArgError, "invalid # of arguments");
  }

  CHECK_TYPE_STRING(argv[0]);

  if ( argc == 1 ) {
    status = nc_create_mem(StringValuePtr(argv[0]), NC_CLOBBER, NC_SIZEHINT_DEFAULT, &nc_id);
  }
  else if ( argc == 2 ) {
    CHECK_TYPE_INT(argv[1]);
    status = nc_create_mem(StringValuePtr(argv[0]), 
                           NUM2INT(argv[1]), 
                           NC_SIZEHINT_DEFAULT,                            
                           &nc_id);
  }
  else if ( argc == 3 ) {
    CHECK_TYPE_INT(argv[1]);
    CHECK_TYPE_INT(argv[2]);
    status = nc_create_mem(StringValuePtr(argv[0]), 
                           NUM2INT(argv[1]), 
                           NUM2INT(argv[2]),                            
                           &nc_id);
  }
  else {
    rb_raise(rb_eArgError, "too many argument");
  }

  CHECK_STATUS(status);

  return LONG2NUM(nc_id);
}
*/

static VALUE
rb_nc_open (int argc, VALUE *argv, VALUE mod)
{
  int status, nc_id;

  if ( argc < 1 ) {
    rb_raise(rb_eArgError, "invalid # of arguments");
  }

  CHECK_TYPE_STRING(argv[0]);

  if ( argc == 1 ) {
    status = nc_open(StringValuePtr(argv[0]), NC_NOWRITE, &nc_id);
  }
  else if ( argc == 2 ) {
    CHECK_TYPE_INT(argv[1]);
    status = nc_open(StringValuePtr(argv[0]), NUM2INT(argv[1]), &nc_id);
  }
  else {
    rb_raise(rb_eArgError, "too many argument");
  }

  CHECK_STATUS(status);

  return LONG2NUM(nc_id);
}

static VALUE
rb_nc_close (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]);
  
  status = nc_close(NUM2INT(argv[0]));

  CHECK_STATUS(status);

  return LONG2NUM(status);
}

static VALUE
rb_nc_redef (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]);
  
  status = nc_redef(NUM2INT(argv[0]));

  CHECK_STATUS(status);

  return LONG2NUM(status);
}

static VALUE
rb_nc_enddef (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]);
  
  status = nc_enddef(NUM2INT(argv[0]));

  CHECK_STATUS(status);

  return LONG2NUM(status);
}

static VALUE
rb_nc_sync (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]);
  
  status = nc_sync(NUM2INT(argv[0]));

  CHECK_STATUS(status);

  return LONG2NUM(status);
}

static VALUE
rb_nc_inq_ndims (int argc, VALUE *argv, VALUE mod)
{
  int status, ndims;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]);
  
  status = nc_inq_ndims(NUM2INT(argv[0]), &ndims);

  CHECK_STATUS(status);

  return LONG2NUM(ndims);
}

static VALUE
rb_nc_inq_nvars (int argc, VALUE *argv, VALUE mod)
{
  int status, nvars;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]);
  
  status = nc_inq_nvars(NUM2INT(argv[0]), &nvars);

  CHECK_STATUS(status);

  return LONG2NUM(nvars);
}

static VALUE
rb_nc_inq_natts (int argc, VALUE *argv, VALUE mod)
{
  int status, natts;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]);
  
  status = nc_inq_natts(NUM2INT(argv[0]), &natts);

  CHECK_STATUS(status);

  return LONG2NUM(natts);
}

static VALUE
rb_nc_inq_unlimdim (int argc, VALUE *argv, VALUE mod)
{
  int status, uldim;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]);
  
  status = nc_inq_unlimdim(NUM2INT(argv[0]), &uldim);

  CHECK_STATUS(status);

  return LONG2NUM(uldim);
}

static VALUE
rb_nc_inq_dimid (int argc, VALUE *argv, VALUE mod)
{
  int status, dimid;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_STRING(argv[1]);
  
  status = nc_inq_dimid(NUM2INT(argv[0]), StringValuePtr(argv[1]), &dimid);

  return ( status != NC_NOERR ) ? Qnil : LONG2NUM(dimid);
}

static VALUE
rb_nc_inq_varid (int argc, VALUE *argv, VALUE mod)
{
  int status, varid;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_STRING(argv[1]);
  
  status = nc_inq_varid(NUM2INT(argv[0]), StringValuePtr(argv[1]), &varid);

  return ( status != NC_NOERR ) ? Qnil : LONG2NUM(varid);
}

static VALUE
rb_nc_inq_attid (int argc, VALUE *argv, VALUE mod)
{
  int status, attid;

  CHECK_ARGC(3);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_STRING(argv[2]);
  
  status = nc_inq_attid(NUM2INT(argv[0]), NUM2INT(argv[1]), 
		                    StringValuePtr(argv[2]), &attid);

  return ( status != NC_NOERR ) ? Qnil : LONG2NUM(attid);
}

static VALUE
rb_nc_inq_dimlen (int argc, VALUE *argv, VALUE mod)
{
  size_t dimlen;
  int status;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  
  status = nc_inq_dimlen(NUM2INT(argv[0]), NUM2INT(argv[1]), &dimlen);

  CHECK_STATUS(status);

  return ULONG2NUM(dimlen);
}

static VALUE
rb_nc_inq_dimname (int argc, VALUE *argv, VALUE mod)
{
  int status;
  char dimname[NC_MAX_NAME];

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  
  status = nc_inq_dimname(NUM2INT(argv[0]), NUM2INT(argv[1]), dimname);

  CHECK_STATUS(status);

  return rb_str_new2(dimname);
}

static VALUE
rb_nc_inq_varname (int argc, VALUE *argv, VALUE mod)
{
  int status;
  char varname[NC_MAX_NAME];

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  
  status = nc_inq_varname(NUM2INT(argv[0]), NUM2INT(argv[1]), varname);

  CHECK_STATUS(status);

  return rb_str_new2(varname);
}

static VALUE
rb_nc_inq_vartype (int argc, VALUE *argv, VALUE mod)
{
  int status;
  nc_type type;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  
  status = nc_inq_vartype(NUM2INT(argv[0]), NUM2INT(argv[1]), &type);

  CHECK_STATUS(status);

  return LONG2NUM(type);
}

static VALUE
rb_nc_inq_varndims (int argc, VALUE *argv, VALUE mod)
{
  int status, ndims;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  
  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  return LONG2NUM(ndims);
}

static VALUE
rb_nc_inq_vardimid (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE rdim;
  int dimid[NC_MAX_DIMS];
  int ndims;
  int status;
  int i;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]); /* nc_id */
  CHECK_TYPE_ID(argv[1]); /* var_id */

  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  status = nc_inq_vardimid(NUM2INT(argv[0]), NUM2INT(argv[1]), dimid);

  CHECK_STATUS(status);

  rdim = rb_ary_new();
  for (i=0; i<ndims; i++) {
    rb_ary_store(rdim, i, ULONG2NUM(dimid[i]));
  }

  return rdim;
}

static VALUE
rb_nc_inq_varnatts (int argc, VALUE *argv, VALUE mod)
{
  int status, natts;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  
  status = nc_inq_varnatts(NUM2INT(argv[0]), NUM2INT(argv[1]), &natts);

  CHECK_STATUS(status);

  return LONG2NUM(natts);
}


static VALUE
rb_nc_inq_attname (int argc, VALUE *argv, VALUE mod)
{
  int status;
  char attname[NC_MAX_NAME];

  CHECK_ARGC(3);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_ID(argv[2]);
  
  status = nc_inq_attname(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			  NUM2INT(argv[2]), attname);

  CHECK_STATUS(status);

  return rb_str_new2(attname);
}

static VALUE
rb_nc_inq_atttype (int argc, VALUE *argv, VALUE mod)
{
  int status;
  nc_type type;

  CHECK_ARGC(3);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_STRING(argv[2]);
  
  status = nc_inq_atttype(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			  StringValuePtr(argv[2]), &type);

  CHECK_STATUS(status);

  return LONG2NUM(type);
}

static VALUE
rb_nc_inq_attlen (int argc, VALUE *argv, VALUE mod)
{
  size_t len;
  int status;

  CHECK_ARGC(3);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_STRING(argv[2]);
  
  status = nc_inq_attlen(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			 StringValuePtr(argv[2]), &len);

  CHECK_STATUS(status);

  return LONG2NUM(len);
}

static VALUE
rb_nc_def_dim (int argc, VALUE *argv, VALUE mod)
{
  int status, dimid;

  CHECK_ARGC(3);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_STRING(argv[1]);
  CHECK_TYPE_INT(argv[2]);
  
  status = nc_def_dim(NUM2INT(argv[0]), StringValuePtr(argv[1]), 
		      NUM2INT(argv[2]), &dimid);

  CHECK_STATUS(status);

  return LONG2NUM(dimid);
}

static VALUE
rb_nc_def_grp (int argc, VALUE *argv, VALUE mod)
{
  int status, grpid;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_STRING(argv[1]);

  status = nc_def_grp(NUM2INT(argv[0]), StringValuePtr(argv[1]), 
           		        &grpid);

  CHECK_STATUS(status);

  return LONG2NUM(grpid);
}

static VALUE
rb_nc_inq_ncid (int argc, VALUE *argv, VALUE mod)
{
  int status, ncid;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_STRING(argv[1]);

  status = nc_inq_ncid(NUM2INT(argv[0]), StringValuePtr(argv[1]), 
           		         &ncid);

  CHECK_STATUS(status);

  return LONG2NUM(ncid);
}


static VALUE
rb_nc_inq_grp_full_ncid (int argc, VALUE *argv, VALUE mod)
{
  int status, ncid;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_STRING(argv[1]);

  status = nc_inq_grp_full_ncid(NUM2INT(argv[0]), StringValuePtr(argv[1]), 
           		         &ncid);

  CHECK_STATUS(status);

  return LONG2NUM(ncid);
}


static VALUE
rb_nc_inq_grp_ncid (int argc, VALUE *argv, VALUE mod)
{
  int status, ncid;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_STRING(argv[1]);

  status = nc_inq_grp_ncid(NUM2INT(argv[0]), StringValuePtr(argv[1]), 
           		         &ncid);

  CHECK_STATUS(status);

  return LONG2NUM(ncid);
}

static VALUE
rb_nc_inq_grp_parent (int argc, VALUE *argv, VALUE mod)
{
  int parent;
  int status;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]);
  
  status = nc_inq_grp_parent(NUM2INT(argv[0]), &parent);

  CHECK_STATUS(status);

  return ULONG2NUM(parent);
}

static VALUE
rb_nc_inq_grpname (int argc, VALUE *argv, VALUE mod)
{
  int status;
  char grpname[NC_MAX_NAME];

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]);
  
  status = nc_inq_grpname(NUM2INT(argv[0]), grpname);

  CHECK_STATUS(status);

  return rb_str_new2(grpname);
}

static VALUE
rb_nc_inq_grpname_full (int argc, VALUE *argv, VALUE mod)
{
  int status;
  size_t len;
  char grpname[NC_MAX_NAME];

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]);
  
  status = nc_inq_grpname_full(NUM2INT(argv[0]), &len, grpname);

  CHECK_STATUS(status);
  
  return rb_str_new2(grpname);
}

static VALUE
rb_nc_inq_grpname_len (int argc, VALUE *argv, VALUE mod)
{
  int status;
  size_t len;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_STRING(argv[1]);

  status = nc_inq_grpname_len(NUM2INT(argv[0]), &len);

  CHECK_STATUS(status);

  return LONG2NUM(len);
}

static VALUE
rb_nc_inq_grps (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE rgrps;
  int grps[NC_MAX_DIMS];
  int i, ngrps;
  int status;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]); /* nc_id */

  status = nc_inq_grps(NUM2INT(argv[0]), 
                       &ngrps,
                       grps);

  CHECK_STATUS(status);

  rgrps = rb_ary_new();
  for (i=0; i<ngrps; i++) {
    rb_ary_store(rgrps, i, ULONG2NUM(grps[i]));
  }

  return rgrps;
}


static VALUE
rb_nc_inq_dimids (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE rdimids;
  int dimids[NC_MAX_DIMS];
  int i, ndims;
  int status;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]); /* nc_id */
  CHECK_TYPE_ID(argv[1]); /* nc_id */

  status = nc_inq_dimids(NUM2INT(argv[0]), 
                         &ndims,
                         dimids, 
                         NUM2INT(argv[1]));

  CHECK_STATUS(status);

  rdimids = rb_ary_new();
  for (i=0; i<ndims; i++) {
    rb_ary_store(rdimids, i, ULONG2NUM(dimids[i]));
  }

  return rdimids;
}

static VALUE
rb_nc_inq_varids (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE rvarids;
  int varids[NC_MAX_DIMS];
  int i, nvars;
  int status;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]); /* nc_id */

  status = nc_inq_varids(NUM2INT(argv[0]), 
                         &nvars,
                         varids);

  CHECK_STATUS(status);

  rvarids = rb_ary_new();
  for (i=0; i<nvars; i++) {
    rb_ary_store(rvarids, i, ULONG2NUM(varids[i]));
  }

  return rvarids;
}

static VALUE
rb_nc_inq_typeids (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE rtypeids;
  int typeids[NC_MAX_DIMS];
  int i, ntypes;
  int status;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]); /* nc_id */

  status = nc_inq_typeids(NUM2INT(argv[0]), 
                         &ntypes,
                         typeids);

  CHECK_STATUS(status);

  rtypeids = rb_ary_new();
  for (i=0; i<ntypes; i++) {
    rb_ary_store(rtypeids, i, ULONG2NUM(typeids[i]));
  }

  return rtypeids;
}

static VALUE
rb_nc_show_metadata (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(1);
  CHECK_TYPE_ID(argv[0]); /* nc_id */

  status = nc_show_metadata(NUM2INT(argv[0]));

  CHECK_STATUS(status);

  return Qnil;
}

static VALUE
rb_nc_def_var (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE vdim;
  int status, varid;
  int ndims;
  int dimids[NC_MAX_DIMS];
  int i;

  CHECK_ARGC(4);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_STRING(argv[1]);
  CHECK_TYPE_INT(argv[2]);
  CHECK_TYPE_ARRAY(argv[3]);

  vdim = argv[3];

  ndims = (int) RARRAY_LEN(vdim);
  for (i=0; i<ndims; i++) {
    dimids[i] = NUM2INT(RARRAY_PTR(vdim)[i]);
  }

  status = nc_def_var(NUM2INT(argv[0]), StringValuePtr(argv[1]), 
		      NUM2INT(argv[2]), ndims, dimids, &varid);

  CHECK_STATUS(status);

  return LONG2NUM(varid);
}

static VALUE
rb_nc_def_var_chunking (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE vdim;
  int status;
  int ndims;
  size_t chunksizes[NC_MAX_DIMS];
  int i;

  CHECK_ARGC(4);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_INT(argv[2]);
  CHECK_TYPE_ARRAY(argv[3]);

  vdim = argv[3];

  ndims = (int) RARRAY_LEN(vdim);
  for (i=0; i<ndims; i++) {
    chunksizes[i] = NUM2INT(RARRAY_PTR(vdim)[i]);
  }

  status = nc_def_var_chunking(NUM2INT(argv[0]), 
                               NUM2INT(argv[1]), 
                               NUM2INT(argv[2]), 
                               chunksizes);

  CHECK_STATUS(status);

  return Qnil;
}

static VALUE
rb_nc_inq_var_chunking (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE retval;
  volatile VALUE rchunksize;
  int storage;
  size_t chunksizes[NC_MAX_DIMS];
  int i, ndims;
  int status;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]); /* nc_id */
  CHECK_TYPE_ID(argv[1]); /* var_id */

  status = nc_inq_var_chunking(NUM2INT(argv[0]), 
                               NUM2INT(argv[1]),
                               &storage, 
                               chunksizes);

  CHECK_STATUS(status);

  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  rchunksize = rb_ary_new();
  for (i=0; i<ndims; i++) {
    rb_ary_store(rchunksize, i, ULONG2NUM(chunksizes[i]));
  }

  retval = rb_ary_new();
  rb_ary_store(retval, 0, ULONG2NUM(storage));
  rb_ary_store(retval, 1, rchunksize);

  return retval;
}

static VALUE
rb_nc_def_var_deflate (int argc, VALUE *argv, VALUE mod)
{
  int status;
  
  CHECK_ARGC(5);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_INT(argv[2]);
  CHECK_TYPE_INT(argv[3]);
  CHECK_TYPE_INT(argv[4]);
  
  status = nc_def_var_deflate(NUM2INT(argv[0]), NUM2INT(argv[1]),
                              NUM2INT(argv[2]), NUM2INT(argv[3]), NUM2INT(argv[4]));


  CHECK_STATUS(status);

  return Qnil;
}

static VALUE
rb_nc_inq_var_deflate (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE retval;
  int shuffle, deflate, deflate_level;
  int status;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]); /* nc_id */
  CHECK_TYPE_ID(argv[1]); /* var_id */

  status = nc_inq_var_deflate(NUM2INT(argv[0]), NUM2INT(argv[1]),
                              &shuffle, &deflate, &deflate_level);

  CHECK_STATUS(status);

  retval = rb_ary_new();
  rb_ary_store(retval, 0, ULONG2NUM(shuffle));
  rb_ary_store(retval, 1, ULONG2NUM(deflate));
  rb_ary_store(retval, 2, ULONG2NUM(deflate_level));

  return retval;
}


static VALUE
rb_nc_del_att (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(3);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_STRING(argv[2]);

  status = nc_del_att(NUM2INT(argv[0]), NUM2INT(argv[1]), StringValuePtr(argv[2]));

  CHECK_STATUS(status);

  return LONG2NUM(status);
}

static int
nc_get_att_numeric (int ncid, int varid, const char name[], 
		    nc_type type, void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_get_att_schar(ncid, varid, name, value);
  case NC_UBYTE:
    return nc_get_att_ubyte(ncid, varid, name, value);
  case NC_CHAR:
    return nc_get_att_uchar(ncid, varid, name, value);
  case NC_SHORT:
    return nc_get_att_short(ncid, varid, name, value);
  case NC_USHORT:
    return nc_get_att_ushort(ncid, varid, name, value);
  case NC_INT:
    return nc_get_att_int(ncid, varid, name, value);
  case NC_UINT:
    return nc_get_att_uint(ncid, varid, name, value);
  case NC_INT64:
    return nc_get_att_longlong(ncid, varid, name, value);
  case NC_UINT64:
    return nc_get_att_ulonglong(ncid, varid, name, value);
  case NC_FLOAT:
    return nc_get_att_float(ncid, varid, name, value);
  case NC_DOUBLE:
    return nc_get_att_double(ncid, varid, name, value);
  default:
    return NC_EBADTYPE;
  }
}

static int
nc_put_att_numeric (int ncid, int varid, const char name[], 
		    nc_type type, nc_type xtype, size_t len, void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_put_att_schar(ncid, varid, name, xtype, len, value);
  case NC_UBYTE:
    return nc_put_att_ubyte(ncid, varid, name, xtype, len, value);
  case NC_CHAR:
    return nc_put_att_uchar(ncid, varid, name, xtype, len, value);
  case NC_SHORT:
    return nc_put_att_short(ncid, varid, name, xtype, len, value);
  case NC_USHORT:
    return nc_put_att_ushort(ncid, varid, name, xtype, len, value);
  case NC_INT:
    return nc_put_att_int(ncid, varid, name, xtype, len, value);
  case NC_UINT:
    return nc_put_att_uint(ncid, varid, name, xtype, len, value);
  case NC_INT64:
    return nc_put_att_longlong(ncid, varid, name, xtype, len, value);
  case NC_UINT64:
    return nc_put_att_longlong(ncid, varid, name, xtype, len, value);
  case NC_FLOAT:
    return nc_put_att_float(ncid, varid, name, xtype, len, value);
  case NC_DOUBLE:
    return nc_put_att_double(ncid, varid, name, xtype, len, value);
  default:
    return NC_EBADTYPE;
  }
}

static VALUE
rb_nc_get_att (int argc, VALUE *argv, VALUE mod)
{
  nc_type type;
  size_t len;
  int status;

  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_STRING(argv[2]);

  status = nc_inq_atttype(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			  StringValuePtr(argv[2]), &type);

  if ( status != NC_NOERR) {
    return Qnil;
  }

  status = nc_inq_attlen(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			 StringValuePtr(argv[2]), &len);
  CHECK_STATUS(status);

  if ( argc == 3 ) {

    if ( type == NC_CHAR ) {
      volatile VALUE text = rb_str_new(NULL, len);
      status = nc_get_att_text(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			       StringValuePtr(argv[2]), 
			       StringValuePtr(text));
      return text;
    }
    else if ( len == 1 ) {
      switch ( type ) {
      case NC_BYTE: {
      	int8_t val;
      	status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
      				  StringValuePtr(argv[2]), type, &val);
      	CHECK_STATUS(status);
      	return INT2NUM(val);
      }
      case NC_UBYTE: {
      	uint8_t val;
      	status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
      				  StringValuePtr(argv[2]), type, &val);
      	CHECK_STATUS(status);
      	return INT2NUM(val);
      }
      case NC_SHORT: {
      	int16_t val;
      	status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
      				    StringValuePtr(argv[2]), type, &val);
      	CHECK_STATUS(status);
      	return INT2NUM(val);
      }
      case NC_USHORT: {
      	uint16_t val;
      	status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
      				    StringValuePtr(argv[2]), type, &val);
      	CHECK_STATUS(status);
      	return INT2NUM(val);
      }
      case NC_INT: {
      	int32_t val;
      	status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
      				    StringValuePtr(argv[2]), type, &val);
      	CHECK_STATUS(status);
      	return INT2NUM(val);
      }
      case NC_UINT: {
      	uint32_t val;
      	status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
      				    StringValuePtr(argv[2]), type, &val);
      	CHECK_STATUS(status);
      	return INT2NUM(val);
      }
      case NC_INT64: {
      	int64_t val;
      	status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
      				    StringValuePtr(argv[2]), type, &val);
      	CHECK_STATUS(status);
      	return INT2NUM(val);
      }
      case NC_UINT64: {
      	uint64_t val;
      	status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
      				    StringValuePtr(argv[2]), type, &val);
      	CHECK_STATUS(status);
      	return INT2NUM(val);
      }
      case NC_FLOAT: {
      	float32_t val;
      	status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
      				    StringValuePtr(argv[2]), type, &val);
      	CHECK_STATUS(status);
      	return rb_float_new(val);
      }
      case NC_DOUBLE: {
      	float64_t val;
      	status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
      				    StringValuePtr(argv[2]), type, &val);
      	CHECK_STATUS(status);
      	return rb_float_new(val);
      }
      default: 
      	rb_raise(rb_eRuntimeError, "unknown att nc_type");
      }
    }
    else {
      volatile VALUE out;
      CArray *ca;
      int8_t  data_type;
      ca_size_t dim0 = len;

      data_type = rb_nc_typemap (type);
      out = rb_carray_new(data_type, 1, &dim0, 0, NULL);

      Data_Get_Struct(out, CArray, ca);

      status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				  StringValuePtr(argv[2]), type, ca->ptr);

      CHECK_STATUS(status);

      return out;
    }
  } 
  else {

    CHECK_ARGC(4);

    if ( type == NC_CHAR ) {
      volatile VALUE text = argv[3];
      rb_str_resize(text, len);
      status = nc_get_att_text(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			       StringValuePtr(argv[2]), 
			       StringValuePtr(text));
    }  
    else {
      CArray *ca;
      nc_type xtype;
      if ( ! rb_obj_is_kind_of(argv[3], rb_cCArray) ) {
      	rb_raise(rb_eTypeError, "arg4 must be a CArray object");
      }
      Data_Get_Struct(argv[3], CArray, ca);
      xtype = rb_nc_rtypemap(ca->data_type);
      ca_attach(ca);
      status = nc_get_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				  StringValuePtr(argv[2]), type, ca->ptr);
      ca_sync(ca);
      ca_detach(ca);
    }
    
    CHECK_STATUS(status);

    return LONG2NUM(status);
  }
}

static VALUE
rb_nc_put_att (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(4);

  if ( TYPE(argv[3]) == T_STRING ) {
    volatile VALUE text = argv[3];
    status = nc_put_att_text(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			     StringValuePtr(argv[2]), 
			     strlen(StringValuePtr(text)), StringValuePtr(text));
  }
  else if ( rb_obj_is_kind_of(argv[3], rb_cInteger) ) {
    int32_t val = NUM2INT(argv[3]);
    status = nc_put_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				StringValuePtr(argv[2]), 
				NC_INT, NC_INT, 1, &val);
  }
  else if ( rb_obj_is_kind_of(argv[3], rb_cFloat) ) {
    float64_t val = NUM2DBL(argv[3]);
    status = nc_put_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				StringValuePtr(argv[2]), 
				NC_DOUBLE, NC_DOUBLE, 1, &val);
  }
  else {
    CArray *ca;
    nc_type xtype;
    if ( ! rb_obj_is_kind_of(argv[3], rb_cCArray) ) {
      rb_raise(rb_eTypeError, "arg4 must be a CArray object");
    }
    Data_Get_Struct(argv[3], CArray, ca);
    xtype = rb_nc_rtypemap(ca->data_type);
    ca_attach(ca);
    status = nc_put_att_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				StringValuePtr(argv[2]), 
				xtype, xtype, ca->elements, ca->ptr);
    ca_detach(ca);
  }

  CHECK_STATUS(status);

  return LONG2NUM(status);
}


static int
nc_get_var1_numeric (int ncid, int varid, 
		     nc_type type, size_t index[], void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_get_var1_schar(ncid, varid, index, value);
  case NC_UBYTE:
    return nc_get_var1_ubyte(ncid, varid, index, value);
  case NC_CHAR:
    return nc_get_var1_uchar(ncid, varid, index, value);
  case NC_SHORT:
    return nc_get_var1_short(ncid, varid, index, value);
  case NC_USHORT:
    return nc_get_var1_ushort(ncid, varid, index, value);
  case NC_INT:
    return nc_get_var1_int(ncid, varid, index, value);
  case NC_UINT:
    return nc_get_var1_uint(ncid, varid, index, value);
  case NC_INT64:
    return nc_get_var1_longlong(ncid, varid, index, value);
  case NC_UINT64:
    return nc_get_var1_ulonglong(ncid, varid, index, value);
  case NC_FLOAT:
    return nc_get_var1_float(ncid, varid, index, value);
  case NC_DOUBLE:
    return nc_get_var1_double(ncid, varid, index, value);
  default:
    return NC_EBADTYPE;
  }
}

static int
nc_put_var1_numeric (int ncid, int varid, 
		     nc_type type, size_t index[], void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_put_var1_schar(ncid, varid, index, value);
  case NC_UBYTE:
    return nc_put_var1_ubyte(ncid, varid, index, value);
  case NC_CHAR:
    return nc_put_var1_uchar(ncid, varid, index, value);
  case NC_SHORT:
    return nc_put_var1_short(ncid, varid, index, value);
  case NC_USHORT:
    return nc_put_var1_ushort(ncid, varid, index, value);
  case NC_INT:
    return nc_put_var1_int(ncid, varid, index, value);
  case NC_UINT:
    return nc_put_var1_uint(ncid, varid, index, value);
  case NC_INT64:
    return nc_put_var1_longlong(ncid, varid, index, value);
  case NC_UINT64:
    return nc_put_var1_ulonglong(ncid, varid, index, value);
  case NC_FLOAT:
    return nc_put_var1_float(ncid, varid, index, value);
  case NC_DOUBLE:
    return nc_put_var1_double(ncid, varid, index, value);
  default:
    return NC_EBADTYPE;
  }
}

static int
nc_get_var_numeric (int ncid, int varid, nc_type type, void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_get_var_schar(ncid, varid, value);
  case NC_UBYTE:
    return nc_get_var_ubyte(ncid, varid, value);
  case NC_CHAR:
    return nc_get_var_uchar(ncid, varid, value);
  case NC_SHORT:
    return nc_get_var_short(ncid, varid, value);
  case NC_USHORT:
    return nc_get_var_ushort(ncid, varid, value);
  case NC_INT:
    return nc_get_var_int(ncid, varid, value);
  case NC_UINT:
    return nc_get_var_uint(ncid, varid, value);
  case NC_INT64:
    return nc_get_var_longlong(ncid, varid, value);
  case NC_UINT64:
    return nc_get_var_ulonglong(ncid, varid, value);
  case NC_FLOAT:
    return nc_get_var_float(ncid, varid, value);
  case NC_DOUBLE:
    return nc_get_var_double(ncid, varid, value);
  default:
    return NC_EBADTYPE;
  }
}

static int
nc_put_var_numeric (int ncid, int varid, nc_type type, void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_put_var_schar(ncid, varid, value);
  case NC_UBYTE:
    return nc_put_var_ubyte(ncid, varid, value);
  case NC_CHAR:
    return nc_put_var_uchar(ncid, varid, value);
  case NC_SHORT:
    return nc_put_var_short(ncid, varid, value);
  case NC_USHORT:
    return nc_put_var_ushort(ncid, varid, value);
  case NC_INT:
    return nc_put_var_int(ncid, varid, value);
  case NC_UINT:
    return nc_put_var_uint(ncid, varid, value);
  case NC_INT64:
    return nc_put_var_longlong(ncid, varid, value);
  case NC_UINT64:
    return nc_put_var_ulonglong(ncid, varid, value);
  case NC_FLOAT:
    return nc_put_var_float(ncid, varid, value);
  case NC_DOUBLE:
    return nc_put_var_double(ncid, varid, value);
  default:
    return NC_EBADTYPE;
  }
}

static int
nc_get_vara_numeric (int ncid, int varid, nc_type type, 
		     const size_t start[], const size_t count[], 
		     void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_get_vara_schar(ncid, varid, start, count, value);
  case NC_UBYTE:
    return nc_get_vara_ubyte(ncid, varid, start, count, value);
  case NC_CHAR:
    return nc_get_vara_uchar(ncid, varid, start, count, value);
  case NC_SHORT:
    return nc_get_vara_short(ncid, varid, start, count, value);
  case NC_USHORT:
    return nc_get_vara_ushort(ncid, varid, start, count, value);
  case NC_INT:
    return nc_get_vara_int(ncid, varid, start, count, value);
  case NC_UINT:
    return nc_get_vara_uint(ncid, varid, start, count, value);
  case NC_INT64:
    return nc_get_vara_longlong(ncid, varid, start, count, value);
  case NC_UINT64:
    return nc_get_vara_ulonglong(ncid, varid, start, count, value);
  case NC_FLOAT:
    return nc_get_vara_float(ncid, varid, start, count, value);
  case NC_DOUBLE:
    return nc_get_vara_double(ncid, varid, start, count, value);
  default:
    return NC_EBADTYPE;
  }
}

static int
nc_put_vara_numeric (int ncid, int varid, nc_type type, 
		     const size_t start[], const size_t count[], 
		     void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_put_vara_schar(ncid, varid, start, count, value);
  case NC_UBYTE:
    return nc_put_vara_ubyte(ncid, varid, start, count, value);
  case NC_CHAR:
    return nc_put_vara_uchar(ncid, varid, start, count, value);
  case NC_SHORT:
    return nc_put_vara_short(ncid, varid, start, count, value);
  case NC_USHORT:
    return nc_put_vara_ushort(ncid, varid, start, count, value);
  case NC_INT:
    return nc_put_vara_int(ncid, varid, start, count, value);
  case NC_UINT:
    return nc_put_vara_uint(ncid, varid, start, count, value);
  case NC_INT64:
    return nc_put_vara_longlong(ncid, varid, start, count, value);
  case NC_UINT64:
    return nc_put_vara_ulonglong(ncid, varid, start, count, value);
  case NC_FLOAT:
    return nc_put_vara_float(ncid, varid, start, count, value);
  case NC_DOUBLE:
    return nc_put_vara_double(ncid, varid, start, count, value);
  default:
    return NC_EBADTYPE;
  }
}

static int
nc_get_vars_numeric (int ncid, int varid, nc_type type, 
		     const size_t start[], const size_t count[], 
		     const ptrdiff_t stride[],
		     void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_get_vars_schar(ncid, varid, start, count, stride, value);
  case NC_UBYTE:
    return nc_get_vars_ubyte(ncid, varid, start, count, stride, value);
  case NC_CHAR:
    return nc_get_vars_uchar(ncid, varid, start, count, stride, value);
  case NC_SHORT:
    return nc_get_vars_short(ncid, varid, start, count, stride, value);
  case NC_USHORT:
    return nc_get_vars_ushort(ncid, varid, start, count, stride, value);
  case NC_INT:
    return nc_get_vars_int(ncid, varid, start, count, stride, value);
  case NC_UINT:
    return nc_get_vars_uint(ncid, varid, start, count, stride, value);
  case NC_INT64:
    return nc_get_vars_longlong(ncid, varid, start, count, stride, value);
  case NC_UINT64:
    return nc_get_vars_ulonglong(ncid, varid, start, count, stride, value);
  case NC_FLOAT:
    return nc_get_vars_float(ncid, varid, start, count, stride, value);
  case NC_DOUBLE:
    return nc_get_vars_double(ncid, varid, start, count, stride, value);
  default:
    return NC_EBADTYPE;
  }
}

static int
nc_put_vars_numeric (int ncid, int varid, nc_type type, 
		     const size_t start[], const size_t count[], 
		     const ptrdiff_t stride[],
		     void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_put_vars_schar(ncid, varid, start, count, stride, value);
  case NC_UBYTE:
    return nc_put_vars_ubyte(ncid, varid, start, count, stride, value);
  case NC_CHAR:
    return nc_put_vars_uchar(ncid, varid, start, count, stride, value);
  case NC_SHORT:
    return nc_put_vars_short(ncid, varid, start, count, stride, value);
  case NC_USHORT:
    return nc_put_vars_ushort(ncid, varid, start, count, stride, value);
  case NC_INT:
    return nc_put_vars_int(ncid, varid, start, count, stride, value);
  case NC_UINT:
    return nc_put_vars_uint(ncid, varid, start, count, stride, value);
  case NC_INT64:
    return nc_put_vars_longlong(ncid, varid, start, count, stride, value);
  case NC_UINT64:
    return nc_put_vars_ulonglong(ncid, varid, start, count, stride, value);
  case NC_FLOAT:
    return nc_put_vars_float(ncid, varid, start, count, stride, value);
  case NC_DOUBLE:
    return nc_put_vars_double(ncid, varid, start, count, stride, value);
  default:
    return NC_EBADTYPE;
  }
}

static int
nc_get_varm_numeric (int ncid, int varid, nc_type type, 
		     const size_t start[], const size_t count[], 
		     const ptrdiff_t stride[], const ptrdiff_t imap[],
		     void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_get_varm_schar(ncid, varid, start, count, stride, imap, value);
  case NC_UBYTE:
    return nc_get_varm_ubyte(ncid, varid, start, count, stride, imap, value);
  case NC_CHAR:
    return nc_get_varm_uchar(ncid, varid, start, count, stride, imap, value);
  case NC_SHORT:
    return nc_get_varm_short(ncid, varid, start, count, stride, imap, value);
  case NC_USHORT:
    return nc_get_varm_ushort(ncid, varid, start, count, stride, imap, value);
  case NC_INT:
    return nc_get_varm_int(ncid, varid, start, count, stride, imap, value);
  case NC_UINT:
    return nc_get_varm_uint(ncid, varid, start, count, stride, imap, value);
  case NC_INT64:
    return nc_get_varm_longlong(ncid, varid, start, count, stride, imap, value);
  case NC_UINT64:
    return nc_get_varm_ulonglong(ncid, varid, start, count, stride, imap, value);
  case NC_FLOAT:
    return nc_get_varm_float(ncid, varid, start, count, stride, imap, value);
  case NC_DOUBLE:
    return nc_get_varm_double(ncid, varid, start, count, stride, imap, value);
  default:
    return NC_EBADTYPE;
  }
}

static int
nc_put_varm_numeric (int ncid, int varid, nc_type type, 
		     const size_t start[], const size_t count[], 
		     const ptrdiff_t stride[], const ptrdiff_t imap[],
		     void *value)
{
  switch (type) {
  case NC_BYTE:
    return nc_put_varm_schar(ncid, varid, start, count, stride, imap, value);
  case NC_UBYTE:
    return nc_put_varm_ubyte(ncid, varid, start, count, stride, imap, value);
  case NC_CHAR:
    return nc_put_varm_uchar(ncid, varid, start, count, stride, imap, value);
  case NC_SHORT:
    return nc_put_varm_short(ncid, varid, start, count, stride, imap, value);
  case NC_USHORT:
    return nc_put_varm_ushort(ncid, varid, start, count, stride, imap, value);
  case NC_INT:
    return nc_put_varm_int(ncid, varid, start, count, stride, imap, value);
  case NC_UINT:
    return nc_put_varm_uint(ncid, varid, start, count, stride, imap, value);
  case NC_INT64:
    return nc_put_varm_longlong(ncid, varid, start, count, stride, imap, value);
  case NC_UINT64:
    return nc_put_varm_ulonglong(ncid, varid, start, count, stride, imap, value);
  case NC_FLOAT:
    return nc_put_varm_float(ncid, varid, start, count, stride, imap, value);
  case NC_DOUBLE:
    return nc_put_varm_double(ncid, varid, start, count, stride, imap, value);
  default:
    return NC_EBADTYPE;
  }
}

static VALUE
rb_nc_get_var1 (int argc, VALUE *argv, VALUE mod)
{
  int status;
  nc_type type;
  int ndims;
  size_t index[NC_MAX_DIMS];
  int i;

  if ( argc < 3 ) {
    rb_raise(rb_eArgError, "invalid # of arguments");
  }

  status = nc_inq_vartype(NUM2INT(argv[0]), NUM2INT(argv[1]), &type);

  CHECK_STATUS(status);

  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  Check_Type(argv[2], T_ARRAY);
  for (i=0; i<ndims; i++) {
    index[i] = NUM2ULONG(RARRAY_PTR(argv[2])[i]);
  }

  if ( argc == 3 ) {
    switch ( type ) {
    case NC_BYTE: {
      int8_t val;
      status = nc_get_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				   type, index, &val);
      CHECK_STATUS(status);
      return INT2NUM(val);
    }
    case NC_UBYTE: {
      uint8_t val;
      status = nc_get_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				   type, index, &val);
      CHECK_STATUS(status);
      return INT2NUM(val);
    }
    case NC_SHORT: {
      int16_t val;
      status = nc_get_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				   type, index, &val);
      CHECK_STATUS(status);
      return INT2NUM(val);
    }
    case NC_USHORT: {
      uint16_t val;
      status = nc_get_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				   type, index, &val);
      CHECK_STATUS(status);
      return INT2NUM(val);
    }
    case NC_INT: {
      int32_t val;
      status = nc_get_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				   type, index, &val);
      CHECK_STATUS(status);
      return INT2NUM(val);
    }
    case NC_UINT: {
      uint32_t val;
      status = nc_get_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				   type, index, &val);
      CHECK_STATUS(status);
      return INT2NUM(val);
    }
    case NC_INT64: {
      int64_t val;
      status = nc_get_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				   type, index, &val);
      CHECK_STATUS(status);
      return INT2NUM(val);
    }
    case NC_UINT64: {
      uint64_t val;
      status = nc_get_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				   type, index, &val);
      CHECK_STATUS(status);
      return INT2NUM(val);
    }
    case NC_FLOAT: {
      float32_t val;
      status = nc_get_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				   type, index, &val);
      CHECK_STATUS(status);
      return rb_float_new(val);
    }
    case NC_DOUBLE: {
      float64_t val;
      status = nc_get_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				   type, index, &val);
      CHECK_STATUS(status);
      return rb_float_new(val);
    }
    default: 
      rb_raise(rb_eRuntimeError, "unknown att nc_type");
    }
  }
  else {
    volatile VALUE data = argv[3];
    CArray *ca;

    if ( ! rb_obj_is_kind_of(data, rb_cCArray) ) {
      rb_raise(rb_eTypeError, "arg4 must be a CArray object");
    }

    Data_Get_Struct(data, CArray, ca);

    type = rb_nc_rtypemap(ca->data_type);

    ca_attach(ca);
    status = nc_get_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			       type, index, ca->ptr);

    ca_sync(ca);
    ca_detach(ca);

    CHECK_STATUS(status);

    return LONG2NUM(status);
  }

}

static VALUE
rb_nc_put_var1 (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE data;
  int status;
  nc_type type;
  size_t index[NC_MAX_DIMS];
  int ndims;
  CArray *ca;
  int i;

  CHECK_ARGC(4);

  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  Check_Type(argv[2], T_ARRAY);

  for (i=0; i<ndims; i++) {
    index[i] = NUM2ULONG(RARRAY_PTR(argv[2])[i]);
  }

  data = argv[3];

  if ( ! rb_obj_is_kind_of(data, rb_cCArray) ) {
    rb_raise(rb_eTypeError, "arg4 must be a CArray object");
  }

  Data_Get_Struct(data, CArray, ca);

  type = rb_nc_rtypemap(ca->data_type);

  ca_attach(ca);
  status = nc_put_var1_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			       type, index, ca->ptr);
  ca_detach(ca);

  CHECK_STATUS(status);
  
  return LONG2NUM(status);
}

static VALUE
rb_nc_get_var (int argc, VALUE *argv, VALUE mod)
{
  int status;
  int ndims;
  int dimid[NC_MAX_DIMS];
  nc_type type;

  if ( argc < 2 ) {
    rb_raise(rb_eArgError, "invalid # of arguments");
  }
  
  status = nc_inq_vartype(NUM2INT(argv[0]), NUM2INT(argv[1]), &type);

  CHECK_STATUS(status);

  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  status = nc_inq_vardimid(NUM2INT(argv[0]), NUM2INT(argv[1]), dimid);

  CHECK_STATUS(status);

  if ( argc == 2 ) {
    volatile VALUE out;
    CArray *ca;
    int8_t rank, data_type;
    ca_size_t dim[CA_RANK_MAX];
    size_t len;
    int i;

    data_type = rb_nc_typemap (type);
    rank = ndims;
    for (i=0; i<rank; i++) {
      status = nc_inq_dimlen(NUM2INT(argv[0]), dimid[i], &len);
      CHECK_STATUS(status);
      dim[i] = len;
    }

    out = rb_carray_new(data_type, rank, dim, 0, NULL);
    Data_Get_Struct(out, CArray, ca);

    status = nc_get_var_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				type, ca->ptr);

    CHECK_STATUS(status);
  
    return out;
  }
  else {
    volatile VALUE data = argv[2];
    CArray *ca;

    if ( ! rb_obj_is_kind_of(data, rb_cCArray) ) {
      rb_raise(rb_eTypeError, "arg3 must be a CArray object");
    }

    Data_Get_Struct(data, CArray, ca);

    type = rb_nc_rtypemap(ca->data_type);

    ca_attach(ca);
    status = nc_get_var_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				type, ca->ptr);
    ca_sync(ca);
    ca_detach(ca);

    CHECK_STATUS(status);
  
    return LONG2NUM(status);
  }
}

static VALUE
rb_nc_put_var (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE data;
  int status;
  nc_type type;
  CArray *ca;

  CHECK_ARGC(3);
  
  status = nc_inq_vartype(NUM2INT(argv[0]), NUM2INT(argv[1]), &type);

  CHECK_STATUS(status);

  data = argv[2];

  if ( ! rb_obj_is_kind_of(data, rb_cCArray) ) {
    rb_raise(rb_eTypeError, "arg3 must be a CArray object");
  }

  Data_Get_Struct(data, CArray, ca);

  type = rb_nc_rtypemap(ca->data_type);
  ca_attach(ca);
  status = nc_put_var_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			      type, ca->ptr);
  ca_detach(ca);

  CHECK_STATUS(status);
  
  return LONG2NUM(status);
}

static VALUE
rb_nc_get_vara (int argc, VALUE *argv, VALUE mod)
{
  int status;
  nc_type type;
  int ndims;
  int dimid[NC_MAX_DIMS];
  size_t start[NC_MAX_DIMS], count[NC_MAX_DIMS];
  int i;

  if ( argc < 4 ) {
    rb_raise(rb_eArgError, "invalid # of arguments");
  }
  
  status = nc_inq_vartype(NUM2INT(argv[0]), NUM2INT(argv[1]), &type);

  CHECK_STATUS(status);

  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  status = nc_inq_vardimid(NUM2INT(argv[0]), NUM2INT(argv[1]), dimid);

  CHECK_STATUS(status);

  Check_Type(argv[2], T_ARRAY);
  Check_Type(argv[3], T_ARRAY);

  for (i=0; i<ndims; i++) {
    start[i] = NUM2ULONG(RARRAY_PTR(argv[2])[i]);
    count[i] = NUM2ULONG(RARRAY_PTR(argv[3])[i]);
  }

  if ( argc == 4 ) {
    volatile VALUE out;
    CArray *ca;
    int8_t rank, data_type;
    ca_size_t dim[CA_RANK_MAX];
    int i;

    data_type = rb_nc_typemap (type);
    rank = ndims;
    for (i=0; i<rank; i++) {
      dim[i] = count[i];
    }

    out = rb_carray_new(data_type, rank, dim, 0, NULL);
    Data_Get_Struct(out, CArray, ca);

    status = nc_get_vara_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				 type, start, count, ca->ptr);

    CHECK_STATUS(status);
  
    return out;
  }
  else {
    volatile VALUE data = argv[4];
    CArray *ca;

    if ( ! rb_obj_is_kind_of(data, rb_cCArray) ) {
      rb_raise(rb_eTypeError, "arg5 must be a CArray object");
    }
    Data_Get_Struct(data, CArray, ca);

    if ( ca->rank != ndims ) {
      rb_raise(rb_eRuntimeError, "rank mismatch");
    }

    for (i=0; i<ca->rank; i++) {
      if ( ca->dim[i] != (ca_size_t) count[i] ) {
      	rb_raise(rb_eRuntimeError, "dim[%i] mismatch", i);
      }
    }

    type = rb_nc_rtypemap(ca->data_type);

    ca_attach(ca);
    status = nc_get_vara_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			       type, start, count, ca->ptr);
    ca_sync(ca);
    ca_detach(ca);

    CHECK_STATUS(status);
  
    return LONG2NUM(status);
  }
}

static VALUE
rb_nc_put_vara (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE data;
  int status;
  nc_type type;
  int ndims;
  size_t start[NC_MAX_DIMS], count[NC_MAX_DIMS];
  CArray *ca;
  int i;

  CHECK_ARGC(5);

  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  Check_Type(argv[2], T_ARRAY);
  Check_Type(argv[3], T_ARRAY);

  for (i=0; i<ndims; i++) {
    start[i] = NUM2ULONG(RARRAY_PTR(argv[2])[i]);
    count[i] = NUM2ULONG(RARRAY_PTR(argv[3])[i]);
  }

  data = argv[4];

  if ( ! rb_obj_is_kind_of(data, rb_cCArray) ) {
    rb_raise(rb_eTypeError, "arg5 must be a CArray object");
  }

  Data_Get_Struct(data, CArray, ca);

  type = rb_nc_rtypemap(ca->data_type);

  ca_attach(ca);
  status = nc_put_vara_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			       type, start, count, ca->ptr);
  ca_detach(ca);

  CHECK_STATUS(status);
  
  return LONG2NUM(status);
}

static VALUE
rb_nc_get_vars (int argc, VALUE *argv, VALUE mod)
{
  int status;
  nc_type type;
  int ndims;
  int dimid[NC_MAX_DIMS];
  size_t start[NC_MAX_DIMS], count[NC_MAX_DIMS];
  ptrdiff_t stride[NC_MAX_DIMS];
  CArray *ca;
  int i;

  if ( argc < 5 ) {
    rb_raise(rb_eArgError, "invalid # of arguments");
  }
  
  status = nc_inq_vartype(NUM2INT(argv[0]), NUM2INT(argv[1]), &type);

  CHECK_STATUS(status);

  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  status = nc_inq_vardimid(NUM2INT(argv[0]), NUM2INT(argv[1]), dimid);

  CHECK_STATUS(status);

  Check_Type(argv[2], T_ARRAY);
  Check_Type(argv[3], T_ARRAY);
  Check_Type(argv[4], T_ARRAY);

  for (i=0; i<ndims; i++) {
    start[i]  = NUM2ULONG(RARRAY_PTR(argv[2])[i]);
    count[i]  = NUM2ULONG(RARRAY_PTR(argv[3])[i]);
    stride[i] = NUM2ULONG(RARRAY_PTR(argv[4])[i]);
  }

  if ( argc == 5 ) {
    volatile VALUE out;
    CArray *ca;
    int8_t rank, data_type;
    ca_size_t dim[CA_RANK_MAX];
    int i;

    data_type = rb_nc_typemap (type);
    rank = ndims;
    for (i=0; i<rank; i++) {
      dim[i] = count[i];
    }

    out = rb_carray_new(data_type, rank, dim, 0, NULL);
    Data_Get_Struct(out, CArray, ca);

    status = nc_get_vars_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
				 type, start, count, stride, ca->ptr);

    CHECK_STATUS(status);
  
    return out;
  }
  else {
    volatile VALUE data = argv[5];
    int i;

    if ( ! rb_obj_is_kind_of(data, rb_cCArray) ) {
      rb_raise(rb_eTypeError, "arg6 must be a CArray object");
    }

    Data_Get_Struct(data, CArray, ca);

    if ( ca->rank != ndims ) {
      rb_raise(rb_eRuntimeError, "rank mismatch");
    }

    for (i=0; i<ca->rank; i++) {
      if ( ca->dim[i] != (ca_size_t) count[i] ) {
      	rb_raise(rb_eRuntimeError, "dim[%i] mismatch", i);
      }
    }

    type = rb_nc_rtypemap(ca->data_type);

    ca_attach(ca);
    status = nc_get_vars_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			       type, start, count, stride, ca->ptr);
    ca_sync(ca);
    ca_detach(ca);

    CHECK_STATUS(status);
  
    return LONG2NUM(status);
  }
}

static VALUE
rb_nc_put_vars (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE data;
  int status;
  nc_type type;
  int ndims;
  size_t start[NC_MAX_DIMS], count[NC_MAX_DIMS];
  ptrdiff_t stride[NC_MAX_DIMS];
  CArray *ca;
  int i;

  CHECK_ARGC(6);

  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  Check_Type(argv[2], T_ARRAY);
  Check_Type(argv[3], T_ARRAY);
  Check_Type(argv[4], T_ARRAY);

  for (i=0; i<ndims; i++) {
    start[i]  = NUM2ULONG(RARRAY_PTR(argv[2])[i]);
    count[i]  = NUM2ULONG(RARRAY_PTR(argv[3])[i]);
    stride[i] = NUM2ULONG(RARRAY_PTR(argv[4])[i]);
  }

  data = argv[5];

  if ( ! rb_obj_is_kind_of(data, rb_cCArray) ) {
    rb_raise(rb_eTypeError, "arg6 must be a CArray object");
  }

  Data_Get_Struct(data, CArray, ca);

  type = rb_nc_rtypemap(ca->data_type);

  ca_attach(ca);
  status = nc_put_vars_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			       type, start, count, stride, ca->ptr);
  ca_detach(ca);

  CHECK_STATUS(status);
  
  return LONG2NUM(status);
}

static VALUE
rb_nc_get_varm (int argc, VALUE *argv, VALUE mod)
{
  int status;
  nc_type type;
  int ndims;
  int dimid[NC_MAX_DIMS];
  size_t    start[NC_MAX_DIMS], count[NC_MAX_DIMS];
  ptrdiff_t stride[NC_MAX_DIMS], imap[NC_MAX_DIMS];
  CArray *ca;
  int i;

  if ( argc < 6 ) {
    rb_raise(rb_eArgError, "invalid # of arguments");
  }
  
  status = nc_inq_vartype(NUM2INT(argv[0]), NUM2INT(argv[1]), &type);

  CHECK_STATUS(status);

  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  status = nc_inq_vardimid(NUM2INT(argv[0]), NUM2INT(argv[1]), dimid);

  CHECK_STATUS(status);

  Check_Type(argv[2], T_ARRAY);
  Check_Type(argv[3], T_ARRAY);
  Check_Type(argv[4], T_ARRAY);
  Check_Type(argv[5], T_ARRAY);

  for (i=0; i<ndims; i++) {
    start[i]  = NUM2ULONG(RARRAY_PTR(argv[2])[i]);
    count[i]  = NUM2ULONG(RARRAY_PTR(argv[3])[i]);
    stride[i] = NUM2ULONG(RARRAY_PTR(argv[4])[i]);
    imap[i]   = NUM2ULONG(RARRAY_PTR(argv[5])[i]);
  }

  if ( argc == 6 ) {
    volatile VALUE out;
    CArray *ca;
    int8_t rank, data_type;
    ca_size_t dim[CA_RANK_MAX];
    int i;

    data_type = rb_nc_typemap (type);
    rank = ndims;
    for (i=0; i<rank; i++) {
      dim[i] = count[i];
    }

    out = rb_carray_new(data_type, rank, dim, 0, NULL);
    Data_Get_Struct(out, CArray, ca);

    status = nc_get_varm_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			       type, start, count, stride, imap, ca->ptr);

    CHECK_STATUS(status);
  
    return out;
  }
  else {
    volatile VALUE data = argv[6];
    int i;

    if ( ! rb_obj_is_kind_of(data, rb_cCArray) ) {
      rb_raise(rb_eTypeError, "arg6 must be a CArray object");
    }

    Data_Get_Struct(data, CArray, ca);

    if ( ca->rank != ndims ) {
      rb_raise(rb_eRuntimeError, "rank mismatch");
    }

    for (i=0; i<ca->rank; i++) {
      if ( ca->dim[i] != (ca_size_t) count[i] ) {
      	rb_raise(rb_eRuntimeError, "dim[%i] mismatch", i);
      }
    }

    type = rb_nc_rtypemap(ca->data_type);

    ca_attach(ca);
    status = nc_get_varm_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			       type, start, count, stride, imap, ca->ptr);
    ca_sync(ca);
    ca_detach(ca);
  
    CHECK_STATUS(status);
  
    return LONG2NUM(status);
  }
}

static VALUE
rb_nc_put_varm (int argc, VALUE *argv, VALUE mod)
{
  volatile VALUE data;
  int status;
  nc_type type;
  int ndims;
  size_t start[NC_MAX_DIMS], count[NC_MAX_DIMS];
  ptrdiff_t stride[NC_MAX_DIMS], imap[NC_MAX_DIMS];
  CArray *ca;
  int i;

  CHECK_ARGC(7);

  status = nc_inq_varndims(NUM2INT(argv[0]), NUM2INT(argv[1]), &ndims);

  CHECK_STATUS(status);

  Check_Type(argv[2], T_ARRAY);
  Check_Type(argv[3], T_ARRAY);
  Check_Type(argv[4], T_ARRAY);
  Check_Type(argv[5], T_ARRAY);

  for (i=0; i<ndims; i++) {
    start[i]  = NUM2ULONG(RARRAY_PTR(argv[2])[i]);
    count[i]  = NUM2ULONG(RARRAY_PTR(argv[3])[i]);
    stride[i] = NUM2ULONG(RARRAY_PTR(argv[4])[i]);
    imap[i]   = NUM2ULONG(RARRAY_PTR(argv[5])[i]);
  }

  data = argv[6];

  if ( ! rb_obj_is_kind_of(data, rb_cCArray) ) {
    rb_raise(rb_eTypeError, "arg7 must be a CArray object");
  }

  Data_Get_Struct(data, CArray, ca);

  type = rb_nc_rtypemap(ca->data_type);

  ca_attach(ca);
  status = nc_put_varm_numeric(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			       type, start, count, stride, imap, ca->ptr);
  ca_detach(ca);

  CHECK_STATUS(status);
  
  return LONG2NUM(status);
}

static VALUE
rb_nc_rename_dim (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(3);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_STRING(argv[2]);

  status = nc_rename_dim(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			 StringValuePtr(argv[2]));

  CHECK_STATUS(status);

  return LONG2NUM(status);
}

static VALUE
rb_nc_rename_grp (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_STRING(argv[1]);
  
  status = nc_rename_grp(NUM2INT(argv[0]), StringValuePtr(argv[1]));

  CHECK_STATUS(status);

  return LONG2NUM(status);
}

static VALUE
rb_nc_rename_var (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(3);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_STRING(argv[2]);
  
  status = nc_rename_var(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			 StringValuePtr(argv[2]));

  CHECK_STATUS(status);

  return LONG2NUM(status);
}

static VALUE
rb_nc_rename_att (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(4);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_STRING(argv[2]);
  CHECK_TYPE_STRING(argv[3]);
  
  status = nc_rename_att(NUM2INT(argv[0]), NUM2INT(argv[1]), 
			 StringValuePtr(argv[2]), StringValuePtr(argv[3]));

  CHECK_STATUS(status);

  return LONG2NUM(status);
}

static VALUE
rb_nc_set_fill (int argc, VALUE *argv, VALUE mod)
{
  int status;
  int old_fillmode;

  CHECK_ARGC(2);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_INT(argv[1]);

  status = nc_set_fill(NUM2INT(argv[0]), NUM2INT(argv[1]), &old_fillmode);

  CHECK_STATUS(status);

  return LONG2NUM(old_fillmode);
}

static VALUE
rb_nc_copy_att (int argc, VALUE *argv, VALUE mod)
{
  int status;

  CHECK_ARGC(5);
  CHECK_TYPE_ID(argv[0]);
  CHECK_TYPE_ID(argv[1]);
  CHECK_TYPE_STRING(argv[2]);
  CHECK_TYPE_ID(argv[3]);
  CHECK_TYPE_ID(argv[4]);
  
  status = nc_copy_att(NUM2INT(argv[0]), NUM2INT(argv[1]), 
		      StringValuePtr(argv[2]), NUM2INT(argv[3]), NUM2INT(argv[4]));

  CHECK_STATUS(status);

  return LONG2NUM(status);
}

void
Init_simple_netcdf ()
{

  mNetCDF = rb_define_module("NC");

  rb_define_singleton_method(mNetCDF, "ca_type",  rb_nc_ca_type, -1);
  rb_define_singleton_method(mNetCDF, "nc_type",  rb_nc_nc_type, -1);

  rb_define_module_function(mNetCDF, "nc_create", rb_nc_create, -1);
  rb_define_singleton_method(mNetCDF,   "create", rb_nc_create, -1);
  rb_define_module_function(mNetCDF, "nc_open",   rb_nc_open, -1);
  rb_define_singleton_method(mNetCDF,   "open",   rb_nc_open, -1);
  rb_define_module_function(mNetCDF, "nc_close",  rb_nc_close, -1);
  rb_define_singleton_method(mNetCDF,   "close",  rb_nc_close, -1);
  rb_define_module_function(mNetCDF, "nc_redef",  rb_nc_redef, -1);
  rb_define_singleton_method(mNetCDF,   "redef",  rb_nc_redef, -1);
  rb_define_module_function(mNetCDF, "nc_enddef", rb_nc_enddef, -1);
  rb_define_singleton_method(mNetCDF,   "enddef", rb_nc_enddef, -1);
  rb_define_module_function(mNetCDF, "nc_sync",   rb_nc_sync, -1);
  rb_define_singleton_method(mNetCDF,   "sync",   rb_nc_sync, -1);
  rb_define_module_function(mNetCDF, "nc_set_fill",  rb_nc_set_fill, -1);
  rb_define_singleton_method(mNetCDF,   "set_fill",  rb_nc_set_fill, -1);

  rb_define_module_function(mNetCDF, "nc_def_dim",  rb_nc_def_dim, -1);
  rb_define_singleton_method(mNetCDF,   "def_dim",  rb_nc_def_dim, -1);
  // nc_inq_dim is not defined
  rb_define_module_function(mNetCDF, "nc_inq_dimid",   rb_nc_inq_dimid, -1);
  rb_define_singleton_method(mNetCDF,   "inq_dimid",   rb_nc_inq_dimid, -1);
  rb_define_module_function(mNetCDF, "nc_inq_dimlen",  rb_nc_inq_dimlen, -1);
  rb_define_singleton_method(mNetCDF,   "inq_dimlen",  rb_nc_inq_dimlen, -1);
  rb_define_module_function(mNetCDF, "nc_inq_dimname", rb_nc_inq_dimname, -1);
  rb_define_singleton_method(mNetCDF,   "inq_dimname", rb_nc_inq_dimname, -1);
  rb_define_module_function(mNetCDF, "nc_inq_ndims",   rb_nc_inq_ndims, -1);
  rb_define_singleton_method(mNetCDF,   "inq_ndims",   rb_nc_inq_ndims, -1);
  rb_define_module_function(mNetCDF, "nc_inq_unlimdim",   rb_nc_inq_unlimdim, -1);
  rb_define_singleton_method(mNetCDF,   "inq_unlimdim",   rb_nc_inq_unlimdim, -1);
  rb_define_module_function(mNetCDF, "nc_rename_dim",  rb_nc_rename_dim, -1);
  rb_define_singleton_method(mNetCDF,   "rename_dim",  rb_nc_rename_dim, -1);

  rb_define_module_function(mNetCDF, "nc_def_var",  rb_nc_def_var, -1);
  rb_define_singleton_method(mNetCDF,   "def_var",  rb_nc_def_var, -1);
  rb_define_module_function(mNetCDF, "nc_def_var_chunking",  rb_nc_def_var_chunking, -1);
  rb_define_singleton_method(mNetCDF,   "def_var_chunking",  rb_nc_def_var_chunking, -1);
  rb_define_module_function(mNetCDF, "nc_def_var_deflate",  rb_nc_def_var_deflate, -1);
  rb_define_singleton_method(mNetCDF,   "def_var_deflate",  rb_nc_def_var_deflate, -1);
  // nc_def_var_endian
  // nc_def_var_fill
  // nc_def_var_fletcher32
  // nc_free_string
  rb_define_module_function(mNetCDF, "nc_get_var1", rb_nc_get_var1, -1);
  rb_define_singleton_method(mNetCDF,   "get_var1", rb_nc_get_var1, -1);
  rb_define_module_function(mNetCDF, "nc_get_var",  rb_nc_get_var, -1);
  rb_define_singleton_method(mNetCDF,   "get_var",  rb_nc_get_var, -1);
  rb_define_module_function(mNetCDF, "nc_get_vara", rb_nc_get_vara, -1);
  rb_define_singleton_method(mNetCDF,   "get_vara", rb_nc_get_vara, -1);
  rb_define_module_function(mNetCDF, "nc_get_vars", rb_nc_get_vars, -1);
  rb_define_singleton_method(mNetCDF,   "get_vars", rb_nc_get_vars, -1);
  rb_define_module_function(mNetCDF, "nc_get_varm", rb_nc_get_varm, -1);
  rb_define_singleton_method(mNetCDF,   "get_varm", rb_nc_get_varm, -1);
  //nc_inq_var
  rb_define_module_function(mNetCDF, "nc_inq_var_chunking",  rb_nc_inq_var_chunking, -1);
  rb_define_singleton_method(mNetCDF,   "inq_var_chunking",  rb_nc_inq_var_chunking, -1);
  rb_define_module_function(mNetCDF, "nc_inq_var_deflate",  rb_nc_inq_var_deflate, -1);
  rb_define_singleton_method(mNetCDF,   "inq_var_deflate",  rb_nc_inq_var_deflate, -1);
  //nc_inq_var_endian
  //nc_inq_var_fill
  //nc_inq_var_filter
  //nc_inq_var_fletcher32
  //nc_inq_var_szip
  rb_define_module_function(mNetCDF, "nc_inq_vardimid",  rb_nc_inq_vardimid, -1);
  rb_define_singleton_method(mNetCDF,    "inq_vardimid",  rb_nc_inq_vardimid, -1);
  rb_define_module_function(mNetCDF, "nc_inq_varid",   rb_nc_inq_varid, -1);
  rb_define_singleton_method(mNetCDF,   "inq_varid",   rb_nc_inq_varid, -1);
  rb_define_module_function(mNetCDF, "nc_inq_varname",    rb_nc_inq_varname, -1);
  rb_define_singleton_method(mNetCDF,   "inq_varname",    rb_nc_inq_varname, -1);
  rb_define_module_function(mNetCDF, "nc_inq_varnatts",   rb_nc_inq_varnatts, -1);
  rb_define_singleton_method(mNetCDF,   "inq_varnatts",  rb_nc_inq_varnatts, -1);
  rb_define_module_function(mNetCDF, "nc_inq_varndims",   rb_nc_inq_varndims, -1);
  rb_define_singleton_method(mNetCDF,   "inq_varndims",   rb_nc_inq_varndims, -1);
  rb_define_module_function(mNetCDF, "nc_inq_vartype",    rb_nc_inq_vartype, -1);
  rb_define_singleton_method(mNetCDF,   "inq_vartype",    rb_nc_inq_vartype, -1);
  rb_define_module_function(mNetCDF, "nc_put_var1", rb_nc_put_var1, -1);
  rb_define_singleton_method(mNetCDF,   "put_var1", rb_nc_put_var1, -1);
  rb_define_module_function(mNetCDF, "nc_put_var",  rb_nc_put_var, -1);
  rb_define_singleton_method(mNetCDF,   "put_var",  rb_nc_put_var, -1);
  rb_define_module_function(mNetCDF, "nc_put_vara", rb_nc_put_vara, -1);
  rb_define_singleton_method(mNetCDF,   "put_vara", rb_nc_put_vara, -1);
  rb_define_module_function(mNetCDF, "nc_put_vars", rb_nc_put_vars, -1);
  rb_define_singleton_method(mNetCDF,   "put_vars", rb_nc_put_vars, -1);
  rb_define_module_function(mNetCDF, "nc_put_varm", rb_nc_put_varm, -1);
  rb_define_singleton_method(mNetCDF,   "put_varm", rb_nc_put_varm, -1);
  rb_define_module_function(mNetCDF, "nc_rename_var",  rb_nc_rename_var, -1);
  rb_define_singleton_method(mNetCDF,   "rename_var",  rb_nc_rename_var, -1);
  // nc_set_var_chunk_cache
  
  rb_define_module_function(mNetCDF, "nc_del_att",  rb_nc_del_att, -1);
  rb_define_singleton_method(mNetCDF,   "del_att",  rb_nc_del_att, -1);
  rb_define_module_function(mNetCDF, "nc_get_att",  rb_nc_get_att, -1);
  rb_define_singleton_method(mNetCDF,   "get_att",  rb_nc_get_att, -1);
  // nc_inq_att
  rb_define_module_function(mNetCDF, "nc_inq_attid",   rb_nc_inq_attid, -1);
  rb_define_singleton_method(mNetCDF,   "inq_attid",   rb_nc_inq_attid, -1);
  rb_define_module_function(mNetCDF, "nc_inq_attlen",  rb_nc_inq_attlen, -1);
  rb_define_singleton_method(mNetCDF,   "inq_attlen",  rb_nc_inq_attlen, -1);
  rb_define_module_function(mNetCDF, "nc_inq_attname", rb_nc_inq_attname, -1);
  rb_define_singleton_method(mNetCDF,   "inq_attname", rb_nc_inq_attname, -1);
  rb_define_module_function(mNetCDF, "nc_inq_atttype", rb_nc_inq_atttype, -1);
  rb_define_singleton_method(mNetCDF,   "inq_atttype", rb_nc_inq_atttype, -1);
  rb_define_module_function(mNetCDF, "nc_inq_natts",   rb_nc_inq_natts, -1);
  rb_define_singleton_method(mNetCDF,   "inq_natts",   rb_nc_inq_natts, -1);
  rb_define_module_function(mNetCDF, "nc_put_att",  rb_nc_put_att, -1);
  rb_define_singleton_method(mNetCDF,   "put_att",  rb_nc_put_att, -1);
  rb_define_module_function(mNetCDF, "nc_rename_att",  rb_nc_rename_att, -1);
  rb_define_singleton_method(mNetCDF,   "rename_att",  rb_nc_rename_att, -1);

  rb_define_module_function(mNetCDF, "nc_def_grp",  rb_nc_def_grp, -1);
  rb_define_singleton_method(mNetCDF,   "def_grp",  rb_nc_def_grp, -1);
  rb_define_module_function(mNetCDF, "nc_inq_dimids",  rb_nc_inq_dimids, -1);
  rb_define_singleton_method(mNetCDF,   "inq_dimids",  rb_nc_inq_dimids, -1);
  rb_define_module_function(mNetCDF, "nc_inq_grp_full_ncid",  rb_nc_inq_grp_full_ncid, -1);
  rb_define_singleton_method(mNetCDF,   "inq_grp_full_ncid",  rb_nc_inq_grp_full_ncid, -1);
  rb_define_module_function(mNetCDF, "nc_inq_grp_ncid",  rb_nc_inq_grp_ncid, -1);
  rb_define_singleton_method(mNetCDF,   "inq_grp_ncid",  rb_nc_inq_grp_ncid, -1);
  rb_define_module_function(mNetCDF, "nc_inq_grp_parent",  rb_nc_inq_grp_parent, -1);
  rb_define_singleton_method(mNetCDF,   "inq_grp_parent",  rb_nc_inq_grp_parent, -1);
  rb_define_module_function(mNetCDF, "nc_inq_grpname",  rb_nc_inq_grpname, -1);
  rb_define_singleton_method(mNetCDF,   "inq_grpname",  rb_nc_inq_grpname, -1);
  rb_define_module_function(mNetCDF, "nc_inq_grpname_full",  rb_nc_inq_grpname_full, -1);
  rb_define_singleton_method(mNetCDF,   "inq_grpname_full",  rb_nc_inq_grpname_full, -1);
  rb_define_module_function(mNetCDF, "nc_inq_grpname_len",  rb_nc_inq_grpname_len, -1);
  rb_define_singleton_method(mNetCDF,   "inq_grpname_len",  rb_nc_inq_grpname_len, -1);
  rb_define_module_function(mNetCDF, "nc_inq_grps",  rb_nc_inq_grps, -1);
  rb_define_singleton_method(mNetCDF,   "inq_grps",  rb_nc_inq_grps, -1);
  rb_define_module_function(mNetCDF, "nc_inq_ncid",  rb_nc_inq_ncid, -1);
  rb_define_singleton_method(mNetCDF,   "inq_ncid",  rb_nc_inq_ncid, -1);
  rb_define_module_function(mNetCDF, "nc_inq_typeids",  rb_nc_inq_typeids, -1);
  rb_define_singleton_method(mNetCDF,   "inq_typeids",  rb_nc_inq_typeids, -1);
  rb_define_module_function(mNetCDF, "nc_inq_varids",  rb_nc_inq_varids, -1);
  rb_define_singleton_method(mNetCDF,   "inq_varids",  rb_nc_inq_varids, -1);
  rb_define_module_function(mNetCDF, "nc_rename_grp",  rb_nc_rename_grp, -1);
  rb_define_singleton_method(mNetCDF,   "rename_grp",  rb_nc_rename_grp, -1);
  rb_define_module_function(mNetCDF, "nc_show_metadata",  rb_nc_show_metadata, -1);
  rb_define_singleton_method(mNetCDF,   "show_metadata",  rb_nc_show_metadata, -1);


  rb_define_module_function(mNetCDF, "nc_copy_att", rb_nc_copy_att, -1);
  rb_define_singleton_method(mNetCDF,   "copy_att", rb_nc_copy_att, -1);

  rb_define_module_function(mNetCDF, "nc_inq_nvars",   rb_nc_inq_nvars, -1);
  rb_define_singleton_method(mNetCDF,   "inq_nvars",   rb_nc_inq_nvars, -1);

  rb_define_const(mNetCDF, "NC_NOERR",        INT2NUM(NC_NOERR));

  rb_define_const(mNetCDF, "NC_NOWRITE",      INT2NUM(NC_NOWRITE));
  rb_define_const(mNetCDF, "NC_WRITE",        INT2NUM(NC_WRITE));
  rb_define_const(mNetCDF, "NC_CLOBBER",      INT2NUM(NC_CLOBBER));
  rb_define_const(mNetCDF, "NC_NOCLOBBER",    INT2NUM(NC_NOCLOBBER));
  rb_define_const(mNetCDF, "NC_DISKLESS",        INT2NUM(NC_DISKLESS));
  rb_define_const(mNetCDF, "NC_MMAP",        INT2NUM(NC_MMAP));
#if defined(NC_64BIT_DATA)
  rb_define_const(mNetCDF, "NC_64BIT_DATA",        INT2NUM(NC_64BIT_DATA));
#endif
#if defined(NC_CDF5)
  rb_define_const(mNetCDF, "NC_CDF5",        INT2NUM(NC_CDF5));
#endif
  rb_define_const(mNetCDF, "NC_CLASSIC_MODEL",    INT2NUM(NC_CLASSIC_MODEL));
  rb_define_const(mNetCDF, "NC_LOCK",         INT2NUM(NC_LOCK));
  rb_define_const(mNetCDF, "NC_SHARE",        INT2NUM(NC_SHARE));
  rb_define_const(mNetCDF, "NC_NETCDF4",      INT2NUM(NC_NETCDF4));
  rb_define_const(mNetCDF, "NC_MPIIO",      INT2NUM(NC_MPIIO));
#if defined(NC_INMEMORY)
  rb_define_const(mNetCDF, "NC_INMEMORY",      INT2NUM(NC_INMEMORY));
#endif
  rb_define_const(mNetCDF, "NC_PNETCDF",      INT2NUM(NC_PNETCDF));

  rb_define_const(mNetCDF, "NC_FORMAT_CLASSIC",      INT2NUM(NC_FORMAT_CLASSIC));
#if defined(NC_FORMAT_64BIT_OFFSET)
  rb_define_const(mNetCDF, "NC_FORMAT_64BIT_OFFSET",      INT2NUM(NC_FORMAT_64BIT_OFFSET));
#endif
  rb_define_const(mNetCDF, "NC_FORMAT_64BIT",      INT2NUM(NC_FORMAT_64BIT));
  rb_define_const(mNetCDF, "NC_FORMAT_NETCDF4",      INT2NUM(NC_FORMAT_NETCDF4));
  rb_define_const(mNetCDF, "NC_FORMAT_NETCDF4_CLASSIC",      INT2NUM(NC_FORMAT_NETCDF4_CLASSIC));
#if defined(NC_FORMAT_64BIT_DATA)
  rb_define_const(mNetCDF, "NC_FORMAT_64BIT_DATA",      INT2NUM(NC_FORMAT_64BIT_DATA));
#endif
#if defined(NC_FORMAT_CDF5)
  rb_define_const(mNetCDF, "NC_FORMAT_CDF5",      INT2NUM(NC_FORMAT_CDF5));
#endif

  rb_define_const(mNetCDF, "NC_SIZEHINT_DEFAULT", INT2NUM(NC_SIZEHINT_DEFAULT));

  rb_define_const(mNetCDF, "NC_ALIGN_CHUNK",       INT2NUM(NC_ALIGN_CHUNK));
  rb_define_const(mNetCDF, "NC_UNLIMITED",       INT2NUM(NC_UNLIMITED));

  rb_define_const(mNetCDF, "NC_GLOBAL",       INT2NUM(NC_GLOBAL));
  rb_define_const(mNetCDF, "NC_MAX_DIMS",     INT2NUM(NC_MAX_DIMS));
  rb_define_const(mNetCDF, "NC_MAX_ATTRS",     INT2NUM(NC_MAX_ATTRS));
  rb_define_const(mNetCDF, "NC_MAX_VARS",     INT2NUM(NC_MAX_VARS));
  rb_define_const(mNetCDF, "NC_MAX_NAME",     INT2NUM(NC_MAX_NAME));
  rb_define_const(mNetCDF, "NC_MAX_VAR_DIMS", INT2NUM(NC_MAX_VAR_DIMS));
  rb_define_const(mNetCDF, "NC_FILL",         INT2NUM(NC_FILL));
  rb_define_const(mNetCDF, "NC_NOFILL",       INT2NUM(NC_NOFILL));

  rb_define_const(mNetCDF, "NC_ENDIAN_NATIVE",         INT2NUM(NC_ENDIAN_NATIVE));
  rb_define_const(mNetCDF, "NC_ENDIAN_LITTLE",         INT2NUM(NC_ENDIAN_LITTLE));
  rb_define_const(mNetCDF, "NC_ENDIAN_BIG",         INT2NUM(NC_ENDIAN_BIG));

  rb_define_const(mNetCDF, "NC_CHUNKED",      INT2NUM(NC_CHUNKED));
  rb_define_const(mNetCDF, "NC_CONTIGUOUS",   INT2NUM(NC_CONTIGUOUS));

  rb_define_const(mNetCDF, "NC_NOCHECKSUM",      INT2NUM(NC_NOCHECKSUM));
  rb_define_const(mNetCDF, "NC_FLETCHER32",      INT2NUM(NC_FLETCHER32));

  rb_define_const(mNetCDF, "NC_NOSHUFFLE",      INT2NUM(NC_NOSHUFFLE));
  rb_define_const(mNetCDF, "NC_SHUFFLE",      INT2NUM(NC_SHUFFLE));


  rb_define_const(mNetCDF, "NC_NAT",     INT2NUM(NC_NAT));
  rb_define_const(mNetCDF, "NC_BYTE",    INT2NUM(NC_BYTE));
  rb_define_const(mNetCDF, "NC_UBYTE",   INT2NUM(NC_UBYTE));
  rb_define_const(mNetCDF, "NC_CHAR",    INT2NUM(NC_CHAR));
  rb_define_const(mNetCDF, "NC_SHORT",   INT2NUM(NC_SHORT));
  rb_define_const(mNetCDF, "NC_USHORT",  INT2NUM(NC_USHORT));
  rb_define_const(mNetCDF, "NC_INT",     INT2NUM(NC_INT));
  rb_define_const(mNetCDF, "NC_LONG",    INT2NUM(NC_LONG));
  rb_define_const(mNetCDF, "NC_UINT",    INT2NUM(NC_UINT));
  rb_define_const(mNetCDF, "NC_INT64",   INT2NUM(NC_INT64));
  rb_define_const(mNetCDF, "NC_UINT64",  INT2NUM(NC_UINT64));
  rb_define_const(mNetCDF, "NC_FLOAT",   INT2NUM(NC_FLOAT));
  rb_define_const(mNetCDF, "NC_DOUBLE",  INT2NUM(NC_DOUBLE));
  rb_define_const(mNetCDF, "NC_STRING",  INT2NUM(NC_STRING));

//  rb_define_const(mNetCDF, "NC_",     INT2NUM(NC_));

  rb_define_const(mNetCDF, "NC_VLEN",     INT2NUM(NC_VLEN));
  rb_define_const(mNetCDF, "NC_OPAQUE",     INT2NUM(NC_OPAQUE));
  rb_define_const(mNetCDF, "NC_ENUM",     INT2NUM(NC_ENUM));
  rb_define_const(mNetCDF, "NC_COMPOUND",     INT2NUM(NC_COMPOUND));


  rb_define_const(mNetCDF, "NC_FILL_BYTE",     INT2NUM(NC_FILL_BYTE));
  rb_define_const(mNetCDF, "NC_FILL_CHAR",     INT2NUM(NC_FILL_CHAR));
  rb_define_const(mNetCDF, "NC_FILL_SHORT",     INT2NUM(NC_FILL_SHORT));
  rb_define_const(mNetCDF, "NC_FILL_INT",     INT2NUM(NC_FILL_INT));
  rb_define_const(mNetCDF, "NC_FILL_FLOAT",     rb_float_new(NC_FILL_FLOAT));
  rb_define_const(mNetCDF, "NC_FILL_DOUBLE",     rb_float_new(NC_FILL_DOUBLE));
  rb_define_const(mNetCDF, "NC_FILL_UBYTE",     UINT2NUM(NC_FILL_UBYTE));
  rb_define_const(mNetCDF, "NC_FILL_USHORT",     UINT2NUM(NC_FILL_USHORT));
  rb_define_const(mNetCDF, "NC_FILL_UINT",     UINT2NUM(NC_FILL_UINT));
  rb_define_const(mNetCDF, "NC_FILL_IN64",     LL2NUM(NC_FILL_INT64));
  rb_define_const(mNetCDF, "NC_FILL_UIN64",     ULL2NUM(NC_FILL_INT64));
  rb_define_const(mNetCDF, "NC_FILL_STRING",     rb_str_new2(NC_FILL_STRING));

}
