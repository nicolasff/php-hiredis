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
        ZVAL_STRINGL(z_ret, str, len, 1);
        // php_printf("created string object (%zd)[%s], z_ret=%p\n", len, str, z_ret);

#if 0
        VALUE v, enc;
        v = rb_str_new(str,len);

        /* Force default external encoding if possible. */
        if (enc_default_external) {
                enc = rb_funcall(enc_klass,enc_default_external,0);
                v = rb_funcall(v,str_force_encoding,1,enc);
        }

#endif
#if 0
        if (task->type == REDIS_REPLY_ERROR) {
                rb_ivar_set(v,ivar_hiredis_error,v);
                if (task && task->parent != NULL) {
                        /* Also make the parent respond to this method. Redis currently
                         * only emits nested multi bulks of depth 2, so we don't need
                         * to cascade setting this ivar. Make sure to only set the first
                         * error reply on the parent. */
                        VALUE parent = (VALUE)task->parent;
                        if (!rb_ivar_defined(parent,ivar_hiredis_error))
                                rb_ivar_set(parent,ivar_hiredis_error,v);
                }
        }
#endif

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
        /* Garbage collection will clean things up. */
}




redisReplyObjectFunctions redisExtReplyObjectFunctions = {
    createStringObject,
    createArrayObject,
    createIntegerObject,
    createNilObject,
    freeObject
};


PHPAPI int redis_sock_disconnect(redisContext *redis_ctx TSRMLS_DC)
{
    if(!redis_ctx) {
            return 0;
    }

    redisFree(redis_ctx);
    return 1;
}

/**
 * redis_destructor_redis_sock
 */
static void redis_destructor_redis_sock(zend_rsrc_list_entry * rsrc TSRMLS_DC)
{
    redisContext *c = (redisContext *) rsrc->ptr;
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
PHPAPI int redis_sock_get(zval *id, redisContext **redis_ctx TSRMLS_DC)
{

    zval **socket;
    int resource_type;

    if (Z_TYPE_P(id) != IS_OBJECT || zend_hash_find(Z_OBJPROP_P(id), "socket",
                                  sizeof("socket"), (void **) &socket) == FAILURE) {
        return -1;
    }

    *redis_ctx = (redisContext *) zend_list_find(Z_LVAL_PP(socket), &resource_type);

    if (!*redis_ctx || resource_type != le_redis_sock) {
            return -1;
    }

    (*redis_ctx)->reader = redisReplyReaderCreate(); /* TODO: add to phpredis object */
    redisReplyReaderSetReplyObjectFunctions((*redis_ctx)->reader, &redisExtReplyObjectFunctions);

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
    redisContext *redis_ctx  = NULL;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|ll",
                                     &object, hiredis_ce, &host, &host_len, &port,
                                     &timeout.tv_sec) == FAILURE) {
       RETURN_FALSE;
    }

    redisContext *c = redisConnect(host, port);
    if (c->errstr != NULL) {
            printf("Error: %s\n", c->errstr);
            RETURN_FALSE;
    }


    /*
    if (timeout.tv_sec < 0L || timeout.tv_sec > INT_MAX) {
        zend_throw_exception(redis_exception_ce, "Invalid timeout", 0 TSRMLS_CC);
        RETURN_FALSE;
    }
    */

    id = zend_list_insert(c, le_redis_sock);
    add_property_resource(object, "socket", id);

    RETURN_TRUE;
}
/* }}} */

/* {{{ proto boolean HiRedis::close()
 */
PHP_METHOD(HiRedis, close)
{
    zval *object;
    redisContext *redis_ctx = NULL;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O",
        &object, hiredis_ce) == FAILURE) {
        RETURN_FALSE;
    }
    if (redis_sock_get(object, &redis_ctx TSRMLS_CC) < 0) {
        RETURN_FALSE;
    }

    if (redis_sock_disconnect(redis_ctx TSRMLS_CC)) {
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
    redisContext *redis_ctx;
    char *key = NULL, *val = NULL;
    int key_len, val_len, success = 0;

    zval *z_reply;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss",
                                     &object, hiredis_ce, &key, &key_len,
                                     &val, &val_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (redis_sock_get(object, &redis_ctx TSRMLS_CC) < 0) {
        RETURN_FALSE;
    }

    z_reply = redisCommand(redis_ctx, "SET %b %b", key, key_len, val, val_len);
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
    /*
    if(redisReplyReaderGetReply(reader,(void**)&reply) != REDIS_OK) {
            freeReplyObject(reply);
            RETURN_FALSE;
    }
    if(reply->type == REDIS_REPLY_STATUS && strncmp(reply->str, "OK", 2) == 0) {
            freeReplyObject(reply);
            RETURN_TRUE;
    }
    */
}
/* }}} */

/* {{{ proto string HiRedis::get(string key)
 */
PHP_METHOD(HiRedis, get)
{
    zval *object;
    redisContext *redis_ctx;
    char *key = NULL;
    int key_len;
    zval *z_reply;

    if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os",
                                     &object, hiredis_ce,
                                     &key, &key_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (redis_sock_get(object, &redis_ctx TSRMLS_CC) < 0) {
        RETURN_FALSE;
    }

    z_reply = redisCommand(redis_ctx, "GET %b", key, key_len);


    if(!z_reply) {
            RETURN_FALSE;
    }

    if(Z_TYPE_P(z_reply) == IS_STRING) {
            Z_TYPE_P(return_value) = IS_STRING;
            Z_STRVAL_P(return_value) = Z_STRVAL_P(z_reply);
            Z_STRLEN_P(return_value) = Z_STRLEN_P(z_reply);
            efree(z_reply);
    } else {
            efree(z_reply);
            RETURN_FALSE;
    }
}
/* }}} */

/* vim: set tabstop=4 expandtab: */
