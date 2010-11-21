
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_redis.h"
#include "macros.h"
#include "library.h"

#ifdef POSIX
/* setsockopt */
#include <sys/types.h>
#include <netinet/tcp.h>  /* TCP_NODELAY */
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <zend_exceptions.h>

PHPAPI int
redis_reply_string(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args) {

    if(Z_TYPE_P(z_reply) == IS_STRING) { /* valid */
	    if(mode == REDIS_MODE_BLOCKING) { /* copy directly into return_value */
		    Z_TYPE_P(return_value) = IS_STRING;
		    Z_STRVAL_P(return_value) = Z_STRVAL_P(z_reply);
		    Z_STRLEN_P(return_value) = Z_STRLEN_P(z_reply);
	    } else { /* append */
		    add_next_index_stringl(return_value, Z_STRVAL_P(z_reply), Z_STRLEN_P(z_reply), 0);
	    }
            efree(z_reply);
            return 0;
    } else { /* invalid */
            zval_dtor(z_reply);
            efree(z_reply);
	    if(mode == REDIS_MODE_BLOCKING) { /* return false */
            	ZVAL_BOOL(return_value, 0);
	    } else { /* append false. */
	        add_next_index_bool(return_value, 0);
	    }
            return 1;
    }
}


PHPAPI int
redis_reply_long(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args) {
    if(Z_TYPE_P(z_reply) == IS_LONG) { /* valid */
	    if(mode == REDIS_MODE_BLOCKING) { /* copy directly into return_value */
		    ZVAL_LONG(return_value, Z_LVAL_P(z_reply));
	    } else { /* append long */
		    add_next_index_long(return_value, Z_LVAL_P(z_reply));
	    }
            efree(z_reply);
            return 0;
    } else {
            zval_dtor(z_reply);
            efree(z_reply);
	    if(mode == REDIS_MODE_BLOCKING) { /* return false */
            	ZVAL_BOOL(return_value, 0);
	    } else { /* append false. */
	        add_next_index_bool(return_value, 0);
	    }
            return 1;
    }
}

PHPAPI int
redis_reply_status(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args) {
        int success = 0;
        if(z_reply && Z_TYPE_P(z_reply) == IS_BOOL) {
                success = Z_BVAL_P(z_reply);
        }

        zval_dtor(z_reply);
        efree(z_reply);

	if(mode == REDIS_MODE_BLOCKING) { /* return bool directly */
                ZVAL_BOOL(return_value, success);
        } else {
                add_next_index_bool(return_value, success); /* append bool */
        }
        return 0;
}

PHPAPI int
redis_reply_zip(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args) {

        zval *z_ret;
        MAKE_STD_ZVAL(z_ret);

        int use_atof = 0; /* FIXME */

        if(Z_TYPE_P(z_reply) != IS_ARRAY) {
                ZVAL_NULL(z_ret);
        } else {
            array_init(z_ret);

            HashTable *keytable = Z_ARRVAL_P(z_reply);

            for(zend_hash_internal_pointer_reset(keytable);
                            zend_hash_has_more_elements(keytable) == SUCCESS;
                            zend_hash_move_forward(keytable)) {

                    char *tablekey, *hkey, *hval;
                    int tablekey_len, hkey_len, hval_len;
                    unsigned long idx;
                    int type;
                    zval **z_value_pp;

                    type = zend_hash_get_current_key_ex(keytable, &tablekey, &tablekey_len, &idx, 0, NULL);
                    if(zend_hash_get_current_data(keytable, (void**)&z_value_pp) == FAILURE) {
                            continue; 	/* this should never happen, according to the PHP people. */
                    }

                    /* get current value, a key */
                    hkey = Z_STRVAL_PP(z_value_pp);
                    hkey_len = Z_STRLEN_PP(z_value_pp);

                    /* move forward */
                    zend_hash_move_forward(keytable);

                    /* fetch again */
                    type = zend_hash_get_current_key_ex(keytable, &tablekey, &tablekey_len, &idx, 0, NULL);
                    if(zend_hash_get_current_data(keytable, (void**)&z_value_pp) == FAILURE) {
                            continue; 	/* this should never happen, according to the PHP people. */
                    }

                    /* get current value, a hash value now. */
                    hval = Z_STRVAL_PP(z_value_pp);
                    hval_len = Z_STRLEN_PP(z_value_pp);

                    if(use_atof) {
                            add_assoc_double_ex(z_ret, hkey, 1+hkey_len, atof(hval));
                    } else {
                            add_assoc_stringl_ex(z_ret, hkey, 1+hkey_len, hval, hval_len, 1);
                    }
            }

        }
	if(mode == REDIS_MODE_BLOCKING) { /* copy z_ret into return_value directly */
		*return_value = *z_ret;
		zval_copy_ctor(return_value);
		zval_dtor(z_ret);
		efree(z_ret);
	} else { /* append z_ret to return_value array */
		add_next_index_zval(return_value, z_ret);
	}

        return 0;
}

