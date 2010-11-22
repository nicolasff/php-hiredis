--TEST--
getmultiple
--FILE--
<?php
$r = new HiRedis;
$r->connect('127.0.0.1', 6379);

$r->delete('k0', array('k1', 'k2'));
$r->delete('k1');
$r->delete('k2');
$r->delete('k3');

$r->set('k1', 'v1');
$r->set('k2', 'v2');
$r->set('k3', 'v3');

var_dump(array('v1') === $r->getMultiple(array('k1')));
var_dump(array('v1', 'v3', false) === $r->getMultiple(array('k1', 'k3', 'NoKey')));
var_dump(array('v1', 'v2', 'v3') === $r->getMultiple(array('k1', 'k2', 'k3')));
var_dump(array('v1', 'v2', 'v3') === $r->getMultiple(array('k1', 'k2', 'k3')));

$r->set('k5', '$1111111111');
// var_dump($r->getMultiple(array('k5')));
// var_dump(array(0 => '$1111111111') === $r->getMultiple(array('k5')));
var_dump(array(0 => '$1111111111') === $r->getMultiple('k5'));
flush();
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
