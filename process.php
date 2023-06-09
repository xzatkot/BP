<?php
require_once 'config.php';

try {
    $pdo = new PDO("mysql:host=$host;dbname=$database;charset=utf8mb4", $username, $password);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
} catch (PDOException $e) {
    $error = $e->getMessage();
}

$stmt = $pdo->prepare('SELECT * FROM data WHERE Timestamp > :start AND Timestamp < :end');
$start = date('Y-m-d H:i:s', strtotime('-48 hours'));
$end = date('Y-m-d H:i:s', strtotime('-24 hours'));
$stmt->execute(['start' => $start, 'end' => $end]);
$data = $stmt->fetchAll(PDO::FETCH_ASSOC);

$average_values = calculate_average($data);
foreach ($average_values as $timestamp => $row) {
    $stmt = $pdo->prepare('INSERT INTO archive (timestamp, LAeq, temperature, pressure, humidity) VALUES (:timestamp, :laeq, :temperature, :pressure, :humidity)');
    $stmt->execute(['timestamp' => $timestamp, 'laeq' => $row['LAeq'], 'temperature' => $row['Temperature'], 'pressure' => $row['Pressure'], 'humidity' => $row['Humidity']]);
}

$stmt = $pdo->prepare('DELETE FROM data WHERE Timestamp > :start AND Timestamp < :end');
$stmt->execute(['start' => $start, 'end' => $end]);

function calculate_average($data) {
    $average_values = [];
    $laeq_hourly = [];
    $temp = [];
    $temp_hourly = [];
    $press = [];
    $press_hourly = [];
    $humid = [];
    $humid_hourly = [];
    foreach ($data as $row) {
        $timestamp = DateTime::createFromFormat('Y-m-d H:i:s', $row['Timestamp']);
        $hour = $timestamp->format('Y-m-d H');
        if (!isset($hourly_data[$hour])) {
            $laeq_hourly[$hour] = [];
            $temp_hourly[$hour] = [];
            $press_hourly[$hour] = [];
            $humid_hourly[$hour] = [];
        }
        $laeq_hourly[$hour][] = $row['LA'];
        $temp_hourly[$hour][] = $row['Temperature'];
        $press_hourly[$hour][] = $row['Pressure'];
        $humid_hourly[$hour][] = $row['Humidity'];
    }
    foreach ($laeq_hourly as $hour => $la_values) {
        $sum = 0;
        foreach ($la_values as $la) {
            $sum += pow(10, $la / 10);
        }
        $laeq = 10 * log10($sum / count($la_values));
        $average_values[$hour] = [
            'LAeq' => $laeq,
            'Temperature' => array_sum($temp_hourly[$hour]) / count($temp_hourly[$hour]),
            'Pressure' => array_sum($press_hourly[$hour]) / count($press_hourly[$hour]),
            'Humidity' => array_sum($humid_hourly[$hour]) / count($humid_hourly[$hour]),
        ];
    }
    return $average_values;
}
?>