<?php
function foo($a = A) {
    echo "$a\n";
}
function bar($a = array(A => B)) {
    foreach ($a as $key => $val) {
        echo "$key\n";
        echo "$val\n";
    }
}
const A = "ok";
const B = A;
<<__EntryPoint>> function main() {
echo A . "\n";
echo B . "\n";
foo();
bar();
}
