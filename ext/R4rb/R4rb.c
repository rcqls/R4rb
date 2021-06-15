/**********************************************************************

  R4rb.c

**********************************************************************/
#include <stdio.h>
#include <string.h>

#include "ruby.h"
#include "ruby/version.h"
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include <Rversion.h>


/* From Parse.h -- must find better solution: */
#define PARSE_NULL              0
#define PARSE_OK                1
#define PARSE_INCOMPLETE        2
#define PARSE_ERROR             3
#define PARSE_EOF               4


#define Need_Integer(x) (x) = rb_Integer(x)
#define Need_Float(x) (x) = rb_Float(x)
#define Need_Float2(x,y) {\
    Need_Float(x);\
    Need_Float(y);\
}
#define Need_Float3(x,y,z) {\
    Need_Float(x);\
    Need_Float(y);\
    Need_Float(z);\
}

#if (R_VERSION < 132352) /* before 2.5 to check!*/
SEXP R_ParseVector(SEXP, int, int *);
#define RR_ParseVector(x,y,z) R_ParseVector(x, y, z)
#else
SEXP R_ParseVector(SEXP, int, int *,SEXP);
#define RR_ParseVector(x,y,z) R_ParseVector(x, y, z, R_NilValue)
#endif

/************* INIT *********************/

extern Rboolean R_Interactive;
extern int Rf_initEmbeddedR(int argc, char *argv[]);

VALUE R2rb_init(VALUE obj, VALUE args)
{
  char **argv;//={"REmbed","--save","--slave","--quiet"};
  int i,argc;//=sizeof(argv)/sizeof(argv[0]);
  VALUE tmp;

  argc=RARRAY_LEN(args) + 1;
  //printf("argc=%d\n",argc);
  argv=malloc(sizeof(char*)*argc);
  argv[0]="REmbed";
  for (i = 1 ; i < argc ; i++) {
    tmp=rb_ary_entry(args,i-1);
    argv[i]=StringValuePtr(tmp);
    //printf("argv[%d]=%s\n",i,argv[i]);
  }
  //printf("argc=%d\n",argc);
  Rf_initEmbeddedR(argc,argv);
  R_Interactive = FALSE;
  return Qtrue;
}

/***************** EVAL **********************/

VALUE R2rb_eval(VALUE obj, VALUE cmd, VALUE print)
{
  char *cmdString;
  int nbCmds;
  VALUE tmp;
  int errorOccurred,status, i;

  SEXP text, expr, ans=R_NilValue /* -Wall */;


  //printf("Avant parsing\n");

  nbCmds=RARRAY_LEN(cmd);

  //printf("nbCmds : %d\n",nbCmds);

  text = PROTECT(allocVector(STRSXP, nbCmds));
  for (i = 0 ; i < nbCmds ; i++) {
    tmp=rb_ary_entry(cmd,i);
    cmdString=StringValuePtr(tmp);
    SET_STRING_ELT(text, i, mkChar(cmdString));
  }
  expr = PROTECT(RR_ParseVector(text, -1, &status));

  if (status != PARSE_OK) {
    //printf("Parsing error (status=%d) in:\n",status);
    for (i = 0 ; i < nbCmds ; i++) {
      tmp=rb_ary_entry(cmd,i);
      cmdString=StringValuePtr(tmp);
      //printf("%s\n",cmdString);
    }
    UNPROTECT(2);
    return Qfalse;
  }
  
  /* Note that expr becomes an EXPRSXP and hence we need the loop
     below (a straight eval(expr, R_GlobalEnv) won't work) */
  {
    for(i = 0 ; i < nbCmds ; i++)
      ans = R_tryEval(VECTOR_ELT(expr, i),NULL, &errorOccurred);
      if(errorOccurred) {
        //fprintf(stderr, "Caught another error calling sqrt()\n");
        fflush(stderr);
        UNPROTECT(2);
        return Qfalse;
      }

      if (print != Qnil) {
        Rf_PrintValue(ans);
      }
  }

  UNPROTECT(2);
  return Qtrue;
}

/***************** PARSE **********************/

VALUE R2rb_parse(VALUE obj, VALUE cmd,VALUE print)
{
  char *cmdString;
  int nbCmds;
  VALUE tmp;
  int status,i;

  SEXP text, expr, ans=R_NilValue /* -Wall */;


  //printf("Avant parsing\n");

  nbCmds=RARRAY_LEN(cmd);

  //printf("nbCmds : %d\n",nbCmds);

  text = PROTECT(allocVector(STRSXP, nbCmds));
  for (i = 0 ; i < nbCmds ; i++) {
    tmp=rb_ary_entry(cmd,i);
    cmdString=StringValuePtr(tmp);
    SET_STRING_ELT(text, i, mkChar(cmdString));
  }
  expr = PROTECT(RR_ParseVector(text, -1, &status));

  if (status != PARSE_OK) {
    if (print != Qnil) printf("Parsing error (status=%d) in:\n",status);
    for (i = 0 ; i < nbCmds ; i++) {
      tmp=rb_ary_entry(cmd,i);
      cmdString=StringValuePtr(tmp);
      if (print != Qnil) printf("%s\n",cmdString);
    }
    //UNPROTECT(2);
    //return Qfalse;
  }
  UNPROTECT(2);
  //return Qtrue;
  return INT2FIX(status);
}


