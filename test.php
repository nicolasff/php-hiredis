<?php

function test($className) {
	$r = new $className;
	$c = $r->connect('127.0.0.1', 6379);
	printf("[%s] Connection: %s\n", $className, $c?"OK":"FAILURE");

	$count = 10000;

	// SET
	$t0 = microtime(true);
	for($i = 0; $i < $count; $i++) {
		$r->set('key', 'value');
	}
	$t1 = microtime(true);
	printf("[%s] %d sets: %0.2f sec (%d/sec)\n", $className, $count, $t1-$t0, ($count)/($t1-$t0));

	// GET
	for($i = 0; $i < $count; $i++) {
		$r->get('key');
	}
	$t2 = microtime(true);

	printf("[%s] %d gets: %0.2f sec (%d/sec)\n", $className, $count, $t2-$t1, ($count)/($t2-$t1));

	// PIPELINE
	printf("[%s] Testing pipelining support, SET followed by GET.\n", $className);

	for($i = 0; $i < $count; $i++) {
		$r->pipeline()
			->set('key', 'value')
			->get('key')
			->exec();
	}
	$t3 = microtime(true);

	printf("[%s] %d pipelined (SET+GET): %0.2f sec (%d commands/sec)\n", $className, $count, $t3-$t2, 2*($count)/($t3-$t2));

	// MULTI/EXEC
	for($i = 0; $i < $count; $i++) {
		$ret = $r->multi()
			->set('key', 'value')
			->get('key')
			->exec();
	}

	$t4 = microtime(true);
	printf("[%s] %d MULTI/SET/GET/EXEC: %0.2f sec (%d/sec)\n", $className, $count, $t4-$t3, ($count)/($t4-$t3));

	$r->close();
	printf("\n");
}

test("Redis");
test("HiRedis");
?>
