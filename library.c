
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

int
redis_reply_string_(zval *z, zval **z_args) {

    if(Z_TYPE_P(z) != IS_STRING) { /* invalid */
	    zval_dtor(z);
	    ZVAL_BOOL(z, 0);
    }
    return 0;
}


int
redis_reply_long_(zval *z, zval **z_args) {
	if(Z_TYPE_P(z) != IS_LONG) { /* invalid */
		zval_dtor(z);
		ZVAL_BOOL(z, 0);
	}
    return 0;
}

int
redis_reply_skip_(zval *z, zval **z_args) {
	// php_printf("SKIP THIS\n");
	return 1;
}


int
redis_reply_status_(zval *z, zval **z_args) {
        if(Z_TYPE_P(z) != IS_BOOL) {
		zval_dtor(z);
		ZVAL_BOOL(z, 0);
	}
	return 0;
}


int
redis_reply_array_(zval *z, zval **z_args) {
	if(Z_TYPE_P(z) != IS_ARRAY) { /* invalid */
		zval_dtor(z);
		ZVAL_BOOL(z, 0);
	}
	return 0;
}

int
redis_reply_zip_(zval *z, zval **z_args) {

	if(Z_TYPE_P(z) != IS_ARRAY) { /* invalid */
		zval_dtor(z);
		ZVAL_BOOL(z, 0);
		return 0;
	}

        zval *z_ret;
        MAKE_STD_ZVAL(z_ret);
	array_init(z_ret);

	HashTable *keytable = Z_ARRVAL_P(z);

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

		add_assoc_stringl_ex(z_ret, hkey, 1+hkey_len, hval, hval_len, 1);
	}
	/* copy back into z */
	zval_dtor(z);
	*z = *z_ret;
	zval_copy_ctor(z);
	zval_dtor(z_ret);
	efree(z_ret);

	return 0;

}

int
redis_reply_zip_closure_(zval *z, zval **z_args) {

	int i;

        zval *z_ret;
        MAKE_STD_ZVAL(z_ret);

        if(Z_TYPE_P(z) != IS_ARRAY) {
                ZVAL_NULL(z_ret);
        } else {
            array_init(z_ret);

            HashTable *ht_vals = Z_ARRVAL_P(z);

	    /* zip together z_args as keys, z_reply as values */

            for(i = 1, zend_hash_internal_pointer_reset(ht_vals);
                            zend_hash_has_more_elements(ht_vals) == SUCCESS;
                            i++, zend_hash_move_forward(ht_vals)) {

                    zval **z_val_pp;

                    if(zend_hash_get_current_data(ht_vals, (void**)&z_val_pp) == FAILURE) {
                            continue; 	/* this should never happen, according to the PHP people. */
                    }

		    zval *z_copy;
		    MAKE_STD_ZVAL(z_copy);
		    *z_copy = **z_val_pp;
		    zval_copy_ctor(z_copy);

                    add_assoc_zval_ex(z_ret, Z_STRVAL_P(z_args[i]), 1+Z_STRLEN_P(z_args[i]), z_copy);
            }
        }

	/* copy back into z directly */
	zval_dtor(z);
	*z = *z_ret;
	zval_copy_ctor(z);
	zval_dtor(z_ret);
	efree(z_ret);

	efree(z_args);

        return 0;
}

static void
redis_enqueue_(redis_command **h, redis_command **t, redis_command *c) {

	if(*t == NULL) {
		*t = c;
		*h = c;
	} else {
		(*t)->next = c;
		*t = c;
	}

}

PHPAPI void
redis_enqueue(RedisSock *redis_sock, validator_fun fun, zval **z_args) {

	redis_command *c = ecalloc(1, sizeof(redis_command));
	c->fun = fun;

	if(z_args) {
		c->z_args = z_args;
	}

	if(redis_sock->multi) {
		/* add skip to the main queue */
		redis_command *skip = ecalloc(1, sizeof(redis_command));
		skip->fun = redis_reply_skip_;
		redis_enqueue_(&redis_sock->queue_head, &redis_sock->queue_tail, skip);

		/* add real reader to a temporary queue */
		redis_enqueue_(&redis_sock->multi_head, &redis_sock->multi_tail, c);

		redis_sock->multi_commands++; /* count recorded commands */
	} else { /* enqueue normally */
		redis_enqueue_(&redis_sock->queue_head, &redis_sock->queue_tail, c);
	}
}

PHPAPI void
redis_varg_run(INTERNAL_FUNCTION_PARAMETERS, char *keyword, validator_fun fun, int keep_args) {


    zval *object = getThis();
    RedisSock *redis_sock;
    int argc = ZEND_NUM_ARGS(), i;

    const char **args;
    size_t *arglen;
    int copy_array_to = -1;

    zval *z_reply, *z_array;

    /* get all args into the zval array z_args */
    zval **z_args = emalloc(argc * sizeof(zval*));
    if(zend_get_parameters_array(ht, argc, z_args) == FAILURE) {
        efree(z_args);
        RETURN_FALSE;
    }

    /* check if there is only one argument, an array */
    if(argc == 2 && Z_TYPE_P(z_args[1]) == IS_ARRAY) {
	    zval *z_key = z_args[0];
	    z_array = z_args[1];
	    efree(z_args);
	    argc = 1 + zend_hash_num_elements(Z_ARRVAL_P(z_array));
	    z_args = ecalloc(argc, sizeof(zval*));

	    MAKE_STD_ZVAL(z_args[0]);
	    *z_args[0] = *z_key;
	    zval_copy_ctor(z_args[0]);
	    copy_array_to = 1;

    } else if(argc == 1 && Z_TYPE_P(z_args[0]) == IS_ARRAY) {
	    copy_array_to = 0;
	    z_array = z_args[0];
	    argc = zend_hash_num_elements(Z_ARRVAL_P(z_array));
    }

    if(copy_array_to != -1) {
	    for(i = 0; i < zend_hash_num_elements(Z_ARRVAL_P(z_array)); ++i) {
		    zval **z_i_pp;
		    if(zend_hash_index_find(Z_ARRVAL_P(z_array), i, (void **)&z_i_pp) == FAILURE) {
			    efree(z_args);
			    RETURN_FALSE;
		    }
		    MAKE_STD_ZVAL(z_args[i+copy_array_to]);
		    *z_args[i+copy_array_to] = **z_i_pp;
		    zval_copy_ctor(z_args[i+copy_array_to]);
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

int
redis_reply_long_as_bool_(zval *z, zval **z_args) {

	if(Z_TYPE_P(z) == IS_LONG) { /* valid */
		zend_bool b = Z_LVAL_P(z) == 1 ? 1 : 0;
		ZVAL_BOOL(z, b);
	} else {
		zval_dtor(z);
		efree(z);
		ZVAL_BOOL(z, 0);
	}
	return 0;
}
