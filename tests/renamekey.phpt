--TEST--
renameKey
--FILE--
<?php
$r = new HiRedis;
$r->connect('127.0.0.1', 6379);

// strings
$r->delete('key0');
$r->set('key0', 'val0');
$r->renameKey('key0', 'key1');
var_dump($r->get('key0') === FALSE);
var_dump($r->get('key1') === 'val0');
?>
--EXPECT--
bool(true)
bool(true)
