#ifndef PHPREDIS_MACROS_H
#define PHPREDIS_MACROS_H

#define REDIS_SOCK_GET(sock) {\
    if (redis_sock_get(object, &sock TSRMLS_CC) < 0) {\
        RETURN_FALSE;\
    }\
}

#define REDIS_RUN(sock, fun, pattern, ...) do {\
	redisAppendCommand(sock->ctx, pattern, ##__VA_ARGS__);\
	redis_enqueue(sock, (void*)fun, NULL);\
\
	if(sock->pipeline == 0 && sock->multi == 0) { /* no pipeline, read the result. */\
		redisReplyReaderSetPrivdata(sock->ctx->reader, return_value);\
	        redisGetReply(sock->ctx, NULL);\
		fun(return_value, NULL);\
	} else {\
		RETURN_ZVAL(object, 1, 0);\
	}\
} while(0)

#define REDIS_RUN_ARGS(sock, fun, argc, args, arglen, closure_args) do {\
	redisAppendCommandArgv(sock->ctx, argc, args, arglen);\
	redis_enqueue(sock, (void*)fun, NULL);\
\
	if(sock->pipeline == 0 && sock->multi == 0) { /* no pipeline, read the result. */\
		redisReplyReaderSetPrivdata(sock->ctx->reader, return_value);\
	        redisGetReply(sock->ctx, NULL);\
		fun(return_value, closure_args);\
	} else {\
		RETURN_ZVAL(object, 1, 0);\
	}\
} while(0)

#endif

