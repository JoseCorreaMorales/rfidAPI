<?php

require_once "connection.php";
require_once "jwt.php";

$json = file_get_contents("php://input");
$data = json_decode($json);
$_REQUEST = (array)$data;
if(isset($_REQUEST['user']) && isset($_REQUEST['pass'])){
    $u = $_REQUEST['user'];
    $p = $_REQUEST['pass'];
    $c = connection();
    $s = $c->prepare("SELECT user,role FROM users WHERE user=:u AND pass=:p");
    //$s = $c->prepare("SELECT username FROM users WHERE username=:u AND password=:p");
    $s->bindValue(":u", $u);
    $s->bindValue(":p", $p);
    $s->execute();
    $s->setFetchMode(PDO::FETCH_ASSOC);
    $r = $s->fetch();
    if($r){
        $r = [
            "status" => "ok",
            "jwt" => JWT::create($r, "3kH4ZjD6Rt9qPvY2XyAe7Sg1u8oL5mNc")
        ];
    }else{
        $r = ["status" => "error"];
    }
    echo json_encode($r);
}else{
    header(("HTTP/1.1 400 Bad Request"));
}