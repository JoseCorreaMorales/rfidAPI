<?php
require_once "connection.php";

$c = connection();

if($c) echo "Conectado a base de datos";
