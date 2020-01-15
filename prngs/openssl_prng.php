<?php
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
