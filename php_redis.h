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

#ifndef PHP_REDIS_H
#define PHP_REDIS_H

#include "hiredis/hiredis.h"

PHP_METHOD(HiRedis, __construct);
PHP_METHOD(HiRedis, connect);
PHP_METHOD(HiRedis, close);
PHP_METHOD(HiRedis, ping);
PHP_METHOD(HiRedis, get);
PHP_METHOD(HiRedis, set);
PHP_METHOD(HiRedis, delete);
PHP_METHOD(HiRedis, lrange);
PHP_METHOD(HiRedis, pipeline);
PHP_METHOD(HiRedis, send);
PHP_METHOD(HiRedis, multi);
PHP_METHOD(HiRedis, exec);
PHP_METHOD(HiRedis, incr);
PHP_METHOD(HiRedis, decr);
PHP_METHOD(HiRedis, incrby);
PHP_METHOD(HiRedis, decrby);
PHP_METHOD(HiRedis, hset);
PHP_METHOD(HiRedis, hgetall);
PHP_METHOD(HiRedis, hmget);
PHP_METHOD(HiRedis, setnx);
PHP_METHOD(HiRedis, getset);
PHP_METHOD(HiRedis, ping);
PHP_METHOD(HiRedis, renamekey);
PHP_METHOD(HiRedis, getmultiple);
PHP_METHOD(HiRedis, randomKey);
PHP_METHOD(HiRedis, exists);
/*
PHP_METHOD(HiRedis, renameNx);
PHP_METHOD(HiRedis, getMultiple);
PHP_METHOD(HiRedis, decr);
PHP_METHOD(HiRedis, type);
PHP_METHOD(HiRedis, getKeys);
PHP_METHOD(HiRedis, sortAsc);
PHP_METHOD(HiRedis, sortAscAlpha);
PHP_METHOD(HiRedis, sortDesc);
PHP_METHOD(HiRedis, sortDescAlpha);
PHP_METHOD(HiRedis, lPush);
PHP_METHOD(HiRedis, rPush);
PHP_METHOD(HiRedis, lPop);
PHP_METHOD(HiRedis, rPop);
PHP_METHOD(HiRedis, lSize);
PHP_METHOD(HiRedis, lRemove);
PHP_METHOD(HiRedis, listTrim);
PHP_METHOD(HiRedis, lGet);
PHP_METHOD(HiRedis, lGetRange);
PHP_METHOD(HiRedis, lSet);
PHP_METHOD(HiRedis, sAdd);
PHP_METHOD(HiRedis, sSize);
PHP_METHOD(HiRedis, sRemove);
PHP_METHOD(HiRedis, sMove);
PHP_METHOD(HiRedis, sPop);
PHP_METHOD(HiRedis, sContains);
PHP_METHOD(HiRedis, sMembers);
PHP_METHOD(HiRedis, sInter);
PHP_METHOD(HiRedis, sInterStore);
PHP_METHOD(HiRedis, sUnion);
PHP_METHOD(HiRedis, sUnionStore);
PHP_METHOD(HiRedis, sDiff);
PHP_METHOD(HiRedis, sDiffStore);
PHP_METHOD(HiRedis, setTimeout);
PHP_METHOD(HiRedis, save);
PHP_METHOD(HiRedis, bgSave);
PHP_METHOD(HiRedis, lastSave);
PHP_METHOD(HiRedis, flushDB);
PHP_METHOD(HiRedis, flushAll);
PHP_METHOD(HiRedis, dbSize);
PHP_METHOD(HiRedis, auth);
PHP_METHOD(HiRedis, ttl);
PHP_METHOD(HiRedis, info);
PHP_METHOD(HiRedis, select);
PHP_METHOD(HiRedis, move);
PHP_METHOD(HiRedis, zAdd);
PHP_METHOD(HiRedis, zDelete);
PHP_METHOD(HiRedis, zRange);
PHP_METHOD(HiRedis, zReverseRange);
PHP_METHOD(HiRedis, zRangeByScore);
PHP_METHOD(HiRedis, zDeleteRangeByScore);
PHP_METHOD(HiRedis, zCard);
PHP_METHOD(HiRedis, zScore);
PHP_METHOD(HiRedis, zIncrBy);
PHP_METHOD(HiRedis, zInter);
PHP_METHOD(HiRedis, zUnion);
PHP_METHOD(HiRedis, expireAt);

PHP_METHOD(HiRedis, mset);
PHP_METHOD(HiRedis, rpoplpush);

PHP_METHOD(HiRedis, hSet);
PHP_METHOD(HiRedis, hDel);
PHP_METHOD(HiRedis, hLen);
PHP_METHOD(HiRedis, hKeys);
PHP_METHOD(HiRedis, hVals);
PHP_METHOD(HiRedis, hExists);
PHP_METHOD(HiRedis, hIncrBy);
*/

