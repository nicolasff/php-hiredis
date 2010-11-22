#ifndef PHPREDIS_MACROS_H
#define PHPREDIS_MACROS_H

#define REDIS_SOCK_GET(sock) {\
    if (redis_sock_get(object, &sock TSRMLS_CC) < 0) {\
        RETURN_FALSE;\
    }\
}

#define REDIS_RUN(sock, fun, pattern, ...) {\
	zval *z_reply;\
	switch(redis_sock->mode) {\
		case REDIS_MODE_PIPELINE:\
			redisAppendCommand(redis_sock->ctx, pattern, ##__VA_ARGS__);\
			redis_enqueue(redis_sock, (void*)fun, NULL);\
			RETURN_ZVAL(object, 1, 0);\
			break;\
\
		case REDIS_MODE_TRANSACTION:\
			z_reply = redisCommand(redis_sock->ctx, pattern, ##__VA_ARGS__);\
			if(Z_TYPE_P(z_reply) == IS_BOOL && Z_BVAL_P(z_reply) == 1) {\
				redis_enqueue(redis_sock, (void*)fun, NULL);\
				efree(z_reply);\
				RETURN_ZVAL(object, 1, 0);\
			} else {\
				efree(z_reply);\
				RETURN_FALSE;\
			}\
			break;\
\
		case REDIS_MODE_BLOCKING:\
			 z_reply = redisCommand(redis_sock->ctx, pattern, ##__VA_ARGS__);\
			fun(return_value, redis_sock->mode, z_reply, NULL); \
			break;\
	}\
}


#define REDIS_RUN_ARGS(redis_sock, fun, argc, args, arglen, closure_args) {\
	zval *z_reply;\
	int i;\
	switch(redis_sock->mode) {\
		case REDIS_MODE_PIPELINE:\
			redisAppendCommandArgv(redis_sock->ctx, argc, args, arglen);\
			efree(args);\
			efree(arglen);\
			redis_enqueue(redis_sock, (void*)fun, closure_args);\
			RETURN_ZVAL(object, 1, 0);\
			break;\
\
		case REDIS_MODE_TRANSACTION:\
			z_reply = redisCommandArgv(redis_sock->ctx, argc, args, arglen);\
			efree(args);\
			efree(arglen);\
			if(Z_TYPE_P(z_reply) == IS_BOOL && Z_BVAL_P(z_reply) == 1) {\
				redis_enqueue(redis_sock, (void*)fun, closure_args);\
				efree(z_reply);\
				RETURN_ZVAL(object, 1, 0);\
			} else {\
				efree(z_reply);\
				RETURN_FALSE;\
			}\
			break;\
\
		case REDIS_MODE_BLOCKING:\
			z_reply = redisCommandArgv(redis_sock->ctx, argc, args, arglen);\
			/* for(i = 1; i < argc-1; i++) efree((void*)args[i]);*/ \
			efree(args);\
			efree(arglen);\
			((reader_function)fun)(return_value, redis_sock->mode, z_reply, closure_args); \
			break;\
	}\
}

#endif

