<?php

function test($className) {
	printf("testing %s\n", $className);
	$r = new $className;
	$c = $r->connect('127.0.0.1', 6379);
	printf("[%s] Connection: %s\n", $className, $c?"OK":"FAILURE");

	$count = 10000;

	// SET
	$t0 = microtime(true);
	for($i = 0; $i < $count; $i++) {
		$ret = $r->set('key', 'value');
		assert($ret === true);
	}
	$t1 = microtime(true);
	printf("[%s] %d sets: %0.2f sec (%d/sec)\n", $className, $count, $t1-$t0, ($count)/($t1-$t0));

	// GET
	for($i = 0; $i < $count; $i++) {
		$ret = $r->get('key');
		assert($ret === 'value');
	}
	$t2 = microtime(true);

	printf("[%s] %d gets: %0.2f sec (%d/sec)\n", $className, $count, $t2-$t1, ($count)/($t2-$t1));

	// PIPELINE
	printf("[%s] Testing pipelining support, SET followed by GET.\n", $className);

	for($i = 0; $i < $count; $i++) {
		$ret = $r->pipeline()
			->set('key', 'value')
			->get('key')
			->exec();
		assert($ret === array(true, 'value'));
	}
	$t3 = microtime(true);

	printf("[%s] %d pipelined (SET+GET): %0.2f sec (%d commands/sec)\n", $className, $count, $t3-$t2, 2*($count)/($t3-$t2));

	// MULTI/EXEC
	for($i = 0; $i < $count; $i++) {
		$ret = $r->multi()
			->set('key', 'value')
			->get('key')
			->exec();
		assert($ret === array(true, 'value'));
	}

	$t4 = microtime(true);
	printf("[%s] %d MULTI/SET/GET/EXEC: %0.2f sec (%d/sec)\n", $className, $count, $t4-$t3, ($count)/($t4-$t3));


	$ret = $r->delete('key');
	assert($ret === 1);
	$r->set('x', 'a');
	$r->set('y', 'b');
	$ret = $r->delete('x', 'y');
	assert($ret === 2);


	for($i = 0; $i < $count; $i++) {
		$ret = $r->incr('key');
		assert($ret === $i + 1);
	}

	$t5 = microtime(true);

	printf("[%s] %d INCR: %0.2f sec (%d/sec)\n", $className, $count, $t5-$t4, ($count)/($t5-$t4));

	$ret = $r->delete('key');

	for($i = 0; $i < $count; $i++) {
		$ret = $r->decr('key');
		assert($ret === -$i - 1);
	}

	$t6 = microtime(true);

	printf("[%s] %d DECR: %0.2f sec (%d/sec)\n", $className, $count, $t6-$t5, ($count)/($t6-$t5));

	// hset, hgetall
	$r->delete('h');
	$r->hset('h', 'a', 'x');
	$r->hset('h', 'b', 'y');
	$r->hset('h', 'c', 'z');

	$tab = $r->hgetall('h');
	assert($tab === array('a' => 'x', 'b' => 'y', 'c' => 'z'));

	/*
	$tab = $r->hmget('h', array('a', 'b'));
	assert($tab === array('a' => 'x', 'b' => 'y'));

	if($className === 'HiRedis') {
		$tab = $r->hmget('h', 'a', 'b');
		assert($tab === array('a' => 'x', 'b' => 'y'));
	}

	$tab = $r->multi()
		->hset('h', 'a', 'plop')
		->hgetall('h')
		->exec();
	assert($tab === array(0, array('a' => 'plop', 'b' => 'y', 'c' => 'z')));

	$tab = $r->multi()
		->hset('h', 'a', 'plop')
		->hgetall('h')
		->exec();
	assert($tab === array(0, array('a' => 'plop', 'b' => 'y', 'c' => 'z')));
	*/

	$r->delete('key');
	assert($r->setnx('key', 'val') === TRUE);
	assert($r->setnx('key', 'val') === FALSE);


	// close
	$r->close();
	printf("\n");
}

function bench() {
	try {
		test("Redis");
		test("HiRedis");
	} catch(Exception $e) {
		var_dump($e);
	}
}

bench();


?>