#ifdef PHP_WIN32
#define PHP_REDIS_API __declspec(dllexport)
#else
#define PHP_REDIS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(hiredis);
PHP_MSHUTDOWN_FUNCTION(hiredis);
PHP_RINIT_FUNCTION(hiredis);
PHP_RSHUTDOWN_FUNCTION(hiredis);
PHP_MINFO_FUNCTION(hiredis);

typedef enum {REDIS_MODE_DIRECT, REDIS_MODE_TRANSACTION} redis_mode;
typedef int (*reader_function)(zval*, zval **); /* callback function */


typedef struct redis_command_ {
	reader_function fun; /* callback function */
	zval **z_args;
	struct redis_command_ *next;
	
	struct redis_command_ *multi;

} redis_command;

/* {{{ struct RedisSock */
typedef struct {
	redisContext *ctx;

	redis_mode mode;

	/* pipeline count */
	int pipeline;

	/* count for the current transaction */
	int multi;
	int multi_commands;

	redis_command *queue_head;
	redis_command *queue_tail;

	redis_command *multi_head;
	redis_command *multi_tail;
} RedisSock;
/* }}} */


#define redis_sock_name "Redis Socket Buffer"

#define REDIS_SOCK_STATUS_FAILED 0
#define REDIS_SOCK_STATUS_DISCONNECTED 1
#define REDIS_SOCK_STATUS_UNKNOWN 2
#define REDIS_SOCK_STATUS_CONNECTED 3

/* properties */
#define REDIS_NOT_FOUND 0
#define REDIS_STRING 1
#define REDIS_SET 2
#define REDIS_LIST 3


/* {{{ internal function protos */
void add_constant_long(zend_class_entry *ce, char *name, int value);

PHPAPI void redis_check_eof(RedisSock *redis_sock TSRMLS_DC);
PHPAPI RedisSock* redis_sock_create(char *host, int host_len, unsigned short port, long timeout);
PHPAPI int redis_sock_connect(RedisSock *redis_sock TSRMLS_DC);
PHPAPI int redis_sock_disconnect(RedisSock *redis_sock TSRMLS_DC);
PHPAPI int redis_sock_server_open(RedisSock *redis_sock, int TSRMLS_DC);
PHPAPI char * redis_sock_read(RedisSock *redis_sock, int *buf_len TSRMLS_DC);
PHPAPI char * redis_sock_read_bulk_reply(RedisSock *redis_sock, int bytes);
PHPAPI int redis_sock_read_multibulk_reply(INTERNAL_FUNCTION_PARAMETERS, RedisSock *redis_sock TSRMLS_DC);
PHPAPI int redis_sock_write(RedisSock *redis_sock, char *cmd, size_t sz);
PHPAPI void redis_free_socket(RedisSock *redis_sock);

PHPAPI void redis_boolean_response(INTERNAL_FUNCTION_PARAMETERS, RedisSock *redis_sock TSRMLS_DC);
PHPAPI void redis_long_response(INTERNAL_FUNCTION_PARAMETERS, RedisSock *redis_sock TSRMLS_DC);
PHPAPI void redis_bulk_double_response(INTERNAL_FUNCTION_PARAMETERS, RedisSock *redis_sock TSRMLS_DC);
PHPAPI void redis_1_response(INTERNAL_FUNCTION_PARAMETERS, RedisSock *redis_sock TSRMLS_DC);
PHPAPI void redis_atomic_increment(INTERNAL_FUNCTION_PARAMETERS, char *keyword, int keyword_len TSRMLS_DC);
PHPAPI int generic_multiple_args_cmd(INTERNAL_FUNCTION_PARAMETERS, char *keyword, int keyword_len,
                int min_argc, RedisSock **redis_sock TSRMLS_DC);
PHPAPI void generic_sort_cmd(INTERNAL_FUNCTION_PARAMETERS, char *sort, int use_alpha TSRMLS_DC);
PHPAPI void generic_empty_cmd(INTERNAL_FUNCTION_PARAMETERS, char *cmd, int cmd_len TSRMLS_DC);
PHPAPI void generic_empty_long_cmd(INTERNAL_FUNCTION_PARAMETERS, char *cmd, int cmd_len TSRMLS_DC);
PHPAPI void generic_push_function(INTERNAL_FUNCTION_PARAMETERS, char *keyword, int keyword_len TSRMLS_DC);
PHPAPI void generic_pop_function(INTERNAL_FUNCTION_PARAMETERS, char *keyword, int keyword_len TSRMLS_DC);

PHPAPI void array_zip_values_and_scores(INTERNAL_FUNCTION_PARAMETERS, int use_atof TSRMLS_DC);

/* }}} */

ZEND_BEGIN_MODULE_GLOBALS(hiredis)
ZEND_END_MODULE_GLOBALS(hiredis)

	/*
#ifdef ZTS
#define REDIS_G(v) TSRMG(redis_globals_id, zend_redis_globals *, v)
#else
#define REDIS_G(v) (redis_globals.v)
#endif
*/

#define PHP_HIREDIS_VERSION "0.1"

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