/*****************************************

Interface to get values of RObj from Ruby
The basic idea : no copy of the R Vector
just methods to extract value !!!

******************************************/

// used internally !!! -> eval only one string line
SEXP util_eval1string(VALUE cmd)
{
  char *cmdString;
  int  errorOccurred,status, i;
    
  SEXP text, expr, ans=R_NilValue /* -Wall */;

  text = PROTECT(allocVector(STRSXP, 1)); 
  cmdString=StringValuePtr(cmd);
//printf("cmd: %s\n",cmdString);
  SET_STRING_ELT(text, 0, mkChar(cmdString));
  expr = PROTECT(RR_ParseVector(text, -1, &status));
  if (status != PARSE_OK) {
    printf("Parsing error in: %s\n",cmdString);
    UNPROTECT(2);
    return R_NilValue;
  }
  /* Note that expr becomes an EXPRSXP and hence we need the loop
     below (a straight eval(expr, R_GlobalEnv) won't work) */
  ans = R_tryEval(VECTOR_ELT(expr, 0),R_GlobalEnv,&errorOccurred);
  //ans = eval(VECTOR_ELT(expr, 0),R_GlobalEnv);
  if(errorOccurred) {
    //fflush(stderr);
    printf("Exec error in: %s\n",cmdString);
    UNPROTECT(2);
    return R_NilValue;
  }
  UNPROTECT(2);
  return ans;
}

int util_isVector(SEXP ans)
{
  return ((!isNewList(ans)) & isVector(ans));
}

int util_isVariable(VALUE self)
{
  VALUE tmp;
  tmp=rb_iv_get(self,"@type");
  return strcmp(StringValuePtr(tmp),"var")==0;
}

SEXP util_getVar(VALUE self)
{
  SEXP ans;
  char *name;
  VALUE tmp;

  tmp=rb_iv_get(self,"@name");
  name=StringValuePtr(tmp);
  if(util_isVariable(self)) {
    ans = findVar(install(name),R_GlobalEnv); //currently in  R_GlobalEnv!!!
  } else {
    //printf("getVar:%s\n",name);
    ans=util_eval1string(rb_iv_get(self,"@name"));
    if(ans==R_NilValue) return ans;
  }
  if(!util_isVector(ans)) return R_NilValue;
  return ans;
}

//with argument!! necessarily an expression and not a variable
SEXP util_getExpr_with_arg(VALUE self)
{
  SEXP ans;
  VALUE tmp;

  //printf("getVar:%s\n",name);
  tmp=rb_str_dup(rb_iv_get(self,"@arg"));
  ans=util_eval1string(rb_str_cat2(rb_str_dup(rb_iv_get(self,"@name")),StringValuePtr(tmp)));
  if(ans==R_NilValue) return ans;
  if(!util_isVector(ans)) return R_NilValue;
  return ans;
}


VALUE util_SEXP2VALUE(SEXP ans)
{
  VALUE res;
  int n,i;
  Rcomplex cpl;
  VALUE res2; 
  
  n=length(ans);
  res = rb_ary_new2(n);
  switch(TYPEOF(ans)) {
  case REALSXP:
    for(i=0;i<n;i++) {
      rb_ary_store(res,i,rb_float_new(REAL(ans)[i]));
    }
    break;
  case INTSXP:
    for(i=0;i<n;i++) {
      rb_ary_store(res,i,INT2FIX(INTEGER(ans)[i]));
    }
    break;
  case LGLSXP:
    for(i=0;i<n;i++) {
      rb_ary_store(res,i,(INTEGER(ans)[i] ? Qtrue : Qfalse));
    }
    break;
  case STRSXP:
    for(i=0;i<n;i++) {
      rb_ary_store(res,i,rb_str_new2(CHAR(STRING_ELT(ans,i))));
    }
    break;
  case CPLXSXP:
    rb_require("complex");
    for(i=0;i<n;i++) {
      cpl=COMPLEX(ans)[i];
      res2 = rb_eval_string("Complex.new(0,0)");
      rb_iv_set(res2,"@real",rb_float_new(cpl.r));
      rb_iv_set(res2,"@image",rb_float_new(cpl.i));
      rb_ary_store(res,i,res2);
    }
    break;
  }

  return res;
}


