<?php

$r = new HiRedis;
var_dump($r->connect('127.0.0.1'));
var_dump($r->set('key', 'value'));
var_dump($r->get('key'));

?>
