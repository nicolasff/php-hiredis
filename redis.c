/* -*- Mode: C; tab-width: 8 -*- */
/*
   +----------------------------------------------------------------------+
   | PHP Version 5							  |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2009 The PHP Group				  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is	  |
   | available through the world-wide-web at the following url:	   	  |
   | http://www.php.net/license/3_01.txt				  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to	  |
   | license@php.net so we can mail you a copy immediately.	       	  |
   +----------------------------------------------------------------------+
   | Author: Nicolas Favre-Felix <n.favre-felix@owlient.eu>	   	  |
   | Author: Nasreddine Bouafif <n.bouafif@owlient.eu>		  	  |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_redis.h"
#include "macros.h"
#include "library.h"
#include "hooks.h"

#ifdef __linux__
/* setsockopt */
#include <sys/types.h>
#include <netinet/tcp.h>  /* TCP_NODELAY */
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <zend_exceptions.h>


static int le_redis_sock;
static zend_class_entry *hiredis_ce;

ZEND_DECLARE_MODULE_GLOBALS(hiredis)

	static zend_function_entry hiredis_functions[] = {
		PHP_ME(HiRedis, __construct, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, connect, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, close, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, get, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, set, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, delete, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, lrange, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, pipeline, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, send, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, multi, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, exec, NULL, ZEND_ACC_PUBLIC)

			PHP_ME(HiRedis, incr, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, decr, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, incrby, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, decrby, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, hset, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, hgetall, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, hmget, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, setnx, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, getset, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, ping, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, randomKey, NULL, ZEND_ACC_PUBLIC)
			PHP_ME(HiRedis, exists, NULL, ZEND_ACC_PUBLIC)
/*
			PHP_ME(HiRedis, renamekey, NULL, ZEND_ACC_PUBLIC)
*/
			PHP_ME(HiRedis, getmultiple, NULL, ZEND_ACC_PUBLIC)

			PHP_MALIAS(HiRedis, open, connect, NULL, ZEND_ACC_PUBLIC)
			{NULL, NULL, NULL}
	};

zend_module_entry hiredis_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"hiredis",
	NULL,
	PHP_MINIT(hiredis),
	PHP_MSHUTDOWN(hiredis),
	PHP_RINIT(hiredis),
	PHP_RSHUTDOWN(hiredis),
	PHP_MINFO(hiredis),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_HIREDIS_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_HIREDIS
ZEND_GET_MODULE(hiredis)
#endif

PHPAPI int redis_sock_disconnect(RedisSock *redis_sock TSRMLS_DC)
{
	if(!redis_sock || !redis_sock->ctx) {
		return 0;
	}

	redisFree(redis_sock->ctx);
	return 1;
}

/**
 * redis_destructor_redis_sock
 */
static void redis_destructor_redis_sock(zend_rsrc_list_entry * rsrc TSRMLS_DC)
{
	RedisSock *redis_sock = (RedisSock*) rsrc->ptr;
	/* TODO */
	/*   redis_sock_disconnect(redis_ctx TSRMLS_CC);
	     redis_free_socket(redis_ctx);
	     */
}

/**
 * PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(hiredis)
{
	zend_class_entry hiredis_class_entry;
	INIT_CLASS_ENTRY(hiredis_class_entry, "HiRedis", hiredis_functions);
	hiredis_ce = zend_register_internal_class(&hiredis_class_entry TSRMLS_CC);

	le_redis_sock = zend_register_list_destructors_ex(
			redis_destructor_redis_sock,
			NULL,
			redis_sock_name, module_number
			);

	return SUCCESS;
}

/**
 * PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(hiredis)
{
	return SUCCESS;
}

/**
 * PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(hiredis)
{
	return SUCCESS;
}

/**
 * PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(hiredis)
{
	return SUCCESS;
}

/**
 * PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(hiredis)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "HiRedis Support", "enabled");
	php_info_print_table_row(2, "Version", PHP_HIREDIS_VERSION);
	php_info_print_table_end();
}

/* {{{ proto HiRedis HiRedis::__construct()
   Public constructor */
PHP_METHOD(HiRedis, __construct)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		RETURN_FALSE;
	}
}
/* }}} */

/**
 * redis_sock_get
 */
