#ifndef PHP_HIREDIS_LIBRARY_H
#define PHP_HIREDIS_LIBRARY_H

typedef int (*validator_fun)(zval *, zval **);

/* new readers */
int redis_reply_string_(zval *z, zval **z_args);
int redis_reply_status_(zval *z, zval **z_args);
int redis_reply_array_(zval *z, zval **z_args);
int redis_reply_skip_(zval *z, zval **z_args);
int redis_reply_long_(zval *z, zval **z_args);
int redis_reply_long_as_bool_(zval *z, zval **z_args);
int redis_reply_zip_(zval *z, zval **z_args);
int redis_reply_zip_closure_(zval *z, zval **z_args);

PHPAPI void redis_varg_run(INTERNAL_FUNCTION_PARAMETERS, char *keyword, validator_fun fun, int keep_args);
PHPAPI void redis_enqueue(RedisSock *redis_sock, validator_fun fun, zval **z_args);

#endif

