<?php

$r = new HiRedis;
var_dump($r->connect('127.0.0.1'));

$count = 10000;

$t0 = microtime(true);
for($i = 0; $i < $count; $i++) {
	$r->set('key', 'value');
}
$t1 = microtime(true);
printf("%d sets: %0.2f sec (%d/sec)\n", $count, $t1-$t0, ($count)/($t1-$t0));
for($i = 0; $i < $count; $i++) {
	$r->get('key');
}
$t2 = microtime(true);

printf("%d gets: %0.2f sec (%d/sec)\n", $count, $t2-$t1, ($count)/($t2-$t1));

?>