PHPAPI int redis_sock_get(zval *id, RedisSock **redis_sock TSRMLS_DC)
{

	zval **socket;
	int resource_type;

	if (Z_TYPE_P(id) != IS_OBJECT || zend_hash_find(Z_OBJPROP_P(id), "socket",
				sizeof("socket"), (void **) &socket) == FAILURE) {
		return -1;
	}

	*redis_sock = (RedisSock *) zend_list_find(Z_LVAL_PP(socket), &resource_type);

	if (!*redis_sock || resource_type != le_redis_sock) {
		return -1;
	}

	(*redis_sock)->ctx->reader = redisReplyReaderCreate(); /* TODO: add to phpredis object */
	redisReplyReaderSetReplyObjectFunctions((*redis_sock)->ctx->reader, &redisExtReplyObjectFunctions);

	return Z_LVAL_PP(socket);
}

/* {{{ proto boolean HiRedis::connect(string host, int port [, int timeout])
*/
PHP_METHOD(HiRedis, connect)
{
	zval *object;
	int host_len, id;
	char *host = NULL;
	long port = 6379;

	struct timeval timeout = {0L, 0L};
	RedisSock *redis_sock  = NULL;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|ll",
				&object, hiredis_ce, &host, &host_len, &port,
				&timeout.tv_sec) == FAILURE) {
		RETURN_FALSE;
	}

	redis_sock = ecalloc(1, sizeof(RedisSock));
	redis_sock->pipeline = 0;

	if(host_len != 0 && *host == '/') { /* unix socket */
		redis_sock->ctx = redisConnectUnix(host);
	} else {
		redis_sock->ctx = redisConnect(host, port);
	}
	if (!redis_sock || redis_sock->ctx->errstr != NULL) {
		printf("Error: %s\n", redis_sock->ctx->errstr);
		RETURN_FALSE;
	}

#ifdef __linux__
	int tcp_flag = 1;
	int result = setsockopt(redis_sock->ctx->fd, IPPROTO_TCP, TCP_NODELAY, (char *) &tcp_flag, sizeof(int));
#endif

	redis_sock->mode = REDIS_MODE_DIRECT;

	/* TODO: add timeout support */
	/*
	   if (timeout.tv_sec < 0L || timeout.tv_sec > INT_MAX) {
	   zend_throw_exception(redis_exception_ce, "Invalid timeout", 0 TSRMLS_CC);
	   RETURN_FALSE;
	   }
	   */

	id = zend_list_insert(redis_sock, le_redis_sock);
	add_property_resource(object, "socket", id);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto boolean HiRedis::close()
*/
PHP_METHOD(HiRedis, close)
{
	zval *object;
	RedisSock *redis_sock = NULL;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",
				&object, hiredis_ce) == FAILURE) {
		RETURN_FALSE;
	}
	if (redis_sock_get(object, &redis_sock TSRMLS_CC) < 0) {
		RETURN_FALSE;
	}

	if (redis_sock_disconnect(redis_sock TSRMLS_CC)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
/* }}} */


/* {{{ proto boolean HiRedis::set(string key, string value)
*/
PHP_METHOD(HiRedis, set)
{
	zval *object = getThis();
	RedisSock *redis_sock;
	char *key = NULL, *val = NULL;
	int key_len, val_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss",
				&object, hiredis_ce, &key, &key_len,
				&val, &val_len) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	REDIS_RUN(redis_sock, redis_reply_status_, "SET %b %b", key, (size_t)key_len, val, (size_t)val_len);
}
/* }}} */

/* {{{ proto string HiRedis::get(string key)
*/
PHP_METHOD(HiRedis, get)
{
	zval *object;
	RedisSock *redis_sock;
	char *key = NULL;
	int key_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
				&object, hiredis_ce,
				&key, &key_len) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	REDIS_RUN(redis_sock, redis_reply_string_, "GET %b", key, (size_t)key_len);
}
/* }}} */

PHPAPI void
common_incr(INTERNAL_FUNCTION_PARAMETERS, char *keyword, char *format_string TSRMLS_DC) {

	zval *object;
	RedisSock *redis_sock;
	char *key = NULL;
	int key_len;
	long delta = 1;


	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), format_string,
				&object, hiredis_ce,
				&key, &key_len, &delta) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	if(delta == 1) {
		char format[] = "XXXX %b";
		memcpy(format, keyword, 4);
		REDIS_RUN(redis_sock, redis_reply_long_, format, key, (size_t)key_len);
	} else {
		char format[] = "XXXXBY %b %d";
		memcpy(format, keyword, 4);
		REDIS_RUN(redis_sock, redis_reply_long_, format, key, (size_t)key_len, (int)delta);
	}

}


