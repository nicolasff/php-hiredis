<?php

function test($className) {
	$r = new $className;
	var_dump($r->connect('127.0.0.1', 6379));

	$count = 100000;

	$t0 = microtime(true);
	for($i = 0; $i < $count; $i++) {
		$r->set('key', 'value');
	}
	$t1 = microtime(true);
	printf("[%s] %d sets: %0.2f sec (%d/sec)\n", $className, $count, $t1-$t0, ($count)/($t1-$t0));
	for($i = 0; $i < $count; $i++) {
		$r->get('key');
	}
	$t2 = microtime(true);

	printf("[%s] %d gets: %0.2f sec (%d/sec)\n", $className, $count, $t2-$t1, ($count)/($t2-$t1));

	$r->close();
}

test("Redis");
test("HiRedis");
?>