PHPAPI int
redis_reply_zip_closure(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args) {

	int i;

        zval *z_ret;
        MAKE_STD_ZVAL(z_ret);

        if(Z_TYPE_P(z_reply) != IS_ARRAY) {
                ZVAL_NULL(z_ret);
        } else {
            array_init(z_ret);

            HashTable *ht_vals = Z_ARRVAL_P(z_reply);

	    /* zip together z_args as keys, z_reply as values */

            for(i = 1, zend_hash_internal_pointer_reset(ht_vals);
                            zend_hash_has_more_elements(ht_vals) == SUCCESS;
                            i++, zend_hash_move_forward(ht_vals)) {

                    zval **z_val_pp;

                    if(zend_hash_get_current_data(ht_vals, (void**)&z_val_pp) == FAILURE) {
                            continue; 	/* this should never happen, according to the PHP people. */
                    }

                    add_assoc_stringl_ex(z_ret, Z_STRVAL_P(z_args[i]), 1+Z_STRLEN_P(z_args[i]), Z_STRVAL_PP(z_val_pp), Z_STRLEN_PP(z_val_pp), 1);
            }
        }

	if(mode == REDIS_MODE_BLOCKING) { /* copy z_ret into return_value directly */
		*return_value = *z_ret;
		zval_copy_ctor(return_value);
		zval_dtor(z_ret);
		efree(z_ret);
	} else { /* append z_ret to return_value array */
		add_next_index_zval(return_value, z_ret);
	}

	efree(z_args);

        return 0;
}

PHPAPI void
redis_enqueue(RedisSock *redis_sock, void *fun, zval **z_args) {

	redis_command *c = ecalloc(1, sizeof(redis_command));
	c->fun = fun;

	if(z_args) {
		c->z_args = z_args;
	}

	/* enqueue */
	if(redis_sock->queue_tail == NULL) {
		redis_sock->queue_tail = redis_sock->queue_head = c;
	} else {
		redis_sock->queue_tail->next = c;
	}
	redis_sock->enqueued_commands++;
}



PHPAPI void
redis_varg_run(INTERNAL_FUNCTION_PARAMETERS, char *keyword, void *fun, int keep_args) {


    zval *object = getThis();
    RedisSock *redis_sock;
    int argc = ZEND_NUM_ARGS(), i;

    const char **args;
    size_t *arglen;

    zval *z_reply;

    /* get all args into the zval array z_args */
    zval **z_args = emalloc(argc * sizeof(zval*));
    if(zend_get_parameters_array(ht, argc, z_args) == FAILURE) {
        efree(z_args);
        RETURN_FALSE;
    }

    /* check if there is only one argument, an array */
    if(argc == 2 && Z_TYPE_P(z_args[1]) == IS_ARRAY) {
	    zval *z_key = z_args[0];
	    zval *z_array = z_args[1];
	    efree(z_args);
	    argc = zend_hash_num_elements(Z_ARRVAL_P(z_array));
	    z_args = ecalloc(argc, sizeof(zval*));

	    MAKE_STD_ZVAL(z_args[0]);
	    *z_args[0] = *z_key;
	    zval_copy_ctor(z_args[0]);

	    for(i = 1; i <= argc; ++i) {
		    zval **z_i_pp;
		    if(zend_hash_index_find(Z_ARRVAL_P(z_array), i-1, (void **)&z_i_pp) == FAILURE) {
			    efree(z_args);
			    RETURN_FALSE;
		    }
		    MAKE_STD_ZVAL(z_args[i]);
		    *z_args[i] = **z_i_pp;
		    zval_copy_ctor(z_args[i]);
	    }
    }

    /* copy all args as strings */
    args = emalloc((argc+1) * sizeof(char*));
    arglen = emalloc((argc+1) * sizeof(size_t));
    for(i = 0; i < argc; ++i) {
        convert_to_string(z_args[i]);
        args[i+1] = Z_STRVAL_P(z_args[i]);
        arglen[i+1] = (size_t)Z_STRLEN_P(z_args[i]);
    }

    args[0] = keyword;
    arglen[0] = strlen(keyword);

    REDIS_SOCK_GET(redis_sock);
    if(keep_args) {
	    REDIS_RUN_ARGS(redis_sock, fun, argc+1, args, arglen, z_args);
    } else {
	    REDIS_RUN_ARGS(redis_sock, fun, argc+1, args, arglen, NULL);
    }
}

PHPAPI int redis_reply_long_as_bool(zval *return_value, redis_mode mode, zval *z_reply, zval **z_args) {

    if(Z_TYPE_P(z_reply) == IS_LONG) { /* valid */
	    if(mode == REDIS_MODE_BLOCKING) { /* copy directly into return_value */
		    ZVAL_BOOL(return_value, Z_LVAL_P(z_reply) == 1 ? 1 : 0);
	    } else { /* append long */
		    add_next_index_bool(return_value, Z_LVAL_P(z_reply) == 1 ? 1 : 0);
	    }
            efree(z_reply);
            return 0;
    } else {
            zval_dtor(z_reply);
            efree(z_reply);
	    if(mode == REDIS_MODE_BLOCKING) { /* return false */
            	ZVAL_BOOL(return_value, 0);
	    } else { /* append false. */
	        add_next_index_bool(return_value, 0);
	    }
            return 1;
    }
}