SEXP util_VALUE2SEXP(VALUE arr)
{
  SEXP ans;
  VALUE res,class,tmp;
  int i,n=0;

  if(!rb_obj_is_kind_of(arr,rb_cArray)) {
    n=1;
    res = rb_ary_new2(1);
    rb_ary_push(res,arr);
    arr=res;
  } else {
    n=RARRAY_LEN(arr);
  }  

  class=rb_class_of(rb_ary_entry(arr,0));
  
  if(class==rb_cFloat) {
    PROTECT(ans=allocVector(REALSXP,n));
    for(i=0;i<n;i++) {
      REAL(ans)[i]=NUM2DBL(rb_ary_entry(arr,i));
    }
#if RUBY_API_VERSION_CODE >= 20400
  } else if(class==rb_cInteger) {
#else
  } else if(class==rb_cFixnum || class==rb_cBignum) {
#endif
    PROTECT(ans=allocVector(INTSXP,n));
    for(i=0;i<n;i++) {
      INTEGER(ans)[i]=NUM2INT(rb_ary_entry(arr,i));
    }
  } else if(class==rb_cTrueClass || class==rb_cFalseClass) {
    PROTECT(ans=allocVector(LGLSXP,n));
    for(i=0;i<n;i++) {
      LOGICAL(ans)[i]=(rb_class_of(rb_ary_entry(arr,i))==rb_cFalseClass ? FALSE : TRUE);
    }
  } else if(class==rb_cString) {
    PROTECT(ans=allocVector(STRSXP,n));
    for(i=0;i<n;i++) {
      tmp=rb_ary_entry(arr,i);
      SET_STRING_ELT(ans,i,mkChar(StringValuePtr(tmp)));
    }
  } else ans=R_NilValue;

  if(n>0) UNPROTECT(1);
  return ans; 
}



VALUE RVect_initialize(VALUE self, VALUE name)
{
  rb_iv_set(self,"@name",name);
  rb_iv_set(self,"@type",rb_str_new2("var"));
  rb_iv_set(self,"@arg",rb_str_new2(""));
  return self;
}

VALUE RVect_isValid(VALUE self)
{
  SEXP ans;
  char *name;

#ifdef cqls
  VALUE tmp;
  tmp=rb_iv_get(self,"@name");
  name = StringValuePtr(tmp);
  ans = findVar(install(name),R_GlobalEnv); //currently in  R_GlobalEnv!!!
#else
  ans = util_getVar(self);
#endif
  if(!util_isVector(ans)) {
#ifndef cqls
    VALUE tmp;
    tmp=rb_iv_get(self,"@name");
    name = StringValuePtr(tmp);
#endif
    rb_warn("%s is not a R vector !!!",name); //TODO name not defined
    return Qfalse;
  }
  return Qtrue;
}

VALUE RVect_length(VALUE self)
{
  SEXP ans;
  char *name;
#ifdef cqls
  VALUE tmp;
  tmp=rb_iv_get(self,"@name");
  if(!RVect_isValid(self)) return Qnil;
  name = StringValuePtr(tmp);
  ans = findVar(install(name),R_GlobalEnv); //currently in  R_GlobalEnv!!!
#else
  ans = util_getVar(self);

  if(ans==R_NilValue) {
    //printf("Sortie de length avec nil\n");
    return Qnil;
  }
#endif
  return INT2NUM(length(ans));
}

VALUE RVect_get(VALUE self)
{
  SEXP ans;
  VALUE res;
  char *name;
  int n,i;
  Rcomplex cpl;
  VALUE res2; 

  //#define cqls
#ifdef cqls 
  VALUE tmp;
  if(!RVect_isValid(self)) return Qnil;
#else  
  ans = util_getVar(self);

  if(ans==R_NilValue) {
    //printf("Sortie de get avec nil\n");
    return Qnil;
  }
#endif
#ifdef cqls 
  tmp=rb_iv_get(self,"@name");
  name = StringValuePtr(tmp);
  ans = findVar(install(name),R_GlobalEnv); 
#endif

  res=util_SEXP2VALUE(ans);
  if(length(ans)==1) res=rb_ary_entry(res,0);
  return res; 
}

VALUE RVect_get_with_arg(VALUE self)
{
  SEXP ans;
  VALUE res;
  char *name;
  int n,i;
  Rcomplex cpl;
  VALUE res2; 

  ans = util_getExpr_with_arg(self);
 
  if(ans==R_NilValue) {
    //printf("Sortie de get avec nil\n");
    return Qnil;
  }
  res=util_SEXP2VALUE(ans);
 
//printf("RVect_get_with_arg: length(ans)=%d\n",length(ans));
 if (length(ans)==1) res=rb_ary_entry(res,0);

  return res;
}



