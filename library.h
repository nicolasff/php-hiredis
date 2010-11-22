#ifndef PHP_HIREDIS_LIBRARY_H
#define PHP_HIREDIS_LIBRARY_H

/* new readers */
PHPAPI int redis_reply_string(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args);
PHPAPI int redis_reply_status(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args);
PHPAPI int redis_reply_status_as_string(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args);
PHPAPI int redis_reply_long(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args);
PHPAPI int redis_reply_zip(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args);
PHPAPI int redis_reply_zip_closure(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args);
PHPAPI int redis_reply_long_as_bool(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args);

PHPAPI void redis_varg_run(INTERNAL_FUNCTION_PARAMETERS, char *keyword, void *fun, int keep_args);
PHPAPI void redis_enqueue(RedisSock *redis_sock, void *fun, zval **z_args);

#endif

