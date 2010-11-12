/* -*- Mode: C; tab-width: 4 -*- */
/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2009 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Original author: Alfonso Jimenez <yo@alfonsojimenez.com>             |
  | Maintainer: Nicolas Favre-Felix <n.favre-felix@owlient.eu>           |
  | Maintainer: Nasreddine Bouafif <n.bouafif@owlient.eu>                |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_redis.h"

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
     PHP_ME(HiRedis, pipeline, NULL, ZEND_ACC_PUBLIC)
     PHP_ME(HiRedis, multi, NULL, ZEND_ACC_PUBLIC)
     PHP_ME(HiRedis, exec, NULL, ZEND_ACC_PUBLIC)

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

static void *tryParentize(const redisReadTask *task, zval *v) {
        // php_printf("CALLBACK: %s\n", __FUNCTION__);
        if (task && task->parent != NULL) {
                // php_printf("INSIDE\n");
                zval *parent = (zval *)task->parent;
                assert(Z_TYPE_P(parent) == IS_ARRAY);
                add_index_zval(parent, task->idx, v);
        }
        return (void*)v;
}

static void *createStringObject(const redisReadTask *task, char *str, size_t len) {

        // php_printf("CALLBACK: %s\n", __FUNCTION__);
        zval *z_ret;
        MAKE_STD_ZVAL(z_ret);

        switch(task->type) {
                case REDIS_REPLY_ERROR:
                    ZVAL_BOOL(z_ret, 0);
                    break;

                case REDIS_REPLY_STATUS:
                    ZVAL_BOOL(z_ret, 1);
                    break;

                case REDIS_REPLY_STRING:
                    ZVAL_STRINGL(z_ret, str, len, 1);
                    break;
        }
        // php_printf("created string object (%zd)[%s], z_ret=%p\n", len, str, z_ret);

        return tryParentize(task, z_ret);
}

static void *createArrayObject(const redisReadTask *task, int elements) {
        // php_printf("CALLBACK: %s\n", __FUNCTION__);
        zval *z_ret;
        MAKE_STD_ZVAL(z_ret);
        array_init(z_ret);

        return tryParentize(task, z_ret);
}

static void *createIntegerObject(const redisReadTask *task, long long value) {
        // php_printf("CALLBACK: %s\n", __FUNCTION__);
        zval *z_ret;
        MAKE_STD_ZVAL(z_ret);
        ZVAL_LONG(z_ret, value);
        return tryParentize(task, z_ret);
}

static void *createNilObject(const redisReadTask *task) {
        // php_printf("CALLBACK: %s\n", __FUNCTION__);
        zval *z_ret;
        MAKE_STD_ZVAL(z_ret);
        ZVAL_NULL(z_ret);
        return tryParentize(task, z_ret);
}

static void freeObject(void *ptr) {
        // php_printf("CALLBACK: %s\n", __FUNCTION__);
        /* TODO */
}




redisReplyObjectFunctions redisExtReplyObjectFunctions = {
    createStringObject,
    createArrayObject,
    createIntegerObject,
    createNilObject,
    freeObject
};


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

    redis_sock = emalloc(sizeof(RedisSock));
    redis_sock->ctx = redisConnect(host, port);
    if (!redis_sock || redis_sock->ctx->errstr != NULL) {
            printf("Error: %s\n", redis_sock->ctx->errstr);
            RETURN_FALSE;
    }

#ifdef __linux__
    int tcp_flag = 1;
    int result = setsockopt(redis_sock->ctx->fd, IPPROTO_TCP, TCP_NODELAY, (char *) &tcp_flag, sizeof(int));
