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
                    redisAppendCommand(redis_sock->ctx, pattern, __VA_ARGS__);\
                    redis_sock->enqueued_commands++;\
                    RETURN_ZVAL(object, 1, 0);\
                    break;\
\
            case REDIS_MODE_TRANSACTION:\
                    z_reply = redisCommand(redis_sock->ctx, pattern, __VA_ARGS__);\
                    if(Z_TYPE_P(z_reply) == IS_BOOL && Z_BVAL_P(z_reply) == 1) {\
                            redis_sock->enqueued_commands++;\
                            efree(z_reply);\
                            RETURN_ZVAL(object, 1, 0);\
                    } else {\
                            efree(z_reply);\
                            RETURN_FALSE;\
                    }\
                    break;\
\
            case REDIS_MODE_BLOCKING:\
                    z_reply = redisCommand(redis_sock->ctx, pattern, __VA_ARGS__);\
                    fun(INTERNAL_FUNCTION_PARAM_PASSTHRU, z_reply); \
                    break;\
    }\
}

#endif

