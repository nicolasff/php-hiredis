--TEST--
set
--FILE--
<?php
$r = new HiRedis;
$r->connect('127.0.0.1', 6379);

var_dump($r->set('key', 'nil'));
var_dump($r->get('key'));

var_dump($r->set('key', 'val'));
var_dump($r->get('key'));
var_dump($r->get('key'));
var_dump($r->get('kjfi3qur89 34ur89wer7 9034r8934r8093'));

var_dump($r->set('key2', 'val'));
var_dump($r->get('key2'));


$value = 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA';
$r->set('key2', $value);
var_dump($r->get('key2'));
var_dump($r->get('key2'));

$r->delete('key');
$r->delete('key2');


$i = 6; // 66000;
$value2 = 'X';
while($i--) {
	$value2 .= 'A';
}
$value2 .= 'X';

$r->set('key', $value2);
var_dump($value2 === $r->get('key'));
$r->delete('key');
var_dump(false === $r->get('key'));


$data = gzcompress('42');
var_dump(true === $r->set('key', $data));
var_dump('42' === gzuncompress($r->get('key')));

$r->delete('key');
$data = gzcompress('value1');
var_dump(true === $r->set('key', $data));
var_dump('value1' === gzuncompress($r->get('key')));

$r->delete('key');
var_dump(TRUE === $r->set('key', 0));
var_dump('0' === $r->get('key'));
var_dump(TRUE === $r->set('key', 1));
var_dump('1' === $r->get('key'));
var_dump(TRUE === $r->set('key', 0.1));
var_dump('0.1' === $r->get('key'));
var_dump(TRUE === $r->set('key', '0.1'));
var_dump('0.1' === $r->get('key'));
var_dump(TRUE === $r->set('key', TRUE));
var_dump('1' === $r->get('key'));

var_dump(true === $r->set('key', ''));
var_dump('' === $r->get('key'));
var_dump(true === $r->set('key', NULL));
var_dump('' === $r->get('key'));

var_dump(true === $r->set('key', gzcompress('42')));
var_dump('42' === gzuncompress($r->get('key')));
?>
--EXPECT--
bool(true)
string(3) "nil"
bool(true)
string(3) "val"
string(3) "val"
bool(false)
bool(true)
string(3) "val"
string(112) "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
string(112) "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
