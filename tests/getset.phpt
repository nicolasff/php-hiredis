--TEST--
getset
--FILE--
<?php
$r = new HiRedis;
$r->connect('127.0.0.1', 6379);

$r->delete('key');
var_dump($r->getSet('key', '42') === FALSE);
var_dump($r->getSet('key', '123') === '42');
var_dump($r->getSet('key', '123') === '123');


?>
--EXPECT--
bool(true)
bool(true)
bool(true)
