#include "hooks.h"
#include "php.h"

static void *tryParentize(const redisReadTask *task, zval *v) {
	// php_printf("CALLBACK: %s\n", __FUNCTION__);
        if (task && task->parent != NULL) {
                // php_printf("INSIDE\n");
                zval *parent = (zval *)task->parent->obj;
                assert(Z_TYPE_P(parent) == IS_ARRAY);
                add_index_zval(parent, task->idx, v);
        }
        return (void*)v;
}

void *
createStringObject(const redisReadTask *task, char *str, size_t len) {

        zval *z_ret;
	if(task->parent) { /* adding to an array */
		MAKE_STD_ZVAL(z_ret);
	} else {
		z_ret = task->privdata;
	}
	// php_printf("CALLBACK: %s (%zd)[%s] (parent=%p)\n", __FUNCTION__, len, str, task->parent);

        switch(task->type) {
                case REDIS_REPLY_ERROR:
                    ZVAL_BOOL(z_ret, 0);
                    break;

                case REDIS_REPLY_STATUS:
		    /*
		    if(len == 6 && strncmp(str, "QUEUED", 6) == 0) {
			    return z_ret;
		    }
		    */
                    if(len == 4 && strncmp(str, "PONG", 4) == 0) {
                        ZVAL_STRINGL(z_ret, "+PONG", 5, 1);
                    } else {
                        ZVAL_BOOL(z_ret, 1);
                    }
                    break;

                case REDIS_REPLY_STRING:
                    ZVAL_STRINGL(z_ret, str, len, 1);
                    break;
        }
        // php_printf("created string object (%zd)[%s], z_ret=%p\n", len, str, z_ret);

	return tryParentize(task, z_ret);
}

void *
createArrayObject(const redisReadTask *task, int elements) {
	// php_printf("CALLBACK: %s (parent=%p)\n", __FUNCTION__, task->parent);
        zval *z_ret;

	if(task->parent) { /* adding to an array */
		MAKE_STD_ZVAL(z_ret);
	} else {
		z_ret = task->privdata;
	}

        array_init(z_ret);

        return tryParentize(task, z_ret);
}

void *
createIntegerObject(const redisReadTask *task, long long value) {
	// php_printf("CALLBACK: %s, %lld\n", __FUNCTION__, value);

        zval *z_ret;
	if(task->parent) { /* adding to an array */
		MAKE_STD_ZVAL(z_ret);
	} else {
		z_ret = task->privdata;
	}

        ZVAL_LONG(z_ret, (long)value);
        return tryParentize(task, z_ret);
}

void *
createNilObject(const redisReadTask *task) {
        // php_printf("CALLBACK: %s\n", __FUNCTION__);

        zval *z_ret;
	if(task->parent) { /* adding to an array */
		MAKE_STD_ZVAL(z_ret);
	} else {
		z_ret = task->privdata;
	}
        ZVAL_BOOL(z_ret, 0);
        return tryParentize(task, z_ret);
}

void
freeObject(void *ptr) {
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


