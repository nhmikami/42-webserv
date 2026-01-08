<?php
#!/usr/bin/php-cgi

// filepath: data/42sp/cgi/test_cookie.php

// Ler cookies existentes
$cookies = [];
if (isset($_SERVER['HTTP_COOKIE'])) {
    $cookie_pairs = explode('; ', $_SERVER['HTTP_COOKIE']);
    foreach ($cookie_pairs as $pair) {
        list($key, $value) = explode('=', $pair);
        $cookies[$key] = $value;
    }
}

// Definir novos cookies
header("Content-Type: text/html");
header("Set-Cookie: user_id=123; Path=/; Max-Age=3600");
header("Set-Cookie: theme=dark; Path=/; HttpOnly");

echo "<html><body>";
echo "<h1>Teste de Cookies em PHP</h1>";
echo "<h2>Cookies recebidos:</h2>";
echo "<pre>" . print_r($cookies, true) . "</pre>";
echo "<h2>Cookies enviados:</h2>";
echo "<ul>";
echo "<li>user_id=123 (expira em 1 hora)</li>";
echo "<li>theme=dark (HttpOnly)</li>";
echo "</ul>";
echo "</body></html>";
?>