#include "zstd.h"

struct dic_struct {
  ZSTD_CDict* cdic;
  ZSTD_CCtx* cctx;
  ZSTD_DDict* ddic;
  ZSTD_DCtx* dctx;
};

static off_t fsize_or_raise(const char *filename) {
  struct stat st;
  if (stat(filename, &st) == 0) return st.st_size;
  rb_raise(rb_eRuntimeError, "file not found: %s", filename);
}

static FILE* fopen_or_raise(const char *filename, const char *instruction) {
  FILE* const inFile = fopen(filename, instruction);
  if (inFile) return inFile;
  rb_raise(rb_eRuntimeError, "cannot open file: %s", filename);
}

static void* malloc_or_raise(size_t size) {
  void* const buff = malloc(size);
  if (buff) return buff;
  rb_raise(rb_eRuntimeError, "malloc failed");
}

static void* load_file_or_raise(const char* filename, size_t* size) {
  off_t const buffSize = fsize_or_raise(filename);
  FILE* const inFile = fopen_or_raise(filename, "rb");
  void* const buffer = malloc_or_raise(buffSize);
  size_t const readSize = fread(buffer, 1, buffSize, inFile);
  if (readSize != (size_t)buffSize) {
    rb_raise(rb_eRuntimeError, "failed to read file: %s", filename);
  }
  fclose(inFile);
  *size = buffSize;
  return buffer;
}

static ZSTD_CDict* createCDict_or_raise(const char* dictFileName, int cLevel) {
  size_t dictSize;
  void* const dictBuffer = load_file_or_raise(dictFileName, &dictSize);
  ZSTD_CDict* const cdict = ZSTD_createCDict(dictBuffer, dictSize, cLevel);
  if (!cdict) {
    rb_raise(rb_eRuntimeError, "failed to create dictionary: %s", dictFileName);
  }
  free(dictBuffer);
  return cdict;
}

static ZSTD_DDict* createDDict_or_raise(const char* dictFileName) {
  size_t dictSize;
  void* const dictBuffer = load_file_or_raise(dictFileName, &dictSize);
  ZSTD_DDict* const ddict = ZSTD_createDDict(dictBuffer, dictSize);
  if (!ddict) {
    rb_raise(rb_eRuntimeError, "failed to create dictionary: %s", dictFileName);
  }
  free(dictBuffer);
  return ddict;
}

static VALUE compress(int argc, VALUE *argv, VALUE self) {
  VALUE src_buf, dst_buf, comp_level_arg;
  const char* src_ptr;
  char* dst_ptr;
  size_t src_size, dst_size, res;
  int comp_level;

  rb_scan_args(argc, argv, "11", &src_buf, &comp_level_arg);
  
  Check_Type(src_buf, RUBY_T_STRING);
  src_ptr = RSTRING_PTR(src_buf);
  src_size = RSTRING_LEN(src_buf);
  
  dst_size = ZSTD_compressBound(src_size);
  dst_buf = rb_str_new(NULL, dst_size);
  dst_ptr = RSTRING_PTR(dst_buf);

  comp_level = 3;
  if (!NIL_P(comp_level_arg)) {
    Check_Type(comp_level_arg, RUBY_T_FIXNUM);
    comp_level = FIX2INT(comp_level_arg);    
  }

  struct dic_struct *ds;
  Data_Get_Struct(self, struct dic_struct, ds);

  if (ds->cdic == NULL) {
    res = ZSTD_compress((void*)dst_ptr, dst_size, (const void*)src_ptr, src_size, comp_level);
  } else {    
    res = ZSTD_compress_usingCDict(ds->cctx, (void*)dst_ptr, dst_size, (const void*)src_ptr, src_size, ds->cdic);
  }

  if (ZSTD_isError(res)) {
    rb_raise(rb_eRuntimeError, "error compressing: %s", ZSTD_getErrorName(res));
  } else {
    rb_str_resize(dst_buf, res);
  }

  return dst_buf;
}

static VALUE decompress(int argc, VALUE *argv, VALUE self) {
  VALUE src_buf, dst_buf;
  const char* src_ptr;
  char* dst_ptr;
  size_t src_size, dst_size, res;
  
  rb_scan_args(argc, argv, "10", &src_buf);

  Check_Type(src_buf, RUBY_T_STRING);
  src_ptr = RSTRING_PTR(src_buf);
  src_size = RSTRING_LEN(src_buf);

  dst_size = ZSTD_getDecompressedSize(src_ptr, src_size);
  dst_buf = rb_str_new(NULL, dst_size);
  dst_ptr = RSTRING_PTR(dst_buf);

  struct dic_struct *ds;
  Data_Get_Struct(self, struct dic_struct, ds);

  if (ds->ddic == NULL) {
    res = ZSTD_decompress((void*)dst_ptr, dst_size, (const void*)src_ptr, src_size);
  } else {
    res = ZSTD_decompress_usingDDict(ds->dctx, (void*)dst_ptr, dst_size, (const void*)src_ptr, src_size, ds->ddic);
  }

  if (ZSTD_isError(res)) {
    rb_raise(rb_eRuntimeError, "error decompressing: %s", ZSTD_getErrorName(res));
  } else {
    rb_str_resize(dst_buf, res);
  }

  return dst_buf;  
}

static void deallocate(void *ds) {
  if ((((struct dic_struct *)ds)->cctx) != NULL) {
    ZSTD_freeCCtx(((struct dic_struct *)ds)->cctx);
    ZSTD_freeCDict(((struct dic_struct *)ds)->cdic);
    ZSTD_freeDCtx(((struct dic_struct *)ds)->dctx);
    ZSTD_freeDDict(((struct dic_struct *)ds)->ddic);
  }
  //xfree(ds); // TODO: verify
}

static VALUE allocate(VALUE claes) {
  struct dic_struct *ds;
  ds = ALLOC(struct dic_struct);
  return Data_Wrap_Struct(claes, NULL, deallocate, ds);
}

static VALUE initialize(int argc, VALUE *argv, VALUE self) {
  VALUE dic_arg;
  struct dic_struct *ds;
  Data_Get_Struct(self, struct dic_struct, ds);
  ds->cdic = NULL;
  ds->cctx = NULL;
  ds->ddic = NULL;
  ds->dctx = NULL;
  
  rb_scan_args(argc, argv, "01", &dic_arg);

  if (!NIL_P(dic_arg)) {
    Check_Type(dic_arg, RUBY_T_STRING);    
    const char* dic_ptr = RSTRING_PTR(dic_arg);
    ds->cdic = createCDict_or_raise(dic_ptr, 19);
    ds->cctx = ZSTD_createCCtx();
    ds->ddic = createDDict_or_raise(dic_ptr);
    ds->dctx = ZSTD_createDCtx();
  }

  return self;
}

void Init_zstd(void) {
  VALUE rb_cZstd = rb_define_class("Zstd", rb_cObject);

  rb_define_alloc_func(rb_cZstd, allocate);
  rb_define_method(rb_cZstd, "initialize", initialize, -1);
  
  rb_define_method(rb_cZstd, "compress", compress, -1);
  rb_define_method(rb_cZstd, "decompress", decompress, -1);
}
