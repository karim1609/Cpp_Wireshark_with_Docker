CREATE DATABASE IF NOT EXISTS packet_capture_db;
USE packet_capture_db;

CREATE TABLE IF NOT EXISTS packets (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    source_ip VARCHAR(45) NOT NULL,
    destination_ip VARCHAR(45) NOT NULL,
    protocol VARCHAR(30) NOT NULL,
    length INT NOT NULL,
    captured_at DATETIME NOT NULL
);