/* {{{ proto long HiRedis::incr(long key [, long add])
*/
PHP_METHOD(HiRedis, incr)
{
	common_incr(INTERNAL_FUNCTION_PARAM_PASSTHRU, "INCR", "Os|l" TSRMLS_CC);
}
/* }}} */

/* {{{ proto long HiRedis::decr(long key)
*/
PHP_METHOD(HiRedis, decr)
{
	common_incr(INTERNAL_FUNCTION_PARAM_PASSTHRU, "DECR", "Os|l" TSRMLS_CC);
}
/* }}} */

/* {{{ proto long HiRedis::incrBy(long key, long delta)
*/
PHP_METHOD(HiRedis, incrby)
{
	common_incr(INTERNAL_FUNCTION_PARAM_PASSTHRU, "INCR", "Osl" TSRMLS_CC);
}
/* }}} */

/* {{{ proto long HiRedis::decrBy(long key, long delta)
*/
PHP_METHOD(HiRedis, decrby)
{
	common_incr(INTERNAL_FUNCTION_PARAM_PASSTHRU, "DECR", "Osl" TSRMLS_CC);
}
/* }}} */

PHP_METHOD(HiRedis, pipeline)
{
	zval *object = getThis();
	RedisSock *redis_sock;

	REDIS_SOCK_GET(redis_sock);

	if(redis_sock->mode == REDIS_MODE_DIRECT && redis_sock->pipeline == 0) {
		redis_sock->pipeline = 1;
		redis_sock->queue_head = NULL;
		redis_sock->queue_tail = NULL;
		RETURN_ZVAL(object, 1, 0);
	}
	RETURN_FALSE;
}

PHP_METHOD(HiRedis, send)
{
	zval *object = getThis();
	RedisSock *redis_sock;
	int i;

	REDIS_SOCK_GET(redis_sock);
	if (redis_sock->pipeline == 0 || redis_sock->mode != REDIS_MODE_DIRECT) {
		RETURN_FALSE;
	}
	/* send pipelined commands */
	array_init(return_value);

	redis_command *c;
	for(c = redis_sock->queue_head; c; c = c->next) {

		zval *z_current;
		MAKE_STD_ZVAL(z_current);
		redisReplyReaderSetPrivdata(redis_sock->ctx->reader, z_current);
		redisGetReply(redis_sock->ctx, NULL);

		if(c->multi) { /* apply function on a multi block */
			redis_command *cm;

			zval *z_tab;
			MAKE_STD_ZVAL(z_tab);
			array_init(z_tab);

			HashTable *h_index = Z_ARRVAL_P(z_current);
			for(cm = c->multi, zend_hash_internal_pointer_reset(h_index);
					cm && zend_hash_has_more_elements(h_index) == SUCCESS;
					cm = cm->next, zend_hash_move_forward(h_index)) {

				char *key;
				unsigned int key_len;
				unsigned long idx;
				int type;
				zval **z_value_pp;

				type = zend_hash_get_current_key_ex(h_index, &key, &key_len, &idx, 0, NULL);
				if(zend_hash_get_current_data(h_index, (void**)&z_value_pp) == FAILURE) {
					continue; 	/* this should never happen, according to the PHP people. */
				}

				if(cm->fun(*z_value_pp, cm->z_args) == 0) {
					zval *z_copy;
					MAKE_STD_ZVAL(z_copy);
					*z_copy = **z_value_pp;
					zval_copy_ctor(z_copy);
					add_next_index_zval(z_tab, z_copy);
				}
			}
			zval_dtor(z_current);
			efree(z_current);
			add_next_index_zval(return_value, z_tab);
		} else {
			if(c->fun(z_current, c->z_args) == 0) {
				add_next_index_zval(return_value, z_current);
			}
		}
		/* TODO: clean c->multi */
	}
	/* TODO: clean this properly */
	redis_sock->pipeline = 0;
	redis_sock->queue_head = redis_sock->queue_tail = NULL;
}

