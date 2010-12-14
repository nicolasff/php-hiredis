<?php


function test($className, $host, $port = NULL) {
	
/*
	$r = new Redis;
	$r->connect($host, $port);
	$r->delete('list');
	$r->lpush('list', 'def');
	$r->lpush('list', 'abc');

	printf("testing %s on %s\n", $className, $host);
 */
	$r = new $className;
	$c = $r->connect($host, $port);
/*	printf("[%s] Connection: %s\n", $className, $c?"OK":"FAILURE");


	var_dump($r->set('key', 'value'));
	var_dump($r->lrange('list', 0, -1));

	echo "------------- PIPELINE ----------------\n";

	$out = $r->pipeline();
	$out->set('key', 'value');
	var_dump($r->lrange('list', 0, -1));
	var_dump($out);
	var_dump($out->send());
	 
	echo "------------- MULTI/EXEC ----------------\n";

	$out = $r->multi()
		->set('key', 'value')
		->lrange('list', 0, -1)
		->exec();
	var_dump($out);

	echo "------------- BOTH ----------------\n";

	echo "\n\nSET\n";
	var_dump($r->set('key', 'val1'));
	var_dump($r->get('key'));

	echo "\n\nPIPELINE\n";
	$r->pipeline();

	echo "\n\nSET\n";
	$r->set('key', 'val1');

	echo "\n\nMULTI\n";
	$r->multi();

	echo "\n\nSET\n";
	$r->set('key', 'value');

	echo "\n\nLRANGE\n";
	$r->lrange('list', 0, -1);

	echo "\n\nEXEC\n";
	$r->exec();

	$out = $r->send();
	var_dump($out);

*/

	echo "----------------------------------------\n";
	$r->delete('key');
	$r->hset('key', 'x', 'a');
	$r->hset('key', 'y', 'b');
	$r->hset('key', 'z', 'c');
	$r->hset('key', 't', 'd');
	var_dump($r->hgetall('key'));

	var_dump($r->hmget('key', array('x', 't')));
	var_dump($r->hmget('key', 'x', 'd'));


	return;

	$count = 1;

	// SET
	$t0 = microtime(true);
	/*
	for($i = 0; $i < $count; $i++) {
		$ret = $r->set('key', 'value');
		assert($ret === true);
	}
	 */
	$t1 = microtime(true);
	printf("[%s] %d sets: %0.2f sec (%d/sec)\n", $className, $count, $t1-$t0, ($count)/($t1-$t0));

	// GET
	/*
	for($i = 0; $i < $count; $i++) {
		$ret = $r->get('key');
		assert($ret === 'value');
	}
	 */
	$t2 = microtime(true);

	printf("[%s] %d gets: %0.2f sec (%d/sec)\n", $className, $count, $t2-$t1, ($count)/($t2-$t1));

	return;
	/*
	// PIPELINE
	printf("[%s] Testing pipelining support, SET followed by GET.\n", $className);

	for($i = 0; $i < $count; $i++) {
		$ret = $r->pipeline()
			->set('key', 'value')
			->get('key')
			->exec();
		assert($ret === array(true, 'value'));
	}
	 */
	$t3 = microtime(true);

	printf("[%s] %d pipelined (SET+GET): %0.2f sec (%d commands/sec)\n", $className, $count, $t3-$t2, 2*($count)/($t3-$t2));

	/*
	// MULTI/EXEC
	for($i = 0; $i < $count; $i++) {
		$ret = $r->multi()
			->set('key', 'value')
			->get('key')
			->exec();
		assert($ret === array(true, 'value'));
	}
	 */

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
//		test("Redis", '127.0.0.1', 6379);
		test("HiRedis", '127.0.0.1', 6379);
//		test("HiRedis", '/tmp/redis.sock');
	} catch(Exception $e) {
		var_dump($e);
	}
}

bench();


?>
