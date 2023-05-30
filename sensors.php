<?php
require_once "connection.php";
require_once "jwt.php";

if($_SERVER["REQUEST_METHOD"]=="OPTIONS") exit();
$jwt = apache_request_headers()["Authorization"];
if(strstr($jwt, "Bearer")) $jwt = substr($jwt, 7);
if(JWT::verify($jwt, "3kH4ZjD6Rt9qPvY2XyAe7Sg1u8oL5mNc")){
    header(("HTTP/1.1 401 Unauthorized"));
    exit();
}
$metodo = $_SERVER["REQUEST_METHOD"];


$json = file_get_contents("php://input");
$data = json_decode($json);
$_REQUEST = (array)$data;

switch($metodo){
    case "GET":
        //Consulta
        $c = connection();
        $c->exec("use iot");
        if(isset($_GET['id'])){
            $id = $_GET['id'];
            $s = $c->prepare("SELECT * FROM sensors WHERE id=:pid");
            $s->bindValue(":pid",$id);
            $s->execute();
            $s->setFetchMode(PDO::FETCH_ASSOC);
            $r = $s->fetch();
        }else{
            $s = $c->prepare("SELECT * FROM sensors");
            $s->execute();
            $s->setFetchMode(PDO::FETCH_ASSOC);
            $r = $s->fetchAll();
        }
        echo json_encode($r);
        break;
    case "POST":
        //Insertar
        if(!isset($_REQUEST['type']) || !isset($_REQUEST['value'])){
            header("HTTP/1.1 400 Bad Request");
            return;
        }
        $c = connection();
        $s = $c->prepare("INSERT INTO sensors(user,type,value,date) VALUES(:u, :t, :v, :d)");
        $s->bindValue(":u", "admin");
        $s->bindValue(":t", $_REQUEST['type']);
        $s->bindValue(":v", $_REQUEST['value']);
        $s->bindValue(":d", date("Y-m-d H:i:s"));
        $s->execute();
        // if($s->rowCount()==0){
        //     header("HTTP/1.1 400 Bad Request");
        //     return;
        // }
        echo json_encode(["status"=>"ok", "id"=>$c->lastInsertId()]);
        break;
    case "PUT":
        //Actualizar
        if(!isset($_GET['type']) || !isset($_GET['value']) 
        || !isset($_GET['id'])){
            header("HTTP/1.1 400 Bad Request");
            return;
        }
        $c = connection();
        $s = $c->prepare("UPDATE sensors SET type=:t, value=:v WHERE id=:id");
        $s->bindValue(":id", $_GET['id']);
        $s->bindValue(":t", $_GET['type']);
        $s->bindValue(":v", $_GET['value']);
        $s->execute();
        echo json_encode(["status"=>"ok"]);
        break;
    case "DELETE":
        //Eliminar
        if(!isset($_GET['id'])){
            header("HTTP/1.1 400 Bad Request");
            return;
        }
        $c = connection();
        $s = $c->prepare("DELETE FROM sensors WHERE id=:id");
        $s->bindValue(":id", $_GET['id']);
        $s->execute();
        echo json_encode(["status"=>"ok"]);
        break;
}