// faster than self.to_a[index]
VALUE RVect_aref(VALUE self, VALUE index)
{
  SEXP ans;
  VALUE res;
  char *name;
  int n,i;
  Rcomplex cpl;
#ifdef cqls
  VALUE tmp;
#endif
  i = FIX2INT(index);
  
#ifdef cqls
  if(!RVect_isValid(self)) return Qnil;
  tmp=rb_iv_get(self,"@name");
  name = StringValuePtr(tmp);
  ans = findVar(install(name),R_GlobalEnv); //currently in  R_GlobalEnv!!!
#else
  ans = util_getVar(self);
#endif
  n=length(ans);
  //printf("i=%d and n=%d\n",i,n);
  if(i<n) {
    switch(TYPEOF(ans)) {
    case REALSXP:
      res=rb_float_new(REAL(ans)[i]);
      break;
    case INTSXP:
      res=INT2FIX(INTEGER(ans)[i]);
      break;
    case LGLSXP:
      res=(INTEGER(ans)[i] ? Qtrue : Qfalse);
      break;
    case STRSXP:
      res=rb_str_new2(CHAR(STRING_ELT(ans,i)));
      break;
    case CPLXSXP:
      rb_require("complex");
      cpl=COMPLEX(ans)[i];
      res = rb_eval_string("Complex.new(0,0)");
      rb_iv_set(res,"@real",rb_float_new(cpl.r));
      rb_iv_set(res,"@image",rb_float_new(cpl.i));
      break;
    }
  } else {
    res = Qnil;
  }
  return res;
}

VALUE RVect_set(VALUE self,VALUE arr)
{
  SEXP ans;
  char *name;
  VALUE tmp;

  ans=util_VALUE2SEXP(arr);
  
  tmp=rb_iv_get(self,"@name");
  name = StringValuePtr(tmp);
  if(util_isVariable(self)) {
    defineVar(install(name),ans,R_GlobalEnv); //currently in R_GlobalEnv!!!
  } else {
    defineVar(install(".rubyExport"),ans,R_GlobalEnv);
    util_eval1string(rb_str_cat2(rb_str_dup(rb_iv_get(self,"@name")),"<-.rubyExport"));
  }

  return self; 
}

VALUE RVect_assign(VALUE obj, VALUE name,VALUE arr)
{
  SEXP ans;
  char *tmp;

  ans=util_VALUE2SEXP(arr);

  tmp = StringValuePtr(name);
  defineVar(install(tmp),ans,R_GlobalEnv);

  return Qnil; 
}

VALUE RVect_set_with_arg(VALUE self,VALUE arr)
{
  VALUE tmp;
  defineVar(install(".rubyExport"),util_VALUE2SEXP(arr),R_GlobalEnv);
  tmp=rb_iv_get(self,"@arg"); 
  util_eval1string(rb_str_cat2(rb_str_cat2(rb_str_dup(rb_iv_get(self,"@name")),StringValuePtr(tmp)),"<-.rubyExport"));
  return self;
}



void
Init_R4rb()
{
  VALUE mR2rb;

  mR2rb = rb_define_module("R2rb");

  rb_define_module_function(mR2rb, "initR", R2rb_init, 1);

  rb_define_module_function(mR2rb, "evalLines", R2rb_eval, 2);

  rb_define_module_function(mR2rb, "parseLines", R2rb_parse, 2);

  VALUE cRVect;

  cRVect = rb_define_class_under(mR2rb,"RVector",rb_cObject);

  rb_define_module_function(cRVect, "assign", RVect_assign, 2);

  rb_define_method(cRVect,"initialize",RVect_initialize,1);

  rb_define_method(cRVect,"get",RVect_get,0);
  rb_define_alias(cRVect,"to_a","get");
  rb_define_alias(cRVect,"value","get");

  rb_define_method(cRVect,"set",RVect_set,1);
  rb_define_alias(cRVect,"<","set");
  rb_define_alias(cRVect,"value=","set");

  //method "arg=" defined in eval.rb!! @arg initialized in method "initialize"
  rb_define_method(cRVect,"get_with_arg",RVect_get_with_arg,0);
  rb_define_alias(cRVect,"value_with_arg","get_with_arg");
  rb_define_method(cRVect,"set_with_arg",RVect_set_with_arg,1);
  rb_define_alias(cRVect,"value_with_arg=","set_with_arg");

  rb_define_method(cRVect,"valid?",RVect_isValid,0);
  rb_define_method(cRVect,"length",RVect_length,0);
  rb_define_method(cRVect,"[]",RVect_aref,1);
  //[]= iter !!!
  rb_define_attr(cRVect,"name",1,1);
  rb_define_attr(cRVect,"type",1,1);
}