PHP_METHOD(HiRedis, multi)
{
	zval *object = getThis();
	RedisSock *redis_sock;

	REDIS_SOCK_GET(redis_sock);

	if(redis_sock->mode == REDIS_MODE_DIRECT) {

		REDIS_SOCK_GET(redis_sock);
		redisAppendCommand(redis_sock->ctx, "MULTI");

		if(redis_sock->pipeline == 0) { /* no pipeline, read the result. */
			redis_sock->multi = 1;
			zval *z_reply;
			MAKE_STD_ZVAL(z_reply);
			redisReplyReaderSetPrivdata(redis_sock->ctx->reader, z_reply);
			redisGetReply(redis_sock->ctx, NULL);

			if(Z_TYPE_P(z_reply) == IS_BOOL && Z_BVAL_P(z_reply) == 1) {
				efree(z_reply);
				redis_sock->mode = REDIS_MODE_TRANSACTION;
				redis_sock->multi_head = NULL;
				redis_sock->multi_tail = NULL;
				RETURN_ZVAL(object, 1, 0);
			} else {
				zval_dtor(z_reply);
				efree(z_reply);
			}
		} else {
			redis_sock->multi = 1;
			redis_enqueue(redis_sock, (void*)redis_reply_status_, NULL);
			redis_sock->mode = REDIS_MODE_TRANSACTION;
			redis_sock->multi_head = NULL;
			redis_sock->multi_tail = NULL;
			RETURN_ZVAL(object, 1, 0);
		}
	}
	RETURN_FALSE;
}

PHP_METHOD(HiRedis, exec)
{
	zval *object = getThis(), *z_reply, *z_raw_tab;
	RedisSock *redis_sock;
	int i, count;
	redis_mode mode;

	REDIS_SOCK_GET(redis_sock);

	redisAppendCommand(redis_sock->ctx, "EXEC");

	if(redis_sock->pipeline == 0) { /* send commands */
		array_init(return_value);

		redis_command *c, *cm;
		for(c = redis_sock->queue_head; c; c = c->next) { /* skip confirmations */
			zval *z_tmp;
			MAKE_STD_ZVAL(z_tmp);
			redisReplyReaderSetPrivdata(redis_sock->ctx->reader, z_tmp);
			redisGetReply(redis_sock->ctx, NULL);
			zval_dtor(z_tmp);
			efree(z_tmp);
		}

		zval *z_tab;
		MAKE_STD_ZVAL(z_tab);
		redisReplyReaderSetPrivdata(redis_sock->ctx->reader, z_tab);
		redisGetReply(redis_sock->ctx, NULL);

		HashTable *h_index = Z_ARRVAL_P(z_tab);
		for(cm = redis_sock->multi_head, zend_hash_internal_pointer_reset(h_index);
				cm && zend_hash_has_more_elements(h_index) == SUCCESS;
				cm = cm->next, zend_hash_move_forward(h_index)) {

			char *key;
			unsigned int key_len;
			unsigned long idx;
			int type;
			zval **z_value_pp;

			type = zend_hash_get_current_key_ex(h_index, &key, &key_len, &idx, 0, NULL);
			if(zend_hash_get_current_data(h_index, (void**)&z_value_pp) == FAILURE) {
				continue; 	/* this should never happen, according to the PHP people. */
			}

			if(cm->fun(*z_value_pp, cm->z_args) == 0) {
				zval *z_copy;
				MAKE_STD_ZVAL(z_copy);
				*z_copy = **z_value_pp;
				zval_copy_ctor(z_copy);
				add_next_index_zval(return_value, z_copy);
			}
		}
		zval_dtor(z_tab);
		efree(z_tab);

		redis_sock->multi = 0;
		redis_sock->mode = REDIS_MODE_DIRECT;
	} else {

		/* add multi/exec callback queue to the side. */
		redis_command *join = ecalloc(1, sizeof(redis_command));
		join->multi = redis_sock->multi_head;

		/* add this new node at the end of the queue */
		redis_sock->queue_tail->next = join;
		redis_sock->queue_tail = join;

		redis_sock->multi = 0;
		redis_sock->multi_commands = 0;
		redis_sock->mode = REDIS_MODE_DIRECT;

		RETURN_ZVAL(getThis(), 1, 0);
	}
	/* reset multi/exec tracker. TODO: clean it properly. */
	redis_sock->multi_head = redis_sock->multi_tail = NULL;
}

/* {{{ proto long HiRedis::hset(string key, string field, string val)
*/
PHP_METHOD(HiRedis, hset)
{
	zval *object;
	RedisSock *redis_sock;
	char *key, *field, *val;
	int key_len, field_len, val_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osss",
				&object, hiredis_ce,
				&key, &key_len, &field, &field_len, &val, &val_len) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	REDIS_RUN(redis_sock, redis_reply_long_, "HSET %b %b %b",
			key, (size_t)key_len,
			field, (size_t)field_len,
			val, (size_t)val_len);
}
/* }}} */