#endif

    redis_sock->mode = REDIS_MODE_BLOCKING;

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
    zval *object;
    RedisSock *redis_sock;
    char *key = NULL, *val = NULL;
    int key_len, val_len, success = 0;

    zval *z_reply;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss",
                                     &object, hiredis_ce, &key, &key_len,
                                     &val, &val_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (redis_sock_get(object, &redis_sock TSRMLS_CC) < 0) {
        RETURN_FALSE;
    }

    switch(redis_sock->mode) {
            case REDIS_MODE_PIPELINE:
                    redisAppendCommand(redis_sock->ctx, "SET %b %b", key, key_len, val, val_len);
                    redis_sock->enqueued_commands++;
                    RETURN_ZVAL(object, 1, 0);
                    break;

            case REDIS_MODE_TRANSACTION:
                    z_reply = redisCommand(redis_sock->ctx, "SET %b %b", key, key_len, val, val_len);
                    if(Z_TYPE_P(z_reply) == IS_BOOL && Z_BVAL_P(z_reply) == 1) {
                            redis_sock->enqueued_commands++;
                            efree(z_reply);
                            RETURN_ZVAL(object, 1, 0);
                    } else {
                            efree(z_reply);
                            RETURN_FALSE;
                    }
                    break;

            case REDIS_MODE_BLOCKING:
                    z_reply = redisCommand(redis_sock->ctx, "SET %b %b", key, key_len, val, val_len);
                    if(z_reply && Z_TYPE_P(z_reply) == IS_STRING && strncmp(Z_STRVAL_P(z_reply), "OK", 2) == 0) {
                            success = 1;
                    }
    
                    zval_dtor(z_reply);
                    efree(z_reply);

                    if(success) {
                            RETURN_TRUE;
                    } else {
                            RETURN_FALSE;
                    }
                    break;
    }
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
    zval *z_reply;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
                                     &object, hiredis_ce,
                                     &key, &key_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (redis_sock_get(object, &redis_sock TSRMLS_CC) < 0) {
        RETURN_FALSE;
    }

    switch(redis_sock->mode) {
            case REDIS_MODE_PIPELINE:
                    redisAppendCommand(redis_sock->ctx, "GET %b", key, key_len);
                    redis_sock->enqueued_commands++;
                    RETURN_ZVAL(object, 1, 0);
                    break;

            case REDIS_MODE_TRANSACTION:
                    z_reply = redisCommand(redis_sock->ctx, "GET %b", key, key_len);
                    if(Z_TYPE_P(z_reply) == IS_BOOL && Z_BVAL_P(z_reply) == 1) {
                            redis_sock->enqueued_commands++;
                            efree(z_reply);
                            RETURN_ZVAL(object, 1, 0);
                    } else {
                            efree(z_reply);
                            RETURN_FALSE;
                    }
                    break;

            case REDIS_MODE_BLOCKING:
                    z_reply = redisCommand(redis_sock->ctx, "GET %b", key, key_len);
                    if(Z_TYPE_P(z_reply) == IS_STRING) {
                            Z_TYPE_P(return_value) = IS_STRING;
                            Z_STRVAL_P(return_value) = Z_STRVAL_P(z_reply);
                            Z_STRLEN_P(return_value) = Z_STRLEN_P(z_reply);
                            efree(z_reply);
                    } else {
                            zval_dtor(z_reply);
                            efree(z_reply);
                            RETURN_FALSE;
                    }
    }
}
/* }}} */

