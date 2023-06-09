<?php
require_once 'config.php';

try {
    $pdo = new PDO("mysql:host=$host;dbname=$database;charset=utf8mb4", $username, $password);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch (PDOException $e) {
    $error = $e->getMessage();
}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $json = file_get_contents('php://input');
    $data = json_decode($json, true);
    $stmt = $pdo->prepare("INSERT INTO data (Timestamp, LA, Temperature, Pressure, Humidity) VALUES (:timestamp, :la, :temperature, :pressure, :humidity)");

    $timestamp = new DateTime('now', new DateTimeZone('Europe/Bratislava'));
    $timestamp->subtract(new DateInterval('PT15M'));

    foreach ($data as $reading) {
        $stmt->execute([
            'timestamp' => $timestamp->format('Y-m-d H:i:s'),
            'la' => $reading['LA'],
            'temperature' => $reading['Temperature'],
            'pressure' => $reading['Pressure'],
            'humidity' => $reading['Humidity']
        ]);
        $timestamp->add(new DateInterval('PT15S'));
    }

    echo "Data inserted successfully.";
}

unset($pdo);