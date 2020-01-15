<?php

/***

Array
(
    [0] => aes-128-cbc
    [1] => aes-128-cbc-hmac-sha1
    [2] => aes-128-cbc-hmac-sha256
    [3] => aes-128-ccm
    [4] => aes-128-cfb
    [5] => aes-128-cfb1
    [6] => aes-128-cfb8
    [7] => aes-128-ctr
    [8] => aes-128-ecb
    [9] => aes-128-gcm
    [10] => aes-128-ocb
    [11] => aes-128-ofb
    [12] => aes-128-xts
    [13] => aes-192-cbc
    [14] => aes-192-ccm
    [15] => aes-192-cfb
    [16] => aes-192-cfb1
    [17] => aes-192-cfb8
    [18] => aes-192-ctr
    [19] => aes-192-ecb
    [20] => aes-192-gcm
    [21] => aes-192-ocb
    [22] => aes-192-ofb
    [23] => aes-256-cbc
    [24] => aes-256-cbc-hmac-sha1
    [25] => aes-256-cbc-hmac-sha256
    [26] => aes-256-ccm
    [27] => aes-256-cfb
    [28] => aes-256-cfb1
    [29] => aes-256-cfb8
    [30] => aes-256-ctr
    [31] => aes-256-ecb
    [32] => aes-256-gcm
    [33] => aes-256-ocb
    [34] => aes-256-ofb
    [35] => aes-256-xts
    [36] => aria-128-cbc
    [37] => aria-128-ccm
    [38] => aria-128-cfb
    [39] => aria-128-cfb1
    [40] => aria-128-cfb8
    [41] => aria-128-ctr
    [42] => aria-128-ecb
    [43] => aria-128-gcm
    [44] => aria-128-ofb
    [45] => aria-192-cbc
    [46] => aria-192-ccm
    [47] => aria-192-cfb
    [48] => aria-192-cfb1
    [49] => aria-192-cfb8
    [50] => aria-192-ctr
    [51] => aria-192-ecb
    [52] => aria-192-gcm
    [53] => aria-192-ofb
    [54] => aria-256-cbc
    [55] => aria-256-ccm
    [56] => aria-256-cfb
    [57] => aria-256-cfb1
    [58] => aria-256-cfb8
    [59] => aria-256-ctr
    [60] => aria-256-ecb
    [61] => aria-256-gcm
    [62] => aria-256-ofb
    [63] => bf-cbc
    [64] => bf-cfb
    [65] => bf-ecb
    [66] => bf-ofb
    [67] => camellia-128-cbc
    [68] => camellia-128-cfb
    [69] => camellia-128-cfb1
    [70] => camellia-128-cfb8
    [71] => camellia-128-ctr
    [72] => camellia-128-ecb
    [73] => camellia-128-ofb
    [74] => camellia-192-cbc
    [75] => camellia-192-cfb
    [76] => camellia-192-cfb1
    [77] => camellia-192-cfb8
    [78] => camellia-192-ctr
    [79] => camellia-192-ecb
    [80] => camellia-192-ofb
    [81] => camellia-256-cbc
    [82] => camellia-256-cfb
    [83] => camellia-256-cfb1
    [84] => camellia-256-cfb8
    [85] => camellia-256-ctr
    [86] => camellia-256-ecb
    [87] => camellia-256-ofb
    [88] => cast5-cbc
    [89] => cast5-cfb
    [90] => cast5-ecb
    [91] => cast5-ofb
    [92] => chacha20
    [93] => chacha20-poly1305
    [94] => des-cbc
    [95] => des-cfb
    [96] => des-cfb1
    [97] => des-cfb8
    [98] => des-ecb
    [99] => des-ede
    [100] => des-ede-cbc
    [101] => des-ede-cfb
    [102] => des-ede-ofb
    [103] => des-ede3
    [104] => des-ede3-cbc
    [105] => des-ede3-cfb
    [106] => des-ede3-cfb1
    [107] => des-ede3-cfb8
    [108] => des-ede3-ofb
    [109] => des-ofb
    [110] => desx-cbc
    [111] => id-aes128-CCM
    [112] => id-aes128-GCM
    [113] => id-aes128-wrap
    [114] => id-aes128-wrap-pad
    [115] => id-aes192-CCM
    [116] => id-aes192-GCM
    [117] => id-aes192-wrap
    [118] => id-aes192-wrap-pad
    [119] => id-aes256-CCM
    [120] => id-aes256-GCM
    [121] => id-aes256-wrap
    [122] => id-aes256-wrap-pad
    [123] => id-smime-alg-CMS3DESwrap
    [124] => rc2-40-cbc
    [125] => rc2-64-cbc
    [126] => rc2-cbc
    [127] => rc2-cfb
    [128] => rc2-ecb
    [129] => rc2-ofb
    [130] => rc4
    [131] => rc4-40
    [132] => rc4-hmac-md5
    [133] => seed-cbc
    [134] => seed-cfb
    [135] => seed-ecb
    [136] => seed-ofb
    [137] => sm4-cbc
    [138] => sm4-cfb
    [139] => sm4-ctr
    [140] => sm4-ecb
    [141] => sm4-ofb
)


***/


ini_set("display_errors", 0);




if ($argc != 4)
{
    die("try: php openssl_prng.php cipher pathToSeeds len\n");
}

$cipher = $argv[1];
$path = $argv[2];
$lenlog = $argv[3];

if ($lenlog < 4)
{
    die("length too short\n");
}
$len = pow(2, $lenlog - 3);

$handle = fopen($path, "r");
// $contents = fread($handle, filesize($filename));
if (FALSE === $handle) {
    exit("Failed to open stream to URL");
}

$firstline = true;
while(!feof($handle))
{
    if ($firstline) {
        echo "number of seeds: " . fgets($handle)  . "\n";
        $firstline = false;
    }
    $seed = fgets($handle);
    echo encrypt($cipher, $seed, $len);        
}

fclose($handle);
die();

# sprawdzenie ile bitow ma plik
# wc -c aes_tmp.txt |  awk '{print $1*8}'


function encrypt($cipher, $seed, $len)
{

    $zdanie = str_repeat("0", $len);
    {
        $iv = "0000000000000000";
        $ivlen = openssl_cipher_iv_length($cipher);
        $iv = substr($iv, 0, $ivlen);
		$key = hash("sha256", $seed);
        $encrypted = openssl_encrypt($zdanie, $cipher, hex2bin($key), OPENSSL_RAW_DATA, $iv);
		return $encrypted;
    }
}

?>