/* {{{ proto array HiRedis::hgetall(string key)
*/
PHP_METHOD(HiRedis, hgetall)
{
	zval *object;
	RedisSock *redis_sock;
	char *key;
	int key_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
				&object, hiredis_ce,
				&key, &key_len) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	REDIS_RUN(redis_sock, redis_reply_zip_, "HGETALL %b", key, (size_t)key_len);
}
/* }}} */

/* {{{ proto array HiRedis::hmget(string key, array fields)
*/
PHP_METHOD(HiRedis, hmget) {
	redis_varg_run(INTERNAL_FUNCTION_PARAM_PASSTHRU, "HMGET", redis_reply_zip_closure_, 1);
}
/* }}} */


/* {{{ proto array HiRedis::delete(string key)
*/
PHP_METHOD(HiRedis, delete) {

	redis_varg_run(INTERNAL_FUNCTION_PARAM_PASSTHRU, "DEL", redis_reply_long_, 0);
}
/* }}} */

/* {{{ proto array HiRedis::lrange(string key, int from, int to)
*/
PHP_METHOD(HiRedis, lrange)
{
	zval *object;
	RedisSock *redis_sock;
	char *key;
	int key_len;
	long from, to;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osll",
				&object, hiredis_ce,
				&key, &key_len, &from, &to) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	REDIS_RUN(redis_sock, redis_reply_array_, "LRANGE %b %d %d", key, (size_t)key_len, (int)from, (int)to);
}
/* }}} */

/* {{{ proto bool HiRedis::setnx(string key, string value)
*/
PHP_METHOD(HiRedis, setnx) {
	zval *object;
	RedisSock *redis_sock;
	char *key = NULL, *val = NULL;
	int key_len, val_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss",
				&object, hiredis_ce, &key, &key_len,
				&val, &val_len) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	REDIS_RUN(redis_sock, redis_reply_long_as_bool_, "SETNX %b %b", key, (size_t)key_len, val, (size_t)val_len);
}
/* }}} */

/* {{{ proto string HiRedis::getset(string key, string value)
*/
PHP_METHOD(HiRedis, getset) {
	zval *object;
	RedisSock *redis_sock;
	char *key = NULL, *val = NULL;
	int key_len, val_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss",
				&object, hiredis_ce, &key, &key_len,
				&val, &val_len) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	REDIS_RUN(redis_sock, redis_reply_string_, "GETSET %b %b", key, (size_t)key_len, val, (size_t)val_len);
}
/* }}} */

/* {{{ proto string HiRedis::ping()
*/
PHP_METHOD(HiRedis, ping) {
	zval *object;
	RedisSock *redis_sock;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, hiredis_ce) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	REDIS_RUN(redis_sock, redis_reply_string_, "PING");
}
/* }}} */

/* {{{ proto string HiRedis::randomKey()
*/
PHP_METHOD(HiRedis, randomKey) {
	zval *object;
	RedisSock *redis_sock;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &object, hiredis_ce) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	REDIS_RUN(redis_sock, redis_reply_string_, "RANDOMKEY");
}
/* }}} */

/* {{{ proto bool HiRedis::exists(string key)
*/
PHP_METHOD(HiRedis, exists) {
	zval *object;
	RedisSock *redis_sock;
	char *key;
	int key_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
				&object, hiredis_ce, &key, &key_len) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	REDIS_RUN(redis_sock, redis_reply_long_as_bool_, "EXISTS %b", key, (size_t)key_len);
}
/* }}} */

/* {{{ proto string HiRedis::renamekey(string from, string to)
*/
PHP_METHOD(HiRedis, renamekey) {
	zval *object;
	RedisSock *redis_sock;
	char *from = NULL, *to = NULL;
	int from_len, to_len;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss",
				&object, hiredis_ce, &from, &from_len,
				&to, &to_len) == FAILURE) {
		RETURN_FALSE;
	}

	REDIS_SOCK_GET(redis_sock);
	REDIS_RUN(redis_sock, redis_reply_status_, "RENAME %b %b", from, (size_t)from_len, to, (size_t)to_len);
}
/* }}} */

/* {{{ proto array HiRedis::getmultiple(array keys)
*/
PHP_METHOD(HiRedis, getmultiple) {
	redis_varg_run(INTERNAL_FUNCTION_PARAM_PASSTHRU, "MGET", redis_reply_array_, 0);
}
/* }}} */

/* vim: set tabstop=8 expandtab */
