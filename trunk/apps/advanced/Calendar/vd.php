<?php 

require("vd-array.php");

?>
#ifndef VD_NAMES
#define VD_NAMES

#include <avr/pgmspace.h>

// namedays, stored in program flash!
<?

$maxNamesLen = 0; // max nameday string len for a particular day

foreach ($_name_days as $i => & $month) {
    echo "const char MONTH_{$i}[] PROGMEM = {\n";
    foreach ($month as $di => & $day) {
        $n = "";
        foreach ($day as $ni => & $name) {
            if ($ni != 0) $n .= " ";
            $n .= $name;
        }
        $l = strlen($n);
        if ($l > $maxNamesLen) $maxNamesLen = $l;
        if ($di != count($month)) $n .= "\\0";
        echo "    \"{$n}\"\n";
    }
    echo "};\n";
}

$maxNamesLen++; // count \0 as part of the string

echo "\n";
echo "enum { MAX_NAMES_LEN = {$maxNamesLen} };";
echo "\n";

?>

#endif // VD_NAMES