PHP_METHOD(HiRedis, delete)
{
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

    /* get socket */
    if (redis_sock_get(object, &redis_sock TSRMLS_CC) < 0) {
        RETURN_FALSE;
    }

    /* copy all args as strings */
    args = emalloc((argc+1) * sizeof(char*));
    arglen = emalloc((argc+1) * sizeof(size_t));
    for(i = 0; i < argc; ++i) {
        convert_to_string(z_args[i]);
        args[i+1] = Z_STRVAL_P(z_args[i]);
        arglen[i+1] = (size_t)Z_STRLEN_P(z_args[i]);
    }

    args[0] = "DEL";
    arglen[0] = 3;

    switch(redis_sock->mode) {
            case REDIS_MODE_PIPELINE:
                    redisAppendCommandArgv(redis_sock->ctx, argc+1, args, arglen);
                    efree(args); efree(arglen);
                    redis_sock->enqueued_commands++;
                    RETURN_ZVAL(object, 1, 0);
                    break;

            case REDIS_MODE_TRANSACTION:
                    z_reply = redisCommandArgv(redis_sock->ctx, argc+1, args, arglen);
                    efree(args); efree(arglen);
                    if(Z_TYPE_P(z_reply) == IS_BOOL && Z_BVAL_P(z_reply) == 1) {
                            redis_sock->enqueued_commands++;
                            efree(z_reply);
                            RETURN_ZVAL(object, 1, 0);
                    } else {
                            efree(z_reply);
                            RETURN_FALSE;
                    }
                    break;

            case REDIS_MODE_BLOCKING:
                    z_reply = redisCommandArgv(redis_sock->ctx, argc+1, args, arglen);
                    efree(args); efree(arglen);
                    if(Z_TYPE_P(z_reply) == IS_LONG) {
                            Z_TYPE_P(return_value) = IS_LONG;
                            Z_LVAL_P(return_value) = Z_LVAL_P(z_reply);
                            efree(z_reply);
                    } else {
                            zval_dtor(z_reply);
                            efree(z_reply);
                            RETURN_FALSE;
                    }
    }
}

PHP_METHOD(HiRedis, pipeline)
{
    zval *object = getThis();
    RedisSock *redis_sock;

    if (redis_sock_get(object, &redis_sock TSRMLS_CC) < 0) {
        RETURN_FALSE;
    }

    if(redis_sock->mode == REDIS_MODE_BLOCKING) {
            redis_sock->mode = REDIS_MODE_PIPELINE;
            redis_sock->enqueued_commands = 0;
            RETURN_ZVAL(object, 1, 0);
    }
    RETURN_FALSE;
}

PHP_METHOD(HiRedis, multi)
{
    zval *object = getThis();
    RedisSock *redis_sock;

    if (redis_sock_get(object, &redis_sock TSRMLS_CC) < 0) {
        RETURN_FALSE;
    }

    if(redis_sock->mode == REDIS_MODE_BLOCKING) {
            zval *z_reply = redisCommand(redis_sock->ctx, "MULTI");
            if(Z_TYPE_P(z_reply) == IS_BOOL && Z_BVAL_P(z_reply) == 1) {
                    efree(z_reply);
                    redis_sock->mode = REDIS_MODE_TRANSACTION;
                    redis_sock->enqueued_commands = 0;
                    RETURN_ZVAL(object, 1, 0);
            } else {
                    zval_dtor(z_reply);
                    efree(z_reply);
            }
    }
    RETURN_FALSE;
}

PHP_METHOD(HiRedis, exec)
{
    zval *object = getThis(), *z_reply;
    RedisSock *redis_sock;
    int i, count;
	redis_mode mode;

    if (redis_sock_get(object, &redis_sock TSRMLS_CC) < 0) {
        RETURN_FALSE;
    }

    count = redis_sock->enqueued_commands;
    redis_sock->enqueued_commands = 0;

	mode = redis_sock->mode;
    redis_sock->mode = REDIS_MODE_BLOCKING;

    switch(mode) {
            case REDIS_MODE_BLOCKING:
                    RETURN_FALSE;

            case REDIS_MODE_TRANSACTION:
                    z_reply = redisCommand(redis_sock->ctx, "EXEC");
                    Z_TYPE_P(return_value) = IS_ARRAY;
                    Z_ARRVAL_P(return_value) = Z_ARRVAL_P(z_reply);
                    efree(z_reply);
                    return;

            case REDIS_MODE_PIPELINE:
                    array_init(return_value);

                    for(i = 0; i < count; ++i) {
                            redisGetReply(redis_sock->ctx, (void**)&z_reply);
                            add_next_index_zval(return_value, z_reply);
                    }
                    break;
    }

}

/* vim: set tabstop=4 expandtab: */
